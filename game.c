#include "game.h"
#include "primitives.h"
#include "cvec.h"
#include "ok_lib.h"


static void new_hashmap( Hashmap *m, int o, int n, ...){
	ok_map_init_with_capacity( m, n );
	va_list vl;
	va_start(vl,n);
	for (int i = 0; i < n; i++){
		ok_map_put( m, va_arg( vl, const char* ), i+o );
	}
	va_end(vl);
}

void load_doodads( char *filename, Library *lib ){

	SDL_Log("loading doodads from \"%s\"", filename );

	SDL_IOStream *f = SDL_IOFromFile( filename, "r" );
	if( f == NULL ){
		SDL_Log( "failed to open \"%s\"", filename );
		return;
	}
	SVG_Layer *L = svg_load_layer( f, "Layer 1" );
	if( L == NULL ){
		SDL_Log( "failed to load \"Layer 1\"" );
		return;
	}
	SDL_CloseIO( f );

	int class_id = -1;
	for (int t = 0; t < vec_size( L->tags ); ++t ){
		if( SDL_strcmp( L->tags[t], "class" ) == 0 ){
			class_id = t;
			break;
		}
	}
	if( class_id < 0 ){
		SDL_Log( "nothing in this layer contains a \"class\" attribute." );
		return;
	}

	int doods_N = 0;

	// how many elements in this layer have a "class" attribute
	int Es = vec_size(L->E);
	for (int e = 0; e < Es; ++e ){
		int ms = vec_size( L->E[e].metadata );
		for (int m = 0; m < ms; ++m ){
			if( L->E[e].metadata[m].tag_index == class_id ){
				doods_N++;
			}
		}
	}

	SDL_Log( "Found %d doods", doods_N );
	SDL_memset( lib, 0, sizeof(Library) );
	vec_init( lib->doodads, doods_N );
	int D = 0;
	// hand over styles library
	lib->styles = L->styles;
	L->styles = NULL;

	Hashmap class_map;
	new_hashmap( &class_map, 2, 3, "ship", "bomb", "item" );
	Hashmap doodelements_map;//                0         1        2        3        4
	new_hashmap( &doodelements_map, 1, 5, "physical", "visual", "exhaust", "center", "smoke" );
	Hashmap ship_attribs_map;
	new_hashmap( &ship_attribs_map, 1, 5, "thrust", "turn_speed", "hull_max", "fuel_max", "fuel_consumption" );


	for (int e = 0; e < Es; ++e ){

		int C = -1;
		int ms = vec_size( L->E[e].metadata );
		for (int m = 0; m < ms; ++m ){
			if( L->E[e].metadata[m].tag_index == class_id ){ //SDL_strncmp( L->tags[t], "class", 5 ) == 0
				C = ok_map_get( &class_map, L->E[e].metadata[m].data )-1;
				if( C < 0 ) SDL_Log( "unreccd class: <%s>", L->E[e].metadata[m].data );
				break;
			}
		}
		if( C >= 0 ){
			lib->doodads[D].type = C;

			switch( C ){
				case SHIP:{

					SDL_Log("shippin'");

					if( L->E[e].type != SVG_GROUP ){
						SDL_Log( "how you gon make a ship and it's not a group!? (%s)", L->E[e].id );
					}

					int gs = vec_size( L->E[e].u.group );
					int doodelements [5];
					for (int g = 0; g < gs; ++g ){
						char *des = sseek_char( L->E[e].u.group[g].id, ':' );
						int de = ok_map_get( &doodelements_map, des )-1;
						if( de >= 0 && de < 5 ){
							doodelements[de] = g;
						}
						else{
							SDL_Log( "element is not a recognized doodelement: \"%s\"", L->E[e].u.group[g].id );
						}
					}

					SDL_strlcpy( lib->doodads[D].name, L->E[e].id, 64 );

					for (int m = 0; m < ms; ++m ){
						int t = L->E[e].metadata[m].tag_index;
						int A = ok_map_get( &ship_attribs_map, L->tags[t] )-1;
						switch( A ){
							case 0: // "thrust"
								lib->doodads[D].u.ship.thrust = SDL_atof( L->E[e].metadata[m].data );
								break;
							case 1:// "turn_speed"
								lib->doodads[D].u.ship.turn_speed = SDL_atof( L->E[e].metadata[m].data );
								break;
							case 2:// "hull_max"
								lib->doodads[D].u.ship.hull_max = SDL_atof( L->E[e].metadata[m].data );
								break;
							case 3:// "fuel_max"
								lib->doodads[D].u.ship.fuel_max = SDL_atof( L->E[e].metadata[m].data );
								break;
							case 4:// "fuel_consumption"
								lib->doodads[D].u.ship.fuel_consumption = SDL_atof( L->E[e].metadata[m].data );
								break;
						}
					}

					// CENTER : ELEMENT 3
					vec2d offset = v2dzero;
					if( doodelements[3] >= 0 ){
						SVG_Element *CE = L->E[e].u.group + doodelements[3];
						if( CE->type != SVG_GEO || CE->u.geo.type != geo_CIRCLE ){
							SDL_Log( "that's not how you do a doodad's center! (%s)", CE->id );
						}
						offset = v2d_neg( CE->u.geo.u.circle.pos );
					}

					// PHYSICAL : ELEMENT 0
					SVG_Element *PE = L->E[e].u.group + doodelements[0];
					if( PE->type == SVG_GROUP ){
						int N = vec_size( PE->u.group );
						vec_init( lib->doodads[D].u.ship.physical, N );
						vec_init( lib->doodads[D].u.ship.properties, N );
						for (int p = 0; p < N; ++p ){
							lib->doodads[D].u.ship.physical[p] = PE->u.group[p].u.geo;
							if( PE->u.group[p].u.geo.type == geo_PATH ) PE->u.group[p].u.geo.u.path.N = 0;// take ownership away from layer
							geo_offset( lib->doodads[D].u.ship.physical + p, offset );

							cpProperties pr = retrieve_cpProperties_from_SVG_metadata( PE->u.group + p, L->tags, NULL, NULL );
							lib->doodads[D].u.ship.properties[p] = pr;
						}
					}
					else{// just one
						vec_init( lib->doodads[D].u.ship.physical, 1 );
						vec_init( lib->doodads[D].u.ship.properties, 1 );
						lib->doodads[D].u.ship.physical[0] = PE->u.geo;
						if( PE->u.geo.type == geo_PATH ) PE->u.geo.u.path.N = 0;// take ownership away from layer
						geo_offset( lib->doodads[D].u.ship.physical + 0, offset );
						cpProperties pr = retrieve_cpProperties_from_SVG_metadata( PE, L->tags, NULL, NULL );
						lib->doodads[D].u.ship.properties[0] = pr;
					}

					//~~~~~I am here
					
					// VISUAL : ELEMENT 1
					SVG_Element *VE = L->E[e].u.group + doodelements[1];
					if( VE->type == SVG_GROUP ){
						int N = vec_size( VE->u.group );
						vec_init( lib->doodads[D].u.ship.visual, N );
						for (int v = 0; v < N; ++v ){
							lib->doodads[D].u.ship.visual[v].geo = VE->u.group[v].u.geo;
							if( VE->u.group[v].u.geo.type == geo_PATH ){
								if( VE->u.group[v].u.geo.u.path.N > lib->longest_path ) lib->longest_path = VE->u.group[v].u.geo.u.path.N;
								VE->u.group[v].u.geo.u.path.N = 0;// take ownership away from layer
							}
							geo_offset( &(lib->doodads[D].u.ship.visual[v].geo), offset );

							lib->doodads[D].u.ship.visual[v].style = VE->u.group[v].style;
						}
					}
					else{// just one
						vec_init( lib->doodads[D].u.ship.visual, 1 );
						lib->doodads[D].u.ship.visual[0].geo = VE->u.geo;
						if( VE->u.geo.type == geo_PATH ){
							if( VE->u.geo.u.path.N > lib->longest_path ) lib->longest_path = VE->u.geo.u.path.N;
							VE->u.geo.u.path.N = 0;// take ownership away from layer
						}
						geo_offset( &(lib->doodads[D].u.ship.visual[0].geo), offset );

						lib->doodads[D].u.ship.visual[0].style = VE->style;
					}

					// EXH : ELEMENT 2
					SVG_Element *EE = L->E[e].u.group + doodelements[2];
					lib->doodads[D].u.ship.exhaust = SVG_Element_to_Geo_Animation( L, EE );
					int exhN = 1;
					if( EE->type == SVG_GROUP ) exhN = vec_size( EE->u.group );
					for (int e = 0; e < exhN; ++e ){
						geo_offset( &(lib->doodads[D].u.ship.exhaust.cells[e].geo), offset );
					}

					// SMOKE : ELEMENT 4
					SVG_Element *SE = L->E[e].u.group + doodelements[4];
					if( doodelements[4] >= 0 && SE->type == SVG_GEO && SE->u.geo.type == geo_CIRCLE ){
						lib->doodads[D].u.ship.smoke_outlet = v2d_to_cpv(v2d_sum( SE->u.geo.u.circle.pos, offset ));
					}
					//else SDL_Log("Bad smoke!");
					} break;
			}

			D++;
		}
	}


	ok_map_deinit( &class_map );
	ok_map_deinit( &doodelements_map );
	ok_map_deinit( &ship_attribs_map );
	SVG_Layer_destroy( L );
}


