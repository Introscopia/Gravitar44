
/* CHIPMUNK INTEROPERABILITY LAYER */

#include "ciol.h"
#include "Chipmunk/headers/chipmunk_structs.h"
#include "geometry.h"
#include "primitives.h"
#include "cvec.h"
#include "game.h"


cpVect cpv_polar( double m, double a ){
    return cpv( m * SDL_cos(a), m * SDL_sin(a) );
}

cpVect cpv_rottrig( cpVect v, vec2d trig ){
	double px = v.x;
    v.x = v.x * trig.x - v.y * trig.y;
    v.y =  px * trig.y + v.y * trig.x;
    return v;
}

cpBitmask de_mask( char *code ){

	if( SDL_strcmp( code, "ALL" ) == 0 ) return CP_ALL_CATEGORIES;
	else if( code [0] == 'A' &&
			 code [1] == 'L' &&
			 code [2] == 'L' &&
			 code [3] == '-' ){
	
		int len = SDL_strlen(code);
		cpBitmask mask = CP_ALL_CATEGORIES;
		for (int i = 4; i < len; ++i ){

			if( code[i] >= 'a' && code[i] <= 'z' ){
			mask &= ~( 1 << (code[i] - 'a'));
			}
			else if( code[i] >= 'A' && code[i] <= 'F' ){
			mask &= ~( 1 << (code[i] - 'A' + 26));
			}

		}
		return mask;
	}
	else{
		int len = SDL_strlen(code);
		cpBitmask mask = 0;
		for (int i = 0; i < len; ++i ){
			if( code[i] >= 'a' && code[i] <= 'z' ){
			mask |= ( 1 << (code[i] - 'a'));
			}
			else if( code[i] >= 'A' && code[i] <= 'F' ){
			mask |= ( 1 << (code[i] - 'A' + 26));
			}
		}
		return mask;
	}
}

void en_mask( cpBitmask mask, char *code ){
	if( mask == CP_ALL_CATEGORIES ){
		SDL_snprintf( code, 33, "ALL" );
		return;
	}
	const char encoding [] = "abcdefghijklmnopqrstuvwxyzABCDEF";
	int c = 0;
	for (int i = 0; i < 32; ++i ){
		if( mask & (1 << i) ){
			code[c++] = encoding[i];
		}
	}
	if( c > 16 ){
		SDL_snprintf( code, 33, "ALL-" );
		c = 4;
		for (int i = 0; i < 32; ++i ){
			if( !(mask & (1 << i)) ){
			code[c++] = encoding[i];
			}
		}
	}
	code[c] = '\0';
}

void cpBodyUpdateVelocity_NoGravity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	// Skip kinematic bodies.
	if(cpBodyGetType(body) == CP_BODY_TYPE_KINEMATIC) return;
	
	cpAssertSoft(body->m > 0.0f && body->i > 0.0f, "Body's mass and moment must be positive to simulate. (Mass: %f Moment: %f)", body->m, body->i);
	
  //body->v = cpvadd( cpvmult(body->v, damping), cpvmult(cpvadd(gravity, cpvmult(body->f, body->m_inv)), dt));
	body->v = cpvadd( cpvmult(body->v, damping), cpvmult(cpvmult(body->f, body->m_inv), dt) );
	body->w = body->w * damping + body->t * body->i_inv * dt;
	
	// Reset forces.
	body->f = cpvzero;
	body->t = 0.0f;
	
	//cpAssertSaneBody(body);
}



