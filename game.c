#include "game.h"
#include "primitives.h"
#include "cvec.h"
#include "ok_lib.h"

void load_doodads( char *filename, Library *lib ){

	SDL_Log("loading doodads from \"%s\"", filename );

	SDL_IOStream *f = SDL_IOFromFile( filename, "r" );
	if( f == NULL ){
		SDL_Log( "failed to open \"%s\"", filename );
		return;
	} //else SDL_Log( "file opened successfully" );
	SVG_Layer *L = svg_load_layer( f, "Layer 1" );
	if( L == NULL ){
		SDL_Log( "failed to load \"Layer 1\"" );
		return;
	}

	SDL_memset( lib, 0, sizeof(Library) );

	struct {
		char *name;
		int *Es;
		char **prop;
		int *index;
		int N;
	} *doods;
	int doods_N = 0;
	doods = NULL;

	Hashmap dood_map;
	ok_map_init( &dood_map );

	int reserved_ids_N = 1;
	char reserved_ids [][32] = { "smokepuffs" };
	int reserved_dtype [] =    {  PARTICULARS+1 };
	Hashmap reserved_id_map;
	ok_map_init_with_capacity( &reserved_id_map, reserved_ids_N );
	for (int r = 0; r < reserved_ids_N; ++r ){
		ok_map_put( &reserved_id_map, reserved_ids[r], r+1 );
	}

	//SDL_Log( "now let's find and associate...");	

	// find and associate sets of svg elements together into "doods"
	for (int e = 0; e < vec_size(L->E); ++e ){
		//SDL_Log( "looking at \"%s\"", L->E[e].id );
		char* colon = SDL_strchr( L->E[e].id, ':' );
		if (colon != NULL) {
			*colon = '\0';
			int D = ok_map_get( &dood_map, L->E[e].id ) -1;
			if( D < 0 ){
				D = doods_N;
				doods_N += 1;
				doods = SDL_realloc( doods, doods_N * sizeof(*doods) );
				SDL_memset( doods+D, 0, sizeof(*doods) );
				doods[D].name = SDL_strdup( L->E[e].id );
				ok_map_put( &dood_map, doods[D].name, D+1 );
			}
			int p = doods[D].N;
			doods[D].N += 1;

			doods[D].Es = SDL_realloc( doods[D].Es, doods[D].N * sizeof(int) );
			doods[D].Es[p] = e;

			doods[D].prop = SDL_realloc( doods[D].prop, doods[D].N * sizeof(char*) );
			doods[D].prop[p] = SDL_strdup( colon+1 );

			char* dash = SDL_strchr( colon+1, '-' );
			if( dash != NULL ){
				*dash = '\0';
				doods[D].index = SDL_realloc( doods[D].index, doods[D].N * sizeof(int) );
				doods[D].index[p] = SDL_strtol( dash+1, NULL, 10 );
			}
		}
		else{
			char* dash = SDL_strchr( L->E[e].id, '-' );
			if( dash != NULL ){
				*dash = '\0';
			}
			int RN = ok_map_get( &reserved_id_map, L->E[e].id )-1;

			if( RN >= 0 ){
				int D = ok_map_get( &dood_map, reserved_ids[RN] ) -1;
				if( D < 0 ){
					D = doods_N;
					doods_N += 1;
					doods = SDL_realloc( doods, doods_N * sizeof(*doods) );
					SDL_memset( doods+D, 0, sizeof(*doods) );
					doods[D].name = SDL_strdup( reserved_ids[RN] );
					ok_map_put( &dood_map, doods[D].name, D+1 );
				}
				int p = doods[D].N;
				doods[D].N += 1;
				doods[D].Es = SDL_realloc( doods[D].Es, doods[D].N * sizeof(int) );
				doods[D].Es[p] = e;
				doods[D].prop = SDL_realloc( doods[D].prop, doods[D].N * sizeof(char*) );
				doods[D].prop[p] = SDL_strdup( "none" );
				if( dash != NULL ){
					doods[D].index = SDL_realloc( doods[D].index, doods[D].N * sizeof(int) );
					doods[D].index[p] = SDL_strtol( dash+1, NULL, 10 );
				} else {
					doods[D].index = SDL_realloc( doods[D].index, doods[D].N * sizeof(int) );
					doods[D].index[p] = D;
				}
			}
			else{
				//SDL_Log( "unreccd doodad: {%s}", L->E[e].id );
			}
		}
	}

	SDL_Log( "found and associated %d elements into %d doods.", vec_size(L->E), doods_N );

	vec_init( lib->doodads, doods_N );
	int D = 0;

	Hashmap class_map;
	ok_map_init_with_capacity( &class_map, 3 );
	ok_map_put( &class_map, "ship", SHIP+1 );
	ok_map_put( &class_map, "bomb", BOMB+1 );
	ok_map_put( &class_map, "item", ITEM+1 );

	//                              0         1        2        3        4
	char prop_names [][32] = { "physical", "visual", "exh", "center", "smoke" };
	int prop_names_N = 5;

	int *prop_name_len = SDL_malloc( prop_names_N * sizeof(int) );
	for (int p = 0; p < prop_names_N; ++p ){
		prop_name_len[p] = SDL_strlen( prop_names[p] );
	}

	int **prop_de = SDL_calloc( prop_names_N, sizeof(int*) );

	char ship_attribs [][32] = { "thrust", "turn_speed", "hull_max", "fuel_max", "fuel_consumption" };
	Hashmap ship_attrib_map;
	ok_map_init_with_capacity( &ship_attrib_map, 5 );
	for (int a = 0; a < 5; ++a ){
		ok_map_put( &ship_attrib_map, ship_attribs[a], a+1 );
	}

	// now that we know what things are, extract the data.
	for (int d = 0; d < doods_N; ++d ){
		SDL_Log( "extracing dood %d", d );

		for (int p = 0; p < prop_names_N; ++p ){
			if( vec_size(prop_de[p]) > 0 ) vec_shrinkto(prop_de[p],0);
		}

		for (int de = 0; de < doods[d].N; ++de ){ // for each of this dood's associated svg elements
			int ed = doods[d].Es[de];

			for (int p = 0; p < prop_names_N; ++p ){
				
				if( SDL_strncmp( doods[d].prop[de], prop_names[p], prop_name_len[p] ) == 0 ){

					vec_push( prop_de[p], de );
					
					for (int m = 0; m < vec_size(L->E[ed].metadata); ++m ){
						int t = L->E[ed].metadata[m].tag_index;
						if( SDL_strncmp( L->tags[t], "class", 5 ) == 0 ){

							int C = ok_map_get( &class_map, L->E[ed].metadata[m].data )-1;

							if( C >= 0 ){
								lib->doodads[D].type = C;
							}							
							else{
								SDL_Log( "class: {%s}....", L->E[ed].metadata[m].data ); 
							}
							break;
						}
					}
					break;
				}
			}
		}
		if( lib->doodads[D].type == EMPTY ){

			//int reserved_ids_N = ok_map_count(&reserved_ids);
			int RN = ok_map_get( &reserved_id_map, doods[d].name )-1;

			if( RN >= 0 ){
				lib->doodads[D].type = reserved_dtype[ RN ];
			}
		}
		if( lib->doodads[D].type == EMPTY ){
			SDL_Log( "can't identify the class of doodad {%s}", doods[d].name );
			continue;
		}

		switch( lib->doodads[D].type ){
			case SHIP:

				SDL_strlcpy( lib->doodads[D].u.ship.name, doods[d].name, 64 );

				// CENTER : PROP 3
				vec2d offset = v2dzero;
				if( vec_size( prop_de[3] ) >= 1 ){
					int ed = doods[d].Es[ prop_de[3][0] ];
					offset =  v2d_neg( L->E[ed].u.geo.u.circle.pos );
				}
				// PHYSICAL : PROP 0
				//lib->doodads[D].u.ship.physical_N = vec_size( prop_de[0] );
				int N = vec_size( prop_de[0] );
				vec_init( lib->doodads[D].u.ship.physical, N );
				vec_init( lib->doodads[D].u.ship.properties, N );
				for (int p = 0; p < N; ++p ){
					int de = prop_de[0][p];
					int ed = doods[d].Es[de];
					lib->doodads[D].u.ship.physical[p] = L->E[ed].u.geo;
					if( L->E[ed].u.geo.type == geo_PATH ) L->E[ed].u.geo.u.path.N = 0;// to prevent verts getting freed later
					geo_offset( lib->doodads[D].u.ship.physical + p, offset );

					cpProperties pr = retrieve_cpProperties_from_SVG_metadata( L->E + ed, L->tags, NULL, NULL );
					lib->doodads[D].u.ship.properties[p] = pr;
				}
				// VISUAL : PROP 1
				N = vec_size( prop_de[1] );
				vec_init( lib->doodads[D].u.ship.visual, vec_size( prop_de[1] ) );// = SDL_calloc( lib->doodads[D].u.ship.visual_N, sizeof(Styled_Geo) );
				for (int v = 0; v < N; ++v ){
					int de = prop_de[1][v];
					int ed = doods[d].Es[de];
					lib->doodads[D].u.ship.visual[v].geo = L->E[ed].u.geo;
					if( L->E[ed].u.geo.type == geo_PATH ){
						if( L->E[ed].u.geo.u.path.N > lib->longest_path ) lib->longest_path = L->E[ed].u.geo.u.path.N;
						L->E[ed].u.geo.u.path.N = 0;// to prevent verts getting freed later
					}
					geo_offset( &(lib->doodads[D].u.ship.visual[v].geo), offset );
					lib->doodads[D].u.ship.visual[v].style = *(L->E[ed].style);
				}
				// EXH : PROP 2
				N = vec_size( prop_de[2] );
				vec_init( lib->doodads[D].u.ship.exhaust, vec_size( prop_de[2] ) );// = SDL_calloc( lib->doodads[D].u.ship.exhaust_N, sizeof(Styled_Geo) );
				for (int x = 0; x < N; ++x ){
					int de = prop_de[2][x];
					int ed = doods[d].Es[de];
					lib->doodads[D].u.ship.exhaust[x].geo = L->E[ed].u.geo;
					if( L->E[ed].u.geo.type == geo_PATH ){
						if( L->E[ed].u.geo.u.path.N > lib->longest_path ) lib->longest_path = L->E[ed].u.geo.u.path.N;
						L->E[ed].u.geo.u.path.N = 0;// to prevent verts getting freed later
					}
					geo_offset( &(lib->doodads[D].u.ship.exhaust[x].geo), offset );
					lib->doodads[D].u.ship.exhaust[x].style = *(L->E[ed].style);
				}
				// SMOKE : PROP 4
				if( vec_size( prop_de[4] ) >= 1 ){
					int ed = doods[d].Es[ prop_de[4][0] ];
					lib->doodads[D].u.ship.smoke_outlet = v2d_to_cpv(v2d_sum( L->E[ed].u.geo.u.circle.pos, offset ));
					//lib->doodads[D].u.ship.smoke_outlet = cpvneg( lib->doodads[D].u.ship.smoke_outlet );
					//SDL_Log( "lib->doodads[D].u.ship.smoke_outlet: %lg, %lg", lib->doodads[D].u.ship.smoke_outlet.x, lib->doodads[D].u.ship.smoke_outlet.y );
				}

				for (int de = 0; de < doods[d].N; ++de ){ // for each of this dood's associated svg elements
					int ed = doods[d].Es[de];
					for (int m = 0; m < vec_size(L->E[ed].metadata); ++m ){
						int t = L->E[ed].metadata[m].tag_index;
						int A = ok_map_get( &ship_attrib_map, L->tags[t] )-1;
						switch( A ){
							case 0: // "thrust"
								lib->doodads[D].u.ship.thrust = SDL_atof( L->E[ed].metadata[m].data );
								break;
							case 1:// "turn_speed"
								lib->doodads[D].u.ship.turn_speed = SDL_atof( L->E[ed].metadata[m].data );
								break;
							case 2:// "hull_max"
								lib->doodads[D].u.ship.hull_max = SDL_atof( L->E[ed].metadata[m].data );
								break;
							case 3:// "fuel_max"
								lib->doodads[D].u.ship.fuel_max = SDL_atof( L->E[ed].metadata[m].data );
								break;
							case 4:// "fuel_consumption"
								lib->doodads[D].u.ship.fuel_consumption = SDL_atof( L->E[ed].metadata[m].data );
								break;
						}
					}
				}

				break;

			/*case SMOKE:

				vec_init( lib->doodads[D].u.visuals, doods[d].N );
				for (int e = 0; e < doods[d].N; ++e ){
					int ed = doods[d].Es[e];
					vec_init( lib->doodads[D].u.visuals[e], 1 );
					lib->doodads[D].u.visuals[e][0].geo = L->E[ed].u.geo;
					if( L->E[ed].u.geo.type == geo_PATH ){
						if( L->E[ed].u.geo.u.path.N > lib->longest_path ) lib->longest_path = L->E[ed].u.geo.u.path.N;
						L->E[ed].u.geo.u.path.N = 0;// to prevent verts getting freed later
					}
					lib->doodads[D].u.visuals[e][0].style = *(L->E[ed].style);
					geo_centralize( &(lib->doodads[D].u.visuals[e][0].geo) );
				}
				lib->smokepuffs = D;

				break;*/

			case BOMB:

				break;

			case ITEM:

				break;

			case PARTICULARS:;
				/*
				int P = lib->doodads[D].type - PARTICULARS;
				switch( P ){

					case 1: //smokepuffs

						for (int de = 0; de < doods[d].N; ++de ){ // for each of this dood's associated svg elements
							int ed = doods[d].Es[de];
							for (int m = 0; m < L->E[ed].metadata_count; ++m ){
								int t = L->E[ed].metadata[m].tag_index;
								if( SDL_strcmp( L->tags[t], "path", 5 ) == 0 ){
									lib->puffs = IMG_LoadTexture( R, L->E[ed].metadata[m].data );
								}
								else if( SDL_strcmp( L->tags[t], "dimensions" ) == 0 ){
									if (SDL_sscanf(str, "%d,%d,%d,%d", &(lib->puff_dims.x), &(lib->puff_dims.y), 
										                               &(lib->puff_dims.w), &(lib->puff_dims.h)) != 4) {
										SDL_Log( "hmm... uhhh" );
									}
								}
							}
						}

						break;
				}
				*/

				D--;// don't advance the doodads
				break;
		}

		D++;
	}
	SDL_Log("Done!");

	vec_shrinkto( lib->doodads, D );
	/*
	if( D < *doodad_N ){
		*doodad_N = D;
		lib->doodads = SDL_realloc( lib->doodads, D * sizeof(Doodad) );
	}*/

	for (int p = 0; p < prop_names_N; ++p ){
		vec_free( prop_de[p] );
	}
	SDL_free( prop_de );
	SDL_free( prop_name_len );
	ok_map_deinit( &dood_map );

	for (int i = 0; i < doods_N; ++i) {
		SDL_free(doods[i].name);
		SDL_free(doods[i].Es);
		for (int j = 0; j < doods[i].N; ++j) {
			SDL_free(doods[i].prop[j]);
		}
		SDL_free(doods[i].prop);
		SDL_free(doods[i].index);
	}
	SDL_free(doods);

	SDL_CloseIO( f );
	SVG_Layer_destroy( L );
}