void log_ship_data(const Ship_data* ship) {
	if (!ship) {
		SDL_Log("Ship_data: NULL");
		return;
	}
	
	SDL_Log("=== Ship_data ===");
	//SDL_Log("name: %s", ship->name);
	SDL_Log("thrust: %.2f", ship->thrust);
	SDL_Log("turn_speed: %.2f", ship->turn_speed);
	SDL_Log("smoke_outlet: %lg, %lg", ship->smoke_outlet.x, ship->smoke_outlet.y);
	
	// log physical geometries
	SDL_Log("physical: count=%d", vec_size( ship->physical ) );
	for (int i = 0; i < vec_size( ship->physical ); i++) {
		log_geometric(&ship->physical[i], i);
	}
	
	// log visual styled geometries
	log_styled_geo_array(ship->visual, vec_size( ship->visual ), "visual");
	
	// log exhaust styled geometries
	SDL_Log( "ship->exhaust: %d frames, period: %d (in 60fps frames)", ship->exhaust.dope_sheet[0], ship->exhaust.period );
}


Ship_inst *instantiate_ship( Ship_data *data ){

	Ship_inst *ship = SDL_calloc( 1, sizeof(Ship_inst) );
	ship->data = data;	
	ship->fuel = data->fuel_max;
	ship->hull = data->hull_max;
	return ship;
}


void init_ship_physics( Ship_inst *ship, cpSpace *space, cpVect pos ){
	ship->body = cpSpaceAddBody( space, cpBodyNew(0, 0) );
	cpBodySetPosition( ship->body, pos );

	int pN = vec_size( ship->data->physical );
	for (int p = 0; p < pN; ++p ){

		cpShape *shape = Geometric_to_cpShape( ship->data->physical + p, ship->body, 0.0 );
		cpSpaceAddShape( space, shape );
		cpShapeSetDensity( shape, ship->data->properties[p].density );
		cpShapeSetFriction( shape, ship->data->properties[p].friction );
		cpShapeSetElasticity( shape, ship->data->properties[p].elasticity );
		cpShapeFilter filter = cpShapeFilterNew( 0, de_mask( "v" ), de_mask( "vs" ) );
		cpShapeSetFilter( shape, filter );
	}
	/*SDL_Log( "ship mass: %g", cpBodyGetMass( ship->body ) );
	cpVect cog = cpBodyGetCenterOfGravity( ship->body );
	SDL_Log( "cog: %lg, %lg", cog.x, cog.y );
	*/
}


