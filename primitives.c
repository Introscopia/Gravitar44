#include "basics.h"
#include "primitives.h"


void gp_crosshair( SDL_Renderer *R, float x, float y, float rad ){
    SDL_RenderLine( R, x, y-rad, x, y+rad );
    SDL_RenderLine( R, x-rad, y, x+rad, y );
}


void gp_drawthick_line( SDL_Renderer *R, float ax, float ay, float bx, float by, float radius ){

	float subx = bx - ax;
	float suby = by - ay;
	double len = SDL_sqrt( (subx * subx) + (suby * suby) ) + SDL_FLT_EPSILON;
	int dx = SDL_lround(radius * subx / len);
	int dy = SDL_lround(radius * suby / len);

	SDL_FColor C = SDL_GetRender_SDL_FColor( R );

	SDL_Vertex verts[4];
	verts[0] = (SDL_Vertex){ { ax + dy, ay - dx }, C, {0} };
	verts[1] = (SDL_Vertex){ { ax - dy, ay + dx }, C, {0} };
	verts[2] = (SDL_Vertex){ { bx - dy, by + dx }, C, {0} };
	verts[3] = (SDL_Vertex){ { bx + dy, by - dx }, C, {0} };

	int indices[6] = { 0, 1, 3, 1, 2, 3 };

	SDL_RenderGeometry( R, NULL, verts, 4, indices, 6 );
}

void gp_drawthick_roundedLine( SDL_Renderer *R, float ax, float ay, float bx, float by, float radius ){
	gp_drawthick_line( R, ax, ay, bx, by, radius );
	gp_fill_fastcircle( R, ax, ay, radius );
	gp_fill_fastcircle( R, bx, by, radius );
}


void gp_draw_arrow( SDL_Renderer *R, float ax, float ay, float bx, float by, float LT, float HB, float HH ){

	float subx = bx - ax;
	float suby = by - ay;
	double len = SDL_sqrt( (subx * subx) + (suby * suby) ) + SDL_FLT_EPSILON;
	double nx = subx / len;
	double ny = suby / len;
	float ht = LT * 0.5;
	int ldx = SDL_lround(ht * nx);
	int ldy = SDL_lround(ht * ny);
	float hHB = HB * 0.5;
	int hdx = SDL_lround(hHB * nx);
	int hdy = SDL_lround(hHB * ny);
	int cx = bx - SDL_lround(HH * nx);
	int cy = by - SDL_lround(HH * ny);

	SDL_FColor C = SDL_GetRender_SDL_FColor( R );

	SDL_Vertex verts[7];
	verts[0] = (SDL_Vertex){ { ax + ldy, ay - ldx }, C, {0} };
	verts[1] = (SDL_Vertex){ { ax - ldy, ay + ldx }, C, {0} };
	verts[2] = (SDL_Vertex){ { cx - ldy, cy + ldx }, C, {0} };
	verts[3] = (SDL_Vertex){ { cx + ldy, cy - ldx }, C, {0} };

	verts[4] = (SDL_Vertex){ { cx + hdy, cy - hdx }, C, {0} };
	verts[5] = (SDL_Vertex){ { cx - hdy, cy + hdx }, C, {0} };
	verts[6] = (SDL_Vertex){ { bx,       by       }, C, {0} };

	int indices[9] = { 0, 1, 3, 1, 2, 3, 4, 5, 6 };

	SDL_RenderGeometry( R, NULL, verts, 7, indices, 9 );
}


void gp_draw_bezier1( SDL_Renderer *R, vec2d *a1, vec2d *a2, vec2d *c, int res ){
	vec2d prev = *a1;
	float t = 1.0 / res;
	for (int i = 1; i <= res; ++i){
		float amt = i * t;
		vec2d inter1 = v2d_lerp( *a1, *c, amt );
		vec2d inter2 = v2d_lerp( *c, *a2, amt );
		vec2d point  = v2d_lerp( inter1, inter2, amt );
		SDL_RenderLine(R, prev.x, prev.y, point.x, point.y );
		prev = point;
	}
}

void gp_draw_bezier2( SDL_Renderer *R, vec2d *a1, vec2d *a2, vec2d *c1, vec2d *c2, int res ){
	vec2d prev = *a1;
	float t = 1.0 / res;
	for (int i = 1; i <= res; ++i){
		float amt = i * t;
		vec2d inter01 = v2d_lerp( *a1, *c1, amt );
		vec2d inter02 = v2d_lerp( *c1, *c2, amt );
		vec2d inter03 = v2d_lerp( *c2, *a2, amt );
		vec2d inter11 = v2d_lerp( inter01, inter02, amt );
		vec2d inter12 = v2d_lerp( inter02, inter03, amt );
		vec2d point   = v2d_lerp( inter11, inter12, amt );
		SDL_RenderLine( R, prev.x, prev.y, point.x, point.y );
		prev = point;
	}
}



void gp_fill_diamond(SDL_Renderer *R, float x, float y, float radius){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		SDL_Vertex verts[4];
		verts[0] = (SDL_Vertex){ { x + radius, y          }, C, {0} };
		verts[1] = (SDL_Vertex){ { x         , y + radius }, C, {0} };
		verts[2] = (SDL_Vertex){ { x - radius, y          }, C, {0} };
		verts[3] = (SDL_Vertex){ { x         , y - radius }, C, {0} };
		int indices[6] = { 0, 1, 2, 0, 2, 3 };
		SDL_RenderGeometry( R, NULL, verts, 4, indices, 6 );
}

//https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
void gp_draw_circle(SDL_Renderer *R, float x, float y, float radius){

	int offsetx, offsety, d;
	//int status = 0;

	//CHECK_RENDERER_MAGIC(R, -1);

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	//status = 0;

	while (offsety >= offsetx) {
		SDL_RenderPoint(R, x + offsetx, y + offsety); //status |= 
		SDL_RenderPoint(R, x + offsety, y + offsetx); //status |= 
		SDL_RenderPoint(R, x - offsetx, y + offsety); //status |= 
		SDL_RenderPoint(R, x - offsety, y + offsetx); //status |= 
		SDL_RenderPoint(R, x + offsetx, y - offsety); //status |= 
		SDL_RenderPoint(R, x + offsety, y - offsetx); //status |= 
		SDL_RenderPoint(R, x - offsetx, y - offsety); //status |= 
		SDL_RenderPoint(R, x - offsety, y - offsetx); //status |= 

		//if (status < 0) {	status = -1;	break;}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	//return status;
}
//https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
void gp_fill_circle(SDL_Renderer *R, float x, float y, float radius){
	int offsetx, offsety, d;
	//int status;

	//CHECK_RENDERER_MAGIC(R, -1);

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	//status = 0;

	while (offsety >= offsetx) {

		SDL_RenderLine(R, x - offsety, y + offsetx,     //status |= 
						   x + offsety, y + offsetx);          
		SDL_RenderLine(R, x - offsetx, y + offsety,     //status |= 
						   x + offsetx, y + offsety);          
		SDL_RenderLine(R, x - offsetx, y - offsety,     //status |= 
						   x + offsetx, y - offsety);          
		SDL_RenderLine(R, x - offsety, y - offsetx,     //status |= 
						   x + offsety, y - offsetx);          

		//if (status < 0) {	status = -1;	break;}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	//return status;
}

void gp_draw_8circle(SDL_Renderer *R, float x, float y, float radius){
	const float trig = 0.70710678*radius;
	SDL_FPoint points [9] = {
		{ x + radius, y          },
		{ x + trig  , y + trig   },
		{ x         , y + radius },
		{ x - trig  , y + trig   },
		{ x - radius, y          },
		{ x - trig  , y - trig   },
		{ x         , y - radius },
		{ x + trig  , y - trig   },
		{ x + radius, y          },
	};
	SDL_RenderLines( R, points, 9 );
}
void gp_drawthick_8circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	float rad = C_radius + L_radius;
	float trig = 0.707107*rad;
	SDL_Vertex verts[16];
	verts[ 0] = (SDL_Vertex){ { x + rad , y        }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { x + trig, y + trig }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { x       , y + rad  }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { x - trig, y + trig }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { x - rad , y        }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { x - trig, y - trig }, C, {0} };
	verts[ 6] = (SDL_Vertex){ { x       , y - rad  }, C, {0} };
	verts[ 7] = (SDL_Vertex){ { x + trig, y - trig }, C, {0} };
	rad = C_radius - L_radius;
	trig = 0.707107*rad;
	verts[ 8] = (SDL_Vertex){ { x + rad , y        }, C, {0} };
	verts[ 9] = (SDL_Vertex){ { x + trig, y + trig }, C, {0} };
	verts[10] = (SDL_Vertex){ { x       , y + rad  }, C, {0} };
	verts[11] = (SDL_Vertex){ { x - trig, y + trig }, C, {0} };
	verts[12] = (SDL_Vertex){ { x - rad , y        }, C, {0} };
	verts[13] = (SDL_Vertex){ { x - trig, y - trig }, C, {0} };
	verts[14] = (SDL_Vertex){ { x       , y - rad  }, C, {0} };
	verts[15] = (SDL_Vertex){ { x + trig, y - trig }, C, {0} };
	int indices[48] = { 0, 1, 8, 1, 9, 8, 1, 2, 9, 2, 10, 9, 2, 3, 
		10, 3, 11, 10, 3, 4, 11, 4, 12, 11, 4, 5, 12, 5, 13, 12, 5, 
		6, 13, 6, 14, 13, 6, 7, 14, 7, 15, 14, 7, 0, 15, 0, 8, 15 };
	SDL_RenderGeometry( R, NULL, verts, 16, indices, 48 );
}
void gp_fill_8circle(SDL_Renderer *R, float x, float y, float radius){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		const float trig = { 0.707107*radius};
		SDL_Vertex verts[8];
		verts[0] = (SDL_Vertex){ { x + radius, y          }, C, {0} };
		verts[1] = (SDL_Vertex){ { x + trig  , y + trig   }, C, {0} };
		verts[2] = (SDL_Vertex){ { x         , y + radius }, C, {0} };
		verts[3] = (SDL_Vertex){ { x - trig  , y + trig   }, C, {0} };
		verts[4] = (SDL_Vertex){ { x - radius, y          }, C, {0} };
		verts[5] = (SDL_Vertex){ { x - trig  , y - trig   }, C, {0} };
		verts[6] = (SDL_Vertex){ { x         , y - radius }, C, {0} };
		verts[7] = (SDL_Vertex){ { x + trig  , y - trig   }, C, {0} };
		int indices[18] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7 };
		SDL_RenderGeometry( R, NULL, verts, 8, indices, 18 );
}


