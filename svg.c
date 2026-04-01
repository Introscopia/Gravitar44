#include "svg.h"
#include "geometry.h"
#include "ok_lib.h"
#include "cvec.h"





static vec2d *cubic_bezier_verts( vec2d A, vec2d B, vec2d cp1, vec2d cp2, int res ){
	float t = 1.0 / res;
	vec2d *verts = SDL_malloc( res * sizeof(vec2d) );
	for (int b = 1; b <= res; ++b){
		float amt = b * t;
		vec2d inter01 = v2d_lerp( A  , cp1, amt );
		vec2d inter02 = v2d_lerp( cp1, cp2, amt );
		vec2d inter03 = v2d_lerp( cp2, B  , amt );
		vec2d inter11 = v2d_lerp( inter01, inter02, amt );
		vec2d inter12 = v2d_lerp( inter02, inter03, amt );
		verts[b-1] = v2d_lerp( inter11, inter12, amt );
	}
	return verts;
}

static vec2d *quadratic_bezier_verts( vec2d A, vec2d B, vec2d cp1, int res ){
	float t = 1.0 / res;
	vec2d *verts = SDL_malloc( res * sizeof(vec2d) );
	for (int b = 1; b <= res; ++b){
		float amt = b * t;
		vec2d inter1 = v2d_lerp( A, cp1, amt );
		vec2d inter2 = v2d_lerp( cp1, B, amt );
		verts[b-1] = v2d_lerp( inter1, inter2, amt );
	}
	return verts;
}



