#include "Vector_Font.h"
#include "basics.h"


#define REFRESH_CW() font->cw = font->scale * font->advance


size_t load_Glyph( Glyph *gg, SDL_IOStream *f, int version, bool monospaced, int *max_len ){
	size_t read = 0;
	read += SDL_ReadIO( f, &(gg->path_count), sizeof(int) );
	if(gg->path_count <= 0) {
        gg->verts = NULL;
        gg->offsets = NULL;
        return read;
        SDL_Log("0 paths!?");
    }
	
	gg->offsets = SDL_realloc( gg->offsets, gg->path_count * sizeof(int) );
	read += SDL_ReadIO( f, gg->offsets, sizeof(int) * gg->path_count );
	if( gg->offsets[0] > *max_len ) *max_len = gg->offsets[0];
	for(int p = 1; p < gg->path_count; ++p ){
		if( gg->offsets[p] > *max_len ) *max_len = gg->offsets[p];
		gg->offsets[p] += gg->offsets[p-1];
	}

	int total_verts = gg->offsets[gg->path_count -1];
	gg->verts = SDL_realloc( gg->verts, total_verts * sizeof(SDL_FPoint) );
	read += SDL_ReadIO( f, gg->verts, total_verts * sizeof(SDL_FPoint) );

	if( !monospaced ){
		read += SDL_ReadIO( f, &(gg->adv), sizeof(int) );
	}
	return read;
}

VFont load_VFont( char *filename ){

	VFont font;
	SDL_memset( &font, 0, sizeof(VFont) );

	SDL_IOStream *f = SDL_IOFromFile( filename, "r" );

	int version = -1;
	SDL_ReadIO( f, &version,         sizeof(int)  );
	//SDL_Log( "Opened \"%s\", version: %d\n", filename, version );
	//if( version != 1 ) SDL_Log( "vfont file \"%s\" is version %d, which is not supported!\n", filename, version );

	int grid_horizontal = -1;
	int grid_vertical = -1;
	int native_height = -1;
	float preview_scale = -1;
	int line_skip = -1;
	int default_spacing = -1;

	SDL_ReadIO( f, &grid_horizontal,   sizeof(int)  );
	SDL_ReadIO( f, &grid_vertical,     sizeof(int)  );
	SDL_ReadIO( f, &native_height,     sizeof(int)  );
	SDL_ReadIO( f, &preview_scale,     sizeof(float));
	SDL_ReadIO( f, &line_skip,         sizeof(int)  );
	SDL_ReadIO( f, &(font.monospaced), sizeof(bool) );
	SDL_ReadIO( f, &default_spacing,   sizeof(int)  );

	/*SDL_Log("grid_horizontal: %d\n", grid_horizontal ); 
	SDL_Log("grid_vertical: %d\n", grid_vertical ); 
	SDL_Log("native_height: %d\n", native_height ); 
	SDL_Log("preview_scale: %f\n", preview_scale ); 
	SDL_Log("line_skip: %d\n", line_skip ); 
	SDL_Log("monospaced: %d\n", monospaced ); 
	SDL_Log("default_spacing: %d\n", default_spacing );
	*/

	font.scale = preview_scale;
	font.advance = grid_horizontal;
	font.space = font.advance;
	font.text_h = grid_vertical;
	font.line_height = font.text_h + line_skip;
	
	int max_len = 0;

	while( SDL_GetIOStatus(f) == SDL_IO_STATUS_READY ){
		Uint32 g = 0;
		SDL_ReadU32LE(f, &g);//SDL_ReadIO( f, &g, sizeof(Uint32) );
		if( g == 0 || SDL_GetIOStatus(f) != SDL_IO_STATUS_READY ) break;
		if( g >= 32 && g < 128 ){
			g -= ' ';
			size_t read = load_Glyph( font.ascii + g, f, version, font.monospaced, &max_len );
		}
		else{
			int u = font.unicode_count;
			font.unicode_count += 1;
			font.unicode = SDL_realloc( font.unicode, font.unicode_count * sizeof(Glyph) );
			font.code_points = SDL_realloc( font.code_points, font.unicode_count * sizeof(Uint32) );
			font.code_points[u] = utf8_to_codepoint( (char*)(&g) );
			SDL_memset( font.unicode + u, 0, sizeof(Glyph) );
			load_Glyph( font.unicode + u, f, 2, font.monospaced, &max_len );
		}
	}
	SDL_CloseIO( f );

	if( max_len > MAX_PATH_LEN ){
		SDL_Log("VCT WARNING: %s: max path length: %d. MAX_PATH_LEN: %d\n", filename, max_len, MAX_PATH_LEN );
	}
	return font;
}