void gp_draw_12circle(SDL_Renderer *R, float x, float y, float radius){
	const float trig [2] = { 0.5*radius, 0.86602540*radius };
	SDL_FPoint points [13] = {
		{ x + radius , y           },
		{ x + trig[1], y + trig[0] },
		{ x + trig[0], y + trig[1] },
		{ x          , y + radius  },
		{ x - trig[0], y + trig[1] },
		{ x - trig[1], y + trig[0] },
		{ x - radius , y           },
		{ x - trig[1], y - trig[0] },
		{ x - trig[0], y - trig[1] },
		{ x          , y - radius  },
		{ x + trig[0], y - trig[1] },
		{ x + trig[1], y - trig[0] },
		{ x + radius , y           }
	};
	SDL_RenderLines( R, points, 13 );
}


void gp_draw_16circle(SDL_Renderer *R, float x, float y, float radius){
	const float trig [3] = { 0.38268343*radius, 0.70710678*radius, 0.92387953*radius};
	SDL_FPoint points [17] = {
		{ x + radius , y           },
		{ x + trig[2], y + trig[0] },
		{ x + trig[1], y + trig[1] },
		{ x + trig[0], y + trig[2] },
		{ x          , y + radius  },
		{ x - trig[0], y + trig[2] },
		{ x - trig[1], y + trig[1] },
		{ x - trig[2], y + trig[0] },
		{ x - radius , y           },
		{ x - trig[2], y - trig[0] },
		{ x - trig[1], y - trig[1] },
		{ x - trig[0], y - trig[2] },
		{ x          , y - radius  },
		{ x + trig[0], y - trig[2] },
		{ x + trig[1], y - trig[1] },
		{ x + trig[2], y - trig[0] },
		{ x + radius , y           },
	};
	SDL_RenderLines( R, points, 17 );
}

void gp_drawthick_16circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	float rad = C_radius + L_radius;
	float trig [3] = { 0.382683*rad, 0.707107*rad, 0.923880*rad };
	SDL_Vertex verts[32];
	verts[ 0] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { x + trig[2], y + trig[0] }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { x + trig[1], y + trig[1] }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { x + trig[0], y + trig[2] }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { x - trig[0], y + trig[2] }, C, {0} };
	verts[ 6] = (SDL_Vertex){ { x - trig[1], y + trig[1] }, C, {0} };
	verts[ 7] = (SDL_Vertex){ { x - trig[2], y + trig[0] }, C, {0} };
	verts[ 8] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
	verts[ 9] = (SDL_Vertex){ { x - trig[2], y - trig[0] }, C, {0} };
	verts[10] = (SDL_Vertex){ { x - trig[1], y - trig[1] }, C, {0} };
	verts[11] = (SDL_Vertex){ { x - trig[0], y - trig[2] }, C, {0} };
	verts[12] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
	verts[13] = (SDL_Vertex){ { x + trig[0], y - trig[2] }, C, {0} };
	verts[14] = (SDL_Vertex){ { x + trig[1], y - trig[1] }, C, {0} };
	verts[15] = (SDL_Vertex){ { x + trig[2], y - trig[0] }, C, {0} };
	rad = C_radius - L_radius;
	trig[0] = 0.382683*rad; trig[1] = 0.707107*rad; trig[2] = 0.923880*rad;
	verts[16] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
	verts[17] = (SDL_Vertex){ { x + trig[2], y + trig[0] }, C, {0} };
	verts[18] = (SDL_Vertex){ { x + trig[1], y + trig[1] }, C, {0} };
	verts[19] = (SDL_Vertex){ { x + trig[0], y + trig[2] }, C, {0} };
	verts[20] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
	verts[21] = (SDL_Vertex){ { x - trig[0], y + trig[2] }, C, {0} };
	verts[22] = (SDL_Vertex){ { x - trig[1], y + trig[1] }, C, {0} };
	verts[23] = (SDL_Vertex){ { x - trig[2], y + trig[0] }, C, {0} };
	verts[24] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
	verts[25] = (SDL_Vertex){ { x - trig[2], y - trig[0] }, C, {0} };
	verts[26] = (SDL_Vertex){ { x - trig[1], y - trig[1] }, C, {0} };
	verts[27] = (SDL_Vertex){ { x - trig[0], y - trig[2] }, C, {0} };
	verts[28] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
	verts[29] = (SDL_Vertex){ { x + trig[0], y - trig[2] }, C, {0} };
	verts[30] = (SDL_Vertex){ { x + trig[1], y - trig[1] }, C, {0} };
	verts[31] = (SDL_Vertex){ { x + trig[2], y - trig[0] }, C, {0} };
	int indices[96] = { 0, 1, 16, 1, 17, 16, 1, 2, 17, 2, 18, 17, 2, 3, 
		18, 3, 19, 18, 3, 4, 19, 4, 20, 19, 4, 5, 20, 5, 21, 20, 5, 6, 
		21, 6, 22, 21, 6, 7, 22, 7, 23, 22, 7, 8, 23, 8, 24, 23, 8, 9, 
		24, 9, 25, 24, 9, 10, 25, 10, 26, 25, 10, 11, 26, 11, 27, 26, 
		11, 12, 27, 12, 28, 27, 12, 13, 28, 13, 29, 28, 13, 14, 29, 14, 
		30, 29, 14, 15, 30, 15, 31, 30, 15, 0, 31, 0, 16, 31 };
	SDL_RenderGeometry( R, NULL, verts, 32, indices, 96 );
}
void gp_fill_16circle(SDL_Renderer *R, float x, float y, float radius){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	const float trig [3] = {  0.3826834324*radius, 0.7071067812*radius, 0.9238795325*radius };
	SDL_Vertex verts[16];
	verts[ 0] = (SDL_Vertex){ { x + radius , y           }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { x + trig[2], y + trig[0] }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { x + trig[1], y + trig[1] }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { x + trig[0], y + trig[2] }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { x          , y + radius  }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { x - trig[0], y + trig[2] }, C, {0} };
	verts[ 6] = (SDL_Vertex){ { x - trig[1], y + trig[1] }, C, {0} };
	verts[ 7] = (SDL_Vertex){ { x - trig[2], y + trig[0] }, C, {0} };
	verts[ 8] = (SDL_Vertex){ { x - radius , y           }, C, {0} };
	verts[ 9] = (SDL_Vertex){ { x - trig[2], y - trig[0] }, C, {0} };
	verts[10] = (SDL_Vertex){ { x - trig[1], y - trig[1] }, C, {0} };
	verts[11] = (SDL_Vertex){ { x - trig[0], y - trig[2] }, C, {0} };
	verts[12] = (SDL_Vertex){ { x          , y - radius  }, C, {0} };
	verts[13] = (SDL_Vertex){ { x + trig[0], y - trig[2] }, C, {0} };
	verts[14] = (SDL_Vertex){ { x + trig[1], y - trig[1] }, C, {0} };
	verts[15] = (SDL_Vertex){ { x + trig[2], y - trig[0] }, C, {0} };
	int indices[42] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 
						9, 0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 13, 0, 13, 14, 0, 14, 15 };
	SDL_RenderGeometry( R, NULL, verts, 16, indices, 42 );
}

