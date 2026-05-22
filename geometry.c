#include "geometry.h"
#include "cvec.h"
#include "primitives.h"

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


// based on an algorithm in Andre LeMothe's "Tricks of the Windows Game Programming Gurus"
// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
// intersect the intersection point may be stored in the floats ix and iy.
bool line_intersection(float x0, float y0, float x1, float y1, 
					   float x2, float y2, float x3, float y3, 
					   float *ix, float *iy){

	float s1x, s1y, s2x, s2y;
	s1x = x1 - x0;   s1y = y1 - y0;
	s2x = x3 - x2;   s2y = y3 - y2;

	float s, t;
	s = (-s1y * (x0 - x2) + s1x * (y0 - y2)) / (-s2x * s1y + s1x * s2y);
	t = ( s2x * (y0 - y2) - s2y * (x0 - x2)) / (-s2x * s1y + s1x * s2y);

	if ( (s >= 0.001 && s <= 0.999) && (t >= 0.001 && t <= 0.999) ){
		if (ix != NULL) *ix = x0 + (t * s1x);
		if (iy != NULL) *iy = y0 + (t * s1y);
		return 1;
	}
	return 0;
}

bool v2d_line_intersection(vec2d A1, vec2d A2, vec2d B1, vec2d B2, vec2d *intersection){
	vec2d s1;
	s1.x = A2.x - A1.x;
	s1.y = A2.y - A1.y;
	
	vec2d s2;
	s2.x = B2.x - B1.x;
	s2.y = B2.y - B1.y;
	
	double s, t;
	s = (-s1.y * (A1.x - B1.x) + s1.x * (A1.y - B1.y)) / (-s2.x * s1.y + s1.x * s2.y);
	t = ( s2.x * (A1.y - B1.y) - s2.y * (A1.x - B1.x)) / (-s2.x * s1.y + s1.x * s2.y);
	
	if((s == 0 || s == 1) && (t == 0 || t == 1)){
		if(intersection != NULL){
			intersection->x = A1.x + (t * s1.x);
			intersection->y = A1.y + (t * s1.y);
		}
		return 1;
	}
	return 0;
}

bool Lineseg_intersection(Lineseg LS1, Lineseg LS2, vec2d *intersection){
	vec2d s1;
	s1.x = LS1.B.x - LS1.A.x;
	s1.y = LS1.B.y - LS1.A.y;
	
	vec2d s2;
	s2.x = LS2.B.x - LS2.A.x;
	s2.y = LS2.B.y - LS2.A.y;
	
	double s, t;
	s = (-s1.y * (LS1.A.x - LS2.A.x) + s1.x * (LS1.A.y - LS2.A.y)) / (-s2.x * s1.y + s1.x * s2.y);
	t = ( s2.x * (LS1.A.y - LS2.A.y) - s2.y * (LS1.A.x - LS2.A.x)) / (-s2.x * s1.y + s1.x * s2.y);
	
	if((s == 0 || s == 1) && (t == 0 || t == 1)){
		if(intersection != NULL){
			intersection->x = LS1.A.x + (t * s1.x);
			intersection->y = LS1.A.y + (t * s1.y);
		}
		return 1;
	}
	return 0;
}

bool point_in_path(vec2d p, const Path *path){
    if (!path || path->N < 3) return false;
    int crossings = 0;
    for (int i = 0; i < path->N; ++i) {
        vec2d a = path->verts[i];
        vec2d b = path->verts[(i + 1) % path->N];
        if ((a.y > p.y) != (b.y > p.y)) { // does edge straddle p vertically
            float x_intersect = a.x + (b.x - a.x) * (p.y - a.y) / (b.y - a.y);
            if (x_intersect > p.x) {
                crossings++;
            }
        }
    }
    return (crossings & 1) != 0;
}



// source: http://rosettacode.org/wiki/Sutherland-Hodgman_PathBgon_clipping#C
 
/* tells if vec2d c lies on the left side of directed edge a->b
 * 1 if left, -1 if right, 0 if colinear
 */
