#ifndef GEOMETRY_INCLUDED
#define GEOMETRY_INCLUDED

#include <SDL.h>
#include "basics.h"
#include "vec2d.h"
#include "transform.h"

// optionally returns intersection point in ix, iy;
bool line_intersection(float x0, float y0, float x1, float y1, 
                       float x2, float y2, float x3, float y3, 
                       float *ix, float *iy);

bool v2d_line_intersection(vec2d A1, vec2d A2, vec2d B1, vec2d B2, vec2d *intersection);

typedef struct Path_struct{
	vec2d *verts;
	int N;
	bool closed;
} Path;

typedef struct Circle_struct{
	vec2d pos;
	float radius;
} Circle;

typedef struct Lineseg_struct{
	vec2d A, B;
	float thickness;
	int cps;
	vec2d cp1, cp2;
} Lineseg;

typedef enum { geo_NULL, geo_PATH, geo_CIRCLE, geo_BOX } geo_type;

typedef struct geometric_struct{

	geo_type type;

	union{
		Path path;
		Circle circle;
		SDL_FRect box;
	} u;

} Geometric;


typedef struct style_struct{

	bool stroke;
	SDL_Color stroke_color;
	float stroke_width;

	bool fill;
	SDL_Color fill_color;
	
} Style;

Uint32 hash_style( Style* style );

typedef struct{
	Geometric geo;
	Style *style;
} Styled_Geo;

bool Lineseg_intersection(Lineseg LS1, Lineseg LS2, vec2d *intersection);


typedef struct { 
	vec2d *verts;
	int N, alloc;
} PathB; // Path Builder

int left_of(vec2d a, vec2d b, vec2d c);

int line_sect(vec2d x0, vec2d x1, vec2d y0, vec2d y1, vec2d *res);

void geo_offset(Geometric *geo, vec2d offset);

SDL_Rect geo_bb(Geometric *geo);

Path SDL_FRect_to_Path( SDL_FRect *rect );

vec2d Path_centroid( Path *p );

Circle circumscribe_Path( Path *p );

vec2d geo_centroid( Geometric *geo );

vec2d geo_centralize( Geometric *geo ); //returns the centroid

void Path_rotate( Path *path, double angle );

PathB* PathB_new();

void PathB_free( PathB *p );

void PathB_append(PathB *p, vec2d v);

int Path_winding(Path *p);

void Path_edge_clip(Path *sub, vec2d x0, vec2d x1, int left, PathB *res);

Path Path_clip(Path *sub, Path *clip);


// Rendering

void draw_Path( SDL_Renderer *R, Path *P );
void draw_Circle( SDL_Renderer *R, Circle *C );
void draw_geo( SDL_Renderer *R, Geometric *geo );
void draw_TGeo( SDL_Renderer *R, Geometric *geo, Transform *T, SDL_FPoint *vbuf );
void draw_Styled_TGeo( SDL_Renderer *R, Styled_Geo *sg, Transform *T, SDL_FPoint *vbuf );
void draw_Styled_TGeo_vec( SDL_Renderer *R, Styled_Geo *sgv, Transform *T, SDL_FPoint *vbuf );

// trig = v2d( cos(angle), sin(angle) ) = v2d_trig(angle)
void draw_Styled_RTGeo( SDL_Renderer *R, Styled_Geo *sg, vec2d trig, Transform *T, SDL_FPoint *vbuf );
void draw_Styled_RTGeo_vec( SDL_Renderer *R, Styled_Geo *sgv, vec2d trig, Transform *T, SDL_FPoint *vbuf );


void log_vec2d( const char* name, vec2d v );
void log_color( const char* name, SDL_Color c );
void log_path( const Path* p );
void log_circle( const Circle* c );
void log_sdl_frect( const SDL_FRect* r );
void log_geometric( const Geometric* g, int index );
void log_style( const Style* s );
void log_styled_geo_array( const Styled_Geo* arr, int N, const char* name );


typedef struct{

	Styled_Geo *cells;

	int *dope_sheet;
	/*
	<frame_count>, <"stride" i.e. max cells per frame +1>,
	<frame[0] cell count>, <frame[0] cell[0]>, ...
	...
	<frame[N] cell count>, ... <frame[N] cell[N]>; (array ends abruptly!)
	*/
	int period; // duration of each animation frame in 60fps frames

} Geo_Animation;

// parse a string formatted like this: "{0,1,2},{0,3},{1,4,5,6}"
// where numbers are indices into cells
int* parse_dope_sheet( const char *s );

void draw_Geo_Animation( SDL_Renderer *R, Geo_Animation *A, int *current_frame, int *timer,
                         Transform *T, SDL_FPoint *vbuf );

#endif