void gp_draw_24circle(SDL_Renderer *R, float x, float y, float radius){
 	const float trig [5] = { 0.25881905*radius, 0.5*radius, 0.70710678*radius, 
 	                         0.8660254*radius, 0.96592583*radius};
 	SDL_FPoint points [25] = {
 		{ x + radius , y           },
 		{ x + trig[4], y + trig[0] },
 		{ x + trig[3], y + trig[1] },
 		{ x + trig[2], y + trig[2] },
 		{ x + trig[1], y + trig[3] },
 		{ x + trig[0], y + trig[4] },
 		{ x          , y + radius  },
 		{ x - trig[0], y + trig[4] },
 		{ x - trig[1], y + trig[3] },
 		{ x - trig[2], y + trig[2] },
 		{ x - trig[3], y + trig[1] },
 		{ x - trig[4], y + trig[0] },
 		{ x - radius , y           },
 		{ x - trig[4], y - trig[0] },
 		{ x - trig[3], y - trig[1] },
 		{ x - trig[2], y - trig[2] },
 		{ x - trig[1], y - trig[3] },
 		{ x - trig[0], y - trig[4] },
 		{ x          , y - radius  },
 		{ x + trig[0], y - trig[4] },
 		{ x + trig[1], y - trig[3] },
 		{ x + trig[2], y - trig[2] },
 		{ x + trig[3], y - trig[1] },
 		{ x + trig[4], y - trig[0] },
 		{ x + radius , y           },
 	};
 	SDL_RenderLines( R, points, 25 );
}
void gp_drawthick_24circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	float rad = C_radius + L_radius;
	float trig [5] = { 0.258819*rad, 0.500000*rad, 0.707107*rad, 0.866025*rad, 0.965926*rad};
	SDL_Vertex verts[48];
	verts[ 0] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { x + trig[4], y + trig[0] }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { x + trig[3], y + trig[1] }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { x + trig[2], y + trig[2] }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { x + trig[1], y + trig[3] }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { x + trig[0], y + trig[4] }, C, {0} };
	verts[ 6] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
	verts[ 7] = (SDL_Vertex){ { x - trig[0], y + trig[4] }, C, {0} };
	verts[ 8] = (SDL_Vertex){ { x - trig[1], y + trig[3] }, C, {0} };
	verts[ 9] = (SDL_Vertex){ { x - trig[2], y + trig[2] }, C, {0} };
	verts[10] = (SDL_Vertex){ { x - trig[3], y + trig[1] }, C, {0} };
	verts[11] = (SDL_Vertex){ { x - trig[4], y + trig[0] }, C, {0} };
	verts[12] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
	verts[13] = (SDL_Vertex){ { x - trig[4], y - trig[0] }, C, {0} };
	verts[14] = (SDL_Vertex){ { x - trig[3], y - trig[1] }, C, {0} };
	verts[15] = (SDL_Vertex){ { x - trig[2], y - trig[2] }, C, {0} };
	verts[16] = (SDL_Vertex){ { x - trig[1], y - trig[3] }, C, {0} };
	verts[17] = (SDL_Vertex){ { x - trig[0], y - trig[4] }, C, {0} };
	verts[18] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
	verts[19] = (SDL_Vertex){ { x + trig[0], y - trig[4] }, C, {0} };
	verts[20] = (SDL_Vertex){ { x + trig[1], y - trig[3] }, C, {0} };
	verts[21] = (SDL_Vertex){ { x + trig[2], y - trig[2] }, C, {0} };
	verts[22] = (SDL_Vertex){ { x + trig[3], y - trig[1] }, C, {0} };
	verts[23] = (SDL_Vertex){ { x + trig[4], y - trig[0] }, C, {0} };
	rad = C_radius - L_radius;
	trig[0] = 0.258819*rad; trig[1] = 0.500000*rad; trig[2] = 0.707107*rad; 
	trig[3] = 0.866025*rad; trig[4] = 0.965926*rad;
	verts[24] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
	verts[25] = (SDL_Vertex){ { x + trig[4], y + trig[0] }, C, {0} };
	verts[26] = (SDL_Vertex){ { x + trig[3], y + trig[1] }, C, {0} };
	verts[27] = (SDL_Vertex){ { x + trig[2], y + trig[2] }, C, {0} };
	verts[28] = (SDL_Vertex){ { x + trig[1], y + trig[3] }, C, {0} };
	verts[29] = (SDL_Vertex){ { x + trig[0], y + trig[4] }, C, {0} };
	verts[30] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
	verts[31] = (SDL_Vertex){ { x - trig[0], y + trig[4] }, C, {0} };
	verts[32] = (SDL_Vertex){ { x - trig[1], y + trig[3] }, C, {0} };
	verts[33] = (SDL_Vertex){ { x - trig[2], y + trig[2] }, C, {0} };
	verts[34] = (SDL_Vertex){ { x - trig[3], y + trig[1] }, C, {0} };
	verts[35] = (SDL_Vertex){ { x - trig[4], y + trig[0] }, C, {0} };
	verts[36] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
	verts[37] = (SDL_Vertex){ { x - trig[4], y - trig[0] }, C, {0} };
	verts[38] = (SDL_Vertex){ { x - trig[3], y - trig[1] }, C, {0} };
	verts[39] = (SDL_Vertex){ { x - trig[2], y - trig[2] }, C, {0} };
	verts[40] = (SDL_Vertex){ { x - trig[1], y - trig[3] }, C, {0} };
	verts[41] = (SDL_Vertex){ { x - trig[0], y - trig[4] }, C, {0} };
	verts[42] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
	verts[43] = (SDL_Vertex){ { x + trig[0], y - trig[4] }, C, {0} };
	verts[44] = (SDL_Vertex){ { x + trig[1], y - trig[3] }, C, {0} };
	verts[45] = (SDL_Vertex){ { x + trig[2], y - trig[2] }, C, {0} };
	verts[46] = (SDL_Vertex){ { x + trig[3], y - trig[1] }, C, {0} };
	verts[47] = (SDL_Vertex){ { x + trig[4], y - trig[0] }, C, {0} };
	int indices[144] = { 0, 1, 24, 1, 25, 24, 1, 2, 25, 2, 26, 25, 2, 3, 
		26, 3, 27, 26, 3, 4, 27, 4, 28, 27, 4, 5, 28, 5, 29, 28, 5, 6, 29, 
		6, 30, 29, 6, 7, 30, 7, 31, 30, 7, 8, 31, 8, 32, 31, 8, 9, 32, 9, 
		33, 32, 9, 10, 33, 10, 34, 33, 10, 11, 34, 11, 35, 34, 11, 12, 35, 
		12, 36, 35, 12, 13, 36, 13, 37, 36, 13, 14, 37, 14, 38, 37, 14, 15, 
		38, 15, 39, 38, 15, 16, 39, 16, 40, 39, 16, 17, 40, 17, 41, 40, 17,
		 18, 41, 18, 42, 41, 18, 19, 42, 19, 43, 42, 19, 20, 43, 20, 44, 43, 
		 20, 21, 44, 21, 45, 44, 21, 22, 45, 22, 46, 45, 22, 23, 46, 23, 47, 
		 46, 23, 0, 47, 0, 24, 47 };
	SDL_RenderGeometry( R, NULL, verts, 48, indices, 144 );
}
void gp_fill_24circle(SDL_Renderer *R, float x, float y, float radius){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		const float trig [5] = { 0.258819*radius, 0.500000*radius, 0.707107*radius, 0.866025*radius, 0.965926*radius};
		SDL_Vertex verts[24];
		verts[ 0] = (SDL_Vertex){ { x + radius , y           }, C, {0} };
		verts[ 1] = (SDL_Vertex){ { x + trig[4], y + trig[0] }, C, {0} };
		verts[ 2] = (SDL_Vertex){ { x + trig[3], y + trig[1] }, C, {0} };
		verts[ 3] = (SDL_Vertex){ { x + trig[2], y + trig[2] }, C, {0} };
		verts[ 4] = (SDL_Vertex){ { x + trig[1], y + trig[3] }, C, {0} };
		verts[ 5] = (SDL_Vertex){ { x + trig[0], y + trig[4] }, C, {0} };
		verts[ 6] = (SDL_Vertex){ { x          , y + radius  }, C, {0} };
		verts[ 7] = (SDL_Vertex){ { x - trig[0], y + trig[4] }, C, {0} };
		verts[ 8] = (SDL_Vertex){ { x - trig[1], y + trig[3] }, C, {0} };
		verts[ 9] = (SDL_Vertex){ { x - trig[2], y + trig[2] }, C, {0} };
		verts[10] = (SDL_Vertex){ { x - trig[3], y + trig[1] }, C, {0} };
		verts[11] = (SDL_Vertex){ { x - trig[4], y + trig[0] }, C, {0} };
		verts[12] = (SDL_Vertex){ { x - radius , y           }, C, {0} };
		verts[13] = (SDL_Vertex){ { x - trig[4], y - trig[0] }, C, {0} };
		verts[14] = (SDL_Vertex){ { x - trig[3], y - trig[1] }, C, {0} };
		verts[15] = (SDL_Vertex){ { x - trig[2], y - trig[2] }, C, {0} };
		verts[16] = (SDL_Vertex){ { x - trig[1], y - trig[3] }, C, {0} };
		verts[17] = (SDL_Vertex){ { x - trig[0], y - trig[4] }, C, {0} };
		verts[18] = (SDL_Vertex){ { x          , y - radius  }, C, {0} };
		verts[19] = (SDL_Vertex){ { x + trig[0], y - trig[4] }, C, {0} };
		verts[20] = (SDL_Vertex){ { x + trig[1], y - trig[3] }, C, {0} };
		verts[21] = (SDL_Vertex){ { x + trig[2], y - trig[2] }, C, {0} };
		verts[22] = (SDL_Vertex){ { x + trig[3], y - trig[1] }, C, {0} };
		verts[23] = (SDL_Vertex){ { x + trig[4], y - trig[0] }, C, {0} };
		int indices[66] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 
			6, 7, 0, 7, 8, 0, 8, 9, 0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 
			13, 0, 13, 14, 0, 14, 15, 0, 15, 16, 0, 16, 17, 0, 17, 18, 0, 
			18, 19, 0, 19, 20, 0, 20, 21, 0, 21, 22, 0, 22, 23 };
		SDL_RenderGeometry( R, NULL, verts, 24, indices, 66 );
}