void destroy_VFont( VFont *font ) {
    if (!font) return;

    for (int i = 0; i < 96; ++i) {
        Glyph *g = &font->ascii[i];
        if( g->verts ){
            SDL_free(g->verts);
        	SDL_free(g->offsets);
        }
    }

    if (font->unicode) {
        for (int i = 0; i < font->unicode_count; ++i) {
            Glyph *g = &font->unicode[i];
            if (g->verts) {
                SDL_free(g->verts);
                SDL_free(g->offsets);
            }
        }
        SDL_free(font->unicode);
    	SDL_free(font->code_points);
    	font->unicode = NULL;
	    font->code_points = NULL;
	    font->unicode_count = 0;
    }
}

static void transform_path( SDL_FPoint *dst, SDL_FPoint *src, int len, float tx, float ty, float scl ){
	for (int v = 0; v < len; ++v ){
		dst[v] = (SDL_FPoint){ tx + src[v].x * scl, ty + src[v].y * scl };
	}
}

float VCT_glyph_adv( VFont *font, char C ){
	if( font->monospaced || C <= ' ' ) return font->cw;
	else return font->scale * font->ascii[ C - ' ' ].adv;
}

void VCT_render_char( SDL_Renderer *R, VFont *font, char C, float x, float y ){
	SDL_FPoint path [ MAX_PATH_LEN ];
	int g = C - ' ';
	int off = 0;
	for(int p = 0; p < font->ascii[g].path_count; ++p ){
		int stride = font->ascii[g].offsets[p] - off;
		transform_path( path, font->ascii[g].verts + off, stride, x, y, font->scale );
		SDL_RenderLines( R, path, stride );
		off = font->ascii[g].offsets[p];
	}
}

void VCT_Text_draw_cursor( SDL_Renderer *R, VFont *font, char *str, int tx, int ty, int cursor ){
	int X = tx + (cursor * 8 * font->scale );
	SDL_RenderLine( R, X, ty, X, ty + font->scale * font->line_height );
}

void VCT_render_string( SDL_Renderer *R, VFont *font, char *string, float x, float y ){

	if( string == NULL ) return;

	int len = SDL_strlen( string );
	REFRESH_CW();
	float ox = x;
	for( int i = 0; i < len; i++ ){
		if( string[i] == '\n' ){
			y += font->line_height * font->scale;
			x = ox;
		}
		else if( string[i] == '\t' ){
			x += (TAB_SIZE * font->cw);
		}
		else if( (Uint8)(string[i]) < '!' ){
			x += font->cw;
		}
		else{
			VCT_render_char( R, font, string[i], x, y );
			x += VCT_glyph_adv( font, string[i] );
		}
	}
}


void VCT_render_section( SDL_Renderer *R, VFont *font, char *string, int start, int end, float x, float y ){

	if( string == NULL ) return;

	float ox = x;
	REFRESH_CW();
	float space = font->scale * font->space;
	for( int i = start; i < end; i++ ){
		if( string[i] == '\0' ) return;
		
		if( string[i] == '\n' ){
			y += font->line_height * font->scale;
			x = ox;
		}
		else if( string[i] == '\t' ){
			x += (TAB_SIZE * font->cw);
		}
		else if( (Uint8)(string[i]) < '!' ){
			x += font->cw;
		}
		else{
			VCT_render_char( R, font, string[i], x, y );
			x += VCT_glyph_adv( font, string[i] );
		}
	}
}