cpShape *Geometric_to_cpShape( Geometric *geo, cpBody *body, float stroke_width ){

	cpShape *shape = NULL;

	switch( geo->type ){

			case geo_PATH:;

				cpVect* verts = (cpVect*)(geo->u.path.verts);
				int length = geo->u.path.N;

				if( length > 2 ){

					float area = cpAreaForPoly( length, verts, stroke_width * 0.5 );
					if( area < 0 ){
						int N = length / 2;
						for ( int j = 0; j < N; ++j ){
							cpVect temp = verts[j];
							verts[j] = verts[length-1-j];
							verts[length-1-j] = temp;
						}
						//area = -area;
					}
					shape = cpPolyShapeNew( body, length, verts, cpTransformIdentity, stroke_width * 0.5 );
				}
				else if( length == 2 ){//	SEGMENT

					shape = cpSegmentShapeNew( body, verts[0], verts[1], stroke_width * 0.5 );
				}
				break;

			case geo_BOX:;
				cpBB bb = cpBBNew( geo->u.box.x, geo->u.box.y, 
								   geo->u.box.x + geo->u.box.w, 
								   geo->u.box.y + geo->u.box.h );
				float radius = stroke_width * 0.5;
				if( radius <= 0 ) radius = SDL_min( geo->u.box.w, geo->u.box.h ) * 0.04;
				shape = cpBoxShapeNew2( body, bb, radius );
				break;

			case geo_CIRCLE:;
				float rad = geo->u.circle.radius + stroke_width * 0.5;
				shape = cpCircleShapeNew( body, rad, v2d_to_cpv(geo->u.circle.pos) );
				break;
	}

	return shape;
}


cpProperties retrieve_cpProperties_from_SVG_metadata( SVG_Element *E, char** Ltags,
													  Hashmap *comp_map, Hashmap *group_map,
													  int default_CT ){

	cpProperties out = {
		.behavior = 's',
		.density = 0.02,
		.friction = 0.7,
		.elasticity = 0.35,
		.composite = -1,
		.collisionType = default_CT,
		.group = CP_NO_GROUP,
		.categories = CP_ALL_CATEGORIES,
		.mask = CP_ALL_CATEGORIES,
		};
	//SDL_memset( &out, 0, sizeof(out) );

	static const char cpProperty_tags [9][16] = { "behavior", "density", "friction", "elasticity", "composite", 
						                          "collisionType", "group", "categories", "mask" };
	static Hashmap *tag_map = NULL;
	if( tag_map == NULL ){
		tag_map = SDL_malloc( sizeof(*tag_map) );
		ok_map_init_with_capacity( tag_map, 9 );
		for (int i = 0; i < 9; ++i ){
			ok_map_put( tag_map, cpProperty_tags[i], i+1 );
		}
		//SDL_Log("tag_map initiated!");
	}
	if( E == NULL ){
		ok_map_deinit( tag_map );
		SDL_free( tag_map );
		return out;
	}

	for (int m = 0; m < vec_size(E->metadata); ++m ){
		int mti = E->metadata[m].tag_index;
		int t = ok_map_get( tag_map, Ltags[ mti ] )-1;
		if( t < 0 || t > 9 ) continue;
		switch( t ){
			case 0: //behavior
				if( E->metadata[m].data[0] == 'd' ){
					out.behavior = 'd';
				}
				else if( E->metadata[m].data[0] == 'k' ){
					out.behavior = 'k';
				}
				else if( E->metadata[m].data[0] != 's' ){
					SDL_Log( "unreccd behavior: {%s}", E->metadata[m].data );
				}
				break;
			case 1:
				out.density = SDL_atof( E->metadata[m].data );
				break;
			case 2:
				out.friction = SDL_atof( E->metadata[m].data );
				break;
			case 3:
				out.elasticity = SDL_atof( E->metadata[m].data );
				break;
			case 4:
				if( comp_map != NULL ) out.composite = ok_map_get( comp_map, E->metadata[m].data )-1;
				break;
			case 5:
				out.collisionType = SDL_atoi( E->metadata[m].data );
				break;
			case 6:
				if( group_map != NULL ) out.group = ok_map_get( group_map, E->metadata[m].data )-1;
				break;
			case 7:
				//out.categories = E->metadata[m].data.u.i;
				break;
			case 8:
				//out.mask = E->metadata[m].data.u.i;
				break;
		}
	}

	return out;
}