static vec2d* load_svg_path_d( SDL_IOStream *f, int *length, bool *close ) {

	vec2d* list = NULL;
	if( close != NULL ) *close = 0;
	Sint64 dstart = SDL_TellIO(f);
	*length = 0;
	Sint8 c;
	bool status = SDL_ReadS8(f, &c);
	while( status ){
		//SDL_Log("%c", c );
		if( c == '"' ){
			Sint64 dend = SDL_TellIO(f);
			//SDL_Log( "d: %ld -> %ld (%ld)", dstart, dend, dend-dstart );
			break;
		} 
		else if( c == ' ' ){
			Sint8 nc;
			SDL_ReadS8(f, &nc);
			if( SDL_isdigit( nc ) || nc == '-' ){
				*length += 1;
			}
		}
		status = SDL_ReadS8(f, &c);
	}

	//SDL_Log("\n=== %d\n", *length );
	list = SDL_calloc( (*length), sizeof(vec2d) );
	int i = 0;
	double x = 0, y = 0;
	double dx = 0, dy = 0;
	vec2d lcp = v2dzero;//last control point
	char command = ' ';

	SDL_SeekIO(f, dstart, SDL_IO_SEEK_SET);
	status = SDL_ReadS8(f, &c);
	while( status ){
		//SDL_Log("%c", c );
		if( c == '"' ) break;
		else if( SDL_isalpha( c ) ){
			if( c == 'M' || c == 'm' || //move to
				c == 'L' || c == 'l' || //line to
				c == 'H' || c == 'h' || //horizontal line
				c == 'V' || c == 'v' || //vertical line
				c == 'C' || c == 'c' || //Cubic Bézier
				c == 'S' || c == 's' || //Also Cubic Bézier, but first control point is a reflection of the second control point.
				c == 'Q' || c == 'q' || //Quadratic Bézier
				c == 'T' || c == 't' ){ //Also Quadratic Bézier but the control point is a reflection of the control on the previously listed command relative to the start point of the new T or t command.

				command = c;
			}
			else if( c == 'Z' || c == 'z' ){
				if( close != NULL ) *close = 1;
				break;
			}
			else{
				goto unreccd;
			}
		}
		else if( SDL_isdigit( c ) || c == '-' ){

			SDL_SeekIO(f, -1, SDL_IO_SEEK_CUR);

			int adv = 0;

			//SDL_Log( "cmd: %c", command );

			switch( command ){
				//we could fall through from the Ms to the Ls, since this function doesn't handle subpaths
				//but I'll leave it like this for clarity and for future-proofing
				case 'M': //absolute move to
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					break;
				case 'm': // relative move to
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					x += dx;
					y += dy;
					break;
				case 'L': // absolute line
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					break;
				case 'l': // relative line
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					x += dx;
					y += dy;
					break;
				case 'H': // absolute horizontal
					fscanfIO_til512( f, 32, "%lg %n", &x, &adv );
					break;
				case 'h': // relative horizontal
					fscanfIO_til512( f, 32, "%lg %n", &dx, &adv );
					x += dx;
					break;
				case 'V': // absolute vertical
					fscanfIO_til512( f, 32, "%lg %n", &y, &adv );
					break;
				case 'v': // relative vertical
					fscanfIO_til512( f, 32, "%lg %n", &dy, &adv );
					y += dy;
					break;
				case 'C':{// Absolute Cubic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &(cp1.x), &(cp1.y), &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					vec2d cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &(cp2.x), &(cp2.y), &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					lcp = cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					vec2d now = v2d( x, y );
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,cp2)+v2d_dist(cp2,now)) * BEZ_REZ_QUO );
					if( res > 3 ){
						vec2d *verts = cubic_bezier_verts( prev, now, cp1, cp2, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					} break;
				case 'c':{// Relative Cubic Bézier
					vec2d prev = v2d( x, y );
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					vec2d cp1 = v2d( x+dx, y+dy );
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					vec2d cp2 = v2d( x+dx, y+dy );
					lcp = cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					vec2d now = v2d( x+dx, y+dy );
					x = now.x; y = now.y;
					//SDL_Log("c %lg%*[, ]%lg - %lg%*[, ]%lg. %d/%d\n", prev.x, prev.y, now.x, now.y, i, *length );
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,cp2)+v2d_dist(cp2,now)) * BEZ_REZ_QUO );
					if( res > 3 ){
						vec2d *verts = cubic_bezier_verts( prev, now, cp1, cp2, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 'S':{// Absolute reflected Cubic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1 = v2d_diff( prev, v2d_diff( lcp, prev ) );
					vec2d cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &(cp2.x), &(cp2.y), &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					lcp = cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					vec2d now = v2d( x, y );
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,cp2)+v2d_dist(cp2,now)) * BEZ_REZ_QUO );
					if( res > 3 ){
						vec2d *verts = cubic_bezier_verts( prev, now, cp1, cp2, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 's':{// Relative reflected Cubic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1 = v2d_diff( prev, v2d_diff( lcp, prev ) );
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					vec2d cp2 = v2d( x+dx, y+dy );
					lcp = cp2;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					vec2d now = v2d( x+dx, y+dy );
					x = now.x; y = now.y;
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,cp2)+v2d_dist(cp2,now)) * BEZ_REZ_QUO );
					if( res > 3 ){
						vec2d *verts = cubic_bezier_verts( prev, now, cp1, cp2, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 'Q':{// Absolute Quadratic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &(cp1.x), &(cp1.y), &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					lcp = cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					vec2d now = v2d( x, y );
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,now)) * BEZ_REZ_QUO );
					if( res > 2 ){
						vec2d *verts = quadratic_bezier_verts( prev, now, cp1, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 'q':{// Relative Quadratic Bézier
					vec2d prev = v2d( x, y );
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					vec2d cp1 = v2d( x+dx, y+dy );
					lcp = cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					vec2d now = v2d( x+dx, y+dy );
					x = now.x; y = now.y;
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,now)) * BEZ_REZ_QUO );
					if( res > 2 ){
						vec2d *verts = quadratic_bezier_verts( prev, now, cp1, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 'T':{// Asolute reflected Quadratic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &(cp1.x), &(cp1.y), &adv );
					SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );
					lcp = cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &x, &y, &adv );
					vec2d now = v2d( x, y );
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,now)) * BEZ_REZ_QUO );
					if( res > 2 ){
						vec2d *verts = quadratic_bezier_verts( prev, now, cp1, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
				case 't':{// Relative reflected Quadratic Bézier
					vec2d prev = v2d( x, y );
					vec2d cp1 = v2d_diff( prev, v2d_diff( lcp, prev ) );
					lcp = cp1;
					fscanfIO_til512( f, 64, "%lg%*[, ]%lg %n", &dx, &dy, &adv );
					vec2d now = v2d( x+dx, y+dy );
					x = now.x; y = now.y;
					int res = SDL_lround( (v2d_dist(prev,cp1)+v2d_dist(cp1,now)) * BEZ_REZ_QUO );
					if( res > 2 ){
						vec2d *verts = quadratic_bezier_verts( prev, now, cp1, res );
						*length += res-1;
						list = SDL_realloc( list, (*length) * sizeof(vec2d) );
						for(int v = 0; v < res-1; ++v ){
							list[i++] = verts[v];
						}
						SDL_free( verts );
					}
					}break;
			}

			//fseek_string( f, " " );// fscanfIO_til512 doesn't advance the cursor
			SDL_SeekIO( f, adv, SDL_IO_SEEK_CUR );

			list[i++] = v2d( x, y );
			//if( i >= (*length) ) break;
		}
		else if( c != ' ' ){
			unreccd:
			Sint64 here = SDL_TellIO(f);
			SDL_Log("unreccd command in the d: %c @ %lld", c, here );
		}
		status = SDL_ReadS8(f, &c);
	}
	if( i < *length ){
		*length = i;
		list = SDL_realloc( list, (*length) * sizeof(vec2d) );
	}
	return list;
}