void log_ship_data(const Ship_data* ship) {
	if (!ship) {
		SDL_Log("Ship_data: NULL");
		return;
	}
	
	SDL_Log("=== Ship_data ===");
	SDL_Log("name: %s", ship->name);
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
	log_styled_geo_array(ship->exhaust, vec_size( ship->exhaust ), "exhaust");
}


Ship_inst *instantiate_ship( Ship_data *data, cpSpace *space, cpVect pos ){

	Ship_inst *ship = SDL_calloc( 1, sizeof(Ship_inst) );
	ship->data = data;
	ship->body = cpSpaceAddBody( space, cpBodyNew(0, 0) );
	cpBodySetPosition( ship->body, pos );

	int pN = vec_size( data->physical );
	for (int p = 0; p < pN; ++p ){

		cpShape *shape = Geometric_to_cpShape( data->physical + p, ship->body, 0.0 );
		cpSpaceAddShape( space, shape );
		cpShapeSetDensity( shape, data->properties[p].density );
		cpShapeSetFriction( shape, data->properties[p].friction );
		cpShapeSetElasticity( shape, data->properties[p].elasticity );
		cpShapeFilter filter = cpShapeFilterNew( 0, de_mask( "v" ), de_mask( "vs" ) );
		cpShapeSetFilter( shape, filter );
	}
	
	ship->fuel = data->fuel_max;

	SDL_Log( "ship mass: %g", cpBodyGetMass( ship->body ) );
	cpVect cog = cpBodyGetCenterOfGravity( ship->body );
	SDL_Log( "cog: %lg, %lg", cog.x, cog.y );

	return ship;
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



void init_flat_world( void **W, SDL_FRect bounds, Styled_Geo *map_visuals, int width ){

	*W = SDL_malloc( sizeof(flat_world) );
	flat_world *fw = (flat_world*)(*W);

	fw->chunks_N = 6;
	fw->chunks = SDL_calloc( fw->chunks_N, sizeof(int*) );
	fw->bounds = bounds;
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

	fw->gravity_falloff = (SDL_FRect){-999999};
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

void flat_world_set_gravity_falloff( void *W, SDL_FRect rct ){
	flat_world *fw = (flat_world*) W;
	fw->gravity_falloff = rct;
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

void flat_world_update_camera( Transform *T, cpVect target ){
	T->tx = target.x;
	T->ty = target.y;
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
			if( fw->deja_rendu[me] != uni && sg->style.stroke ){
				SDL_SetRenderDraw_SDL_Color( R, sg->style.stroke_color );
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
	//cpBodyApplyForceAtWorldPoint( body, cpv( 0, g ), p );
	cpBodyAddVelocity( body, cpv( 0, g ) );
}




void create_smokepuff( cpSpace *space, OBJ *O, cpVect pos, cpVect vel, Library *lib ){

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
	cpVect points [3] = { cpv( 1.5, 0), cpv( -0.75, 2.25 / SQRT3 ), cpv( -0.75, -2.25 / SQRT3 ) };
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
	set_scale( &T, 2 );
	int scaleI = logarithm( 1.1, T.s );
	double dscaleI = scaleI;
	float min_scaleI = 0;

	void *world_data = NULL;
	render_world_func render_world = NULL;
	world_bounding_func world_bounding = NULL;
	gravitate_func gravitate = NULL;
	double world_angle = 0;
	update_camera_func update_camera = NULL;
	m_update_func camera_matrix = NULL;
	voidptr_func destroy_world = NULL;

	//SDL_Log( "loading Physical layer from file \"%s\".", spherepath );
	SDL_IOStream *f = SDL_IOFromFile( spherepath, "r" );

	SVG_Layer *PL = svg_load_layer( f, "Physical" );
	//SDL_Log( "converting layer to space...");
	SVG_layer_into_cpSpace( PL, space, false );
	SVG_Layer_destroy( PL );

	//SDL_Log( "done.\n now loading Visual layer" );
	SVG_Layer *VL = svg_load_layer( f, "Visual" );
	Styled_Geo *map_visuals = NULL;
	SVG_Layer_to_Styled_Geo_vec( VL, &map_visuals );
	SVG_Layer_destroy( VL );

	//SDL_Log( "done.\n now loading Zones layer" );
	SVG_Layer *ZL = svg_load_layer( f, "Zones" );
	
	for (int z = 0; z < vec_size(ZL->E); ++z ){
		if( SDL_strcmp( ZL->E[z].id, "bounds" ) == 0 ){
			//SDL_Log("bounds!");
			if( ZL->E[z].u.geo.type == geo_BOX ){
				init_flat_world( &world_data, ZL->E[z].u.geo.u.box, map_visuals, GS->width );
				render_world = render_flat_world;
				world_bounding = flat_world_bounding;
				gravitate = flat_world_gravitate;
				update_camera = flat_world_update_camera;
				camera_matrix = update_TM_object_rotated;
				destroy_world = destroy_flat_world;
				min_scaleI = logarithm( 1.1, GS->width / ZL->E[z].u.geo.u.box.w );
			}
			else if( ZL->E[z].u.geo.type == geo_CIRCLE ){
				camera_matrix = update_TM_combined;
				min_scaleI = logarithm( 1.1, GS->width / 2.4 * ZL->E[z].u.geo.u.circle.radius );
			}
		}
		else if( SDL_strcmp( ZL->E[z].id, "gravity_falloff" ) == 0 ){
			if( world_data == NULL ) SDL_Log( "gravity_falloff before bounds.....(that's bad!)" );
			flat_world_set_gravity_falloff( world_data, ZL->E[z].u.geo.u.box );
			//SDL_Log("gravity_falloff!");
		}
	}
	SVG_Layer_destroy( ZL );

	//cpSpaceSetGravity( space, cpv(0, 10) );
	double grav_max = 10;

	//SDL_Log( "done.\n now loading Holo layer" );
	SVG_Layer *HL = svg_load_layer( f, "Holo" );

	SDL_CloseIO( f );

	int ships_N = 1;
	Ship_inst **ships = SDL_malloc( ships_N * sizeof(Ship_inst*) );

	ships[0] = instantiate_ship( GS->landing_modules[ GS->active_module ], space, cpv(480,-100) );

	//SDL_Log( "puffs: %p, puff_dims.h: %d", GS->lib.puffs, GS->lib.puff_dims.h );

	SDL_Texture *smokey_texture = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                     SDL_TEXTUREACCESS_TARGET, GS->width, GS->height );

    SDL_Texture *puff_mask = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                SDL_TEXTUREACCESS_TARGET, GS->width, GS->height );

    SDL_Texture *smoke_target = SDL_CreateTexture( R, SDL_PIXELFORMAT_RGBA8888,
                                                   SDL_TEXTUREACCESS_TARGET, GS->width, GS->height );

    SDL_SetRenderTarget( R, smokey_texture );
	SDL_SetRenderDrawColor( R, 0, 0, 0, 0 );
	SDL_RenderClear( R );
	SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
	for (int y = 0; y < GS->height; y += 8 ){
		SDL_RenderLine( R, 0, y, GS->width, y );
		//gp_draw_24circle( R, GS->cx, GS->cy, y );
	}
	SDL_SetRenderTarget( R, NULL );

	OBJ_Page PUFFOBJS;
	init_OBJ_Page( &PUFFOBJS );


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
	SDL_Log( "sim_iterations: %d, delta: %lg", sim_iterations, delta_time );

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
						goto end;

					case SDL_EVENT_MOUSE_WHEEL:;

						scaleI -= event.wheel.y;
						if( scaleI < min_scaleI ) scaleI = SDL_ceilf( min_scaleI );
						set_scale( &T, SDL_pow(1.1, scaleI) );

						break;

					case SDL_EVENT_KEY_UP:
						if( event.key.key == SDLK_F9 ){
							debug_view = !debug_view;
							SDL_Log( "T = { %lg, %lg,  %g, %g,  %g }", T.tx, T.ty, T.cx, T.cy, T.s );
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
		
		update_camera( &T, p1pos );

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
				int f = ships[s]->exh_frame / exh_frame_cycle;
				draw_Styled_TGeo( R, ships[s]->data->exhaust + f, &T, vbuf );
				ships[s]->exh_frame += 1;
				if( ships[s]->exh_frame >= vec_size(ships[s]->data->exhaust) * exh_frame_cycle ){
					ships[s]->exh_frame = 0;
				}
				/* Create smoke */
				OBJ *slot = fresh_OBJ_slot( &PUFFOBJS );
				cpVect svel = cpvmult( cpBodyGetVelocity( ships[s]->body ), 0.5 );
				cpVect exhvel = cpv_rottrig( cpv( 0, 32 ), trig );//v2d_rotate( cpv( 0, 32 ), sheading ); //cpBodyLocalToWorld( ships[s]->body, cpv( 0, 32 ) );
				cpVect vel = cpvadd( svel, exhvel );
				cpVect pos = cpBodyLocalToWorld( ships[s]->body, ships[s]->data->smoke_outlet );
				create_smokepuff( space, slot, pos, vel, &(GS->lib) );

				ships[s]->thrusting = false;
			}
		}
		update_camera( &T, p1pos );// reset for next step

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
					gp_fill_8circle( R, obpos.x, obpos.y, T.s * 1.8 );

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

		SDL_RenderPresent(R);
		
	}//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	end:
	cpSpaceFree(space);

	destroy_world( world_data );
	
	SDL_free( vbuf );
}



















