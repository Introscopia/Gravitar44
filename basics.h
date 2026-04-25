#ifndef BASICS_H_INCLUDED
#define BASICS_H_INCLUDED

#include <SDL.h>


#define PHI           1.6180339887498948482045868343656381177203091798057628621
#define SQRT2         1.4142135623730950488016887242096980785696718753769480731
#define SQRT2O2       0.7071067811865475244008443621048490392848359376884740365
#define SQRT3         1.7320508075688772935274463415058723669428052538103806280
#define EULER		    2.7182818284590452353602874713526624977572470936999595749

#define TWO_PI        6.2831853071795864769252867665590057683943387987502116419
#define PI            3.1415926535897932384626433832795028841971693993751058209
#define TWO_THIRDS_PI 2.0943951023931954923084289221863352561314462662500705473
#define HALF_PI       1.5707963267948966192313216916397514420985846996875529104
#define THIRD_PI      1.0471975511965977461542144610931676280657231331250352736
#define QUARTER_PI    0.7853981633974483096156608458198757210492923498437764552
#define FIFTH_PI      0.6283185307179586476925286766559005768394338798750211641
#define SIXTH_PI      0.5235987755982988730771072305465838140328615665625176368
#define EIGTH_PI      0.3926990816987241548078304229099378605246461749218882276
#define TWELFTH_PI    0.2617993877991494365385536152732919070164307832812588184
#define ONE_OVER_PI   0.3183098861837906715377675267450287240689192914809128974

#define ONE_OVER_255  0.0039215686274509803921568627450980392156862745098039215

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   #define rmask 0xff000000
   #define gmask 0x00ff0000
   #define bmask 0x0000ff00
   #define amask 0x000000ff
#else
   #define rmask 0x000000ff
   #define gmask 0x0000ff00
   #define bmask 0x00ff0000
   #define amask 0xff000000
#endif


typedef void (*voidptr_func)( void *vptr );

// COLOR, DRAWING

Uint8   red( Uint32 color );
Uint8 green( Uint32 color );
Uint8  blue( Uint32 color );
Uint8 alpha( Uint32 color );

Uint8 brightness( Uint32 color );

Uint32 rgba_to_Uint32( Uint8 r, Uint8 g, Uint8 b, Uint8 a );
Uint32 SDL_Color_to_Uint32( SDL_Color C );
SDL_Color Uint32_to_SDL_Color( Uint32 C );

int SDL_SetRenderDraw_SDL_Color( SDL_Renderer *R, SDL_Color C );
int SDL_SetRenderDraw_Uint32( SDL_Renderer *R, Uint32 C );
SDL_Color SDL_GetRender_SDL_Color( SDL_Renderer *R );
SDL_FColor SDL_GetRender_SDL_FColor( SDL_Renderer *R );

Uint32 lerp_color( Uint32 CA, Uint32 CB, float amt );
SDL_Color lerp_SDL_Color( SDL_Color CA, SDL_Color CB, float amt );

void SDL_Color_sub( SDL_Color *C, Uint8 r, Uint8 g, Uint8 b, Uint8 a );

//returns the cubehelix color for the hue value t (0~1)
Uint32 cubehelix(float t);


void now_string( char *buf, int precision );
void save_screenshot( SDL_Renderer *renderer, char *prefix, SDL_Rect *rct );
void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);

void RenderCopyMasked( SDL_Renderer *R, SDL_Texture *subject, SDL_Texture *mask, SDL_Texture *target );
void fade_Texture( SDL_Renderer *R, SDL_Texture *T, Uint8 alpha );

// NUMBERS, MATH ---------------------------------------------------------------------------


#define xy(V) V.x, V.y

#define xyp(V) V->x, V->y

#define xywh(R) R.x, R.y, R.w, R.h

#define RGBA(C) C.r, C.g, C.b, C.a



double sq( double a );
double logarithm( double base, double x );

// gets the smaller of each pair of divisors, excluding 1, and puts them in the list you provide, 
// returns the number of items it put on the list.
int get_divisors( int *list, int N );

// RANDOM: "array rules" min inclusive, max not inclusive
int randomI( int min, int max );
float randomF( float min, float max );
float random_angle();
int random_from_list( int n, ... );
double random_gaussian();
double fast_gaussian();
void shuffle( int *deck, int len );
int random_linear_asc(int min, int max);
int random_linear_desc(int min, int max);
//https://www.pcg-random.org/
typedef struct { uint64_t state;  uint64_t inc; } PCG_RNG;
uint32_t pcg_random( PCG_RNG* rng );

double lerp(double start, double stop, double amt);

