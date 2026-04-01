#include "vec2d.h"

double v2d_inside_angle( vec2d *V ){
    double d0 = v2d_distsq( V[1], V[0] );
    double d2 = v2d_distsq( V[1], V[2] );
    return SDL_acos( ( d0 + d2 - v2d_distsq(V[0], V[2]) ) / (2 * SDL_sqrt(d0) * SDL_sqrt(d2)) );
}

void v2d_rotate( vec2d *a, float theta ){
    double co = SDL_cos(theta);
    double si = SDL_sin(theta);
    double px = a->x;
    a->x = a->x * co - a->y * si;
    a->y = px * si + a->y * co;
}

vec2d v2d_rotation( vec2d a, float theta ){
    double px = a.x;
    double co = SDL_cos(theta);
    double si = SDL_sin(theta);
    a.x = a.x * co - a.y * si;
    a.y = px * si + a.y * co;
    return a;
}

vec2d v2d_medicenter( vec2d *list, int len ){
    vec2d min = v2d(  9999999,  9999999 );
    vec2d max = v2d( -9999999, -9999999 );
    for (int i = 0; i < len; ++i ){
        if( list[i].x < min.x ) min.x = list[i].x;
        if( list[i].y < min.y ) min.y = list[i].y;
        if( list[i].x > max.x ) max.x = list[i].x;
        if( list[i].y > max.y ) max.y = list[i].y;
    }
    return v2d_lerp( min, max, 0.5 );
}

int v2d_cmp(const void* a, const void* b){
    const vec2d* s1 = (const vec2d*) a;
    const vec2d* s2 = (const vec2d*) b;
    return (s1->y != s2->y) ? (s1->y < s2->y ? -1 : 1) : (s1->x < s2->x ? -1 : 1);
}