int SVG_layer_into_cpSpace( SVG_Layer *layer, cpSpace *space, bool physical_stroke, int CT ){

	Hashmap comp_map;
	ok_map_init( &comp_map );
	vec2d *comp_centroid = NULL;
	int *comp_count = NULL;

	// first pass to find composites, count them and accumulate their centroids.
	for (int e = 0; e < vec_size(layer->E); ++e ){
		for (int m = 0; m < vec_size(layer->E[e].metadata); ++m ){
			int mti = layer->E[e].metadata[m].tag_index;
			if( SDL_strcmp( layer->tags[ mti ], "composite" ) == 0 ){
				int c = ok_map_get( &comp_map, layer->E[e].metadata[m].data )-1;
				if( c < 0 ){
					c = vec_size( comp_count );
					ok_map_put( &comp_map, layer->E[e].metadata[m].data, c+1 );
					vec_push( comp_centroid, v2dzero );
					vec_push( comp_count, 0 );
				}
				v2d_add( comp_centroid + c, geo_centroid( &(layer->E[e].u.geo) ) );
				comp_count[c] += 1;
			}
		}
	}
	// arrive at the average centroid for each composite
	int comp_N = vec_size( comp_count );
	for (int c = 0; c < comp_N; ++c ){
		v2d_mult( comp_centroid + c, 1.0 / comp_count[c] );
	}

	cpBody **comp_bodies = NULL;
	if( comp_N > 0 ){
		comp_bodies = SDL_calloc( comp_N, sizeof(cpBody*) );
	}

	for (int e = 0; e < vec_size(layer->E); ++e ){

		cpProperties pr = retrieve_cpProperties_from_SVG_metadata( layer->E + e, layer->tags, 
			                                                       &comp_map, NULL, CT );

		cpBody *body = NULL;
		float stroke_width = 0;

		if( pr.behavior == 's' ){
			body = cpSpaceGetStaticBody(space);
		}
		else{
			if( pr.composite >= 0 ){
				vec2d P = comp_centroid[ pr.composite ];
    			geo_offset( &(layer->E[e].u.geo), v2d_neg( P ) );
    			if( comp_bodies[ pr.composite ] == NULL ){
    				     if( pr.behavior == 'k' ) comp_bodies[ pr.composite ] = cpSpaceAddBody( space, cpBodyNewKinematic() );
					else if( pr.behavior == 'd' ) comp_bodies[ pr.composite ] = cpSpaceAddBody( space, cpBodyNew(0, 0) );
					cpBodySetPosition( comp_bodies[ pr.composite ], v2d_to_cpv( P ) );
    			}
    			body = comp_bodies[ pr.composite ];
			}
			else{
				vec2d P = geo_centralize( &(layer->E[e].u.geo) );
				     if( pr.behavior == 'k' ) body = cpSpaceAddBody( space, cpBodyNewKinematic() );
				else if( pr.behavior == 'd' ) body = cpSpaceAddBody( space, cpBodyNew(0, 0) );
				cpBodySetPosition( body, v2d_to_cpv( P ) );
			}			
		}

		//layer->E[e].u.geo.u.path.closed == false || , the "snake" case...
		if( physical_stroke || ( layer->E[e].u.geo.type == geo_PATH && layer->E[e].u.geo.u.path.N == 2 ) ){
			stroke_width = layer->E[e].style->stroke_width;
		}
		
		cpShape *shape = Geometric_to_cpShape( &(layer->E[e].u.geo), body, stroke_width );

		cpSpaceAddShape( space, shape );
		cpShapeSetDensity( shape, pr.density );
		cpShapeSetFriction( shape, pr.friction );
		cpShapeSetElasticity( shape, pr.elasticity );

		cpShapeSetCollisionType( shape, pr.collisionType );
		if( pr.categories == CP_ALL_CATEGORIES ){
			if( pr.behavior == 's' ) pr.categories = de_mask( "r" );
		}
		cpShapeFilter filter = cpShapeFilterNew( pr.group, pr.categories, pr.mask );
		cpShapeSetFilter( shape, filter );
		//cpShapeSetUserData( shape, (cpDataPointer)(&( )) );
	}

	ok_map_deinit( &comp_map );
	vec_free( comp_centroid );
	vec_free( comp_count );
	SDL_free( comp_bodies );

	return 1;
}


int empty_obj_func( OBJ *O ){ return 0; }

int ageing_body_tick( OBJ *O ){
	ageing_body* ab = (ageing_body*)(O->data);
	ab->age -= 1;
	if( ab->age < 0 ){
		return 1;
	}
	return 0;
}

int destroy_ageing_body( OBJ *O ){
	ageing_body* ab = (ageing_body*)(O->data);
	if( cpDestroyBody_and_its_shapes( ab->body ) <= 0 ){
		// send it to heck so it clears any interactions it was having...
		cpBodySetPosition( ab->body, cpv( -999999 - SDL_rand(999999), -999999 - SDL_rand(999999) ) );
		return 0;
	}
	return 1;
}