static SDL_Color parse_color(const char* color_str) {
    SDL_Color color = {0, 0, 0, 255};
    
    if( color_str ){
        Uint32 hex_value = 0;
        if( SDL_sscanf( color_str + 1, "%06x", &hex_value ) == 1 ){
            color.r = (hex_value >> 16) & 0xFF;
            color.g = (hex_value >> 8) & 0xFF;
            color.b = hex_value & 0xFF;
            color.a = 255;
        }
    }
    return color;
}


Style parse_svg_style(const char* style_string) {
    Style style = {
        .stroke = false,
        .stroke_color = {0, 0, 0, 255},
        .stroke_width = 1.0f,
        .fill = false,
        .fill_color = {0, 0, 0, 255}
    };
    
    if (!style_string || !style_string[0]) {
        return style;
    }
    
    char* styledup = SDL_strdup(style_string);
    if (!styledup) {
        return style;
    }
    char saveptr [512];
    char* token = SDL_strtok_r(styledup, ";", (char**)&saveptr );
    while( token ){
        while( *token == ' ' ) token++;
        
        char* colon = SDL_strchr(token, ':');
        if( colon ){
            *colon = '\0';
            char* key = token;
            char* value = colon + 1;
            
            char* key_end = key + SDL_strlen( key )- 1;
            while( key_end > key && *key_end == ' ' ){
                *key_end = '\0';
                key_end--;
            }
            
            while( *value == ' ' )value++;
            
            if( SDL_strcmp( key, "fill" ) == 0 ){
            	if( SDL_strcmp( value, "none" ) != 0 ){
	                style.fill = true;
	                style.fill_color = parse_color(value);
	            }
            }
            else if( SDL_strcmp( key, "stroke" ) == 0 ){
            	if( SDL_strcmp( value, "none" ) != 0 ){
	                style.stroke = true;
	                style.stroke_color = parse_color(value);
	            }
            }
            else if( SDL_strcmp(key, "stroke-width" )== 0 ){
                style.stroke_width = SDL_strtod(value, NULL);
            }
        }
        
        token = SDL_strtok_r(NULL, ";", (char**)&saveptr);
    }
    
    SDL_free(styledup);
    return style;
}

Uint32 hash_style( Style* style ){
    //FNV-1a style hash
    Uint32 h = 2166136261u;
    
    h = (h ^ (style->stroke ? 1 : 0)) * 16777619u;
    h = (h ^ style->stroke_color.r) * 16777619u;
    h = (h ^ style->stroke_color.g) * 16777619u;
    h = (h ^ style->stroke_color.b) * 16777619u;
    h = (h ^ style->stroke_color.a) * 16777619u;
    
    union { float f; Uint32 u; } converter;
    converter.f = style->stroke_width;
    h = (h ^ converter.u) * 16777619u;
    
    h = (h ^ (style->fill ? 1 : 0)) * 16777619u;
    h = (h ^ style->fill_color.r) * 16777619u;
    h = (h ^ style->fill_color.g) * 16777619u;
    h = (h ^ style->fill_color.b) * 16777619u;
    h = (h ^ style->fill_color.a) * 16777619u;
    
    return h;
}


