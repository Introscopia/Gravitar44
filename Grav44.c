/* OPTIONS:
* Map set
* omni-directional gun
* Strafing
* HP, bullet modifier, collision modifier
* fuel tank size
* time factor, sim iterations
* cycle smoke fx { none * stripey * solid }
* Select player color! / game palettes!
* Enemies drop repair, fuel

-- EXTRAS --
Homebase, get fixed up, get fuel
Steal the reactor from NOVA, bring it back to homebase

*/
#include <SDL.h>
//#include <SDL_image.h>
#include "basics.h"
#include "game.h"
#include "cvec.h"
#include "Vector_Font.h"



int *verts_below( Glyph *G, int Y ){
	int *verts = NULL;
	int total_verts = G->offsets[ G->path_count -1 ];
	for (int i = 0; i < total_verts; ++i ){
		if( G->verts[i].y > Y ){
			vec_push( verts, i );
		}
	}
	return verts;
}

double bell_curve( double x ){
	double y = 1.0 / sq( 10*sq(x) + 1 );
	return y;
}



// - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o - o
int main(int argc, char *argv[]){

	//SDL_Log("hi");

	//HWND hwnd_win = GetConsoleWindow();
	//ShowWindow(hwnd_win,SW_HIDE);
	SDL_Window *window;
	SDL_Renderer *R;
	int width = 600;
	int height = 480;
	int cx, cy;
	bool loop = 1;
	cpVect mouse = cpv(0, 0);

	//SteamAPI_InitEx();
	/* If you're using SDL gamepad support in a Steam game, you must call SteamAPI_InitEx() before calling SDL_Init(). */
	/* https://wiki.libsdl.org/SDL3/CategoryGamepad */

	if( !SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) ){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 3;
	}

	if( !SDL_CreateWindowAndRenderer("Gravitar+44", width, height, 
									SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED, 
									&window, &R) ){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return 3;
	}
	SDL_GetWindowSize( window, &width, &height );
	cx = width / 2;
	cy = height / 2;

	bool vsync_outcome = SDL_SetRenderVSync( R, 1 );
	SDL_Log("vsync_outcome: %d", vsync_outcome );

	int moment = 1;

	SDL_srand(0);

	VFont grav_font = load_VFont( "data/GRAV.bin" );
	grav_font.scale = 10;
	float fch = grav_font.line_height * grav_font.scale;
	float gfhl = fch * 0.5;

	VFont square_font = load_VFont( "data/5by7.bin" );
	square_font.scale = 3;
	float sfch = square_font.line_height * square_font.scale;

	float rad = width * 0.5 * 0.4;
	float yoff = height * 0.65;//height + 125;
	float zoff = 1.12;
	float yhead = -0.1;//*PI;
	float xhead = HALF_PI;
	float xarc = 0.15*PI;
	float yarc = 0.027*PI;

	SDL_Log( "%g, %g, %g, %g, %g", rad, yoff, zoff, yhead, xhead );

	char titlestr [] = "GRAVITAR+44";

	Glyph titlestr_consolidated = VCT_consolidate_string( &grav_font, titlestr );
	Paths3D titlestr_3D = Paths3D_from_consolidated_glyph( &titlestr_consolidated );

	int title_mode = 1;

	int *tsc_bottoms = verts_below( &titlestr_consolidated, grav_font.scale*8 );

	float title_radii [11];
	float rtimer = -4;
	float deltart = 0.01;

	// -- MAIN MENU	--
	while(loop){

		SDL_Event event;
		while( SDL_PollEvent(&event) ){
			switch (event.type) {
				case SDL_EVENT_QUIT:
					goto quitting;

				case SDL_EVENT_KEY_UP:
					if( event.key.key == SDLK_LEFT ){
						yhead -= 0.02;
					}
					else if( event.key.key == SDLK_RIGHT ){
						yhead += 0.02;
					}
					else if( event.key.key == SDLK_UP ){
						zoff += 0.02;
					}
					else if( event.key.key == SDLK_DOWN ){
						zoff -= 0.02;
					}
					else if( event.key.key == ',' ){
						rad -= 5;
					}
					else if( event.key.key == '.' ){
						rad += 5;
					}
					else{
						loop = 0;
					}
					break;
			}
		}

		SDL_SetRenderDrawColor( R, 0, 0, 0, 255 );
		SDL_RenderClear( R );

		
		SDL_SetRenderDrawColor( R, 200, 200, 200, 255 );
		SDL_RenderDebugTextFormat( R, 10, 10, "rad: %g, yh: %g, zo: %g", rad, yhead, zoff );
	

		// WAVE MODE
		if( title_mode == 1 ){
			for (int i = 0; i < 11; ++i ){
				title_radii[ i ] = rad * map( bell_curve( rtimer - (i * 0.1) ), 0, 1, 1, 0.2 );
				//SDL_Log("title_radii[ %d ]: %g\n", i, title_radii[ i ] );
			}
			rtimer += deltart;
			if( rtimer > 4 ) deltart *= -1;
			else if( rtimer < -4 ){
				deltart *= -1;
				if( SDL_rand(10) <= 5 ){
					title_mode = 2;
				}
			}
	
			VCT_project_string_on_a_Ball_w_radii( &titlestr_3D, &grav_font, titlestr, 
									   		      title_radii, xhead, xarc, yhead, yarc, zoff );
		}
		// SPIN MODE
		else if( title_mode == 2 ){
			VCT_project_string_on_a_Ball( &titlestr_3D, &titlestr_consolidated, fch,
									  rad, xhead, xarc, yhead, yarc, zoff );
			xhead += 0.004;
			if( xhead > 2.5 * PI ){
				title_mode = 1;
				xhead = HALF_PI;
			}
		}

		SDL_SetRenderDrawColor( R, 255, 20, 20, 255 );
		for( int b = 0; b < vec_size(tsc_bottoms); ++b ){
			int i = tsc_bottoms[ b ];
			float x =   cx + ( titlestr_3D.verts[ i ].x / titlestr_3D.verts[ i ].z );
			float y = yoff + ( titlestr_3D.verts[ i ].y / titlestr_3D.verts[ i ].z );
			SDL_RenderLine( R, cx, height * 0.65, x, y );
		}

		SDL_SetRenderDrawColor( R, 20, 20, 255, 255 );
		draw_Paths3D( R, &titlestr_3D, cx, yoff );

		SDL_SetRenderDrawColor( R, 220, 220, 220, 255 );
		VCT_render_string_wrapped_aligned( R, &square_font, "- INTROSCOPIA * 2026 -", 
			                               0, height-(2*sfch), width, VCT_ALIGN_CENTER );

		//VCT_render_string_wrapped_aligned( R, &grav_font, titlestr, 0, cy+200, width, VCT_ALIGN_CENTER );

		SDL_RenderPresent( R );
		SDL_framerateDelay(17);
	}


	SDL_free( titlestr_consolidated.offsets );
	SDL_free( titlestr_consolidated.verts );
	SDL_free( titlestr_3D.verts );


	
	GameState GS;

	GS.window_rct = (SDL_FRect){ 0, 0, width, height };
	GS.cx = cx;
	GS.cy = cy;

	int gamepad_count = 0;
	SDL_JoystickID *gamepad_list = SDL_GetGamepads(&gamepad_count);
	for (int g = 0; g < gamepad_count; ++g ){
		SDL_Log( "gamepad_list[%d] = %d", g, gamepad_list[g] );
	}

	if( gamepad_count > 0 ){
		SDL_OpenGamepad( gamepad_list[0] );
		GS.flightstick = Directional_Yawer_gamepad( gamepad_list[0] );
		//GS.aim = Directional_arrows();
		GS.pilot = pilot_YAWER;
		GS.controls[THRUST]    = (Input){ .type = INPUT_NULL };
		GS.controls[REVTHRUST] = (Input){ .type = INPUT_NULL };
	}
	else{
		GS.flightstick = Directional_from_String( "wasd" );
		GS.aim = Directional_arrows();
		GS.pilot = pilot_YAWER;//POINTER; //LEANER;// 
		GS.controls[THRUST]    = (Input){ .type = INPUT_NULL };
		GS.controls[REVTHRUST] = (Input){ .type = INPUT_NULL };
	}
	SDL_free( gamepad_list );


	GS.controls[SHIELD]  = (Input){ .type = INPUT_NULL };
	GS.controls[GRAB]    = (Input){ .type = INPUT_NULL };
	GS.controls[DROP]    = (Input){ .type = INPUT_NULL };
	GS.controls[FIRE_A]  = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_COMMA };
	GS.controls[FIRE_B]  = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_PERIOD };
	GS.controls[UI_YES]  = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_E };
	GS.controls[UI_BACK] = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_Q };
	GS.controls[PAUSE]   = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_ESCAPE };


	char FOLDER [] = "Classic";

	load_doodads( "Classic/Doodads.svg", &(GS.lib) );
	
	int hero_id = -1;
	for (int i = 0; i < vec_size( GS.lib.doodads ); ++i ){
		if( GS.lib.doodads[i].type == SHIP &&
			SDL_strcmp( GS.lib.doodads[i].name, "Hero" ) == 0 ){
			GS.hero_ship = instantiate_ship( &(GS.lib.doodads[i].u.ship) );
			GS.hero_ship->gun_cooldown = 275;
			SDL_Log( "found the Hero, instantiated it" );
			break;
		}
		else{
			SDL_Log( "\"%s\" is not my hero...", GS.lib.doodads[i].name );
		}
	}

	SDL_strlcpy( GS.COMING_FROM, "BASE", 64 ); 
	SDL_strlcpy( GS.GOING_TO, "SPACE", 64 );
	GS.going_to_mode = 'T';

	while( 1 ){
		
		char path [128];
		SDL_snprintf( path, 128, "%s/%s.svg", FOLDER, GS.GOING_TO );

		if( GS.going_to_mode == 'T' ){
			among_the_stars( R, &GS, path );
		} 
		else if( GS.going_to_mode == 'P' ){
			upon_a_sphere( R, &GS, path );
		}
		else if( GS.going_to_mode == 'Q' ){
			SDL_Log( "quitting!!" );
			break;
		}
		else if( GS.going_to_mode == 'O' ){
			SDL_Log( "game over!!" );
			break;
		}
		else{
			SDL_Log( ">>>>%s!!", GS.GOING_TO );
			break;
		}

	}

	quitting:

	SDL_DestroyRenderer(R);
	SDL_Quit();

	return 0;
}