void pilot_YAWER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time ){

	if( pilot_vec.y != 0 && ( S->fuel > 0 || S->data->fuel_max <= 0 ) ){
		double THR = delta_time * pilot_vec.y * S->data->thrust;
		cpBodyApplyForceAtLocalPoint( S->body, cpv( 0, THR ), cpvzero );
		S->fuel -= delta_time * S->data->fuel_consumption;
		S->thrusting = true;
	}

	if( prev_pilot_vec.x != 0 && pilot_vec.x == 0 ){
		cpBodySetAngularVelocity( S->body, 0 );
	}
	else if( pilot_vec.x != 0 ){
		cpBodySetAngularVelocity( S->body, delta_time * pilot_vec.x * S->data->turn_speed );
	}
}
void pilot_LEANER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time ){

	if( pilot_vec.y != 0 && ( S->fuel > 0 || S->data->fuel_max <= 0 ) ){
		double THR = delta_time * pilot_vec.y * S->data->thrust;
		cpBodyApplyForceAtLocalPoint( S->body, cpv( 0, THR ), cpvzero );
		S->fuel -= delta_time * S->data->fuel_consumption;
		S->thrusting = true;
	}

	cpFloat current = cpBodyGetAngle( S->body );
	double target = pilot_vec.x * 0.52;
	if( SDL_fabs( target - current ) < 0.025 ){
		cpBodySetAngle( S->body, target );
		cpBodySetAngularVelocity( S->body, 0 );
	}
	else if( current > 0.52 ){
		cpBodySetAngle( S->body, 0.52 );
		cpBodySetAngularVelocity( S->body, 0 );
	}
	else if( current < -0.52 ){
		cpBodySetAngle( S->body, -0.52 );
		cpBodySetAngularVelocity( S->body, 0 );
	}
	else{
		cpBodySetAngularVelocity( S->body, delta_time * SDL_copysign(1.6, target - current) * S->data->turn_speed );
	}
}
void pilot_POINTER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time ){

	cpFloat current = cpBodyGetAngle( S->body );
	pilot_vec = v2d_perp( pilot_vec );
	double THR = v2d_dot( pilot_vec, v2d_trig(current) );
	if( THR > 0.02 && ( S->fuel > 0 || S->data->fuel_max <= 0 ) ){
		THR *= delta_time * S->data->thrust;
		cpBodyApplyForceAtLocalPoint( S->body, cpv( 0, -THR ), cpvzero );
		S->fuel -= delta_time * S->data->fuel_consumption;
		S->thrusting = true;
	}
	
	double target = v2d_heading( pilot_vec );
	double delta = angle_diff( target, current );
	if( SDL_fabs( delta ) < 0.05 || v2d_magsq( pilot_vec ) < 0.1 ){
		cpBodySetAngularVelocity( S->body, 0 );
	}
	else{
		cpBodySetAngularVelocity( S->body, delta_time * SDL_copysign(1, delta) * S->data->turn_speed );
	}
}
void pilot_THRUSTER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time ){

}