#define legal_tag_firstchar(c) (SDL_isalpha(c) || c == '_')

#define legal_tag_char(c) (SDL_isalpha(c) || SDL_isdigit(c) || c == '-' || c == '_' || c == '.' || c == ':')

static bool fscan_tag( SDL_IOStream* f, char tag [] ){
	Sint64 original_pos = SDL_TellIO( f );
	Sint8 c;
	bool status = SDL_ReadS8( f, &c );
	int i = -1;
	while( status ){
		if( i >= 0 ){
			if( !legal_tag_char(c) ){
				tag[i] = '\0';
				i = -2; //ok
				goto tag_finished;
			}
			tag[i++] = c;
		}
		else if( legal_tag_firstchar(c) ){
			i = 0;
			tag[i++] = c;
		}
		status = SDL_ReadS8( f, &c );
	}
	tag_finished:
	if( i != -2 ){
		SDL_Log( "failed to read the tag." );
		SDL_SeekIO( f, original_pos, SDL_IO_SEEK_SET );
		return 0;
	}

	SDL_SeekIO( f, -1, SDL_IO_SEEK_CUR );
	status = fseek_string_before( f, "=", "\n" );
	if( !status ){
		SDL_Log( "failed to find eq. tag: %s", tag );
		SDL_SeekIO( f, original_pos, SDL_IO_SEEK_SET );
		return 0;
	}

	status = fseek_string_before( f, "\"", "\n" );
	if( !status ){
		SDL_Log( "failed to find openquote." );
		SDL_SeekIO( f, original_pos, SDL_IO_SEEK_SET );
		return 0;
	}
	return 1;
}


#define PARSE_ID_AND_ATTRIBUTES() 	while( SDL_GetIOStatus(f) == SDL_IO_STATUS_READY ){                       \
										if( !fscan_tag( f, TAG ) ){                                           \
											SDL_Log("failed to parse attribute tag");                         \
										}                                                                     \
										VAL->len = 0;                                                         \
										if( !fscan_STRBptr_until( f, VAL, "\"" ) ){                           \
											SDL_Log("failed to parse attribute value");                       \
										}                                                                     \
										if( SDL_strcmp( TAG, "id" ) == 0 ){                                   \
											SDL_strlcpy( E->id, VAL->str, 64 );                               \
										}                                                                     \
										else{                                                                 \
											int T = ok_map_get( tag_map, TAG ) -1;                            \
											if( T < 0 ){                                                      \
												vec_push( layer->tags, SDL_strdup(TAG) );                     \
												T = vec_size(layer->tags)-1;                                  \
												ok_map_put( tag_map, vec_last(layer->tags), T+1 );            \
											}                                                                 \
											vec_push( E->metadata, ((Metadata){ T, SDL_strdup(VAL->str) }) ); \
										}                                                                     \
										if( fseek_str_before_category( f, ">", SDL_isalpha ) ){               \
											break;                                                            \
										}                                                                     \
									}