void among_the_stars( SDL_Renderer *R, GameState *GS, char *starspath ){

	Transform T = { 0, 0, GS->cx, GS->cy, 1, 1, {0} };
	set_scale( &T, 0.5 );
	int scaleI = logarithm( 1.1, T.s );
	double dscaleI = scaleI;
	float min_scaleI = 0;
	update_TM( &T, 0, 0, 0, 0 );

	SDL_Log( "going among \"%s\"", starspath );
	SDL_IOStream *f = SDL_IOFromFile( starspath, "r" );

	SVG_Layer *CL = svg_load_layer( f, "Cosmos" );
	//SVG_Layer_dump( CL );

	// cosmic or celestial objects
	int Cobjs_length = vec_size( CL->E );
	SDL_Log("loaded layer Cosmos with %d elements", Cobjs_length);
	Geo_Animation *Cobjs = SDL_calloc( Cobjs_length, sizeof(Geo_Animation) );

	for (int e = 0; e < vec_size( CL->E ); ++e ){
		Cobjs[e] = SVG_Element_to_Geo_Animation( CL, CL->E + e );
	}
	SVG_Layer_destroy( CL );

	SVG_Layer *ZL = svg_load_layer( f, "Zones" );
	
	for (int z = 0; z < vec_size(ZL->E); ++z ){
		if( SDL_strcmp( ZL->E[z].id, "bounds" ) == 0 ){
			if( ZL->E[z].u.geo.type == geo_BOX ){
				//SDL_Log("bounding!");
				SDL_FRect windowfrect = {0,0,GS->width,GS->height};
				SDL_FRect bounds = ZL->E[z].u.geo.u.box;
				fit_frect( &bounds, &windowfrect );
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

	//longest_path
	SDL_FPoint *vbuf = SDL_malloc( 128 * sizeof(SDL_FPoint) );

	int anim_tick_length = 20; //in frames
	int anim_tick = 0;

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
						goto end;

					case SDL_EVENT_MOUSE_MOTION:
						break;
				}
			}
		}

		SDL_SetRenderDrawColor( R, 2, 2, 2, 255 );
		SDL_RenderClear( R );

		//SDL_SetRenderDrawColor( R, 255, 255, 255, 255 );
		//gp_crosshair( R, atfX(0, T), atfY(0, T), 20 );

		for (int o = 0; o < Cobjs_length; ++o ){
			int f = 2 + Cobjs[o].current * Cobjs[o].dope_sheet[1];
			for (int ci = 1; ci <= Cobjs[o].dope_sheet[f]; ++ci ){
				int c = Cobjs[o].dope_sheet[f + ci];
				SDL_SetRenderDraw_SDL_Color( R, Cobjs[o].cells[c].style.stroke_color );
				draw_TGeo( R, &(Cobjs[o].cells[c].geo), &T, vbuf );
			}
		}

		anim_tick++;
		if( anim_tick >= anim_tick_length ){
			anim_tick = 0;
			for (int o = 0; o < Cobjs_length; ++o ){
				Cobjs[o].current += 1;
				if( Cobjs[o].current >= Cobjs[o].dope_sheet[0] ){
					Cobjs[o].current = 0;
				}
			}
		}

		SDL_SetRenderDrawColor( R, 100, 255, 240, 255 );
		SDL_framerate_limit_n_monitor( R, 17 );

		SDL_RenderPresent(R);
		
	}//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	end:

	SDL_free( vbuf );
}