double /*linear*/map(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double ellipticalMap(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double    sigmoidMap(double value, double source_lo, double source_hi, double dest_lo, double dest_hi);
double advSigmoidMap(double value, double source_lo, double source_hi, double Slo, double Shi, double dest_lo, double dest_hi);
double easeInOutQuad( double x );
double QuadSigmoidMap(double value, double source_lo, double source_hi,  double dest_lo, double dest_hi);

int cycle( int a, int min, int max );
int constrain( int a, int min, int max );
float constrainF( float a, float min, float max );
double constrainD( double a, double min, double max );

int count_set_bits( unsigned int v );

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

double minD( double a, double b );
double maxD( double a, double b );

double degrees( double radians );
double radians( double degrees );

double rectify_angle( double a );
double angle_diff( double a, double b );


// CHAR & STRING ---------------------------------------------------------------------------

// String split. eats the original string!
//void strspl( char *string, const char *delimiters, char ***list, int *size );
void split( char *string, char *separator, char ***list, int *list_len );
// String Count character
int strcchr( char *string, char C );
// sub-string. allocates a new char*. start inclusive, stop not-inclusivve.
char *substr( char *string, int start, int stop );
// returns the index of the first char that does not match, or the length of the shortest string.
int str_match( char *A, char *B ); 
// insert char at position. returns whether it fit into the size or not
bool str_insert_char( char *string, char C, int pos, int size );
void str_insert_str( char *string, char *in, int pos );
void str_delete_char( char *string, int pos, int len );


typedef struct stringbuilder{

   char *str;
   int len;
   int cap;

} STRB;

void STRB_init( STRB *S, int sz );
void STRB_ensure( STRB *S, int len );
void STRB_reset( STRB *S, int sz );
void STRB_justify( STRB *S );
void STRB_copy( STRB *S, char *str );

void STRB_append_char( STRB *S, char c );
void STRB_append_utf8( STRB *S, uint32_t c, int endianness );
void STRB_append_str( STRB *S, char *str );

void STRB_insert_char( STRB *S, char c, int pos );
void STRB_insert_utf8( STRB *S, uint32_t c, int endianness, int pos );
void STRB_insert_str( STRB *S, char *str, int pos );

void STRB_delete( STRB *S, int pos );
void STRB_delete_range( STRB *S, int start, int stop );

char STRB_event_handler( STRB *S, int *cursor, SDL_Event *event );

// CON is 1 for RIGHT, -1 for LEFT, else it is the SDL_Keycode of the key...
// if CON is 0, we expect KEY to contain a valid char.
void STRB_control( STRB *S, int *cursor, int CON, int KEY );


void insert_sorted( int* list, int *len, int N );
void delete_repack( int* list, int *len, int i );

void Lshift_str( char *str, int n );
void strtrim( char *string );
void strtrim_fgetsd_str( char *string );

//char getgc( SDL_IOStream *f );
//int lines_in_a_file( SDL_IOStream* f );
bool fseek_lines( SDL_IOStream* f, int N );
bool fseek_string( SDL_IOStream *f, char *str );
void fspy( SDL_IOStream *f );
bool fseek_string_before( SDL_IOStream *f, char *str, char *terminator );
bool fseek_ABC( SDL_IOStream *f, char *A, char *B, char *C, Sint64 locations [3] );
bool fseek_category( SDL_IOStream *f, int(*iscat)(int c) );
bool fseek_str_before_category( SDL_IOStream *f, char *str, int(*iscat)(int c) );
bool fseek_str_before_notcategory( SDL_IOStream *f, char *str, int(*iscat)(int c) );
void fskip_whitespace( SDL_IOStream *f );

// returns ptr to the first address in str after the first instance of c.
char* sseek_char( char *str, char c );

double fscan_double(SDL_IOStream* f);

// FSCAN alternatives... they all return the cursor to the position you send it at!
//looks ahead up to lookahead, or 512 max. uses the stack.
int fscanfIO_til512( SDL_IOStream *f, int lookahead, char *fmt, ... );
//looks ahead up to lookahead, uses the heap.
int fscanfIO_tilN( SDL_IOStream *f, int lookahead, char *fmt, ... );
//looks ahead up to "terminator", uses the heap.
int fscanfIO_tilT( SDL_IOStream *f, char *terminator, char *fmt, ... );

void fscan_str_until( SDL_IOStream *f, char *dest, int size, char *terminator );
STRB fscan_STRB_until( SDL_IOStream *f, char *terminator );

// expects an initialized STRB. returns success. does not reset dest under any circumstance.
bool fscan_STRBptr_until( SDL_IOStream *f, STRB *dest, char *terminator );

void fscan_str_until_any( SDL_IOStream *f, char *dest, int size, char *terminators );
void fscan_preceding_word(SDL_IOStream *f, char buffer [64]);


int sscan_trailing_int( char *str );
//int f_count_char_until( SDL_IOStream *f, char it, char *terminator );
//
////fscan a comma-separated list of strings (or any other separator char)
////length of output list in placed into n (optional, just send NULL elsewise)
////must free output[0] (which contains all the chars), and then output itself (which is the list)
//char **fscan_cslist( SDL_IOStream *f, int *n, char separator );
//
//void fgets_but_good( SDL_IOStream *f, char *dest, int size );
//
char* load_file_as_str( char *filename );

struct tag_data{
   Sint64 *locations;
   int *indices;
   int length;
};
void free_tag_data( struct tag_data *td );
// tags at indices <= stopper halt the search.
struct tag_data tag_finder( SDL_IOStream *f, const char tags[][32], int length, int stopper );

void find_n_replace_nth_char(char *str, char F, int N, char R);
void rev_find_n_replace_nth_char(char *str, char F, int N, char R);

//void get_filenames( char *directory, char ***list, int *length );

// just replaces the last '/' with a '\0'
void up_one_folder( char *path );

Uint16 *ascii_to_unicode( char *str );

bool cursor_in_rect( SDL_Event *event, SDL_Rect *R );
bool coordinates_in_Rect( float x, float y, SDL_Rect R );
bool coordinates_in_FRect( float x, float y, SDL_FRect R );
bool SDL_Rect_overlap( SDL_Rect *A, SDL_Rect *B );
bool SDL_FRect_overlap( SDL_FRect *A, SDL_FRect *B );
bool rect_overlap( int Ax, int Ay, int Aw, int Ah, int Bx, int By, int Bw, int Bh );
bool intersecting_or_touching( SDL_Rect *A, SDL_Rect *B);
SDL_Rect add_rects( SDL_Rect *A, SDL_Rect *B);
//scale and translate A to fit inside B, centralized
void fit_rect( SDL_Rect *A, SDL_Rect *B );
void fit_frect( SDL_FRect *A, SDL_FRect *B );

void constrain_frect( SDL_FRect *A, const SDL_FRect B );


Uint32 char4_to_int( char str [4] );
void int_to_char4( Uint32 N, char str [4] );

int count_digits( int n );

bool i_check( char c );
bool d_check( char c );
bool x_check( char c );
bool f_check( char c );

int bytes_in_a_utf_codepoint( uint8_t ch );
int retrobytes_in_a_utf_codepoint( const char *str );
uint32_t binary_code_point( int bytes, uint32_t key );
size_t utf8_strlen(const char *s);
uint32_t UTF8_to_UINT32( char *str, int *bytes, int endianness );
//returns the number of bytes written into str.
int UINT32_to_UTF8( char *str, uint32_t num, int endianness );

bool str_contains( char *str, bool(*cateorize)(char c) );
bool str_contains_only( char *str, bool(*cateorize)(char c) );

bool list_contains( int *list, int len, int x );
int find_in_list( int *list, int len, int x );

char shifted_keys( char c );

/*
void print_string_with_escape_chars_visible( char *str );
void hard_print( char *str, int N );
void hard_print_f( FILE *f, int N );
//printf("{%d, %d, %d, %d}\n", rect.x, rect.y, rect.w, rect.h );
*/

//return elapsed time
int SDL_framerateDelay( int frame_period );
void SDL_framerate_limit_n_monitor(SDL_Renderer *R, int frame_period);

typedef struct {
    int i, j;
} index2d;

bool i2d_equals( index2d A, index2d B );
int i2d_manhattan( index2d A, index2d B );


int rect_area( SDL_Rect *r );

typedef struct{
   int len, size;
   SDL_Rect *rcts;
   SDL_Rect original;
} rectCluster;

void rectCluster_init( rectCluster *rC, int x, int y, int w, int h );

void clip_rectCluster( rectCluster *rC, SDL_Rect cut );

int rectCluster_area( rectCluster *rC );


#endif

/*
☺☻♥♦♣
♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼ !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»░▒▓│┤ÁÂÀ©╣║╗╝¢¥┐└┴┬├─┼ãÃ╚╔╩╦╠═╬¤ðÐÊËÈıÍÎÏ┘┌█▄¦Ì▀ÓßÔÒõÕµþÞÚÛÙýÝ¯´­±‗¾¶§÷¸°¨·¹³²
*/