void gp_draw_36circle(SDL_Renderer *R, float x, float y, float radius){
	const float trig [8] = { 0.17364818*radius, 0.34202014*radius, 0.5*radius, 0.64278761*radius, 
	                         0.76604444*radius, 0.8660254*radius, 0.93969262*radius, 0.98480775*radius};
	SDL_FPoint points [37] = {
		{ x + radius , y           },
		{ x + trig[7], y + trig[0] },
		{ x + trig[6], y + trig[1] },
		{ x + trig[5], y + trig[2] },
		{ x + trig[4], y + trig[3] },
		{ x + trig[3], y + trig[4] },
		{ x + trig[2], y + trig[5] },
		{ x + trig[1], y + trig[6] },
		{ x + trig[0], y + trig[7] },
		{ x          , y + radius  },
		{ x - trig[0], y + trig[7] },
		{ x - trig[1], y + trig[6] },
		{ x - trig[2], y + trig[5] },
		{ x - trig[3], y + trig[4] },
		{ x - trig[4], y + trig[3] },
		{ x - trig[5], y + trig[2] },
		{ x - trig[6], y + trig[1] },
		{ x - trig[7], y + trig[0] },
		{ x - radius , y           },
		{ x - trig[7], y - trig[0] },
		{ x - trig[6], y - trig[1] },
		{ x - trig[5], y - trig[2] },
		{ x - trig[4], y - trig[3] },
		{ x - trig[3], y - trig[4] },
		{ x - trig[2], y - trig[5] },
		{ x - trig[1], y - trig[6] },
		{ x - trig[0], y - trig[7] },
		{ x          , y - radius  },
		{ x + trig[0], y - trig[7] },
		{ x + trig[1], y - trig[6] },
		{ x + trig[2], y - trig[5] },
		{ x + trig[3], y - trig[4] },
		{ x + trig[4], y - trig[3] },
		{ x + trig[5], y - trig[2] },
		{ x + trig[6], y - trig[1] },
		{ x + trig[7], y - trig[0] },
		{ x + radius , y           },
	};
	SDL_RenderLines( R, points, 37 );
}
void gp_drawthick_36circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius ){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		float rad = C_radius + L_radius;
		float trig [8] = { 0.173648*rad, 0.342020*rad, 0.500000*rad, 0.642788*rad, 
						   0.766044*rad, 0.866025*rad, 0.939693*rad, 0.984808*rad};
		SDL_Vertex verts[72];
		verts[ 0] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
		verts[ 1] = (SDL_Vertex){ { x + trig[7], y + trig[0] }, C, {0} };
		verts[ 2] = (SDL_Vertex){ { x + trig[6], y + trig[1] }, C, {0} };
		verts[ 3] = (SDL_Vertex){ { x + trig[5], y + trig[2] }, C, {0} };
		verts[ 4] = (SDL_Vertex){ { x + trig[4], y + trig[3] }, C, {0} };
		verts[ 5] = (SDL_Vertex){ { x + trig[3], y + trig[4] }, C, {0} };
		verts[ 6] = (SDL_Vertex){ { x + trig[2], y + trig[5] }, C, {0} };
		verts[ 7] = (SDL_Vertex){ { x + trig[1], y + trig[6] }, C, {0} };
		verts[ 8] = (SDL_Vertex){ { x + trig[0], y + trig[7] }, C, {0} };
		verts[ 9] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
		verts[10] = (SDL_Vertex){ { x - trig[0], y + trig[7] }, C, {0} };
		verts[11] = (SDL_Vertex){ { x - trig[1], y + trig[6] }, C, {0} };
		verts[12] = (SDL_Vertex){ { x - trig[2], y + trig[5] }, C, {0} };
		verts[13] = (SDL_Vertex){ { x - trig[3], y + trig[4] }, C, {0} };
		verts[14] = (SDL_Vertex){ { x - trig[4], y + trig[3] }, C, {0} };
		verts[15] = (SDL_Vertex){ { x - trig[5], y + trig[2] }, C, {0} };
		verts[16] = (SDL_Vertex){ { x - trig[6], y + trig[1] }, C, {0} };
		verts[17] = (SDL_Vertex){ { x - trig[7], y + trig[0] }, C, {0} };
		verts[18] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
		verts[19] = (SDL_Vertex){ { x - trig[7], y - trig[0] }, C, {0} };
		verts[20] = (SDL_Vertex){ { x - trig[6], y - trig[1] }, C, {0} };
		verts[21] = (SDL_Vertex){ { x - trig[5], y - trig[2] }, C, {0} };
		verts[22] = (SDL_Vertex){ { x - trig[4], y - trig[3] }, C, {0} };
		verts[23] = (SDL_Vertex){ { x - trig[3], y - trig[4] }, C, {0} };
		verts[24] = (SDL_Vertex){ { x - trig[2], y - trig[5] }, C, {0} };
		verts[25] = (SDL_Vertex){ { x - trig[1], y - trig[6] }, C, {0} };
		verts[26] = (SDL_Vertex){ { x - trig[0], y - trig[7] }, C, {0} };
		verts[27] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
		verts[28] = (SDL_Vertex){ { x + trig[0], y - trig[7] }, C, {0} };
		verts[29] = (SDL_Vertex){ { x + trig[1], y - trig[6] }, C, {0} };
		verts[30] = (SDL_Vertex){ { x + trig[2], y - trig[5] }, C, {0} };
		verts[31] = (SDL_Vertex){ { x + trig[3], y - trig[4] }, C, {0} };
		verts[32] = (SDL_Vertex){ { x + trig[4], y - trig[3] }, C, {0} };
		verts[33] = (SDL_Vertex){ { x + trig[5], y - trig[2] }, C, {0} };
		verts[34] = (SDL_Vertex){ { x + trig[6], y - trig[1] }, C, {0} };
		verts[35] = (SDL_Vertex){ { x + trig[7], y - trig[0] }, C, {0} };
		rad = C_radius - L_radius;
		trig[0] = 0.173648*rad; trig[1] = 0.342020*rad; trig[2] = 0.500000*rad; 
		trig[3] = 0.642788*rad; trig[4] = 0.766044*rad; trig[5] = 0.866025*rad; 
		trig[6] = 0.939693*rad; trig[7] = 0.984808*rad;
		verts[36] = (SDL_Vertex){ { x + rad    , y           }, C, {0} };
		verts[37] = (SDL_Vertex){ { x + trig[7], y + trig[0] }, C, {0} };
		verts[38] = (SDL_Vertex){ { x + trig[6], y + trig[1] }, C, {0} };
		verts[39] = (SDL_Vertex){ { x + trig[5], y + trig[2] }, C, {0} };
		verts[40] = (SDL_Vertex){ { x + trig[4], y + trig[3] }, C, {0} };
		verts[41] = (SDL_Vertex){ { x + trig[3], y + trig[4] }, C, {0} };
		verts[42] = (SDL_Vertex){ { x + trig[2], y + trig[5] }, C, {0} };
		verts[43] = (SDL_Vertex){ { x + trig[1], y + trig[6] }, C, {0} };
		verts[44] = (SDL_Vertex){ { x + trig[0], y + trig[7] }, C, {0} };
		verts[45] = (SDL_Vertex){ { x          , y + rad     }, C, {0} };
		verts[46] = (SDL_Vertex){ { x - trig[0], y + trig[7] }, C, {0} };
		verts[47] = (SDL_Vertex){ { x - trig[1], y + trig[6] }, C, {0} };
		verts[48] = (SDL_Vertex){ { x - trig[2], y + trig[5] }, C, {0} };
		verts[49] = (SDL_Vertex){ { x - trig[3], y + trig[4] }, C, {0} };
		verts[50] = (SDL_Vertex){ { x - trig[4], y + trig[3] }, C, {0} };
		verts[51] = (SDL_Vertex){ { x - trig[5], y + trig[2] }, C, {0} };
		verts[52] = (SDL_Vertex){ { x - trig[6], y + trig[1] }, C, {0} };
		verts[53] = (SDL_Vertex){ { x - trig[7], y + trig[0] }, C, {0} };
		verts[54] = (SDL_Vertex){ { x - rad    , y           }, C, {0} };
		verts[55] = (SDL_Vertex){ { x - trig[7], y - trig[0] }, C, {0} };
		verts[56] = (SDL_Vertex){ { x - trig[6], y - trig[1] }, C, {0} };
		verts[57] = (SDL_Vertex){ { x - trig[5], y - trig[2] }, C, {0} };
		verts[58] = (SDL_Vertex){ { x - trig[4], y - trig[3] }, C, {0} };
		verts[59] = (SDL_Vertex){ { x - trig[3], y - trig[4] }, C, {0} };
		verts[60] = (SDL_Vertex){ { x - trig[2], y - trig[5] }, C, {0} };
		verts[61] = (SDL_Vertex){ { x - trig[1], y - trig[6] }, C, {0} };
		verts[62] = (SDL_Vertex){ { x - trig[0], y - trig[7] }, C, {0} };
		verts[63] = (SDL_Vertex){ { x          , y - rad     }, C, {0} };
		verts[64] = (SDL_Vertex){ { x + trig[0], y - trig[7] }, C, {0} };
		verts[65] = (SDL_Vertex){ { x + trig[1], y - trig[6] }, C, {0} };
		verts[66] = (SDL_Vertex){ { x + trig[2], y - trig[5] }, C, {0} };
		verts[67] = (SDL_Vertex){ { x + trig[3], y - trig[4] }, C, {0} };
		verts[68] = (SDL_Vertex){ { x + trig[4], y - trig[3] }, C, {0} };
		verts[69] = (SDL_Vertex){ { x + trig[5], y - trig[2] }, C, {0} };
		verts[70] = (SDL_Vertex){ { x + trig[6], y - trig[1] }, C, {0} };
		verts[71] = (SDL_Vertex){ { x + trig[7], y - trig[0] }, C, {0} };
		int indices[216] = { 0, 1, 36, 1, 37, 36, 1, 2, 37, 2, 38, 37, 2, 3, 
			38, 3, 39, 38, 3, 4, 39, 4, 40, 39, 4, 5, 40, 5, 41, 40, 5, 6, 41,
			6, 42, 41, 6, 7, 42, 7, 43, 42, 7, 8, 43, 8, 44, 43, 8, 9, 44, 9, 
			45, 44, 9, 10, 45, 10, 46, 45, 10, 11, 46, 11, 47, 46, 11, 12, 47, 
			12, 48, 47, 12, 13, 48, 13, 49, 48, 13, 14, 49, 14, 50, 49, 14, 15, 
			50, 15, 51, 50, 15, 16, 51, 16, 52, 51, 16, 17, 52, 17, 53, 52, 17, 
			18, 53, 18, 54, 53, 18, 19, 54, 19, 55, 54, 19, 20, 55, 20, 56, 55, 
			20, 21, 56, 21, 57, 56, 21, 22, 57, 22, 58, 57, 22, 23, 58, 23, 59, 
			58, 23, 24, 59, 24, 60, 59, 24, 25, 60, 25, 61, 60, 25, 26, 61, 26, 
			62, 61, 26, 27, 62, 27, 63, 62, 27, 28, 63, 28, 64, 63, 28, 29, 64, 
			29, 65, 64, 29, 30, 65, 30, 66, 65, 30, 31, 66, 31, 67, 66, 31, 32, 
			67, 32, 68, 67, 32, 33, 68, 33, 69, 68, 33, 34, 69, 34, 70, 69, 34, 
			35, 70, 35, 71, 70, 35, 0, 71, 0, 36, 71 };
		SDL_RenderGeometry( R, NULL, verts, 72, indices, 216 );
}
void gp_fill_36circle(SDL_Renderer *R, float x, float y, float radius){
		SDL_FColor C = SDL_GetRender_SDL_FColor( R );
		const float trig [8] = { 0.173648*radius, 0.342020*radius, 0.500000*radius, 0.642788*radius, 0.766044*radius, 0.866025*radius, 0.939693*radius, 0.984808*radius};
		SDL_Vertex verts[36];
		verts[ 0] = (SDL_Vertex){ { x + radius , y           }, C, {0} };
		verts[ 1] = (SDL_Vertex){ { x + trig[7], y + trig[0] }, C, {0} };
		verts[ 2] = (SDL_Vertex){ { x + trig[6], y + trig[1] }, C, {0} };
		verts[ 3] = (SDL_Vertex){ { x + trig[5], y + trig[2] }, C, {0} };
		verts[ 4] = (SDL_Vertex){ { x + trig[4], y + trig[3] }, C, {0} };
		verts[ 5] = (SDL_Vertex){ { x + trig[3], y + trig[4] }, C, {0} };
		verts[ 6] = (SDL_Vertex){ { x + trig[2], y + trig[5] }, C, {0} };
		verts[ 7] = (SDL_Vertex){ { x + trig[1], y + trig[6] }, C, {0} };
		verts[ 8] = (SDL_Vertex){ { x + trig[0], y + trig[7] }, C, {0} };
		verts[ 9] = (SDL_Vertex){ { x          , y + radius  }, C, {0} };
		verts[10] = (SDL_Vertex){ { x - trig[0], y + trig[7] }, C, {0} };
		verts[11] = (SDL_Vertex){ { x - trig[1], y + trig[6] }, C, {0} };
		verts[12] = (SDL_Vertex){ { x - trig[2], y + trig[5] }, C, {0} };
		verts[13] = (SDL_Vertex){ { x - trig[3], y + trig[4] }, C, {0} };
		verts[14] = (SDL_Vertex){ { x - trig[4], y + trig[3] }, C, {0} };
		verts[15] = (SDL_Vertex){ { x - trig[5], y + trig[2] }, C, {0} };
		verts[16] = (SDL_Vertex){ { x - trig[6], y + trig[1] }, C, {0} };
		verts[17] = (SDL_Vertex){ { x - trig[7], y + trig[0] }, C, {0} };
		verts[18] = (SDL_Vertex){ { x - radius , y           }, C, {0} };
		verts[19] = (SDL_Vertex){ { x - trig[7], y - trig[0] }, C, {0} };
		verts[20] = (SDL_Vertex){ { x - trig[6], y - trig[1] }, C, {0} };
		verts[21] = (SDL_Vertex){ { x - trig[5], y - trig[2] }, C, {0} };
		verts[22] = (SDL_Vertex){ { x - trig[4], y - trig[3] }, C, {0} };
		verts[23] = (SDL_Vertex){ { x - trig[3], y - trig[4] }, C, {0} };
		verts[24] = (SDL_Vertex){ { x - trig[2], y - trig[5] }, C, {0} };
		verts[25] = (SDL_Vertex){ { x - trig[1], y - trig[6] }, C, {0} };
		verts[26] = (SDL_Vertex){ { x - trig[0], y - trig[7] }, C, {0} };
		verts[27] = (SDL_Vertex){ { x          , y - radius  }, C, {0} };
		verts[28] = (SDL_Vertex){ { x + trig[0], y - trig[7] }, C, {0} };
		verts[29] = (SDL_Vertex){ { x + trig[1], y - trig[6] }, C, {0} };
		verts[30] = (SDL_Vertex){ { x + trig[2], y - trig[5] }, C, {0} };
		verts[31] = (SDL_Vertex){ { x + trig[3], y - trig[4] }, C, {0} };
		verts[32] = (SDL_Vertex){ { x + trig[4], y - trig[3] }, C, {0} };
		verts[33] = (SDL_Vertex){ { x + trig[5], y - trig[2] }, C, {0} };
		verts[34] = (SDL_Vertex){ { x + trig[6], y - trig[1] }, C, {0} };
		verts[35] = (SDL_Vertex){ { x + trig[7], y - trig[0] }, C, {0} };
		int indices[102] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 
			6, 7, 0, 7, 8, 0, 8, 9, 0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 13,
			0, 13, 14, 0, 14, 15, 0, 15, 16, 0, 16, 17, 0, 17, 18, 0, 18, 19, 
			0, 19, 20, 0, 20, 21, 0, 21, 22, 0, 22, 23, 0, 23, 24, 0, 24, 25, 
			0, 25, 26, 0, 26, 27, 0, 27, 28, 0, 28, 29, 0, 29, 30, 0, 30, 31, 
			0, 31, 32, 0, 32, 33, 0, 33, 34, 0, 34, 35 };
		SDL_RenderGeometry( R, NULL, verts, 36, indices, 102 );
}

