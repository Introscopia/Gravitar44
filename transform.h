#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include "basics.h"
#include "vec2d.h"
#include "Chipmunk/headers/chipmunk.h"

typedef struct {
    double a, b, c;
    double d, e, f;
} Mat23;

typedef struct {
	double tx, ty;   // translate
    float cx, cy;    // center, a secondary translate on top of the scale
    float s, invs;   // scale, inverse scale
    Mat23 M;
} Transform;

typedef vec2d (*m_update_func) ( Transform *T, double d1, double d2, double d3, double d4 );
vec2d                update_TM( Transform *T, double unused1,     double unused2,   double unused3, double unused4 );
vec2d update_TM_object_rotated( Transform *T, double unused,      double obj_angle, double obj_x, double obj_y );
vec2d  update_TM_world_rotated( Transform *T, double world_angle, double unused1, double unused2, double unused3 );
vec2d       update_TM_combined( Transform *T, double world_angle, double obj_angle, double obj_x, double obj_y );

void set_scale( Transform *T, float s );

static inline float atfX( float x, Transform T ){
    return T.cx + ( T.s * (x - T.tx) );
}
static inline float atfY( float y, Transform T ){
    return T.cy + ( T.s * (y - T.ty) );
}
static inline float rtfX( float x, Transform T ){
    return ((x - T.cx) * T.invs) + T.tx;
}
static inline float rtfY( float y, Transform T ){
    return ((y - T.cy) * T.invs) + T.ty;
}

vec2d apply_transform_v2d( vec2d *vec, Transform *T );
vec2d reverse_transform_v2d( vec2d *vec, Transform *T );

cpVect apply_transform_cpv( cpVect vec, Transform *T );
cpVect reverse_transform_cpv( cpVect vec, Transform *T );

SDL_FPoint apply_transform_fp( SDL_FPoint p, Transform *T );
SDL_FPoint reverse_transform_fp( SDL_FPoint p, Transform *T );

SDL_Rect apply_transform_rect( SDL_Rect *rct, Transform *T );
SDL_Rect reverse_transform_rect( SDL_Rect *rct, Transform *T );

SDL_FRect apply_transform_frect( SDL_FRect *rct, Transform *T );
SDL_FRect reverse_transform_frect( SDL_FRect *rct, Transform *T );

void constrain_Transform( Transform *T, SDL_FRect window_rct, SDL_FRect bounds );


vec2d apply_Mat23_v2d( vec2d world, const Mat23 *M );

#define TM_APPLY_TO(out, in, M)                               \
    do {                                                      \
        double tx = (M).a * (in).x + (M).b * (in).y + (M).c;  \
        double ty = (M).d * (in).x + (M).e * (in).y + (M).f;  \
        (out).x = tx;                                         \
        (out).y = ty;                                         \
    } while (0)

#define TM_APPLY(type, in, M)                          \
    ((type){                                           \
        .x = (M).a * (in).x + (M).b * (in).y + (M).c,  \
        .y = (M).d * (in).x + (M).e * (in).y + (M).f   \
    })

vec2d reverse_Mat23_v2d( vec2d screen, const Mat23 *M );


#endif