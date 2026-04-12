/* OPTIONS:
* Map set
* omni-directional gun
* Strafing
* HP, bullet modifier, collision modifier
* fuel tank size
* time factor, sim iterations
*/
#include <SDL.h>
#include "basics.h"
#include "game.h"
#include "cvec.h"




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
	
	GameState GS;

	GS.width = width;
	GS.height = height;
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


	GS.controls[SHIELD]    = (Input){ .type = INPUT_NULL };
	GS.controls[GRAB]      = (Input){ .type = INPUT_NULL };
	GS.controls[DROP]      = (Input){ .type = INPUT_NULL };
	GS.controls[FIRE1]     = (Input){ .type = INPUT_NULL };
	GS.controls[FIRE2]     = (Input){ .type = INPUT_NULL };
	GS.controls[UI_YES]    = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_E };
	GS.controls[UI_BACK]   = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_Q };
	GS.controls[PAUSE]     = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_ESCAPE };


	load_doodads( "Classic/Doodads.svg", &(GS.lib) );
	
	int hero_id = -1;
	for (int i = 0; i < GS.lib.doodads; ++i ){
		if( GS.lib.doodads[i].type == SHIP &&
			SDL_strcmp( GS.lib.doodads[i].name, "Hero" ) == 0 ){
			GS.hero_ship = instantiate_ship( &(GS.lib.doodads[i].u.ship) );
			SDL_Log( "found the Hero, instantiated it");
			break;
		}
	}
	
	const char starspath [] = "Classic/SPACE.svg";
	among_the_stars( R, &GS, starspath );

	/*
	SDL_Log("<<<Entering Main Loop>>>");
	while ( loop ) {//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% /LOOP %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|||

		SDL_Event event;
		while( SDL_PollEvent(&event) ){

			//UI_event_handling_function( &main_menu, &event );

			switch (event.type) {
				case SDL_EVENT_QUIT:
					loop = 0;
					break;
				case SDL_EVENT_MOUSE_MOTION:
					mouse = cpv(event.motion.x, event.motion.y);
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					
					break;
				case SDL_EVENT_KEY_DOWN:
					break;
				case SDL_EVENT_KEY_UP:;
					
					break;
				case SDL_EVENT_MOUSE_WHEEL:;


					break;
			}
		}

		
		SDL_SetRenderDrawColor( R, color_scheme[0].r, color_scheme[0].g, color_scheme[0].b, color_scheme[0].a );
		SDL_RenderClear( R );

		//moment = 1;
        switch( moment ){
            case 0:
                //UI_display( R, &main_menu );
                break;
            case 1:
                //the_game( R, &loop, width, height, &fpsm );
                moment = 0;
                break;
            case 2:
               	//options( R, &loop, width, height );
                moment = 0;
                break;
            case 3:
                loop = 0;
                break;
        }

        //SDL_SetRenderDrawColor( R, 0, 24, 222, 255 );
		//SDL_RenderRect( R, &(SDL_Rect){ vertices[0].x -2, vertices[0].y -2, 4, 4 } );



		SDL_RenderPresent( R );



		SDL_framerateDelay(16);
	}//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% /LOOP %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%|||
	*/
	//SDL_DestroyTexture( T );

	SDL_DestroyRenderer(R);
	SDL_Quit();

	return 0;
}