void fscan_svg_group( SDL_IOStream* f, SVG_Element **evec, SVG_Layer* layer, 
					 Hashmap *element_map, Hashmap *attribute_map, 
					 Hashmap *tag_map, map_int_int *style_map, STRB *VAL ){

	char TAG [64];

	while( SDL_GetIOStatus(f) == SDL_IO_STATUS_READY ){
		//fspy( f );

		if( fseek_string( f, "<" ) ){
			fscan_str_until_any( f, TAG, 64, " <\n\t\r" );
			int ET = ok_map_get( element_map, TAG ) -1;

			if( ET < 0 ){
				SVG_Element *E = vec_new( *evec );
				E->type = SVG_OTHER;
				E->u.other = SDL_strdup(TAG);
				PARSE_ID_AND_ATTRIBUTES();
			}
			else if( ET == 0 ){ // "/g>"
				SDL_SeekIO( f, -1, SDL_IO_SEEK_CUR );
				//SDL_Log( "UP WE GOOOO" );
				return;
			}
			else if( ET == 4 ){ // "g"
				SVG_Element *E = vec_new( *evec );
				E->type = SVG_GROUP;
				E->u.group = NULL;
				E->metadata = NULL;

				// read group's metadata
				PARSE_ID_AND_ATTRIBUTES();
				
				fscan_svg_group( f, &(E->u.group), layer, element_map, attribute_map, tag_map, style_map, VAL );
			}
			else{ // --- geo! ---------------------------------------------------------------
				SVG_Element *E = vec_new( *evec );
				E->type = SVG_GEO;
				E->u.geo.type = ET;
				E->style = NULL;
				E->metadata = NULL;

				double rotation = 0;
				// read geo's metadata
				while( SDL_GetIOStatus(f) == SDL_IO_STATUS_READY ){
					if( !fscan_tag( f, TAG ) ){
						SDL_Log("failed to scan group's attribute tag");
					}

					int AT = ok_map_get( attribute_map, TAG ) -1;

					switch( AT ){
						case 1: // id
							fscan_str_until( f, E->id, 64, "\"" );
							break;
						case 2:{// style
							VAL->len = 0;
							if( !fscan_STRBptr_until( f, VAL, "\"" ) ){
								SDL_Log("failed to scan style string");
							}
							Style S = parse_svg_style( VAL->str );
							Uint32 H = hash_style( &S );
							int Si = ok_map_get( style_map, H )-1;
							if( Si < 0 ){
								Si = vec_size( layer->styles );
								vec_push( layer->styles, S );
								ok_map_put( style_map, H, Si+1 );
							}
							E->style = (Style*) Si; // storing an integer in a pointer! go ahead, call the cops
							}break;
						case 3: // d
							if( E->u.geo.type != geo_PATH ) SDL_Log( "a d attrib?? I'm confused." );
							//SDL_Log("let's get that d...");
							E->u.geo.u.path.verts = load_svg_path_d( f, &(E->u.geo.u.path.N), 
																		&(E->u.geo.u.path.closed) );
							break;
						case 4: // cx
							if( E->u.geo.type != geo_CIRCLE ) SDL_Log( "a cx attrib?? I'm confused." );
							E->u.geo.u.circle.pos.x = fscan_double( f );
							break;
						case 5: // cy
							if( E->u.geo.type != geo_CIRCLE ) SDL_Log( "a cy attrib?? I'm confused." );
							E->u.geo.u.circle.pos.y = fscan_double( f );
							break;
						case 6: // r
							if( E->u.geo.type != geo_CIRCLE ) SDL_Log( "an r attrib?? I'm confused." );
							E->u.geo.u.circle.radius = fscan_double( f );
							break;
						case 7: // x
							if( E->u.geo.type != geo_BOX ) SDL_Log( "an x attrib?? I'm confused." );
							E->u.geo.u.box.x = fscan_double( f );
							break;
						case 8: // y
							if( E->u.geo.type != geo_BOX ) SDL_Log( "an y attrib?? I'm confused." );
							E->u.geo.u.box.y = fscan_double( f );
							break;
						case 9: // width
							if( E->u.geo.type != geo_BOX ) SDL_Log( "a width attrib?? I'm confused." );
							E->u.geo.u.box.w = fscan_double( f );
							break;
						case 10: // height
							if( E->u.geo.type != geo_BOX ) SDL_Log( "a height attrib?? I'm confused." );
							E->u.geo.u.box.h = fscan_double( f );
							break;
						case 11: // "transform=\"rotate("
							rotation = fscan_double( f );
							break;

						default:{ // not in attribute map
							int T = ok_map_get( tag_map, TAG ) -1;
							if( T < 0 ){
								vec_push( layer->tags, SDL_strdup(TAG) );
								T = vec_size(layer->tags)-1;
								ok_map_put( tag_map, vec_last(layer->tags), T+1 );
							}
							VAL->len = 0;
							if( !fscan_STRBptr_until( f, VAL, "\"" ) ){
								SDL_Log("failed to scan metadata value");
							}
							vec_push( E->metadata, ((Metadata){ T, SDL_strdup(VAL->str) }) );
						}
					}

					if( fseek_str_before_category( f, ">", SDL_isalpha ) ){
						break;
					}
				}

				if( rotation > 0 && E->u.geo.type != geo_CIRCLE ){
					if( E->u.geo.type == geo_BOX ){
						E->u.geo.type = geo_PATH;
						E->u.geo.u.path = SDL_FRect_to_Path( &(E->u.geo.u.box) );
					}
					Path_rotate( &(E->u.geo.u.path), radians( rotation ) );
				}
			}
		}
		else{

		}
	}
}