void init_flat_world( void **W, SDL_FRect bounds, SDL_FRect goff, Styled_Geo *map_visuals, int width ){

	*W = SDL_malloc( sizeof(flat_world) );
	flat_world *fw = (flat_world*)(*W);

	fw->bounds = bounds;
	fw->gravity_falloff = goff;

	fw->chunks_N = 6;
	fw->chunks = SDL_calloc( fw->chunks_N, sizeof(int*) );
	fw->chunk_w = bounds.w / fw->chunks_N;
	fw->width = width;
	
	//SDL_Log("chunk_w: %g\n", chunk_w );
	int map_N = vec_size( map_visuals );
	for (int i = 0; i < map_N; ++i ){
		SDL_Rect bb = geo_bb( &(map_visuals[i].geo) );
		int l = SDL_floor( bb.x / fw->chunk_w );
		int r = SDL_floor( (bb.x + bb.w) / fw->chunk_w );
		if( l < 0 || r >= fw->chunks_N ){
			//SDL_Log( "there's some shit outta bounds" );
			l = constrain( l, 0, fw->chunks_N-1 );
			r = constrain( r, 0, fw->chunks_N-1 );
		}
		for (int c = l; c <= r; ++c ){
			vec_push( fw->chunks[c], i );
		}
	}

	/* this array helps us make sure we don't draw the same geo more than once,
	   which could happen due to the nature of the chunking system, where
	   geos can span more than one chunk */
	fw->deja_size = map_N * sizeof( Sint8 );
	fw->deja_rendu = SDL_malloc( fw->deja_size );
}
void destroy_flat_world( void *W ){
	flat_world *fw = (flat_world*) W;
	SDL_free( fw->deja_rendu );
	for (int c = 0; c < fw->chunks_N; ++c ){
		vec_delete( fw->chunks[c] );
	}
	SDL_free( fw->chunks );
	SDL_free( W );
}
void flat_world_bounding( void *W, cpBody *b ){
	flat_world *fw = (flat_world*) W;
	cpVect pos = cpBodyGetPosition( b );
	if( pos.x < 0 ){ // warp left
		pos.x += fw->bounds.w;
		cpBodySetPosition( b, pos );
	}
	if( pos.x > fw->bounds.w ){// warp right
		pos.x -= fw->bounds.w;
		cpBodySetPosition( b, pos );
	}
}
void flat_world_update_camera( Transform *T, cpVect target, double *world_angle, 
	                           SDL_FRect window_rct, SDL_FRect bounds ){
	T->tx = target.x;
	
	float viewport_h = window_rct.h * T->invs;
	if( viewport_h + SDL_FLT_EPSILON >= bounds.h ){
		T->ty = bounds.y + (viewport_h * 0.5f);
	}
	else{
		float half_vph = viewport_h * 0.499f;
	    bool out_the_top = target.y - half_vph < bounds.y;
	    bool out_the_bottom = target.y + half_vph > bounds.y + bounds.h;
	    float new_ty = target.y;
	    
	    if( out_the_top && !out_the_bottom ){
	        new_ty = bounds.y + half_vph;
	    } else if( out_the_bottom && !out_the_top ){
	        new_ty = bounds.y + bounds.h - half_vph;
	    }
	    T->ty = new_ty;
	}

	update_TM( T, 0, 0, 0, 0 );
}
void render_flat_world( SDL_Renderer *R, void *W, Styled_Geo *map_visuals, Transform *T, SDL_FPoint *vbuf ){
	flat_world *fw = (flat_world*) W;

	SDL_memset( fw->deja_rendu, -99, fw->deja_size );
	int cl = SDL_floor( rtfX(         0, *T ) / fw->chunk_w );
	int cr = SDL_floor( rtfX( fw->width, *T ) / fw->chunk_w );
	for (int c = cl; c <= cr; ++c ){
		int ac = (c < 0)? fw->chunks_N + c : c % fw->chunks_N;
		double pttx = T->tx;
		int uni = 0; // which parallel universe are we in?
		if( c < ac ){ T->tx += fw->bounds.w; uni = -1; }
		if( c > ac ){ T->tx -= fw->bounds.w; uni =  1; }
		update_TM( T, 0, 0, 0, 0 );
		for (int e = 0; e < vec_size( fw->chunks[ac] ); ++e ){
			int me = fw->chunks[ac][e];
			Styled_Geo *sg = map_visuals + me;
			if( fw->deja_rendu[me] != uni && sg->style->stroke ){
				SDL_SetRenderDraw_SDL_Color( R, sg->style->stroke_color );
				draw_TGeo( R, &(sg->geo), T, vbuf );
				fw->deja_rendu[me] = uni;
			}
		}
		T->tx = pttx;
	}

	update_TM( T, 0, 0, 0, 0 ); //put it back the way it was

	/*char buf [256];
		SDL_snprintf( buf + bl, sizeof(buf)-bl, "%d    ", ac );  
		SDL_snprintf( buf, sizeof(buf), "cl: %d (%d), cr: %d (%d)", cl, fw->chunks_N+cl, cr, cr % fw->chunks_N );
		SDL_SetRenderDrawColor(R, 0, 255, 100, 255);
		SDL_RenderDebugText(R, 300, 20, buf);*/

	/* chunk view
		for (int c = 0; c < chunks_N; ++c ){
		Style s = { .stroke = true, .stroke_color = Uint32_to_SDL_Color( cubehelix( c / 6.0 ) ) };
		for (int e = 0; e < vec_size( chunks[c] ); ++e ){
			Styled_Geo sg;
			sg.geo = map_visuals[ chunks[c][e] ].geo;
			sg.style = s;
			draw_Styled_RTGeo( R, &sg, v2d(1, 0), &T, vbuf );
		}
		}*/
}
void flat_world_gravitate( void *W, cpBody *body, double force ){
	flat_world *fw = (flat_world*) W;
	cpVect p = cpBodyGetPosition( body );
	double g = (p.y - fw->gravity_falloff.y) / fw->gravity_falloff.h;
	g = easeInOutQuad( constrainD( g, 0, 1 ) ) * force;
	cpBodyAddVelocity( body, cpv( 0, g ) );
}
bool inside_flat_world( void *W, cpVect p ){
	flat_world *fw = (flat_world*) W;
	//return coordinates_in_FRect( xy(p), &(fw->bounds) );
	if( coordinates_in_FRect( xy(p), fw->bounds ) ) return true;
	else{
		SDL_Log( "out! %lg, %lg.  %g, %g, %g, %g", p.x, p.y, xywh(fw->bounds) );
		return false;
	}
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void init_round_world( void **W, double brad, double srad ){
	*W = SDL_malloc( sizeof(round_world) );
	round_world *rw = (round_world*)(*W);
	rw->bounds = brad;
	rw->bounds_sq = brad * brad;
	rw->surface_rad = srad;
	rw->surface_radsq = srad * srad;
	rw->surface_radcubed = rw->surface_radsq * srad;
}
void destroy_round_world( void *W ){
	//round_world *rw = (round_world*) W;
	SDL_free( W );
}
void round_world_bounding( void *W, cpBody *b ){
	//round_world *rw = (round_world*) W;
	//cpVect pos = cpBodyGetPosition( b );
}
void round_world_update_camera( Transform *T, cpVect target, double *world_angle, SDL_FRect window_rct, SDL_FRect bounds ){
	T->tx = target.x;
	T->ty = target.y;

	double tmsq = cpvlengthsq( target );
	double cam_theta = angle_diff( -cpvtoangle( target ) -HALF_PI, *world_angle );
	double cam_theta_mag = SDL_fabs( cam_theta );
	double factor = ellipticalMap(constrainD(cam_theta_mag,0,HALF_PI), HALF_PI, 0, 1, 0) * 0.16;
	double cam_power = easeInOutQuad( constrainD(tmsq / 22500, 0, 1) * factor ); // sq(KR) == 150^2 == 22,500
	if(cam_power > cam_theta_mag){
		*world_angle += cam_theta;
	}else{
		*world_angle += SDL_copysign( cam_power, cam_theta );
	}

	update_TM_world_rotated( T, *world_angle, 0, 0, 0 );
}
void render_round_world( SDL_Renderer *R, void *W, Styled_Geo *map_visuals, Transform *T, SDL_FPoint *vbuf ){
	round_world *rw = (round_world*) W;
	draw_Styled_TGeo_vec( R, map_visuals, T, vbuf );
}
void round_world_gravitate( void *W, cpBody *body, double force ){
	round_world *rw = (round_world*) W;
	cpVect grav = cpvzero;
	cpVect p = cpBodyGetPosition( body );
	double plsq = cpvlengthsq(p);
	double pl = SDL_sqrt(plsq);
	if( plsq > rw->surface_radsq ){
		grav = cpvmult( cpvneg(p), force / ((plsq * pl) + CPFLOAT_MIN) );
	} else {
		grav = cpvmult( cpvneg(p), force / rw->surface_radcubed );
	}
	cpBodyAddVelocity( body, grav );
}
void round_world_gravitate_simple( void *W, cpBody *body, double force ){
	round_world *rw = (round_world*) W;
	cpVect p = cpBodyGetPosition( body );
	double plsq = cpvlengthsq(p);
	double pl = SDL_sqrt(plsq);
	cpVect grav = cpvmult( cpvneg(p), force / ((plsq * pl) + CPFLOAT_MIN) );
	cpBodyAddVelocity( body, grav );
}
bool inside_round_world( void *W, cpVect p ){
	round_world *rw = (round_world*) W;
	double plsq = cpvlengthsq(p);
	return ( plsq < rw->bounds_sq );
}


// ps: puffscale
void create_smokepuff( cpSpace *space, OBJ *O, float ps, cpVect pos, cpVect vel, Library *lib ){

	O->body = cpSpaceAddBody( space, cpBodyNew(0, 0) );
	cpBodySetPosition( O->body, pos );
	double vm = cpvlength(vel);
	double va = cpvtoangle(vel);
	va += randomF( -0.25, 0.25 );
	vel = cpv_polar( vm, va );
	cpBodySetVelocity( O->body, vel );
	cpBodySetAngle( O->body, random_angle() );
	//cpShape *shape = cpCircleShapeNew( O->body, 5, cpvzero );
	// equilateral triangle, circumradius=4
	cpVect points [3] = { cpv( ps *  1.5,        0 ), 
						  cpv( ps * -0.75, ps *  2.25 / SQRT3 ), 
						  cpv( ps * -0.75, ps * -2.25 / SQRT3 ) };
	for (int v = 0; v < 3; ++v ){ // now fudge it up a little
		points[v] = cpvadd( points[v], cpv_polar( 0.5, random_angle() ) );
	}
	void *shape = cpPolyShapeAlloc();
	cpPolyShapeInitRaw( shape, O->body, 3, points, 0.0 );
	cpBodySetVelocityUpdateFunc( O->body, cpBodyUpdateVelocity_NoGravity );
	cpSpaceAddShape( space, shape );
	cpShapeSet_DEF( shape, 0.001, 0.8, 0.04 );
	cpShapeFilter filter = cpShapeFilterNew( 0, de_mask( "e" ), de_mask( "es" ) );
	cpShapeSetFilter( shape, filter );
	cpShapeSetUserData( shape, (cpDataPointer)O );

	//int v = SDL_rand( vec_size( lib->doodads[ lib->smokepuffs ].u.visuals ) );
	//vec_copy( O->visual, lib->doodads[ lib->smokepuffs ].u.visuals[v] );

	O->data = SDL_calloc( 1, sizeof(int) );// the age of the puff.
	int *age = (int*)(O->data);
	*age = randomI( 35, 50 );
	O->tick = age_and_pass_away;
}



void upon_a_sphere( SDL_Renderer *R, GameState *GS, char *spherepath ){

	bool debug_view = false;
	
	cpSpace *space;
	space = cpSpaceNew();


	Transform T = { 0, 0, GS->cx, GS->cy, 1, 1, {0} };
	set_scale( &T, 1 );
	int scaleI = logarithm( 1.1, T.s );
	double dscaleI = scaleI;
	float min_scaleI = 0;
	double world_angle = 0;
	float KR = 150;


	void *world_data = NULL;
	render_world_func render_world = NULL;
	world_bounding_func world_bounding = NULL;
	gravitate_func gravitate = NULL;
	inside_func inside = NULL;
	
	update_camera_func update_camera = NULL;
	m_update_func camera_matrix = NULL;
	voidptr_func destroy_world = NULL;

	double grav_max = 0;

	//SDL_Log( "loading Physical layer from file \"%s\".", spherepath );
	SDL_IOStream *f = SDL_IOFromFile( spherepath, "r" );

	SVG_Layer *PL = svg_load_layer( f, "Physical" );
	//SDL_Log( "converting layer to space...");
	SVG_layer_into_cpSpace( PL, space, false );

	for (int m = 0; m < vec_size( PL->metadata ); ++m ){
		int t = PL->metadata[m].tag_index;
		if( SDL_strcmp( PL->tags[t], "gravity" ) == 0 ){
			grav_max = SDL_atof( PL->metadata[m].data );
		}
	}

	SVG_Layer_destroy( PL );

	//SDL_Log( "done.\n now loading Visual layer" );
	SVG_Layer *VL = svg_load_layer( f, "Visual" );
	Styled_Geo *map_visuals = NULL;
	SVG_Layer_to_Styled_Geo_vec( VL, &map_visuals );
	// hand off style lib
	Style **visual_styles = VL->styles;
	VL->styles = NULL;
	SVG_Layer_destroy( VL );

	cpVect spawn = cpvzero;

	//SDL_Log( "done.\n now loading Zones layer" );
	SVG_Layer *ZL = svg_load_layer( f, "Zones" );

	Geometric bounds = { geo_NULL };
	SDL_FRect bounds_rct = {0};
	Geometric gravity_falloff = { geo_NULL };
	Geometric spawn_zone = { geo_NULL };
	int camera = 'D'; //efault
	
	for (int z = 0; z < vec_size(ZL->E); ++z ){
		if( SDL_strcmp( ZL->E[z].id, "bounds" ) == 0 ){
			//SDL_Log("bounds!");
			bounds = ZL->E[z].u.geo;

			int ms = vec_size( ZL->E[z].metadata );
			for (int m = 0; m < ms; ++m ){
				int t = ZL->E[z].metadata[m].tag_index;
				if( SDL_strcmp( ZL->tags[t], "camera" ) == 0 ){
					camera = ZL->E[z].metadata[m].data[0];
				}
			}
		}
		else if( SDL_strcmp( ZL->E[z].id, "gravity_falloff" ) == 0 ){
			//SDL_Log("gravity_falloff!");
			gravity_falloff = ZL->E[z].u.geo;
		}
		else if( SDL_strcmp( ZL->E[z].id, "spawn" ) == 0 ){
			//SDL_Log("spawn!");
			spawn_zone = ZL->E[z].u.geo;
		}
	}

	if( bounds.type == geo_BOX ){
		bounds_rct = bounds.u.box;
		init_flat_world( &world_data, bounds.u.box, gravity_falloff.u.box, map_visuals, GS->window_rct.w );
		render_world = render_flat_world;
		world_bounding = flat_world_bounding;
		gravitate = flat_world_gravitate;
		inside = inside_flat_world;
		update_camera = flat_world_update_camera;
		camera_matrix = update_TM_object_rotated;
		destroy_world = destroy_flat_world;

		SDL_FRect windowrect = (SDL_FRect){0,0,GS->window_rct.w, GS->window_rct.h};
		SDL_FRect dst = bounds.u.box;
		fit_frect( &dst, &windowrect );
		set_scale( &T, dst.h / bounds.u.box.h );
		min_scaleI = logarithm( 1.1, T.s );
		scaleI = min_scaleI;

		if( spawn_zone.type == geo_NULL ){
			spawn = cpv( randomF(0, bounds.u.box.w), 
						 bounds.u.box.y + (0.1 * bounds.u.box.h) );
		}
		else{
			spawn = v2d_to_cpv( random_point_in_geo( &spawn_zone ) );
		}
		SDL_Log("spawn: %lg, %lg\n", xy(spawn) );
	}
	else if( bounds.type == geo_CIRCLE ){
		bounds_rct = (SDL_FRect){  -(bounds.u.circle.radius),  -(bounds.u.circle.radius),
								  2*(bounds.u.circle.radius), 2*(bounds.u.circle.radius) };
		double srad = 0;
		if( gravity_falloff.type == geo_CIRCLE ) srad = gravity_falloff.u.circle.radius;
		init_round_world( &world_data, bounds.u.circle.radius, srad );
		render_world = render_round_world;
		world_bounding = round_world_bounding;
		if( gravity_falloff.type == geo_CIRCLE ){
			gravitate = round_world_gravitate;
		} else {
			gravitate = round_world_gravitate_simple; 
		}
		inside = inside_round_world;
		if( camera == 'D' ){
			update_camera = round_world_update_camera;
			camera_matrix = update_TM_combined;
		}
		else if( camera == 'F' ){
			update_camera = flat_world_update_camera;
			camera_matrix = update_TM_object_rotated;
		}
		destroy_world = destroy_round_world;
		
		SDL_FRect windowrect = (SDL_FRect){0,0,GS->window_rct.w, GS->window_rct.h};
		SDL_FRect dst = bounds_rct;
		fit_frect( &dst, &windowrect );
		set_scale( &T, dst.w / (2*(bounds.u.circle.radius)) );
		min_scaleI = logarithm( 1.1, T.s );
		scaleI = min_scaleI;

		world_angle = random_angle();

		if( spawn_zone.type == geo_NULL ){
			spawn = cpv_polar( 0.97*(bounds.u.circle.radius), world_angle - HALF_PI );
		} else {
			spawn = v2d_to_cpv( random_point_in_geo( &spawn_zone ) );
		}
	}


	SVG_Layer_destroy( ZL );

	//cpSpaceSetGravity( space, cpv(0, 10) );
	

	//SDL_Log( "done.\n now loading Holo layer" );
	//SVG_Layer *HL = svg_load_layer( f, "Holo" );

	SDL_CloseIO( f );

	int ships_N = 1;
	Ship_inst **ships = SDL_malloc( ships_N * sizeof(Ship_inst*) );

	ships[0] = GS->hero_ship;
	init_ship_physics( ships[0], space, spawn );
	//cpVect p0 = cpBodyGetPosition( ships[0]->body ); SDL_Log("p0: %lg, %lg\n", xy(p0) );

	//SDL_Log( "puffs: %p, puff_dims.h: %d", GS->lib.puffs, GS->lib.puff_dims.h );

	SDL_Texture *smokey_texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                     SDL_TEXTUREACCESS_TARGET, GS->window_rct.w, GS->window_rct.h );

    SDL_Texture *puff_mask = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                SDL_TEXTUREACCESS_TARGET, GS->window_rct.w, GS->window_rct.h );

    SDL_Texture *smoke_target = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                   SDL_TEXTUREACCESS_TARGET, GS->window_rct.w, GS->window_rct.h );

    SDL_SetRenderTarget( R, smokey_texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
	SDL_RenderClear( R );
	SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
	for (int y = 0; y < GS->window_rct.h; y += 8 ){
		SDL_RenderLine( R, 0, y, GS->window_rct.w, y );
		//gp_draw_24circle( R, GS->cx, GS->cy, y );
	}
	SDL_SetRenderTarget( R, NULL );

	OBJ_Page PUFFOBJS;
	init_OBJ_Page( &PUFFOBJS );

	float puffscale = 1;
	Circle circumship0 = circumscribe_Path( &(ships[0]->data->physical[0].u.path) );
	puffscale = circumship0.radius / 3.5; //heuristic!


	int longest_path = 0;
	for (int i = 0; i < vec_size(map_visuals); ++i ){
		if( map_visuals[i].geo.type == geo_PATH ){
			int N = map_visuals[i].geo.u.path.N;
			if( map_visuals[i].geo.u.path.closed ) N++;
			if( N > longest_path ){
				longest_path = N;
			}
		}
	}
	//SDL_Log( "GS->lib.longest_path: %d longest_path: %d", GS->lib.longest_path, longest_path );
	if( GS->lib.longest_path > longest_path ) longest_path = GS->lib.longest_path;

	SDL_FPoint *vbuf = SDL_malloc( longest_path * sizeof(SDL_FPoint) );

	vec2d prev_pilot_vec = v2dzero;

	int sim_iterations = 6;
	double delta_time = (1.0 / 30.0) / sim_iterations;
	//SDL_Log( "sim_iterations: %d, delta: %lg", sim_iterations, delta_time );

	// warm up the sim
	for (int i = 0; i < 3; ++i ){
		cpSpaceStep( space, delta_time );
	}
	/* |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| */
	/*   |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o|   */
	/* |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| */
	while(1){

		SDL_Event event;
		while( SDL_PollEvent(&event) ){

			//UI_event_handling_function( &main_menu, &event );

			int captured = Dir_HandleEvent( &event, &(GS->flightstick), GS->cx, GS->cy );

			if( !captured ){
				switch (event.type) {
					case SDL_EVENT_QUIT:
						SDL_strlcpy( GS->GOING_TO, "QUIT", 64 );
						GS->going_to_mode = 'Q';
						goto end;

					case SDL_EVENT_MOUSE_WHEEL:;

						scaleI -= event.wheel.y;
						if( scaleI < min_scaleI ) scaleI = SDL_ceilf( min_scaleI );
						set_scale( &T, SDL_pow(1.1, scaleI) );

						break;

					case SDL_EVENT_KEY_UP:
						if( event.key.key == SDLK_F9 ){
							debug_view = !debug_view;
							//SDL_Log( "T = { %lg, %lg,  %g, %g,  %g }", T.tx, T.ty, T.cx, T.cy, T.s );
						}
						else if( event.key.key == SDLK_P ){
							SDL_Log("exporting textures...");
							save_texture("puff_mask.png", R, puff_mask);
							save_texture("smokey_texture.png", R, smokey_texture);
							save_texture("smoke_target.png", R, smoke_target);
						} 
						break;

					case SDL_EVENT_GAMEPAD_AXIS_MOTION:
						//event.gaxis.which == binding->which_gamepad
						if ( event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTY ) {
							static Uint64 axis_motion_cooldown_time = 0;  /* these are spammy, only show every X milliseconds. */
							const Uint64 now = SDL_GetTicks();
							if (now >= axis_motion_cooldown_time) {
								axis_motion_cooldown_time = now + 16;
								dscaleI += 0.3 * (event.gaxis.value / 32767.0f);
								int ids = (int)dscaleI;
								if( scaleI != ids ){
									scaleI = ids;
									set_scale( &T, SDL_pow(1.1, scaleI) );
								}
							}
						} 
						break;
				}
			}
		}


		vec2d pilot_vec = DirInput_compute( &(GS->flightstick) );

		// -- Simulate --
		for (int i = 0; i < sim_iterations; ++i ){

			GS->pilot( ships[0], pilot_vec, prev_pilot_vec, delta_time );
			for (int s = 0; s < ships_N; ++s ){
				//pilot other ships
				gravitate( world_data, ships[s]->body, grav_max * delta_time );
			}

			cpSpaceStep( space, delta_time );
		}

		for (int s = 0; s < ships_N; ++s ){
			world_bounding( world_data, ships[s]->body );
		}
		

		prev_pilot_vec = pilot_vec;
 
		cpVect p1pos = cpBodyGetPosition( ships[0]->body );
		//SDL_Log("p1pos: %lg, %lg\n", xy(p1pos) );

		if( !inside( world_data, p1pos ) ){
			SDL_strlcpy( GS->COMING_FROM, GS->GOING_TO, 64 );
			SDL_strlcpy( GS->GOING_TO, "SPACE", 64 );
			GS->going_to_mode = 'T';
			goto end;
		}
		
		update_camera( &T, p1pos, &world_angle, GS->window_rct, bounds_rct );
		Mat23 WT = T.M;

		SDL_SetRenderDrawColor( R, 2, 2, 2, 255 );
		SDL_RenderClear( R );

		if( debug_view ){
			SDL_SetRenderDrawColor( R, 128, 0, 128, 255 );
			stroke_cpSpace( R, space, &T );
		}
		// -- Render Map -- 
		render_world( R, world_data, map_visuals, &T, vbuf );

		// -- Render ships, emit FX -- 
		for (int s = 0; s < ships_N; ++s ){
			//SDL_Log( "s: %d", s );

			cpVect spos = cpBodyGetPosition( ships[s]->body );
			double sheading = cpBodyGetAngle( ships[s]->body );
			vec2d trig = camera_matrix( &T, world_angle, sheading, xy(spos) );
			draw_Styled_TGeo_vec( R, ships[s]->data->visual, &T, vbuf );
			/*
				vec2d trig = v2d_trig( cpBodyGetAngle( ships[s]->body ) );
				cpVect spos = cpBodyGetPosition( ships[s]->body );
				T.tx = p1pos.x - spos.x;
				T.ty = p1pos.y - spos.y;
				draw_Styled_RTGeo_vec( R, ships[s]->data->visual, trig, &T, vbuf );
				*/

			if( ships[s]->thrusting ){

				/* Draw exhaust visuals */
				draw_Geo_Animation( R, &(ships[s]->data->exhaust), 
				                       &(ships[s]->exh_frame), 
				                       &(ships[s]->exh_timer),
                         		       &T, vbuf );
				/* Create smoke */
				OBJ *slot = fresh_OBJ_slot( &PUFFOBJS );
				cpVect svel = cpvmult( cpBodyGetVelocity( ships[s]->body ), 0.5 );
				cpVect exhvel = cpv_rottrig( cpv( 0, puffscale * 32 ), trig );//v2d_rotate( cpv( 0, 32 ), sheading ); //cpBodyLocalToWorld( ships[s]->body, cpv( 0, 32 ) );
				cpVect vel = cpvadd( svel, exhvel );
				cpVect pos = cpBodyLocalToWorld( ships[s]->body, ships[s]->data->smoke_outlet );
				create_smokepuff( space, slot, puffscale, pos, vel, &(GS->lib) );

				ships[s]->thrusting = false;
			}
		}
		T.M = WT;// reset for next step

		// -- Smoke FX -- 
		SDL_SetRenderTarget( R, puff_mask );

		OBJ_Page *OP = &PUFFOBJS;
		do{
			int i = OP->oldest;
			for (int c = 0; c < OBJ_PAGE_SIZE; ++c ){

				if( OP->objs[i].body != NULL ){
					cpVect obpos = cpBodyGetPosition( OP->objs[i].body );
					int *age = (int*)(OP->objs[i].data);
					SDL_SetRenderDrawColor( R, 255, 255, 255, constrain(map(*age, 0, 35, 0, 255), 0, 255) );
					TM_APPLY_TO( obpos, obpos, T.M );
					gp_fill_8circle( R, obpos.x, obpos.y, T.s * 1.8 * puffscale );

					if( OP->objs[i].tick( OP->objs + i ) ){
						OBJ_expired( OP, i );
					}
				}

				if( i == OP->index ) break;
				i = cycle( i + 1, 0, OBJ_PAGE_SIZE-1 );
			}
			OP = OP->next;
		} while( OP != NULL );

		RenderCopyMasked( R, smokey_texture, puff_mask, smoke_target );
		fade_Texture( R, puff_mask, 225 );

		

		SDL_SetRenderDrawColor( R, 100, 255, 240, 255 );
		SDL_framerate_limit_n_monitor( R, 17 );
		char buf [256];
		SDL_snprintf( buf, sizeof(buf), "%lg, %lg", xy(p1pos) );
	    SDL_RenderDebugText(R, 20, 40, buf);

		SDL_RenderPresent(R);
		
	}//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	end:
	cpSpaceFree(space);

	destroy_world( world_data );

	int vss = vec_size( visual_styles );
	for (int i = 0; i < vss; ++i ){
		SDL_free( visual_styles[i] );
	}
	vec_delete( visual_styles );
	
	SDL_free( vbuf );
}