int styled_body_tick( OBJ *O ){
	styled_body *sb = (styled_body*)(O->data);
	return sb->status;
}

int destroy_styled_body( OBJ *O ){
	styled_body *sb = (styled_body*)(O->data);
	if( cpDestroyBody_and_its_shapes( sb->body ) <= 0 ){
		// send it to heck so it clears any interactions it was having...
		cpBodySetPosition( sb->body, cpv( -999999 - SDL_rand(999999), -999999 - SDL_rand(999999) ) );
		return 0;
	}
	return 1;
}




void ship_hurt( cpArbiter *arb, cpSpace *space, void *unused ){
	CP_ARBITER_GET_SHAPES(arb, a, b);
	Ship_inst *ship = cpShapeGetUserData( a );
	//double impact_vel = cpvlength( cpArbiterGetSurfaceVelocity( arb ) );

	//cpFloat dt = cpSpaceGetCurrentTimeStep(space);
	//// Convert the impulse to a force by dividing it by the timestep.
	//cpFloat force = cpConstraintGetImpulse(joint)/dt

	//double impulse = cpvlength( cpArbiterTotalImpulse(arb) );
	//cpVect cpArbiterTotalImpulse(cpArbiter *arb);
	double KE = cpArbiterTotalKE(arb);
	ship->hull -= 0.0001 * KE;
	//SDL_Log("impulse: %lg, KE: %lg\n", impulse, KE );
}
void ship_kamikaze( cpArbiter *arb, cpSpace *space, void *unused ){
	CP_ARBITER_GET_SHAPES(arb, a, b);
}
void ship_shot( cpArbiter *arb, cpSpace *space, void *unused ){
	CP_ARBITER_GET_SHAPES(arb, a, b);
}
void sbod_down( cpArbiter *arb, cpSpace *space, void *unused ){
	CP_ARBITER_GET_SHAPES( arb, a, b );
	OBJ *O = cpShapeGetUserData( a );
	styled_body* sb = (styled_body*)(O->data);
	sb->status = 1;

	bullet_impact *bimp = SDL_malloc( sizeof(bullet_impact) );
	bimp->type = BULLET;
	if( cpArbiterGetCount(arb) > 0 ){
		bimp->pos = cpArbiterGetPointA( arb, 0 );//cpBodyLocalToWorld( arb->body_a, arb->contacts[0].r1 );//
	} else {
		bimp->pos = cpBodyGetPosition( arb->body_a );
	}
	bimp->imp = cpArbiterTotalImpulse( arb );
	void ***signal_queue = cpSpaceGetUserData( space );
	vec_push( *signal_queue, bimp );
}


void init_OBJ_Page( OBJ_Page *OP ){
	OP->objs = SDL_calloc( OBJ_PAGE_SIZE, sizeof(OBJ) );
	OP->oldest = 0;
	OP->index = 0;
	OP->full = 0;
	OP->next = NULL;
}

OBJ *fresh_OBJ_slot( OBJ_Page *OP ){
	restart:
	while( OP->full ){
		if( OP->next == NULL ){
			OP->next = SDL_malloc( sizeof(OBJ_Page) );
			init_OBJ_Page( OP->next );
		}
		OP = OP->next;
	}
	if( OP->objs[ OP->index ].type > 0 ){
		int n = cycle( OP->index + 1, 0, OBJ_PAGE_SIZE-1 );
		if( n == OP->oldest ){
			OP->full = 1;
			goto restart;
		}
		OP->index = n;
	}
	return OP->objs + OP->index;
}


void OBJ_expired( OBJ_Page *OP, int i ){
	if( OP->objs[i].destroy( OP->objs + i ) ){
		SDL_free( OP->objs[i].data );
		OP->objs[i].data = NULL;
		OP->objs[i].type = 0;
		OP->full = 0;
		if( OP->oldest == i || OP->objs[OP->oldest].type <= 0 ){
			do{
				OP->oldest = cycle( OP->oldest+1, 0, OBJ_PAGE_SIZE-1 );
				if( OP->oldest == OP->index ) break;
			} while ( OP->objs[ OP->oldest ].type <= 0 );
		}
	}
	// no worries if not! I'll be back...
}