SVG_Layer* svg_load_layer(SDL_IOStream* f, const char* layer_label ){
	if( !f || !layer_label ){
		SDL_Log("   ?   ");
		return NULL;
	}
	
	SDL_SeekIO( f, 0, SDL_IO_SEEK_SET ); //rewind( f );

	char buf [1024];
	SDL_snprintf( buf, 1024, "inkscape:label=\"%s\"", layer_label );
	
	Sint64 layer_head_locs [3];
	bool layer_search = fseek_ABC( f, "<g", buf, ">", layer_head_locs );

	if( !layer_search || SDL_GetIOStatus(f) != SDL_IO_STATUS_READY ){
		SDL_Log( "layer_search: %d. f status: %d. Error: \"%s\"", layer_search, SDL_GetIOStatus(f), SDL_GetError() );
		return NULL;
	}
	//SDL_Log( "let's load this layer!" );//debug

	SDL_SeekIO( f, layer_head_locs[0], SDL_IO_SEEK_SET );

	SVG_Layer* layer = SDL_calloc( 1, sizeof(SVG_Layer) );

	char TAG [64];
	STRB VAL;
	STRB_init( &VAL, 8 );

	Hashmap tag_map;
	ok_map_init( &tag_map );

	map_int_int style_map;
	ok_map_init( &style_map );

	//                         enum { geo_NULL, geo_PATH, geo_CIRCLE, geo_BOX }
	const char element_labels [5][32] = { "/g>", "path", "circle", "rect", "g" };
	Hashmap element_map;
	ok_map_init( &element_map );
	for (int i = 0; i < 5; ++i ){
		ok_map_put( &element_map, element_labels[i], i+1 );
	}
	const char attribute_labels [12][32] = { ">", "id", "style", "d", "cx", "cy", "r", 
	                                         "x", "y", "width", "height", "transform=\"rotate(" };
	Hashmap attribute_map;
	ok_map_init( &attribute_map );
	for (int i = 0; i < 12; ++i ){
		ok_map_put( &attribute_map, attribute_labels[i], i+1 );
	}

	// read the layer's attributes
	while( SDL_TellIO(f) < layer_head_locs[2] ){
		if( !fscan_tag( f, TAG ) ){
			SDL_Log("failed to scan layer's attribute tag");
		}
		VAL.len = 0;
		if( !fscan_STRBptr_until( f, &VAL, "\"" ) ){
			SDL_Log("failed to scan layer's attribute value");
		}

		int T = ok_map_get( &tag_map, TAG ) -1;
		if( T < 0 ){
			vec_push( layer->tags, SDL_strdup(TAG) );
			T = vec_size(layer->tags)-1;
			ok_map_put( &tag_map, vec_last(layer->tags), T+1 );
		}
		vec_push( layer->metadata, ((Metadata){ T, SDL_strdup(VAL.str) }) );

		if( fseek_str_before_category( f, ">", SDL_isalpha ) ){
			break;
		}
	}

	fscan_svg_group( f, &(layer->E), layer, &element_map, &attribute_map, &tag_map, &style_map, &VAL );

	//SDL_Log("layer->styles:%p", layer->styles );
	for (int e = 0; e < vec_size(layer->E); ++e){
		if( layer->E[e].type == SVG_GEO ){
			int S = (int)(layer->E[e].style);
			layer->E[e].style = layer->styles + S;
			//SDL_Log( "e:%d S:%d p:%p", e, S, layer->E[e].style );
		}
		else if( layer->E[e].type == SVG_GROUP ){
			for (int g = 0; g < vec_size( layer->E[e].u.group ); ++g ){
				int S = (int)(layer->E[e].u.group[g].style);
				layer->E[e].u.group[g].style = layer->styles + S;
			}
		}
	}

	ok_map_deinit( &element_map );
	ok_map_deinit( &attribute_map );
	ok_map_deinit( &style_map );
	ok_map_deinit( &tag_map );

	return layer;
}


