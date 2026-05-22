#ifndef VCT_TEXT_H_INCLUDED
#define VCT_TEXT_H_INCLUDED

#include <SDL.h>

#define CHAR_WIDTH 8
#define LINE_HEIGHT 12
#define TAB_SIZE 4

#define MAX_PATH_LEN 32

enum{ VCT_ALIGN_LEFT,
	  VCT_ALIGN_CENTER,
	  VCT_ALIGN_RIGHT,
	  VCT_JUSTIFY };

typedef struct glyph_struct{

	SDL_FPoint *verts;
	int *offsets;
	int path_count;
	int adv;

} Glyph;

typedef struct vfont_struct{

	Glyph ascii [96];
	Glyph *unicode;
	Uint32 *code_points;
	int unicode_count;
	bool monospaced;
	float scale;
	float advance;
	float text_h;
	float line_height;
	float space;
	float cw; // current (scaled) charater width

} VFont;

VFont load_VFont( char *filename );

void destroy_VFont( VFont *font );

void VCT_render_char( SDL_Renderer *R, VFont *font, char C, float x, float y );

void VCT_Text_draw_cursor( SDL_Renderer *R, VFont *font, char *str, int tx, int ty, int cursor );

void VCT_render_string( SDL_Renderer *R, VFont *font, char *string, float x, float y );

void VCT_render_section( SDL_Renderer *R, VFont *font, char *string, int start, int end, float x, float y );

//returns the index of where the present line ends.
int VCT_wrap_line( VFont *font, char *string, int start, float width, float *line_width );

//returns number of lines
int VCT_render_string_wrapped( SDL_Renderer *R, VFont *font, char *string, float x, float y, float width );

void VCT_render_string_wrapped_aligned( SDL_Renderer *R, VFont *font, char *string, int x, int y, int width, int alignment );

void VCT_SizeText( VFont *font, char *string, float *w, float *h );

//returns number of lines
int VCT_WriteIO_string_wrapped( SDL_IOStream *f, VFont *font, char *string, float x, float y, float width, float height );


Glyph VCT_consolidate_string( VFont *font, char *string );

typedef struct vec3d_struct {
	float x, y, z;
} vec3d;

typedef struct paths3d_struct {
	int path_count;
	int *offsets; // Reference to it's parent consolidated Glyph
	vec3d *verts;
} Paths3D;

Paths3D Paths3D_from_consolidated_glyph( Glyph *consolidated );

void VCT_project_string_on_a_Ball( Paths3D *P, Glyph *consolidated, float text_h,
								   float rad, float xhead, float xarc, 
	                               float yhead, float yarc, float zoff );

void VCT_project_string_on_a_Ball_w_radii( Paths3D *P, VFont *font, char *string, 
										   float *radii, float xhead, float xarc, 
	                                       float yhead, float yarc, float zoff );

void draw_Paths3D( SDL_Renderer *R, Paths3D *P, float cx, float cy );

#endif