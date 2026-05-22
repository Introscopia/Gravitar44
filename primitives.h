#ifndef PRIMITIVES_H_INCLUDED
#define PRIMITIVES_H_INCLUDED

#include <SDL.h>
#include "vec2d.h"

void gp_crosshair( SDL_Renderer *R, float x, float y, float rad );

void gp_drawthick_line( SDL_Renderer *R, float ax, float ay, float bx, float by, float thickness );
void gp_drawthick_roundedLine( SDL_Renderer *R, float ax, float ay, float bx, float by, float radius );
void gp_draw_arrow( SDL_Renderer *R, float ax, float ay, float bx, float by, float LT, float HB, float HH );
																//line thickness, head base, head height
void gp_draw_bezier1( SDL_Renderer *renderer, vec2d *a1, vec2d *a2, vec2d *c, int res );
void gp_draw_bezier2( SDL_Renderer *renderer, vec2d *a1, vec2d *a2, vec2d *c1, vec2d *c2, int res );

void gp_fill_diamond(SDL_Renderer *R, float x, float y, float radius);
                                                              
void gp_draw_circle( SDL_Renderer *R, float x, float y, float radius );
void gp_fill_circle( SDL_Renderer *R, float x, float y, float radius );

void gp_draw_8circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_8circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius );
void gp_fill_8circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_12circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_16circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_16circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius );
void gp_fill_16circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_24circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_24circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius );
void gp_fill_24circle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_36circle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_36circle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius );
void gp_fill_36circle(SDL_Renderer *R, float x, float y, float radius);

#define circle_threshold8 8
#define circle_threshold16 50
#define circle_threshold24 180

void gp_draw_fastcircle(SDL_Renderer *R, float x, float y, float radius);
void gp_drawthick_fastcircle(SDL_Renderer *R, float x, float y, float C_radius, float L_radius );
void gp_fill_fastcircle(SDL_Renderer *R, float x, float y, float radius);

void gp_draw_pie(SDL_Renderer *R, float x, float y, float radius, float start, float end );

//fixes rects with negative dimensions
SDL_FRect rectify_rect( SDL_FRect *rect );
// positive thickness draws the border inside the rect. negative thickness for outside also works!
void gp_drawthick_rect( SDL_Renderer *R, SDL_FRect *rect, float thickness );
void gp_drawthickNfill_rect( SDL_Renderer *R, SDL_FRect rect, float thickness,
                             SDL_Color border, SDL_Color fill );

void gp_draw_roundedRect( SDL_Renderer *R, SDL_FRect *rect, float radius );
void gp_drawthickNfill_roundedRect( SDL_Renderer *R, SDL_FRect rect, float radius, float thickness,
                            		SDL_Color border, SDL_Color fill );
void gp_fill_roundedRect( SDL_Renderer *R, SDL_FRect *rect, float radius );

// Rects whose Bottom-Left and Top-Right corners are beveled at a 45 degree angle.
void gp_draw_bevelrect_bltr(SDL_Renderer *R, SDL_FRect *rect, float bevel);
void gp_drawthick_bevelrect_bltr( SDL_Renderer *R, SDL_FRect *rect, float bevel, float thickness );
void gp_fill_bevelrect_bltr( SDL_Renderer *R, SDL_FRect *rect, float bevel );

void gp_draw_poly( SDL_Renderer *R, vec2d *verts, int verts_count, bool close );
void gp_drawthick_roundedPoly( SDL_Renderer *R, vec2d *verts, int verts_count, float thickness );
void gp_drawthick_sharpPoly( SDL_Renderer *R, vec2d *verts, int verts_count, float thickness );
void gp_fill_poly( SDL_Renderer *R, vec2d *verts, int verts_count );



#endif