bool tightly_bound( unsigned char c ){
	if( c <= ' ' ) return false;
	const char solvents [] = "-+*/\\_<>#|`~^";  // some hand-picked non-binders. 
	for (int i = 0; solvents[i] != '\0'; ++i ){
		if( c == solvents[i] ) return false;
	}
	return true;
}

float trim_width( VFont *font, char *string, int start, int end, float lw ){

	for (int i = start; i < end; ++i ){
		if( !SDL_isblank(string[i]) ) break;
		lw -= font->cw;
	}
	for (int i = end-1; i >= start; --i ){
		if( !SDL_isblank(string[i]) ) break;
		lw -= font->cw;
	}
	return lw;
}

int VCT_wrap_line( VFont *font, char *string, int start, float width, float *line_width ){

	if( string == NULL ) return -1;

	//SDL_Log("wrapping @: %d -> %g\n", start, width );
	
	REFRESH_CW();
	float lw = 0;// line width
	int i = start;
	int j = start;
	while( string[i] != '\0' ){
		float tw = 0;// token width
		j = i;
		do{
			tw += VCT_glyph_adv( font, string[j] );
			if( tw > width ){// this token is too big to fit the wrap width
				tw -= VCT_glyph_adv( font, string[j] ); // crack it
				lw += tw;
				if( line_width != NULL ) *line_width = trim_width( font, string, start, j, lw );
				return j;
			}
			j++;
		} while( string[j] != '\0' && tightly_bound( string[j] ) );

		if( lw + tw > width || string[i] == '\n' ){
			if( line_width != NULL ) *line_width = trim_width( font, string, start, i, lw );
			while( string[i] <= ' ' ){ i += 1; } //spaces and tabs disappear in the line breaks
			return i;
		}
		else{
			/*char buf [64];
			for (int c = i; c <= j; ++c ){
				if( c == j ) buf[c-i] = '\0';
				else buf[c-i] = string[c];
			}
			SDL_Log( "this token fits: (%s). tw: %g, lw: %g", buf, tw, lw );*/

			lw += tw;
		}

		i = j;
	}
	if( line_width != NULL ) *line_width = trim_width( font, string, start, j, lw );
	return i;
}

int VCT_render_string_wrapped( SDL_Renderer *R, VFont *font, char *string, float x, float y, float width ){

	if( string == NULL ) return -1;
	int len = SDL_strlen( string );
	float cy = y;
	float line_height = font->line_height * font->scale;

	if( width < font->scale * font->advance ){
		SDL_Log("VCT_render_string_wrapped: umm.. that's a really narrow textbox you got there...");
		return -1;
	}
	int lines = 0;
	int i = 0;
	do{
		int j = VCT_wrap_line( font, string, i, width, NULL );
		if( i == j ) break;
		VCT_render_section( R, font, string, i, j, x, cy );
		cy += line_height;
		i = j;
		lines++;
	} while( i < len );

	return lines;
}

float VCT_whitespace( VFont *font, char *string, int start, int end ){
	float ws = 0;
	bool begun = 0;
	for (int i = start; i < end; ++i ){
		if( string[i] == '\t' ) ws += TAB_SIZE * font->scale * font->advance;
		else if( string[i] == ' '  ) ws += font->scale * font->space;
	}
	return ws;
}