int left_of(vec2d a, vec2d b, vec2d c){
	vec2d tmp1 = v2d_diff(b, a);
	vec2d tmp2 = v2d_diff(c, b);
	double x = v2d_cross( tmp1, tmp2 );
	return ( x < 0 )? -1 : x > 0;
}
 
int line_sect(vec2d x0, vec2d x1, vec2d y0, vec2d y1, vec2d *res){
	vec2d dx = v2d_diff( x1, x0 );
	vec2d dy = v2d_diff( y1, y0 );
	vec2d d  = v2d_diff( x0, y0 );
	/* x0 + a dx = y0 + b dy ->
	   x0 X dx = y0 X dx + b dy X dx ->
	   b = (x0 - y0) X dx / (dy X dx) */
	double dyx = v2d_cross( dy, dx );
	if(!dyx) return 0;
	dyx = v2d_cross( d, dx ) / dyx;
	if( dyx <= 0 || dyx >= 1 ) return 0;
 
	res->x = y0.x + dyx * dy.x;
	res->y = y0.y + dyx * dy.y;
	return 1;
}

void geo_offset(Geometric *geo, vec2d offset) {
	if (!geo) return;

	switch (geo->type) {
		case geo_PATH: {
			Path *path = &geo->u.path;
			for (int i = 0; i < path->N; i++) {
				path->verts[i].x += offset.x;
				path->verts[i].y += offset.y;
			}
			break;
		}
		case geo_CIRCLE: {
			geo->u.circle.pos.x += offset.x;
			geo->u.circle.pos.y += offset.y;
			break;
		}
		case geo_BOX: {
			geo->u.box.x += offset.x;
			geo->u.box.y += offset.y;
			// width and height remain unchanged
			break;
		}
		case geo_NULL:
		default:
			// nothing to do
			break;
	}
}