void gp_draw_fastcircle(SDL_Renderer *R, float x, float y, float radius){
	if( radius < circle_threshold8 ){
		gp_draw_8circle( R, x, y, radius );
	}
	else if( radius < circle_threshold16 ){
		gp_draw_16circle( R, x, y, radius );
	}
	else if( radius < circle_threshold24 ){
		gp_draw_24circle( R, x, y, radius );
	}
	else {
		gp_draw_36circle( R, x, y, radius );
	}
}
void gp_drawthick_fastcircle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius ){
	if( C_radius < circle_threshold8 ){
		gp_drawthick_8circle( R, x, y, C_radius, L_radius );
	}
	else if( C_radius < circle_threshold16 ){
		gp_drawthick_16circle( R, x, y, C_radius, L_radius );
	}
	else if( C_radius < circle_threshold24 ){
		gp_drawthick_24circle( R, x, y, C_radius, L_radius );
	}
	else {
		gp_drawthick_36circle( R, x, y, C_radius, L_radius );
	}
}
void gp_fill_fastcircle(SDL_Renderer *R, float x, float y, float radius){
	if( radius < circle_threshold8 ){
		gp_fill_8circle( R, x, y, radius );
	}
	else if( radius < circle_threshold16 ){
		gp_fill_16circle( R, x, y, radius );
	}
	else if( radius < circle_threshold24 ){
		gp_fill_24circle( R, x, y, radius );
	}
	else {
		gp_fill_36circle( R, x, y, radius );
	}  
}