void destroy_OBJ_Book( OBJ_Page *OP, cpSpace *space ){
	do{
		while(1){
			int clear = 0;
			for( int c = 0; c < OBJ_PAGE_SIZE; ++c ){
				if( OP->objs[c].type <= 0 ){ 
					clear++;
					continue;
				}
				OBJ_expired( OP, c );
			}
			if( clear >= OBJ_PAGE_SIZE ) break;

			cpSpaceStep( space, 0.1 );
		}
		SDL_free( OP->objs );
		OBJ_Page *next = OP->next;
		SDL_free( OP );
		OP = next;
	} while( OP != NULL );
}



bool cpDestroyBody_and_its_shapes(cpBody *body){
	if( body ){
		if( body->arbiterList ) return 0;
		cpShape *shape = body->shapeList;
		while( shape ){
			cpShape *next = shape->next;
			cpSpaceRemoveShape( cpBodyGetSpace(body), shape );
			cpShapeFree( shape );
			shape = next;
		}
		cpSpaceRemoveBody( cpBodyGetSpace(body), body );
		cpfree(body);
		return 1;
	}
	return 0;
}





void stroke_cpBody ( SDL_Renderer *R, cpBody *bod, Transform *T ){

	cpShape *shape = bod->shapeList;

	while( shape != NULL ){

		cpShapeType type = shape->klass->type;//cpShapeGetType( shape );
		//printf("type: %d, ", type );

		switch( type ){

			case CP_SEGMENT_SHAPE:;
				cpVect a = cpBodyLocalToWorld( bod, cpSegmentShapeGetA( shape ) );
				cpVect b = cpBodyLocalToWorld( bod, cpSegmentShapeGetB( shape ) );
				a = apply_transform_cpv( a, T );
				b = apply_transform_cpv( b, T );
				float rad = T->s * cpSegmentShapeGetRadius( shape );
				//printf("lsthi: %g\n", rad );
				if( rad < 1.05 ){
					SDL_RenderLine( R, a.x, a.y, b.x, b.y );
				}
				else{
					gp_drawthick_roundedLine( R, a.x, a.y, b.x, b.y, rad );
				}
				break;
			case CP_CIRCLE_SHAPE:;
				cpVect pos = cpBodyLocalToWorld( bod, cpCircleShapeGetOffset( shape ) );
				pos = apply_transform_cpv( pos, T );
				gp_draw_fastcircle( R, pos.x, pos.y, T->s * cpCircleShapeGetRadius( shape ) );
				//printf("cirad: %g\n", cpCircleShapeGetRadius( shape ) );
				break;
			case CP_POLY_SHAPE:;
				int n = cpPolyShapeGetCount( shape );
				//printf("polyn: %d\n", n );
				cpVect prev = cpBodyLocalToWorld( bod, cpPolyShapeGetVert( shape, 0 ) );
				prev = apply_transform_cpv( prev, T );
				for( int j = 1; j < n; ++j ){
					cpVect v = cpBodyLocalToWorld( bod, cpPolyShapeGetVert( shape, j ) );
					v = apply_transform_cpv( v, T );
					SDL_RenderLine( R, prev.x, prev.y, v.x, v.y );
					prev = v;
				}
				cpVect v = cpBodyLocalToWorld( bod, cpPolyShapeGetVert( shape, 0 ) );
				v = apply_transform_cpv( v, T );
				SDL_RenderLine( R, prev.x, prev.y, v.x, v.y );
				break;
		}

		shape = shape->next;
	}
}

void stroke_cpSpace( SDL_Renderer *R, cpSpace *space, Transform *T ){

	for(int i = 0; i < space->staticBodies->num; i++){
		stroke_cpBody ( R, space->staticBodies->arr[i], T );
	}
	stroke_cpBody ( R, space->staticBody, T );

	for(int i = 0; i < space->dynamicBodies->num; i++){
		stroke_cpBody ( R, space->dynamicBodies->arr[i], T );
	}
	for(int i = 0; i < space->rousedBodies->num; i++){
		stroke_cpBody ( R, space->rousedBodies->arr[i], T );
	}
	for(int i = 0; i < space->sleepingComponents->num; i++){
		stroke_cpBody ( R, space->sleepingComponents->arr[i], T );
	}
}