SDL_Rect geo_bb(Geometric *geo) {
    if (!geo) return (SDL_Rect){0, 0, 0, 0};

    double min_x = 9999999, min_y = 9999999, max_x = -9999999, max_y = -9999999;
    int first = 1;

    switch (geo->type) {
        case geo_PATH: {
            Path *p = &geo->u.path;
            if (p->N == 0) return (SDL_Rect){0, 0, 0, 0};
            min_x = max_x = p->verts[0].x;
            min_y = max_y = p->verts[0].y;
            for (int i = 1; i < p->N; ++i) {
                double x = p->verts[i].x;
                double y = p->verts[i].y;
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
            break;
        }

        case geo_CIRCLE: {
            Circle *c = &geo->u.circle;
            min_x = c->pos.x - c->radius;
            min_y = c->pos.y - c->radius;
            max_x = c->pos.x + c->radius;
            max_y = c->pos.y + c->radius;
            break;
        }

        case geo_BOX: {
            SDL_FRect *r = &geo->u.box;
            min_x = r->x;
            min_y = r->y;
            max_x = r->x + r->w;
            max_y = r->y + r->h;
            break;
        }

        default:
            return (SDL_Rect){0, 0, 0, 0};
    }

    SDL_Rect rct;
    rct.x = (int)SDL_floor(min_x);
    rct.y = (int)SDL_floor(min_y);
    rct.w = (int)SDL_ceil(max_x) - rct.x;
    rct.h = (int)SDL_ceil(max_y) - rct.y;
    return rct;
}


Path SDL_FRect_to_Path( SDL_FRect *rect ){
	Path path;

	path.N = 4;
	path.verts = SDL_malloc(sizeof(vec2d) * 4);
	path.closed = true;
	
	if (path.verts == NULL) {
		path.N = 0;
		return path;
	}
	path.verts[0] = (vec2d){rect->x, rect->y};                     // Top-left
	path.verts[1] = (vec2d){rect->x, rect->y + rect->h};           // Bottom-left
	path.verts[2] = (vec2d){rect->x + rect->w, rect->y + rect->h}; // Bottom-right
	path.verts[3] = (vec2d){rect->x + rect->w, rect->y};           // Top-right
	 
	return path;
}

vec2d Path_centroid( Path *p ){
	double minx = 999999, miny = 999999;
	double maxx = -999999, maxy = -999999;
	for (int i = 0; i < p->N; ++i){
		if( p->verts[i].x < minx ) minx = p->verts[i].x;
		if( p->verts[i].y < miny ) miny = p->verts[i].y;
		if( p->verts[i].x > maxx ) maxx = p->verts[i].x;
		if( p->verts[i].y > maxy ) maxy = p->verts[i].y;
	}
	vec2d c;
	c.x = minx + 0.5*(maxx-minx);
	c.y = miny + 0.5*(maxy-miny);
	return c;
}

Circle circumscribe_Path( Path *p ){
	Circle c = {0};
	c.pos = Path_centroid( p );
	double max_dist = 0;
	for (int i = 0; i < p->N; ++i){
		double d = v2d_distsq( c.pos, p->verts[i] );
		if( d > max_dist ) max_dist = d;
	}
	c.radius = SDL_sqrtf( max_dist );
	return c;
}


vec2d geo_centroid( Geometric *geo ){
	switch (geo->type) {
		case geo_PATH:
			return Path_centroid( &(geo->u.path) );
		case geo_CIRCLE:
			return geo->u.circle.pos;
		case geo_BOX: {
			SDL_FRect *r = &geo->u.box;
			return v2d( r->x + r->w / 2.0f, r->y + r->h / 2.0f );
		}
		default:
			return v2dzero;
	}
}

vec2d geo_centralize( Geometric *geo ){
	vec2d c = geo_centroid(geo);
	geo_offset( geo, v2d_neg(c) );
	return c;
}



vec2d random_point_in_geo(Geometric *geo) {
    vec2d result = (vec2d){0.0f, 0.0f};
    if (!geo) return result;

    switch (geo->type) {
        case geo_CIRCLE: {
            float angle = random_angle();
            float radius = geo->u.circle.radius * SDL_sqrtf( SDL_randf() );
            result = (vec2d){ geo->u.circle.pos.x + radius * SDL_cosf(angle),
                              geo->u.circle.pos.y + radius * SDL_sinf(angle) };
        } break;

        case geo_BOX: {
            result = (vec2d){ geo->u.box.x + SDL_randf() * geo->u.box.w,
                              geo->u.box.y + SDL_randf() * geo->u.box.h };
        } break;

        case geo_PATH: {
            const Path *p = &geo->u.path;
            SDL_Rect bb = geo_bb(geo);

            const int MAX_ATTEMPTS = 24;
            int a;
            for (a = 0; a < MAX_ATTEMPTS; ++a) {
                vec2d test = {
                    bb.x + SDL_randf() * bb.w,
                    bb.y + SDL_randf() * bb.h
                };
                if (point_in_path(test, p)) {
                    result = test;
                    break;
                }
            }
            if (a == MAX_ATTEMPTS) {
                result = Path_centroid(p);
            }
        } break;
    }
    return result;
}


void Path_rotate(Path *path, double angle) {
	
	vec2d centroid = Path_centroid( path );
	
	double cos_a = SDL_cos(angle);
	double sin_a = SDL_sin(angle);
	
	for (int i = 0; i < path->N; i++) {
		double dx = path->verts[i].x - centroid.x;
		double dy = path->verts[i].y - centroid.y;
		path->verts[i].x = centroid.x + (dx * cos_a - dy * sin_a);
		path->verts[i].y = centroid.y + (dx * sin_a + dy * cos_a);
	}
}


PathB* PathB_new(){
	return SDL_calloc( 1, sizeof(PathB) );
}
 
void PathB_free( PathB *p ){
	SDL_free(p->verts);
	SDL_free(p);
}
 
void PathB_append(PathB *p, vec2d v){
	if( p->N >= p->alloc ){
		p->alloc *= 2;
		if (!p->alloc) p->alloc = 4;
		p->verts = SDL_realloc( p->verts, sizeof(vec2d) * p->alloc );
	}
	p->verts[ p->N++ ] = v;
}
 
/* this works only if all of the following are true:
 *   1. PathB has no colinear edges;
 *   2. PathB has no duplicate vertices;
 *   3. PathB has at least three vertices;
 *   4. PathB is convex (implying 3).
*/
int Path_winding(Path *p){

	return left_of(p->verts[0], p->verts[1], p->verts[2]);
}
 
void Path_edge_clip(Path *sub, vec2d x0, vec2d x1, int left, PathB *res){

	int i, side0, side1;
	vec2d tmp;
	vec2d v0 = sub->verts[sub->N - 1], v1;
	res->N = 0;
 
	side0 = left_of(x0, x1, v0);
	if (side0 != -left) PathB_append(res, v0);
 
	for( i = 0; i < sub->N; i++ ){
		v1 = sub->verts[i];
		side1 = left_of(x0, x1, v1);
		if (side0 + side1 == 0 && side0)
			/* last point and current straddle the edge */
			if (line_sect(x0, x1, v0, v1, &tmp))
				PathB_append(res, tmp);
		if (i == sub->N - 1) break;
		if (side1 != -left) PathB_append(res, v1);
		v0 = v1;
		side0 = side1;
	}
}
 
Path Path_clip(Path *sub, Path *clip){

	int i;
	PathB *p1 = PathB_new();
	PathB *p2 = PathB_new();
 
	int dir = Path_winding( clip );
	Path_edge_clip(sub, clip->verts[clip->N - 1], clip->verts[0], dir, p2);
	for (i = 0; i < clip->N - 1; i++) {
		PathB *tmp = p2;
		p2 = p1; 
		p1 = tmp;
		if(p1->N == 0) {
			p2->N = 0;
			break;
		}
		Path_edge_clip( (Path*)(p1), clip->verts[i], clip->verts[i + 1], dir, p2);
	}
 
	PathB_free(p1);

	Path out = (Path){ SDL_malloc( p2->N * sizeof(vec2d) ), p2->N, true };
	SDL_memcpy( out.verts, p2->verts, p2->N * sizeof(vec2d) );
	PathB_free(p2);
	return out;
}



void draw_Path( SDL_Renderer *R, Path *P ){
	if( P->closed ){
		for (int v = 0; v < P->N; ++v ){
			int nv = (v+1) % P->N;
			SDL_RenderLine( R, P->verts[v].x, P->verts[v].y, 
							   P->verts[nv].x, P->verts[nv].y );
		}
	}
	else{
		for (int v = 0; v < P->N-1; ++v ){
			int nv = (v+1) % P->N;
			SDL_RenderLine( R, P->verts[v].x, P->verts[v].y, 
							   P->verts[v+1].x, P->verts[v+1].y );
		}
	}
}
void draw_Circle( SDL_Renderer *R, Circle *C ){
	gp_draw_fastcircle( R, C->pos.x, C->pos.y, C->radius );
}
void draw_geo( SDL_Renderer *R, Geometric *geo ){

	switch( geo->type ){
		case geo_PATH:
			draw_Path( R, &(geo->u.path) );
			break;
		case geo_CIRCLE:
			draw_Circle( R, &(geo->u.circle) );
			break;
		case geo_BOX:
			SDL_RenderRect( R, &(geo->u.box) );
			break;
	}
}



void draw_TGeo( SDL_Renderer *R, Geometric *geo, Transform *T, SDL_FPoint *vbuf ){
	switch( geo->type ){
		case geo_PATH:;
			int c = geo->u.path.N;
			for (int v = 0; v < c; ++v ){
				//vbuf[v] = AT( geo->u.path.verts[v], T );
				TM_APPLY_TO( vbuf[v], geo->u.path.verts[v], T->M );
			}
			if( geo->u.path.closed ){
				vbuf[ c++ ] = vbuf[0];
			}
			SDL_RenderLines( R, vbuf, c );
			break;
		case geo_CIRCLE:;
			vec2d tpos = TM_APPLY( vec2d, geo->u.circle.pos, T->M );
			gp_draw_fastcircle( R, tpos.x, tpos.y, T->s * geo->u.circle.radius );
			break;
		case geo_BOX:;
			SDL_FRect trct = apply_transform_frect( &(geo->u.box), T );
			SDL_RenderRect( R, &trct );
			break;
	}
}

void draw_Styled_TGeo( SDL_Renderer *R, Styled_Geo *sg, Transform *T, SDL_FPoint *vbuf ){
	if( sg->style->stroke ){
		SDL_SetRenderDraw_SDL_Color( R, sg->style->stroke_color );
		draw_TGeo( R, &(sg->geo), T, vbuf );
	}
}

void draw_Styled_TGeo_vec( SDL_Renderer *R, Styled_Geo *sgv, Transform *T, SDL_FPoint *vbuf ){
	int N = vec_size( sgv );
	for (int i = 0; i < N; ++i ){
		//draw_Styled_TGeo( R, sgv + i, T, vbuf );
		if( sgv[i].style->stroke ){
			SDL_SetRenderDraw_SDL_Color( R, sgv[i].style->stroke_color );
			draw_TGeo( R, &(sgv[i].geo), T, vbuf );
		}
	}
}

void draw_Styled_RTGeo( SDL_Renderer *R, Styled_Geo *sg, vec2d trig, Transform *T, SDL_FPoint *vbuf ){
	if( sg->style->stroke ){
		SDL_SetRenderDraw_SDL_Color( R, sg->style->stroke_color );
		switch( sg->geo.type ){
			case geo_PATH:;
				int c = sg->geo.u.path.N;
				vec2d *verts = sg->geo.u.path.verts;
				for (int v = 0; v < c; ++v ){
					vbuf[v].x = verts[v].x * trig.x - verts[v].y * trig.y;
					vbuf[v].y = verts[v].x * trig.y + verts[v].y * trig.x;
					vbuf[v] = apply_transform_fp( vbuf[v], T );
				}
				if( sg->geo.u.path.closed ){
					vbuf[ c++ ] = vbuf[0];
				}
				SDL_RenderLines( R, vbuf, c );
				break;
			case geo_CIRCLE:
				gp_draw_fastcircle( R, atfX( sg->geo.u.circle.pos.x, *T ), 
									   atfY( sg->geo.u.circle.pos.y, *T ), 
									   T->s * sg->geo.u.circle.radius );
				break;
		}
	}
}

void draw_Styled_RTGeo_vec( SDL_Renderer *R, Styled_Geo *sgv, vec2d trig, Transform *T, SDL_FPoint *vbuf ){

	int N = vec_size( sgv );
	for (int i = 0; i < N; ++i ){
		draw_Styled_RTGeo( R, sgv + i, trig, T, vbuf );
	}
}


// Animation


int* parse_dope_sheet( const char *s ){
    
    int frame_count = 0;
    int stride = 0;
    int cell_count = 1;
    const char *p = s;

    while( *p  ){
        while( *p && *p != '{' ) p++;
        if( *p != '{') break;
        p++;
        cell_count = 1;
        while( 1 ){
            if( *p == '\0' || *p == '}' ) break;
            if( *p == ',' ) cell_count++;
            p++;
        }
        if( cell_count > stride ) stride = cell_count;
        frame_count++;
    }
    if( frame_count == 0 || stride == 0 ){
        return NULL;
    }

    stride += 1;// frame starts with its length
    int sheet_length = 2 + frame_count * stride;
    if( (cell_count+1) < stride ){// if the last frame would leave padding at the end of the array
    	sheet_length -= (stride - (cell_count+1));// cut it off
    }
    int *DS = SDL_malloc(sheet_length * sizeof(int));

    DS[0] = frame_count;
    DS[1] = stride;
    
    p = s;  // rewind
    int F = 0;
    while( *p && F < frame_count ){
       
        while( *p && *p != '{') p++;
        if( *p != '{') break;
        p++;

        int *frame = DS + 2 + F * stride;
        int cell_count = 1;
        while( 1 ){
            while( *p && !SDL_isdigit(*p) && *p != '}' ) p++;
            if( *p == '}' ) break;
            char *end;
            frame[ cell_count++ ] = SDL_strtol(p, &end, 10);
            p = end;
        }
        frame[0] = cell_count-1;
        F++;
    }

    return DS;
}

void draw_Geo_Animation( SDL_Renderer *R, Geo_Animation *A, int *current_frame, int *timer,
                         Transform *T, SDL_FPoint *vbuf ){

	int f = 2 + (*current_frame) * A->dope_sheet[1];
	for (int ci = 1; ci <= A->dope_sheet[f]; ++ci ){
		int c = A->dope_sheet[f + ci];
		draw_Styled_TGeo( R, A->cells + c, T, vbuf );
	}

	*timer += 1;
	if( *timer >= A->period ){
		*timer = 0;
		*current_frame += 1;
		if( *current_frame >= A->dope_sheet[0] ){
			*current_frame = 0;
		}
	}
}

void free_Geo_Animation( Geo_Animation *A ){

	int max_cell = 0;
	for (int f = 0; f < A->dope_sheet[0]; ++f ){
		int off = 2 + f * A->dope_sheet[1];
		for (int i = 1; i <= A->dope_sheet[off]; ++i ){
			if( A->dope_sheet[off+i] > max_cell ){
				max_cell = A->dope_sheet[off+i];
			}
		}
	}

	for (int c = 0; c < max_cell; ++c ){
		if( A->cells[c].geo.type == geo_PATH &&
		    A->cells[c].geo.u.path.N > 0 ){
			SDL_free( A->cells[c].geo.u.path.verts );
		}
	}
	SDL_free( A->cells );
	SDL_free( A->dope_sheet );

}




// Logging

void log_vec2d( const char* name, vec2d v ){
	SDL_Log("%s: (%.2f, %.2f)", name, v.x, v.y);
}

void log_color( const char* name, SDL_Color c ){
	SDL_Log("%s: rgba(%d, %d, %d, %d)", name, c.r, c.g, c.b, c.a);
}

void log_path( const Path* p ){
	SDL_Log("  Path: N=%d, closed=%s", p->N, p->closed ? "true" : "false");
	for (int i = 0; i < p->N; i++) {
		SDL_Log("    vert[%d]: (%.2f, %.2f)", i, p->verts[i].x, p->verts[i].y);
	}
}

void log_circle( const Circle* c ){
	SDL_Log("  Circle: pos=(%.2f, %.2f), radius=%.2f", c->pos.x, c->pos.y, c->radius);
}

void log_sdl_frect( const SDL_FRect* r ){
	SDL_Log("  Box: x=%.2f, y=%.2f, w=%.2f, h=%.2f", r->x, r->y, r->w, r->h);
}

void log_geometric( const Geometric* g, int index ){
	const char* type_str = "UNKNOWN";
	switch (g->type) {
		case geo_NULL: type_str = "NULL"; break;
		case geo_PATH: type_str = "PATH"; break;
		case geo_CIRCLE: type_str = "CIRCLE"; break;
		case geo_BOX: type_str = "BOX"; break;
	}
	SDL_Log("  Geometric[%d]: type=%s", index, type_str);
	
	switch (g->type) {
		case geo_PATH:
			log_path(&g->u.path);
			break;
		case geo_CIRCLE:
			log_circle(&g->u.circle);
			break;
		case geo_BOX:
			log_sdl_frect(&g->u.box);
			break;
		case geo_NULL:
		default:
			break;
	}
}

void log_style( const Style* s ){
	SDL_Log("  Style:");
	SDL_Log("    stroke: %s", s->stroke ? "true" : "false");
	if (s->stroke) {
		SDL_Log("    stroke_width: %.2f", s->stroke_width);
		log_color("    stroke_color", s->stroke_color);
	}
	SDL_Log("    fill: %s", s->fill ? "true" : "false");
	if (s->fill) {
		log_color("    fill_color", s->fill_color);
	}
}

void log_styled_geo_array( const Styled_Geo* arr, int N, const char* name ){
	SDL_Log("%s: count=%d", name, N);
	for (int i = 0; i < N; i++) {
		SDL_Log("  %s[%d]:", name, i);
		log_geometric(&arr[i].geo, i);
		log_style(arr[i].style);
	}
}
