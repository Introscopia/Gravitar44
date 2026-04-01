#ifndef VEC2D_H
#define VEC2D_H

#include <SDL.h>

typedef struct vec2d_struct{
    double x, y;
} vec2d;


static const vec2d v2dzero = { 0.0, 0.0 };

/// Convenience constructor for vec2d structs.
static inline vec2d v2d(double x, double y){
    vec2d v = {x, y};
    return v;
}

/// Add two vectors
static inline vec2d v2d_sum(vec2d a, vec2d b){
    return v2d(a.x + b.x, a.y + b.y);
}

static void v2d_add( vec2d *a, vec2d b ){
    a->x += b.x;
    a->y += b.y;
}

/// Subtract two vectors.
static inline vec2d v2d_diff(vec2d a, vec2d b){
    return v2d(a.x - b.x, a.y - b.y);
}

static void v2d_sub(vec2d *a, vec2d b){
    a->x -= b.x;
    a->y -= b.y;
}

/// Negate a vector.
static inline vec2d v2d_neg(vec2d v){
    return v2d(-v.x, -v.y);
}

/// Scalar multiplication.
static inline vec2d v2d_product(vec2d v, double S){
    return v2d(v.x * S, v.y * S);
}

static void v2d_mult( vec2d *a, double S ){
    a->x *= S;
    a->y *= S;
}

/// Vector dot product.
static inline double v2d_dot(vec2d a, vec2d b){
    return a.x * b.x + a.y * b.y;
}
/// determinant.
static inline double v2d_det(vec2d a, vec2d b){
    return a.x * b.y - a.y * b.x;
}

/// 2D vector cross product analog.
/// The cross product of 2D vectors results in a 3D vector with only a z component.
/// This function returns the magnitude of the z value.
static inline double v2d_cross(vec2d a, vec2d b){
    return a.x * b.y - a.y * b.x;
}

/// Returns the squared magnitude of v. Faster than v2d_mag() when you only need to compare lengths.
static inline double v2d_magsq(vec2d v){
    return v2d_dot(v, v);
}

/// Returns the magnitude of v.
static inline double v2d_mag(vec2d v){
    return SDL_sqrt(v2d_dot(v, v));
}

/// Returns a normalized copy of v.
static inline vec2d v2d_normalize(vec2d v){
    return v2d_product( v, 1.0f / (v2d_mag(v) + SDL_FLT_EPSILON) );
}

static inline vec2d v2d_setlen(vec2d v, double len){
    return v2d_product( v2d_normalize(v), len );
}

/// Linearly interpolate between a and b.
static inline vec2d v2d_lerp(vec2d a, vec2d b, double t){
    return v2d_sum( v2d_product(a, 1.0f - t), v2d_product(b, t) );
    //return (vec2d){a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

/// Returns the distance between a and b.
static inline double v2d_dist(vec2d a, vec2d b){
    return v2d_mag( v2d_diff(a, b) );
}

/// Returns the squared distance between a and b. Faster than v2d_dist() when you only need to compare distances.
static inline double v2d_distsq(vec2d a, vec2d b){
    return v2d_magsq( v2d_diff(a, b) );
}

/// Returns a perpendicular vector. (90 degree rotation)
static inline vec2d v2d_perp(vec2d v){
    return v2d(-v.y, v.x);
}

/// Returns a perpendicular vector. (-90 degree rotation)
static inline vec2d v2d_rperp(vec2d v){
    return v2d(v.y, -v.x);
}

/// Returns the vector projection of a onto b.
static inline vec2d v2d_project(vec2d a, vec2d b){
    return v2d_product(b, v2d_dot(a, b)/v2d_dot(b, b));
}

/// Returns the angular direction v is pointing in (in radians).
static inline double v2d_heading(vec2d v){
    return SDL_atan2(v.y, v.x);
}

static inline vec2d v2d_trig( double angle ){
    return v2d( SDL_cos(angle), SDL_sin(angle) );
}

static inline vec2d v2d_from_polar(double m, double a){
    return v2d( m * SDL_cos(a), m * SDL_sin(a) );
}

static inline vec2d v2d_to_polar(vec2d a){
    return v2d( v2d_mag(a), v2d_heading(a) );
}

static inline double v2d_anglebetween(vec2d a, vec2d b){
    return SDL_atan2( v2d_det(a, b), v2d_dot(a, b) );
}

//expects pointer to a sequence of 3 vecs, returns the angle at the middle one.
double v2d_inside_angle( vec2d *V );

void v2d_rotate( vec2d *a, float theta );

vec2d v2d_rotation( vec2d a, float theta );

/// Uses complex number multiplication to rotate a by b. Scaling will occur if a is not a unit vector.
static inline vec2d v2d_rotateV(vec2d a, vec2d b){
    return v2d(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

/// Inverse of v2d_rotate().
static inline vec2d v2d_unrotateV(vec2d a, vec2d b){
    return v2d(a.x*b.x + a.y*b.y, a.y*b.x - a.x*b.y);
}

vec2d v2d_medicenter( vec2d *list, int len );

static inline int v2d_less( const vec2d* a, const vec2d* b ){
    return (a->y == b->y) ? (a->x < b->x) : a->y < b->y;
}

int v2d_cmp(const void* a, const void* b);

static inline int v2d_eq( const vec2d* a, const vec2d* b ){
    return (a->y == b->y) && (a->x == b->x);
}

static inline int v2d_on_edge( const vec2d* pt, const vec2d* min, const vec2d* max ){
    return pt->x == min->x || pt->y == min->y || pt->x == max->x || pt->y == max->y;
}

#endif