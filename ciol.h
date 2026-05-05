
// CHIPMUNK PHYSICS GRAPHICAL WRAPPER
#ifndef CIOL_H
#define CIOL_H

#include "basics.h"
#include "vec2d.h"
#include "transform.h"
#include "Chipmunk/headers/chipmunk.h"
#include "svg.h"
#include "ok_lib.h"

cpVect cpv_polar( double m, double a );
cpVect cpv_rottrig( cpVect v, vec2d trig );

#define v2d_to_cpv( V2D ) cpv(V2D.x,V2D.y)

cpBitmask de_mask( char *code );
void en_mask( cpBitmask mask, char *code );

void cpBodyUpdateVelocity_NoGravity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);


typedef struct {

	int behavior;
	float density;
	float friction;
	float elasticity;
	int composite;
	cpCollisionType collisionType; // used for advanced collision handling with callbacks
	cpGroup group;        // I don't collide with other members of my (non-zero) group.
	cpBitmask categories; // I belong to these categories...
	cpBitmask mask;       // and I only collide with these categories

} cpProperties;

cpProperties retrieve_cpProperties_from_SVG_metadata( SVG_Element *E, char** Ltags,
													  Hashmap *comp_map, Hashmap *group_map,
													  int default_CT );


cpShape *Geometric_to_cpShape( Geometric *geo, cpBody *body, float stroke_width );

int SVG_layer_into_cpSpace( SVG_Layer *layer, cpSpace *space, bool physical_stroke, int CT );


void ship_hurt( cpArbiter *arb, cpSpace *space, void *unused );
void ship_kamikaze( cpArbiter *arb, cpSpace *space, void *unused );
void ship_shot( cpArbiter *arb, cpSpace *space, void *unused );
void sbod_down( cpArbiter *arb, cpSpace *space, void *unused );


typedef struct obj_struct OBJ;
typedef int (*obj_func) ( OBJ *O );

int empty_obj_func( OBJ *O );

typedef struct obj_struct{

	int type;
	void *data;
	obj_func tick;
	obj_func destroy;

} OBJ;


typedef struct{
	cpBody *body;
	int age;
} ageing_body;

int ageing_body_tick( OBJ *O );
int destroy_ageing_body( OBJ *O );

typedef struct{
	cpBody *body;
	Style *style; //reference!
	int status;
} styled_body;

int styled_body_tick( OBJ *O );
int destroy_styled_body( OBJ *O );


#define OBJ_PAGE_SIZE 64

typedef struct obj_page_struct OBJ_Page;

typedef struct obj_page_struct {

	OBJ *objs;
	int oldest, index;
	bool full;

	OBJ_Page *next;

} OBJ_Page;

void init_OBJ_Page( OBJ_Page *OP );
OBJ *fresh_OBJ_slot( OBJ_Page *OP );
void OBJ_expired( OBJ_Page *OP, int i );
void destroy_OBJ_Book( OBJ_Page *OP, cpSpace *space );

bool cpDestroyBody_and_its_shapes(cpBody *body);

void stroke_cpBody ( SDL_Renderer *R, cpBody *bod, Transform *T );

void stroke_cpSpace( SDL_Renderer *R, cpSpace *space, Transform *T );




/*
	Rendering
	
void cpv_RenderDrawLine( SDL_Renderer *renderer, cpVect A, cpVect B );
void cpv_RenderDrawLineP( SDL_Renderer *renderer, cpVect *A, cpVect *B );

void render_cpvList( SDL_Renderer *renderer, cpVect *v, int n, bool close );
void render_cpvList_transformed( SDL_Renderer *renderer, cpVect *v, int n, bool close, Transform *T );

void renderDraw_path( SDL_Renderer *renderer, cpBody *body, cpVect *geometry, 
	                  int n, bool close, Transform *T );

void renderDraw_layer( SDL_Renderer *renderer, cpBody *body, cpVect **geometry, int geo_length, 
					   int *geo_sections_length, bool *geo_sections_close, Transform *T );

void render_cpObj( SDL_Renderer *renderer, cpObj *cpo, Transform *T );
//void aarender_cpObj( SDL_Renderer *renderer, cpObj *cpo, Transform *T );

void render_cosmetics( SDL_Renderer *renderer, void **list, char *type, int *color, int length, Transform *T );
*/



#endif