void VCT_render_string_wrapped_aligned( SDL_Renderer *R, VFont *font, char *string, int x, int y, int width, int alignment ){

	if( string == NULL ) return;

	if( alignment == VCT_ALIGN_LEFT ){
		VCT_render_string_wrapped( R, font, string, x, y, width );
		return;
	}

	int len = SDL_strlen( string );
	float cy = y;
	float line_height = font->line_height * font->scale;

	if( width < font->scale * font->advance ){
		SDL_Log("VCT_render_string_wrapped_aligned: umm.. that's a really narrow textbox you got there...");
		return;
	}

	int i = 0;
	do{
		float lw = 0;
		int j = VCT_wrap_line( font, string, i, width, &lw );
		if( i == j ) break;

		switch( alignment ){
			case VCT_ALIGN_CENTER:
				VCT_render_section( R, font, string, i, j, SDL_roundf(x + (0.5 * (width - lw))), cy );
				break;
			case VCT_ALIGN_RIGHT:
				VCT_render_section( R, font, string, i, j, x + width - lw, cy );
				break;
			case VCT_JUSTIFY:;
				float s = font->space;
				float ws = VCT_whitespace( font, string, i, j );
				font->space = font->space * ((width -lw + ws) / ws);
				VCT_render_section( R, font, string, i, j, x, cy );
				font->space = s;
				break;
		}
		cy += line_height;
		i = j;
	} while( i < len );
}

void VCT_SizeText( VFont *font, char *string, float *w, float *h ){

	if( string == NULL ) return;

	int lines = 0;

	if( w != NULL ){
		lines = 1;
		*w = 0;
		float lw = 0;
		float maxw = 0;
		REFRESH_CW();
		for ( int i = 0; string[i] != '\0'; i++ ){
			if( string[i] == '\n' ){
				if( lw > maxw ){
					maxw = lw;
				}
				lines++;
				lw = 0;
			}
			else if( string[i] == '\t' ){
				lw += TAB_SIZE * font->cw;
			} else if( string[i] < '!' ){
				lw += font->cw;
			}
			else{
				lw += VCT_glyph_adv( font, string[i] );
			}
		}
		if( lw > maxw ){
			maxw = lw;
		}
		*w = maxw;
	}
	if( h != NULL ){
		if( lines <= 0 ){
			lines = 1;
			for ( int i = 0; string[i] != '\0'; ++i ){
				if( string[i] == '\n' ){
					lines++;
				}
			}
		}
		*h = lines * font->scale * font->line_height;
	}
}




Glyph VCT_consolidate_string( VFont *font, char *string ){

	Glyph out;

	out.path_count = 0;
	for (int i = 0; string[i] != '\0'; ++i ){
		int g = string[i] - ' ';
		if( g >= 0 && g < 96 ){
			out.path_count += font->ascii[ g ].path_count;
		}
	}

	int total_verts = 0;
	out.offsets = SDL_malloc( out.path_count * sizeof(int) );
	int pl = 0;
	int prev = 0;
	for (int i = 0; string[i] != '\0'; ++i ){
		int g = string[i] - ' ';
		if( g >= 0 && g < 96 ){
			int off = 0;
			for (int p = 0; p < font->ascii[ g ].path_count; ++p ){
				int stride = font->ascii[g].offsets[p] - off;
				out.offsets[pl] = prev + stride;
				prev = out.offsets[pl];
				off = font->ascii[g].offsets[p];
				pl++;
			}
			if( font->ascii[ g ].path_count > 0 ){
				total_verts += font->ascii[ g ].offsets[ font->ascii[ g ].path_count -1 ];
			}
		}
	}

	REFRESH_CW();
	out.verts = SDL_malloc( total_verts * sizeof(SDL_FPoint) );
	out.adv = 0;
	float cursor = 0;
	int vi = 0;
	for (int i = 0; string[i] != '\0'; ++i ){
		int g = string[i] - ' ';
		if( g >= 0 && g < 96 ){
			int total = 0;
			if( font->ascii[ g ].path_count > 0 ){
				total = font->ascii[ g ].offsets[ font->ascii[ g ].path_count -1 ];
			}
			for (int v = 0; v < total; ++v ){
				out.verts[vi].x = font->scale * font->ascii[ g ].verts[v].x + cursor;
				out.verts[vi].y = font->scale * font->ascii[ g ].verts[v].y;
				int ix = SDL_ceilf( out.verts[vi].x );
				if( ix > out.adv ) out.adv = ix;
				vi++;
			}
		}
		cursor += VCT_glyph_adv( font, string[i] );
	}

	return out;
}