/*
	void gp_drawthick_line( SDL_Renderer *R, float ax, float ay, float bx, float by, float radius ){

	int subx = bx - ax;
	int suby = by - ay;
	double len = SDL_sqrt( (subx * subx) + (suby * suby) ) + SDL_FLT_EPSILON;
	int dx = SDL_lround(radius * subx / len);
	int dy = SDL_lround(radius * suby / len);

	SDL_FColor C = SDL_GetRender_SDL_FColor( R );

	SDL_Vertex verts[4];
	verts[0] = (SDL_Vertex){ { ax + dy, ay - dx }, C, {0,0} };
	verts[1] = (SDL_Vertex){ { ax - dy, ay + dx }, C, {0,0} };
	verts[2] = (SDL_Vertex){ { bx - dy, by + dx }, C, {0,0} };
	verts[3] = (SDL_Vertex){ { bx + dy, by - dx }, C, {0,0} };

	int indices[6] = { 0, 1, 3, 1, 2, 3 };

	SDL_RenderGeometry( R, NULL, verts, 4, indices, 6 );
	}
	void gp_fill_8circle(SDL_Renderer *R, float x, float y, float radius){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		const float trig = { 0.707107*radius};
		SDL_Vertex verts[8];
		verts[0] = (SDL_Vertex){ { x + radius, y          }, C, {0,0} };
		verts[1] = (SDL_Vertex){ { x + trig  , y + trig   }, C, {0,0} };
		verts[2] = (SDL_Vertex){ { x         , y + radius }, C, {0,0} };
		verts[3] = (SDL_Vertex){ { x - trig  , y + trig   }, C, {0,0} };
		verts[4] = (SDL_Vertex){ { x - radius, y          }, C, {0,0} };
		verts[5] = (SDL_Vertex){ { x - trig  , y - trig   }, C, {0,0} };
		verts[6] = (SDL_Vertex){ { x         , y - radius }, C, {0,0} };
		verts[7] = (SDL_Vertex){ { x + trig  , y - trig   }, C, {0,0} };
		int indices[18] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7 };
		SDL_RenderGeometry( R, NULL, verts, 8, indices, 18 );
	}
	
	#define LINETHICKNESS 3
	
	void cpv_RenderDrawLine( SDL_Renderer *renderer, cpVect A, cpVect B ){
	//SDL_RenderLine( renderer, A.x, A.y, B.x, B.y );
	gp_drawthick_line( renderer, A.x, A.y, B.x, B.y, LINETHICKNESS );
	}
	void cpv_RenderDrawLineP( SDL_Renderer *renderer, cpVect *A, cpVect *B ){
	//SDL_RenderLine( renderer, A->x, A->y, B->x, B->y );
	gp_drawthick_line( renderer, A->x, A->y, B->x, B->y, LINETHICKNESS );
	}
	
	void render_cpvList( SDL_Renderer *renderer, cpVect *v, int n, bool close ){
	n -= 1;
	for (int j = 0; j < n; ++j) cpv_RenderDrawLineP( renderer, v + j, v + j+1 );
    if( close ) cpv_RenderDrawLineP( renderer, v + n, v + 0 );

	for (int j = 0; j < n; ++j){
		gp_fill_8circle( renderer, v[j].x, v[j].y, LINETHICKNESS );
	}
	}
	
	void render_cpvList_transformed( SDL_Renderer *renderer, cpVect *v, int n, bool close, Transform *T ){
	n -= 1;
	for (int j = 0; j < n; ++j){
		cpv_RenderDrawLine( renderer, apply_transform_cpv(v + j, T), apply_transform_cpv( v + j+1, T ) );
	}
    if( close ){
    	cpv_RenderDrawLine( renderer, apply_transform_cpv(v + n, T), apply_transform_cpv(v + 0, T) );
    }
	}
	
	void render_cpPolyShape_wireframe_transform( SDL_Renderer *renderer, cpBody *body, cpPolyShape **shapes, 
											 int N, Transform *T ){
    for (int i = 0; i < N; ++i){
        int n = cpPolyShapeGetCount( shapes[i] );
        cpVect *v = SDL_malloc( n * sizeof(cpVect) );
        for( int j = 0; j < n; ++j ){
            v[j] = cpBodyLocalToWorld( body, cpPolyShapeGetVert( shapes[i], j ) );
            v[j] = apply_transform_cpv( v + j, T );
        }
        n -= 1;
        render_cpvList( renderer, v, n, 1 );
        SDL_free( v );
    }
	}
	
	void renderDraw_path( SDL_Renderer *renderer, cpBody *body, cpVect *geometry, int n, bool close, Transform *T ){

    cpVect *v = SDL_malloc( n * sizeof(cpVect) );
    for( int j = 0; j < n; ++j ){
        v[j] = cpBodyLocalToWorld( body, geometry[j] );
        v[j] = apply_transform_cpv( v + j, T );
    }
    render_cpvList( renderer, v, n, close );
    SDL_free( v );
	}
	
	
	void renderDraw_layer( SDL_Renderer *renderer, cpBody *body, cpVect **geometry, int geo_length, 
					   int *geo_sections_length, bool *geo_sections_close, Transform *T ){

    for (int i = 0; i < geo_length; ++i){
    	renderDraw_path( renderer, body, geometry[i], geo_sections_length[i], geo_sections_close[i], T );
    }
	}
	
	void render_cpObj( SDL_Renderer *renderer, cpObj *cpo, Transform *T ){

	Uint8 *S = (Uint8 *)&(cpo->stroke_color);
	Uint8 *F = (Uint8 *)&(cpo->fill_color);

	switch( cpo->type ){
		case CIRCLE:;

			cpVect pos;
			if( cpBodyGetType( cpo->body ) == CP_BODY_TYPE_STATIC ) pos = cpCircleShapeGetOffset( cpo->shape );
			else pos = cpBodyGetPosition( cpo->body );
			pos = apply_transform_cpv( &pos, T );
			Sint16 x = (Sint16) SDL_lround( pos.x );
			Sint16 y = (Sint16) SDL_lround( pos.y );
			//SDL_Log("%d, %d, %d, %d\n", cpo->stroke, cpo->stroke_color, cpo->fill, cpo->fill_color);
			Sint16 rad = (Sint16) SDL_lround( T->s * cpCircleShapeGetRadius( cpo->shape ) );

			if( cpo->fill ){
				SDL_SetRenderDrawColor( renderer, F[2], F[1], F[0], F[3] );
				SDL_RenderFillCircle( renderer, x, y, rad );
			}
			if( cpo->stroke ){
				SDL_SetRenderDrawColor( renderer, S[2], S[1], S[0], S[3] );
				//SDL_RenderDrawCircle( renderer, x, y, rad );
				gp_draw_8circle( renderer, x, y, rad );
			}

			break;
		case SEGMENT:;

			cpVect a = cpBodyLocalToWorld( cpo->body, cpSegmentShapeGetA( cpo->shape ) );
			cpVect b = cpBodyLocalToWorld( cpo->body, cpSegmentShapeGetB( cpo->shape ) );
			a = apply_transform_cpv( &a, T );
			b = apply_transform_cpv( &b, T );
			SDL_SetRenderDrawColor( renderer, S[2], S[1], S[0], 255 );
			SDL_RenderLine(renderer, a.x, a.y, b.x, b.y );

			break;
		case POLYGON:;

			int n = cpPolyShapeGetCount( cpo->shape );
			cpVect *v = SDL_malloc( n * sizeof(cpVect) );
			for( int j = 0; j < n; ++j ){
			    v[j] = cpBodyLocalToWorld( cpo->body, cpPolyShapeGetVert( cpo->shape, j ) );
			    v[j] = apply_transform_cpv( v + j, T );
			}

		    //if( cpo->fill ) filledPolygonRGBA( renderer, vx, vy, n, F[2], F[1], F[0], 255 );
			if( cpo->stroke ){
				SDL_SetRenderDrawColor( renderer, S[2], S[1], S[0], 255 );
				render_cpvList( renderer, v, n, 1 );
			}

			SDL_free(v);

			break;
	}
	}
	
	void render_cosmetics( SDL_Renderer *renderer, void **list, char *type, int *color, int length, Transform *T ){
	for (int i = 0; i < length; ++i){
		//SDL_Log("<");
		Uint8 *S = (Uint8 *)(color+i);
		SDL_SetRenderDrawColor( renderer, S[2], S[1], S[0], S[3] );
		//SDL_Log( "%c", type[i] );
		switch( type[i] ){
			case 'P':;
				{
				Path *PA = (Path*)list[i];
				//SDL_Log("{");
				render_cpvList_transformed( renderer, PA->list, PA->length, PA->close, T );
				//SDL_Log("}");
				}
				break;
			case 'R':;
				{
				Rect *RE = (Rect*)list[i];
				SDL_FRect rct = (SDL_FRect){ RE->x, RE->y, RE->w, RE->h };
				apply_transform_frect( &rct, T );
				SDL_RenderRect( renderer, &rct );
				}
				break;
			case 'C':
				{
				Circle *CI = (Circle*)list[i];
				int x = SDL_lround( atfX(CI->x, T) );
				int y = SDL_lround( atfX(CI->y, T) );
				int rad = SDL_lround( T->s * CI->radius );
				//SDL_RenderDrawCircle( renderer, x, y, rad );
				gp_draw_8circle( renderer, x, y, rad );
				}
				break;
			case 'A':;
				Arc *AR = (Arc*)list[i];

				break;
		}
	}
	}
*/