void among_the_stars( SDL_Renderer *R, GameState *GS, char *starspath ){

	Transform T = { 0, 0, GS->cx, GS->cy, 1, 1, {0} };
	set_scale( &T, 0.5 );
	int scaleI = logarithm( 1.1, T.s );
	double dscaleI = scaleI;
	float min_scaleI = 0;
	update_TM( &T, 0, 0, 0, 0 );

	//SDL_Log( "going among \"%s\"", starspath );
	SDL_IOStream *f = SDL_IOFromFile( starspath, "r" );

	SVG_Layer *CL = svg_load_layer( f, "Cosmos" );
	//SVG_Layer_dump( CL );

	int celestials_length = vec_size( CL->E );
	SDL_Log("loaded layer Cosmos with %d elements", celestials_length);
	Celestial *celestials = SDL_calloc( celestials_length, sizeof(Celestial) );

	int *gravitators = NULL;

	vec2d spawn = v2dzero;

	for (int e = 0; e < vec_size( CL->E ); ++e ){
		SDL_strlcpy( celestials[e].name, CL->E[e].id, 64 );
		celestials[e].visual = SVG_Element_to_Geo_Animation( CL, CL->E + e );		
		if( celestials[e].visual.cells[0].geo.type == geo_PATH ){
			celestials[e].collider = circumscribe_Path( &(celestials[e].visual.cells[0].geo.u.path) );
			celestials[e].collider.radius *= celestials[e].collider.radius; //we only need the square of the radius;
		}
		else SDL_Log( "celestials[e].visual.cells[0].geo != path..... pls make it path" );

		int ms = vec_size( CL->E[e].metadata );
		for (int m = 0; m < ms; ++m ){
			int t = CL->E[e].metadata[m].tag_index;
			if( SDL_strcmp( CL->tags[t], "gravity" ) == 0 ){
				vec_push( gravitators, e );
				celestials[e].gravity = SDL_atof( CL->E[e].metadata[m].data );
			}
			/*else if( SDL_strcmp( CL->tags[t], "spawn" ) == 0 ){
				spawn = celestials[e].collider.pos;
			}*/
			else if( SDL_strcmp( CL->tags[t], "portal" ) == 0 ){
				celestials[e].collision_mode = 'P';
			}
			else if( SDL_strcmp( CL->tags[t], "kill" ) == 0 ){
				celestials[e].collision_mode = 'K';
			}
		}

		if( SDL_strcmp( celestials[e].name, GS->COMING_FROM ) == 0 ){
			spawn = celestials[e].collider.pos;
			if( celestials[e].collision_mode == 'P' ){
				float theta = random_angle();
				GS->hero_ship->heading = theta + HALF_PI;
				v2d_add( &spawn, v2d_from_polar( 1.2 * SDL_sqrtf(celestials[e].collider.radius), theta ) );
			}
		}

		//SDL_Log( "celestials[e].visual.cells[0].style: %d.%d.%d.%d ", RGBA(celestials[e].visual.cells[0].style->stroke_color) );
	}
	//hand off styles
	Style **celestial_styles = CL->styles;
	CL->styles = NULL;
	SVG_Layer_destroy( CL );

	SVG_Layer *ZL = svg_load_layer( f, "Zones" );
	
	for (int z = 0; z < vec_size(ZL->E); ++z ){
		if( SDL_strcmp( ZL->E[z].id, "bounds" ) == 0 ){
			if( ZL->E[z].u.geo.type == geo_BOX ){
				//SDL_Log("bounding!");
				SDL_FRect bounds = ZL->E[z].u.geo.u.box;
				fit_frect( &bounds, &(GS->window_rct) );
				set_scale( &T, bounds.w / ZL->E[z].u.geo.u.box.w );
				//SDL_Log("T.s: %g\n", T.s );
				scaleI = logarithm( 1.1, T.s );
				T.cx = bounds.x - T.s * ZL->E[z].u.geo.u.box.x;
				T.cy = bounds.y - T.s * ZL->E[z].u.geo.u.box.y;
				//SDL_Log( "cx: %g, cy: %g", T.cx, T.cy );
				update_TM( &T, 0, 0, 0, 0 );
			}
		}
	}

	SDL_CloseIO( f );



	int ships_N = 1;
	Ship_inst **ships = SDL_malloc( ships_N * sizeof(Ship_inst*) );

	ships[0] = GS->hero_ship;
	ships[0]->pos = spawn;
	ships[0]->vel = v2dzero;


	//longest_path
	SDL_FPoint *vbuf = SDL_malloc( 128 * sizeof(SDL_FPoint) );

	SDL_Log("I am among...");
	/* |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| */
	/*   |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o|   */
	/* |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| |o| */
	while(1){

		SDL_Event event;
		while( SDL_PollEvent(&event) ){

			//UI_event_handling_function( &main_menu, &event );

			int captured = Dir_HandleEvent( &event, &(GS->flightstick), GS->cx, GS->cy );

			if( !captured ){
				switch (event.type) {
					case SDL_EVENT_QUIT:
						SDL_strlcpy( GS->GOING_TO, "QUIT", 64 );
						GS->going_to_mode = 'Q';
						goto end;

					case SDL_EVENT_MOUSE_MOTION:
						break;
				}
			}
		}

		vec2d pilot_vec = DirInput_compute( &(GS->flightstick) );
		ships[0]->heading += pilot_vec.x * 0.1;
		if( SDL_fabsf( pilot_vec.y ) > 0.02 ){
			vec2d thrust = v2d( 0, pilot_vec.y * 0.0625 );
			v2d_rotate( &thrust, ships[0]->heading );
			v2d_add( &(ships[0]->vel), thrust );
			ships[0]->thrusting = true;
		} else {
			ships[0]->thrusting = false;
		}
		for (int g = 0; g < vec_size( gravitators ); ++g ){
			Celestial *CG = celestials + gravitators[g];
			vec2d diff = v2d_diff( CG->collider.pos, ships[0]->pos );
			vec2d grav = v2d_setlen( diff, CG->gravity / v2d_magsq(diff) );
			v2d_add( &(ships[0]->vel), grav );
		}
		v2d_add( &(ships[0]->pos), ships[0]->vel );

		for (int i = 0; i < celestials_length; ++i ){
			if( v2d_distsq( ships[0]->pos, celestials[i].collider.pos ) < celestials[i].collider.radius ){
				switch( celestials[i].collision_mode ){
					case 'P':
						SDL_strlcpy( GS->GOING_TO, celestials[i].name, 64 );
						GS->going_to_mode = 'P';
						goto end;

					case 'K':
						// trigger particles, check lives, trigger respawan timer or go to GAMEOVER
						break;
				}
			}
		}


		SDL_SetRenderDrawColor( R, 2, 2, 2, 255 );
		SDL_RenderClear( R );

		//SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
		//gp_crosshair( R, atfX(0, T), atfY(0, T), 20 );

		update_TM( &T, 0, 0, 0, 0 );

		for (int i = 0; i < celestials_length; ++i ){
			draw_Geo_Animation( R, &(celestials[i].visual), 
				                   &(celestials[i].frame), 
				                   &(celestials[i].timer),
                         		   &T, vbuf );
		}


		// -- Render ships, emit FX -- 
		for (int s = 0; s < ships_N; ++s ){
			//SDL_Log( "s: %d", s );
			vec2d trig = update_TM_object_rotated( &T, 0, ships[s]->heading, xy(ships[s]->pos) );
			draw_Styled_TGeo_vec( R, ships[s]->data->visual, &T, vbuf );

			if( ships[s]->thrusting ){
				draw_Geo_Animation( R, &(ships[s]->data->exhaust), 
				                       &(ships[s]->exh_frame), 
				                       &(ships[s]->exh_timer),
                         		       &T, vbuf );
			}
		}


		SDL_SetRenderDrawColor( R, 100, 255, 240, 255 );
		SDL_framerate_limit_n_monitor( R, 17 );

		SDL_RenderPresent(R);
		
	}//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	end:

	vec_delete( gravitators );

	for (int i = 0; i < celestials_length; ++i ){
		free_Geo_Animation( &(celestials[i].visual) );
	}
	SDL_free( celestials );

	int css = vec_size( celestial_styles );
	for (int i = 0; i < css; ++i ){
		SDL_free( celestial_styles[i] );
	}
	vec_delete( celestial_styles );

	SDL_free( vbuf );
}