void gp_draw_pie(SDL_Renderer *R, float x, float y, float radius, float start, float end ){

	float total = SDL_fabsf(end-start);
	if( total < 0.001 ){
		SDL_RenderLine( R, x, y, x + radius * SDL_cos(start), y + radius * SDL_sin(start) );
		return;
	}
	int res = SDL_ceil( total / 0.1745329252 );
	float dt = total / res;

	//printf("<%g, %d, %g>\n", fabsf(end-start), res, dt );

	float tx = x + radius*SDL_cos(start);
	float ty = y + radius*SDL_sin(start);
	SDL_RenderLine( R, x, y, tx, ty );
	for (int i = 0; i <= res; ++i ){
		float nx = x + radius*SDL_cos(start + i * dt);
		float ny = y + radius*SDL_sin(start + i * dt);
		SDL_RenderLine( R, tx, ty, nx, ny );
		tx = nx;
		ty = ny;
	}
	SDL_RenderLine( R, tx, ty, x, y );
}

/*
	int x = 100;
	clock_t t8 = clock();
	for (int i = 0; i < 1000; ++i ){
		gp_fill_8circle( renderer, x + i, 100+i, i );
		x += 1;
	}
	x = 100;
	clock_t t10 = clock();
	for (int i = 0; i < 1000; ++i ){
		gp_fill_10circle( renderer, x + i, 200+i, i );
		x += 1;
	}
	x = 100;
	clock_t t12 = clock();
	for (int i = 0; i < 1000; ++i ){
		gp_fill_12circle( renderer, x + i, 300+i, i );
		x += 1;
	}
	x = 100;
	clock_t t16 = clock();
	for (int i = 0; i < 1000; ++i ){
		gp_fill_16circle( renderer, x + i, 400+i, i );
		x += 1;
	}
	x = 100;
	clock_t tperf = clock();
	for (int i = 0; i < 1000; ++i ){
		gp_fill_circle( renderer, x + i, 500+i, i );
		x += 1;
	}
	clock_t tfinal = clock();

	printf("%d, %d, %d, %d, %d, %d\n", t8, t10, t12, t16, tperf, tfinal );
	printf("%d, %d, %d, %d, %d\n", t10-t8, t12-t10, t16-t12, tperf-t16, tfinal-tperf );
	//RESULTS
	//258, 260, 261, 264, 269, 789
	//2, 1, 3, 5, 520
*/

