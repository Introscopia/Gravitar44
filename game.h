#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "basics.h"
#include "vec2d.h"
#include "transform.h"
#include "geometry.h"
#include "Chipmunk/headers/chipmunk.h"
#include "ciol.h"
#include "input.h"


enum types_of_stuff { EMPTY, ROCK, SHIP, BUILDING,
			          SMOKE, BULLET, PARTICLE, 
			          FUEL, REPAIR_PACK, POWERUP };
// if you change these, change class_map as well!


#define exh_frame_cycle 5

typedef struct ship_data_struct{

	Geometric *physical;      // vec
	cpProperties *properties; // vec

	Styled_Geo *visual; // vec

	Geo_Animation exhaust;

	cpVect smoke_outlet;

	double thrust;
    double turn_speed;
    float hull_max;
	float fuel_max;
	float fuel_consumption; //per second of thrust

} Ship_data;

void log_ship_data( const Ship_data* ship );



typedef struct ship_inst_struct{
	
	Ship_data *data;

	cpBody *body;   // used on "land" (or land-like situations)

	vec2d pos, vel; // used in space (there's no physics in space, did you know?)
	double heading;

	float hull;
	float fuel;
	bool thrusting; // this frame (flag for rendering)

	int exh_frame; // frame we're currently showing
	int exh_timer; //counts down the game frames until the next exh animation tick.

	int gun_cooldown;
	Uint64 next_shot;

} Ship_inst;

Ship_inst *instantiate_ship( Ship_data *data );
void init_ship_physics( Ship_inst *ship, cpSpace *space, cpVect pos );


typedef struct {

	char name [64];

	int type; //types of stuff

	union {

		Styled_Geo **visuals; // vec of vecs

		Styled_Geo *visual; // vec

		Ship_data ship;

	} u;

} Doodad;



typedef struct {

	Doodad *doodads;

	int longest_path;

	Style **styles; // vec

	//SDL_Texture *puffs;
	//SDL_Rect puff_dims; //columns, rows, col_w, col_h

} Library;

void load_doodads( char *filename, Library *lib );


typedef void (*world_bounding_func)( void *W, cpBody *b );
typedef void (*update_camera_func)( Transform *T, cpVect target, double *world_angle, SDL_FRect window_rct, SDL_FRect bounds );
typedef void (*render_world_func)( SDL_Renderer *R, void *W, Styled_Geo *map_visuals, Transform *T, SDL_FPoint *vbuf );
typedef void (*gravitate_func)( void *W, cpBody *body, double force );
typedef bool (*inside_func)( void *W, cpVect p );

typedef struct{

	SDL_FRect bounds;
	SDL_FRect gravity_falloff;
	
	int chunks_N;
	int **chunks;
	float chunk_w;
	int width;//window's

	size_t deja_size;
	Sint8 *deja_rendu;

} flat_world;

typedef struct{

	double bounds; //radius
	double bounds_sq;
	double surface_rad;
	double surface_radsq;
	double surface_radcubed;


} round_world;


typedef enum { THRUST, REVTHRUST, SHIELD, GRAB, DROP, FIRE_A, FIRE_B, UI_YES, UI_BACK, PAUSE } ctrl_verb;

typedef void (*pilot_func)( Ship_inst*, vec2d, vec2d, double );
void pilot_YAWER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time );
void pilot_LEANER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time );
void pilot_POINTER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time );
void pilot_THRUSTER( Ship_inst *S, vec2d pilot_vec, vec2d prev_pilot_vec, double delta_time );






typedef struct {

	char name [64];  // filename of the planet, no extention

	Circle collider;
	int collision_mode; // 'N'othing, 'P'ortal, 'K'ill

	Geo_Animation visual;
	int frame;
	int timer;

	float gravity;

} Celestial; // the planets (or whatever) in the space screen.




typedef struct{

	//window
	SDL_FRect window_rct;
	int cx, cy;

	// Spheres and their attributes:
	char **spherepaths;

	// gameplay configs
	enum { NONE, YAWER, LEANER, POINTER, THRUSTER, ORTHO } pilot_mode;
	/* YAWER    : L/R turns (yaws) the ship. aka "relative"
	   LEANER   : L/R leans the ship in that direction. aka "campanella 2"
	   POINTER  : ship faces the direction you point to. aka "absolute"
	   THRUSTER : 2 individually controlled thrusters on the bottom. aka "Lunar Mission Cargo"
	   ORTHO    : 4 individually controlled thrusters, ship does not rotate. aka "DDP2:RH"
	*/
	pilot_func pilot;
	DirInput flightstick, aim;
	Input controls [10];

	// Player:
	
	Ship_inst *hero_ship;

	char COMING_FROM [64];
	char GOING_TO [64];
	int going_to_mode;
	// active module inventory
	// starship inventory
	int *wallet; //heaparray sized at the number currencies in the system


	Library lib;


} GameState;




void upon_a_sphere( SDL_Renderer *R, GameState *GS, char *spherepath );

void among_the_stars( SDL_Renderer *R, GameState *GS, char *starspath );

#endif