Styled_Geo SVG_Element_to_Styled_Geo( SVG_Element *E ){
	Styled_Geo sg = {0};
	if( E->type == SVG_GEO ){
		sg.geo = E->u.geo;
		if( E->u.geo.type == geo_PATH ){// take verts ownership away from the layer.
			E->u.geo.u.path.N = 0;
			E->u.geo.u.path.verts = NULL;
		}
		if( E->style != NULL ){
			sg.style = *(E->style);
		}
	}
	return sg;
}

void SVG_Layer_to_Styled_Geo_vec( SVG_Layer *L, Styled_Geo **SG ){

	vec_init( *SG, vec_size(L->E) );
	for (int e = 0; e < vec_size(L->E); ++e ){
		(*SG)[e] = SVG_Element_to_Styled_Geo( L->E + e );
	}
}

Geo_Animation SVG_Element_to_Geo_Animation( SVG_Layer *L, SVG_Element *E ){
	Geo_Animation GA = {0};
	if( E->type == SVG_GROUP ){
		for (int m = 0; m < vec_size( E->metadata ); ++m ){
			if( SDL_strcmp( L->tags[ E->metadata[m].tag_index ], "frames" ) == 0 ){
				GA.dope_sheet = parse_dope_sheet( E->metadata[m].data );
				int glen = vec_size( E->u.group );
				GA.cells = SDL_calloc( glen, sizeof(Styled_Geo) );
				for (int g = 0; g < glen; ++g ){
					int c = sscan_trailing_int( E->u.group[g].id );
					//SDL_Log( "e:%d, m:%d, g: %d, c:%d", e, m, g, c );
					if( c < 0 || c >= glen ) SDL_Log( "malformed cell name: %d", E->u.group[g].id );
					GA.cells[c] = SVG_Element_to_Styled_Geo( E->u.group + g );
				}
				break;
			}
		}
	}
	else if( E->type == SVG_GEO ){
		GA.dope_sheet = SDL_malloc( 4 * sizeof(int) );
		GA.dope_sheet[0] = 1; GA.dope_sheet[1] = 1;
		GA.dope_sheet[2] = 1; GA.dope_sheet[3] = 0;
		GA.cells = SDL_malloc( sizeof(Styled_Geo) );
		GA.cells[0] = SVG_Element_to_Styled_Geo( E );
	}
	return GA;
}




void SVG_Element_destroy( SVG_Element *E ){
	
	switch( E->type ){
		case SVG_GEO:
			if( E->u.geo.type == geo_PATH &&
			    E->u.geo.u.path.N > 0 ){
				SDL_free(E->u.geo.u.path.verts);
			}
			break;

		case SVG_GROUP:
			for (int g = 0; g < vec_size( E->u.group ); ++g ){
    		 	SVG_Element_destroy( E->u.group + g );
    		 }
    		 vec_delete( E->u.group );
			break;

		case SVG_OTHER:
			SDL_free( E->u.other );
			break;
	}
	for (int m = 0; m < vec_size( E->metadata ); ++m ){
		if( E->metadata[m].data != NULL ){
			SDL_free( E->metadata[m].data );
		}
	}
    vec_delete(E->metadata);
}

void SVG_Layer_destroy( SVG_Layer *layer ){
	if( layer == NULL) return;

	for (int e = 0; e < vec_size(layer->E); ++e ){
		SVG_Element_destroy( layer->E + e );
	}
	vec_delete(layer->E);

	for (int m = 0; m < vec_size(layer->metadata); ++m ){
		if( layer->metadata[m].data != NULL ){
			SDL_free( layer->metadata[m].data );
		}
	}
	vec_delete(layer->metadata);

	for (int i = 0; i < vec_size(layer->tags); ++i) {
	    SDL_free(layer->tags[i]);
	}
	vec_delete(layer->tags);

	vec_delete(layer->styles);

	SDL_free( layer );
}