SDL_FRect rectify_rect( SDL_FRect *rect ){
    SDL_FRect out = *rect;
    if(rect->w < 0){
        out.x += rect->w;
        out.w *= -1;
    }
    if(rect->h < 0){
        out.y += rect->h;
        out.h *= -1;
    }
    return out;
}

void gp_drawthick_rect( SDL_Renderer *R, SDL_FRect *rect, float thickness ){
    if( thickness <= 1 ){
        SDL_RenderRect( R, rect );
    }
    if( rect->w < 0 || rect->h < 0 ){
        SDL_FRect flipped = rectify_rect( rect );
        gp_drawthick_rect( R, &flipped, thickness );
    }
    else if( rect->w < 2 * thickness || rect->h < 2 * thickness ){
        SDL_RenderFillRect( R, rect );
    }
    else{
        SDL_RenderFillRect( R, &(SDL_FRect){ rect->x, rect->y, rect->w -thickness, thickness } );
        SDL_RenderFillRect( R, &(SDL_FRect){ rect->x + rect->w -thickness, rect->y, thickness, rect->h -thickness } );
        SDL_RenderFillRect( R, &(SDL_FRect){ rect->x + thickness, rect->y + rect->h - thickness, rect->w -thickness, thickness } );
        SDL_RenderFillRect( R, &(SDL_FRect){ rect->x, rect->y + thickness, thickness, rect->h -thickness } );
    }
}