Paths3D Paths3D_from_consolidated_glyph( Glyph *consolidated ){
	Paths3D P;

	P.path_count = consolidated->path_count;
	P.offsets = consolidated->offsets;
	int total_verts = consolidated->offsets[ consolidated->path_count -1 ];
	P.verts = SDL_malloc( total_verts * sizeof(vec3d) );

	return P;
}

void VCT_project_string_on_a_Ball( Paths3D *P, Glyph *consolidated, float text_h,
								   float rad, float xhead, float xarc, 
	                               float yhead, float yarc, float zoff ){

	float lxa = xhead + (xarc * 0.5);
	float rxa = xhead - (xarc * 0.5);
	float bya = yhead + (yarc * 0.5);
	float tya = yhead - (yarc * 0.5);
	int total_verts = consolidated->offsets[ consolidated->path_count -1 ];
	for (int v = 0; v < total_verts; ++v ){
		float a = map( consolidated->verts[v].x, 0, consolidated->adv, lxa, rxa );
		P->verts[v].x = rad * SDL_cos(a);
		P->verts[v].z = zoff - SDL_sin(a);
		a = map( consolidated->verts[v].y, 0, text_h, tya, bya );
		P->verts[v].y = rad * SDL_sin(a);
	}
}

void VCT_project_string_on_a_Ball_w_radii( Paths3D *P, VFont *font, char *string, 
										   float *radii, float xhead, float xarc, 
	                                       float yhead, float yarc, float zoff ){

	float text_w, text_h;
	VCT_SizeText( font, string, &text_w, &text_h );
	float lxa = xhead + (xarc * 0.5);
	float rxa = xhead - (xarc * 0.5);
	float bya = yhead + (yarc * 0.5);
	float tya = yhead - (yarc * 0.5);
	float cursor = 0;
	int pv = 0;
	for (int i = 0; string[i] != '\0'; ++i ){
		int g = string[i] - ' ';
		if( g >= 0 && g < 96 ){
			int total = 0;
			if( font->ascii[ g ].path_count > 0 ){
				total = font->ascii[ g ].offsets[ font->ascii[ g ].path_count -1 ];
			}
			for (int v = 0; v < total; ++v ){
				float a = font->scale * font->ascii[ g ].verts[v].x + cursor;
				a = map( a, 0, text_w, lxa, rxa );
				P->verts[pv].x = radii[ i ] * SDL_cos(a);
				P->verts[pv].z = zoff - SDL_sin(a);
				a = font->scale * font->ascii[ g ].verts[v].y;
				a = map( a, 0, text_h, tya, bya );
				P->verts[pv].y = radii[ i ] * SDL_sin(a);
				pv++;
			}
		}
		cursor += VCT_glyph_adv( font, string[i] );
	}
}


void draw_Paths3D( SDL_Renderer *R, Paths3D *P, float cx, float cy ){

	#define projX( i ) cx + ( P->verts[ i ].x / P->verts[ i ].z )
	#define projY( i ) cy + ( P->verts[ i ].y / P->verts[ i ].z )

	int voff = 0;
	for (int p = 0; p < P->path_count; ++p ){
		float px = projX( voff );
		float py = projY( voff );
		for (int v = voff; v < P->offsets[p]; ++v ){
			float x = projX( v );
			float y = projY( v );
			SDL_RenderLine( R, px, py, x, y );
			px = x; py = y;
		}
		voff = P->offsets[p];
	}
}