void SVG_geo_element_dump( SVG_Element *E ){
	switch( E->u.geo.type ){
		case geo_PATH:
	        SDL_Log("Type: PATH");
	        SDL_Log("Id: %s", E->id);
	        SDL_Log("Vertices: %d", E->u.geo.u.path.N);
	        SDL_Log("Closed: %s", E->u.geo.u.path.closed ? "true" : "false");
	        
	        for (int v = 0; v < E->u.geo.u.path.N && v < 3; ++v) {
	            SDL_Log("  Vertex[%d]: (%lg, %lg)", 
	                   v, E->u.geo.u.path.verts[v].x, 
	                   E->u.geo.u.path.verts[v].y);
	        }
	        if (E->u.geo.u.path.N > 3) {
	            SDL_Log("  ... and %d more vertices", E->u.geo.u.path.N - 3);
	        }
	    	break;
	    case  geo_BOX:
	        SDL_Log("Type: BOX");
	        SDL_Log("Name: %s", E->id);
	        SDL_Log("Rect: (%lg, %lg, %lg, %lg)",
	               E->u.geo.u.box.x, E->u.geo.u.box.y,
	               E->u.geo.u.box.w, E->u.geo.u.box.h);
	    	break;
	    case  geo_CIRCLE:
	        SDL_Log("Type: CIRCLE");
	        SDL_Log("Name: %s", E->id);
	        SDL_Log("Center: (%lg, %lg), radius: %lg", 
	               E->u.geo.u.circle.pos.x, E->u.geo.u.circle.pos.y, E->u.geo.u.circle.radius);
	    	break;
    }
}

void SVG_Layer_dump(const SVG_Layer* layer) {
    if (!layer) {
        SDL_Log("SVG_Layer_dump: NULL layer");
        return;
    }
    
    SDL_Log("\n=== SVG Layer Dump ===\n");

    for (int i = 0; i < vec_size(layer->metadata); ++i ){
		SDL_Log( "   layer->metadata[%d]: %s = \"%s\"", i, layer->tags[ layer->metadata[i].tag_index ],
														   layer->metadata[i].data );
	}

    SDL_Log(" # Elements: %d", vec_size(layer->E));

    const char etypes [][16] = { "SVG_NULL", "SVG_GEO", "SVG_GROUP", "SVG_OTHER" };
    
    for (int e = 0; e < vec_size(layer->E); ++e) {
        SDL_Log("\n-->> layer->E[%d] : %s", e, etypes[ layer->E[e].type ] );
        SDL_Log(  "     id: %s", layer->E[e].id );
       	
       	switch( layer->E[e].type ){ 
        	case SVG_GEO:
        		SVG_geo_element_dump( layer->E + e );
        		break;
        	case SVG_GROUP:
        		SDL_Log("     group size: %d", vec_size( layer->E[e].u.group ) );
        		for (int f = 0; f < vec_size( layer->E[e].u.group ); ++f ){
        			SDL_Log("       group[%d] id: %s", f, layer->E[e].u.group[f].id );
        		 	SVG_geo_element_dump( layer->E[e].u.group + f );
        		 }
        		break;
        	case SVG_OTHER:
        		SDL_Log("       layer->E[%d].u.other: %s\n", e, layer->E[e].u.other );
        		break;
        }
        
        //SDL_Log("Metadata entries: %d", layer->metadata_count ? vec_size( layer->E[e].metadata ) : 0);
        
        if (layer->E && layer->E[e].metadata) {
            for (int m = 0; m < vec_size( layer->E[e].metadata ); ++m) {
                Metadata* meta = layer->E[e].metadata + m;
                const char* tag_name = (meta->tag_index >= 0 && meta->tag_index < vec_size(layer->tags)) ? 
                                       layer->tags[meta->tag_index] : "???";
                
                SDL_Log("  Meta[%d]: %s = %s", m, tag_name, meta->data );
            }
        }
    }
}