void gp_drawthickNfill_rect( SDL_Renderer *R, SDL_FRect rect, float thickness,
                             SDL_Color border, SDL_Color fill ){

    if( thickness <= 1 ){
        SDL_Color prev = SDL_GetRender_SDL_Color( R );
        SDL_SetRenderDrawColor( R, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect( R, &rect );
        SDL_SetRenderDrawColor( R, border.r, border.g, border.b, border.a);
        SDL_RenderRect( R, &rect );
        SDL_SetRenderDrawColor( R, prev.r, prev.g, prev.b, prev.a);
        return;
    }
    SDL_Color prev = SDL_GetRender_SDL_Color( R );
    SDL_SetRenderDrawColor( R, border.r, border.g, border.b, border.a);
    SDL_RenderFillRect( R, &rect );
    if( rect.w > 2*thickness && rect.h > 2*thickness ){
        rect.x = SDL_ceil( rect.x + thickness );
        rect.y = SDL_ceil( rect.y + thickness );
        rect.w -= 2*thickness;
        rect.h -= 2*thickness;
        SDL_SetRenderDrawColor( R, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect( R, &rect );
    }
    SDL_SetRenderDrawColor( R, prev.r, prev.g, prev.b, prev.a);
}

//9-40-114
//6-28-78
//3-16-42
#define rounded_res 6 //resolution
#define rounded_len 28// 4*(res + 1)
#define rounded_tris 78// 3 * len -6


void gp_draw_roundedRect( SDL_Renderer *R, SDL_FRect *rect, float radius ){

    if( radius < 1 ){
        SDL_RenderRect( R, rect );
        return;
    }
    if( 2*radius > rect->w ) radius = rect->w / 2;
    if( 2*radius > rect->h ) radius = rect->h / 2;

    float lx = rect->x + radius;
    float rx = rect->x + rect->w - radius;
    float ty = rect->y + radius;
    float by = rect->y + rect->h - radius;
    const float trig [5] = { 0.258819*radius, 0.500000*radius, 
                             0.707107*radius, 0.866025*radius, 
                             0.965926*radius};
    SDL_RenderLine( R, rx + radius , by          , rx + trig[4], by + trig[0] );
    SDL_RenderLine( R, rx + trig[4], by + trig[0], rx + trig[3], by + trig[1] );
    SDL_RenderLine( R, rx + trig[3], by + trig[1], rx + trig[2], by + trig[2] );
    SDL_RenderLine( R, rx + trig[2], by + trig[2], rx + trig[1], by + trig[3] );
    SDL_RenderLine( R, rx + trig[1], by + trig[3], rx + trig[0], by + trig[4] );
    SDL_RenderLine( R, rx + trig[0], by + trig[4], rx          , by + radius  );

    SDL_RenderLine( R, rx          , by + radius,  lx          , by + radius  );

    SDL_RenderLine( R, lx          , by + radius , lx - trig[0], by + trig[4] );
    SDL_RenderLine( R, lx - trig[0], by + trig[4], lx - trig[1], by + trig[3] );
    SDL_RenderLine( R, lx - trig[1], by + trig[3], lx - trig[2], by + trig[2] );
    SDL_RenderLine( R, lx - trig[2], by + trig[2], lx - trig[3], by + trig[1] );
    SDL_RenderLine( R, lx - trig[3], by + trig[1], lx - trig[4], by + trig[0] );
    SDL_RenderLine( R, lx - trig[4], by + trig[0], lx - radius , by           );

    SDL_RenderLine( R, lx - radius , by,           lx - radius , ty           );

    SDL_RenderLine( R, lx - radius , ty          , lx - trig[4], ty - trig[0] );
    SDL_RenderLine( R, lx - trig[4], ty - trig[0], lx - trig[3], ty - trig[1] );
    SDL_RenderLine( R, lx - trig[3], ty - trig[1], lx - trig[2], ty - trig[2] );
    SDL_RenderLine( R, lx - trig[2], ty - trig[2], lx - trig[1], ty - trig[3] );
    SDL_RenderLine( R, lx - trig[1], ty - trig[3], lx - trig[0], ty - trig[4] );
    SDL_RenderLine( R, lx - trig[0], ty - trig[4], lx          , ty - radius  );

    SDL_RenderLine( R, lx          , ty - radius,  rx          , ty - radius  );

    SDL_RenderLine( R, rx          , ty - radius , rx + trig[0], ty - trig[4] );
    SDL_RenderLine( R, rx + trig[0], ty - trig[4], rx + trig[1], ty - trig[3] );
    SDL_RenderLine( R, rx + trig[1], ty - trig[3], rx + trig[2], ty - trig[2] );
    SDL_RenderLine( R, rx + trig[2], ty - trig[2], rx + trig[3], ty - trig[1] );
    SDL_RenderLine( R, rx + trig[3], ty - trig[1], rx + trig[4], ty - trig[0] );
    SDL_RenderLine( R, rx + trig[4], ty - trig[0], rx + radius , ty           );

    SDL_RenderLine( R, rx + radius , ty          , rx + radius , by           );

}

void gp_drawthickNfill_roundedRect( SDL_Renderer *R, SDL_FRect rect, float radius, float thickness,
                               SDL_Color border, SDL_Color fill ){
    
    //printf( "%d, %d, %d, %d\n", rect.x, rect.y, rect.w, rect.h );
    if( rect.w < 0 || rect.h < 0 ){
        SDL_FRect flipped = rectify_rect( &rect );
        gp_drawthickNfill_roundedRect( R, flipped, radius, thickness, border, fill );
        return;
    }
    if( radius < 1 ){
        gp_drawthickNfill_rect( R, rect, thickness, border, fill );
        return;
    }
    if( thickness <= 1 ){
        SDL_Color prev = SDL_GetRender_SDL_Color( R );
        SDL_SetRenderDrawColor( R, fill.r, fill.g, fill.b, fill.a);
        gp_fill_roundedRect( R, &rect, radius );
        SDL_SetRenderDrawColor( R, border.r, border.g, border.b, border.a);
        gp_draw_roundedRect( R, &rect, radius );
        SDL_SetRenderDrawColor( R, prev.r, prev.g, prev.b, prev.a);
        return;
    }
    SDL_Color prev = SDL_GetRender_SDL_Color( R );
    SDL_SetRenderDrawColor( R, border.r, border.g, border.b, border.a);
    gp_fill_roundedRect( R, &rect, radius );
    if( rect.w > 2*thickness && rect.h > 2*thickness ){
        rect.x = SDL_ceil( rect.x + thickness );
        rect.y = SDL_ceil( rect.y + thickness );
        rect.w -= 2*thickness;
        rect.h -= 2*thickness;
        SDL_SetRenderDrawColor( R, fill.r, fill.g, fill.b, fill.a);
        if( radius > thickness ){
            gp_fill_roundedRect( R, &rect, radius-thickness );
        }else{
            SDL_RenderFillRect( R, &rect );
        }
    }
    SDL_SetRenderDrawColor( R, prev.r, prev.g, prev.b, prev.a);
}

void gp_fill_roundedRect( SDL_Renderer *R, SDL_FRect *rect, float radius ){

	if( 2 * radius > rect->w ) radius = rect->w * 0.5;
	if( 2 * radius > rect->h ) radius = rect->h * 0.5;

	SDL_FColor C = SDL_GetRender_SDL_FColor( R );

	SDL_Vertex verts[ rounded_len ];
	double theta = HALF_PI / rounded_res;

	float  xpr = rect->x + radius;
	float xwmr = rect->x + rect->w - radius;
	float  ypr = rect->y + radius;
	float yhmr = rect->y + rect->h - radius;

	for(int i = 0; i <= rounded_res; i++){
		
		double a = i * theta;
		double cosi = SDL_cos( a ) * radius;
		double sine = SDL_sin( a ) * radius;

		verts[  rounded_res   - i] = (SDL_Vertex){ { xwmr + cosi, ypr  - sine }, C, {0} };
		verts[  rounded_res+1 + i] = (SDL_Vertex){ { xwmr + cosi, yhmr + sine }, C, {0} };
		verts[3*rounded_res+2 - i] = (SDL_Vertex){ { xpr  - cosi, yhmr + sine }, C, {0} };
		verts[3*rounded_res+3 + i] = (SDL_Vertex){ { xpr  - cosi, ypr  - sine }, C, {0} };
	}

	int indices [ rounded_tris ];
	for(int i = 0; i < rounded_len-2; i++){
		indices[ 3*i   ] = 0;
		indices[ 3*i+1 ] = i+1;
		indices[ 3*i+2 ] = i+2;
	}
	//int out = 
	SDL_RenderGeometry( R, NULL, verts, rounded_len, indices, rounded_tris );

	//if( out < 0 ){
	//    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_RenderGeometry: %s", SDL_GetError());
	//}

	//return out;
}

void gp_drawthick_roundedRect( SDL_Renderer *R, SDL_FRect *rect, float radius, float thickness ){}




void gp_draw_bevelrect_bltr(SDL_Renderer *R, SDL_FRect *rect, float bevel) {
    // Top edge
    SDL_RenderLine( R, rect->x,                   rect->y, 
    	               rect->x + rect->w - bevel, rect->y );
    // Top-right bevel
    SDL_RenderLine( R, rect->x + rect->w - bevel, rect->y, 
                       rect->x + rect->w,         rect->y + bevel );
    // Right edge
    SDL_RenderLine(R, rect->x + rect->w, rect->y + bevel, 
                      rect->x + rect->w, rect->y + rect->h );
    // Bottom edge
    SDL_RenderLine(R, rect->x + rect->w, rect->y + rect->h, 
                      rect->x + bevel,   rect->y + rect->h );  
    // Bottom-left bevel
    SDL_RenderLine(R, rect->x + bevel, rect->y + rect->h, 
                      rect->x,         rect->y + rect->h - bevel );
    // Left edge
    SDL_RenderLine(R, rect->x, rect->y + rect->h - bevel, 
                      rect->x, rect->y                   );
}

void gp_drawthick_bevelrect_bltr( SDL_Renderer *R, SDL_FRect *rect, float bevel, float thickness ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );

	//double m = (2 - SQRT2) * thickness;
	double off = (SQRT2 - 1) * thickness;
	
	SDL_Vertex verts[12];
	verts[ 0] = (SDL_Vertex){ { rect->x,                   rect->y                   }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { rect->x + rect->w - bevel, rect->y                   }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { rect->x + rect->w,         rect->y + bevel           }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { rect->x + rect->w,         rect->y + rect->h         }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { rect->x + bevel,           rect->y + rect->h         }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { rect->x,                   rect->y + rect->h - bevel }, C, {0} };

	verts[ 6] = (SDL_Vertex){ { verts[0].position.x + thickness, verts[0].position.y + thickness }, C, {0} };
	verts[ 7] = (SDL_Vertex){ { verts[1].position.x - off      , verts[1].position.y + thickness }, C, {0} };
	verts[ 8] = (SDL_Vertex){ { verts[2].position.x - thickness, verts[2].position.y + off       }, C, {0} };
	verts[ 9] = (SDL_Vertex){ { verts[3].position.x - thickness, verts[3].position.y - thickness }, C, {0} };
	verts[10] = (SDL_Vertex){ { verts[4].position.x + off      , verts[4].position.y - thickness }, C, {0} };
	verts[11] = (SDL_Vertex){ { verts[5].position.x + thickness, verts[5].position.y - off       }, C, {0} };

	int indices[36] = { 0, 1, 6,   1, 6, 7,
	                    1, 2, 7,   2, 7, 8,
	                    2, 3, 8,   3, 8, 9,
	                    3, 4, 9,   4, 9,10,
	                    4, 5,10,   5,10,11,
	                    5, 0,11,   0,11, 6 };

	SDL_RenderGeometry( R, NULL, verts, 12, indices, 36 );
}

void gp_fill_bevelrect_bltr( SDL_Renderer *R, SDL_FRect *rect, float bevel ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	SDL_Vertex verts[6];
	verts[ 0] = (SDL_Vertex){ { rect->x,                   rect->y                   }, C, {0} };
	verts[ 1] = (SDL_Vertex){ { rect->x + rect->w - bevel, rect->y                   }, C, {0} };
	verts[ 2] = (SDL_Vertex){ { rect->x + rect->w,         rect->y + bevel           }, C, {0} };
	verts[ 3] = (SDL_Vertex){ { rect->x + rect->w,         rect->y + rect->h         }, C, {0} };
	verts[ 4] = (SDL_Vertex){ { rect->x + bevel,           rect->y + rect->h         }, C, {0} };
	verts[ 5] = (SDL_Vertex){ { rect->x,                   rect->y + rect->h - bevel }, C, {0} };
	int indices[12] = { 0, 1, 2,  0, 2, 3,  0, 3, 4,  0, 4, 5 };
	SDL_RenderGeometry( R, NULL, verts, 6, indices, 12 );
}




void gp_draw_poly( SDL_Renderer *R, vec2d *verts, int verts_count, bool close ){
	int n = verts_count - 1;
	for (int i = 0; i < n; ++i) SDL_RenderLine( R, verts[i].x, verts[i].y, verts[i+1].x, verts[i+1].y );
	if( close ) SDL_RenderLine( R, verts[n].x, verts[n].y, verts[0].x, verts[0].y );
}

void gp_drawthick_roundedPoly( SDL_Renderer *R, vec2d *verts, int verts_count, float radius ){
	for (int i = 0; i < verts_count-1; ++i ){
		gp_drawthick_line( R, verts[i].x, verts[i].y, verts[i+1].x, verts[i+1].y, radius );
		gp_fill_fastcircle( R, verts[i].x, verts[i].y, radius );
	}
	int i = verts_count-1;
	gp_drawthick_line( R, verts[i].x, verts[i].y, verts[0].x, verts[0].y, radius );
	gp_fill_fastcircle( R, verts[i].x, verts[i].y, radius );
}



void gp_fill_poly( SDL_Renderer *R, vec2d *verts, int verts_count ){
	SDL_FColor C = SDL_GetRender_SDL_FColor( R );
	SDL_Vertex *sdlverts = SDL_malloc( verts_count * sizeof(SDL_Vertex) );
	for (int i = 0; i < verts_count; ++i ){
		 sdlverts[i] = (SDL_Vertex){ { verts[i].x, verts[i].y }, C, {0} };
	}
	int N = 3*(verts_count-2);
	int *indices = SDL_malloc( N * sizeof(int) );
	for(int i = 0; i < verts_count-2; i++){
		indices[ 3*i   ] = 0;
		indices[ 3*i+1 ] = i+1;
		indices[ 3*i+2 ] = i+2;
	}
	SDL_RenderGeometry( R, NULL, sdlverts, verts_count, indices, N );
	SDL_free( sdlverts );
	SDL_free( indices );
}