/* SONAR

	cpVect sonar_beams [6];
	for (int i = 0; i < 6; ++i ){
		sonar_beams[i] = cpvp( 10000.0, SIXTH_PI + (i * THIRD_PI) );
	}
	cpShapeFilter sonar_filter = cpShapeFilterNew( 0, de_mask( "z" ), de_mask( "r" ) );
	#define SHL 24
	double sonar_history [SHL];
	for (int i = 0; i < SHL; ++i ){
		sonar_history[i] = -1;
	}
	int sonar_hi = 0;


	// -- Sonar (autozoom) --
		double nearest_rock = 999999999;
		cpVect sonar_centroid = cpvzero;
		int sonar_hits = 0;
		for (int i = 0; i < 6; ++i ){
			cpSegmentQueryInfo segInfo = {0};
			cpVect end = cpvadd( p1pos, sonar_beams[i] );
			if( cpSpaceSegmentQueryFirst( space, p1pos, end, 0, sonar_filter, &segInfo) ) {
				double d = cpvdistsq( p1pos, segInfo.point );
				if( d < nearest_rock ) nearest_rock = d;
				sonar_centroid = cpvadd( sonar_centroid, segInfo.point );
				sonar_hits++;
				    //SDL_SetRenderDrawColor( R, 255, 0, 255, 255 );
					//TM_APPLY_TO( point, point, T.M );
					//gp_fill_8circle( R, point.x, point.y, 4 );
			}
		}
		int pshi = sonar_hi;
		sonar_history[ sonar_hi++ ] = SDL_sqrt(nearest_rock);
		if( sonar_hi >= SHL ) sonar_hi = 0;
		nearest_rock = 0;
		for (int i = 0; i < SHL; ++i ){
			if( sonar_history[i] < 0 ) continue;
			nearest_rock += sonar_history[i];
		}
		nearest_rock /= SHL;
		double target_zoom = map( 5 * nearest_rock, 0, 3.5 * bounds_rct.h, 40*circumship0.radius, bounds_rct.h );
		target_zoom = GS->window_rct.h / target_zoom;
		double zoom_speed = 0.005;
		if( target_zoom < T.s ){
			zoom_speed = 0.8 * ((T.s * sonar_history[ pshi ]) / (GS->window_rct.h * 0.5));
		}
		set_scale( &T, lerp( T.s, target_zoom, zoom_speed ) );
		

		// also move camera target towards sonar centroid... (it stinks)
		sonar_centroid = cpvmult( sonar_centroid, 1.0 / sonar_hits );
		cpVect cam_target_target = cpvlerp( p1pos, sonar_centroid, 0.333 );
		cam_target = cpvslerp( cam_target, cam_target_target, 0.12 );
		TM_APPLY_TO( sonar_centroid, sonar_centroid, T.M );
		SDL_SetRenderDrawColor( R, 255, 126, 200, 255 );
		gp_fill_8circle( R, xy(sonar_centroid), 8 );
	*/