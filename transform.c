#include "transform.h"


vec2d update_TM( Transform *T, double unused1, double unused2, double unused3, double unused4 ){
    T->M.a = T->s;   T->M.b = 0.0;
    T->M.d = 0.0;    T->M.e = T->s;
    T->M.c = T->cx - T->s * T->tx;
    T->M.f = T->cy - T->s * T->ty;
    return v2dzero;
}

vec2d update_TM_object_rotated( Transform *T, double unused, double obj_angle, double obj_x, double obj_y ){
    double cos_o = SDL_cos(obj_angle);
    double sin_o = SDL_sin(obj_angle);
    double s = T->s;
    double tx = T->tx, ty = T->ty;
    double cx = T->cx, cy = T->cy;

    double a_c = s, b_c = 0, d_c = 0, e_c = s;
    double c_c = cx - s * tx;
    double f_c = cy - s * ty;

    // object
    T->M.a =  a_c * cos_o + b_c * sin_o;      
    T->M.b = -a_c * sin_o + b_c * cos_o;      
    T->M.d =  d_c * cos_o + e_c * sin_o;      
    T->M.e = -d_c * sin_o + e_c * cos_o;      
    T->M.c =  a_c * obj_x + b_c * obj_y + c_c;
    T->M.f =  d_c * obj_x + e_c * obj_y + f_c;

    return v2d( cos_o, sin_o );
}

vec2d update_TM_world_rotated( Transform *T, double world_angle, double unused1, double unused2, double unused3 ){

    double cos_w = SDL_cos( world_angle );
    double sin_w = SDL_sin( world_angle );
    double s = T->s;
    double tx = T->tx, ty = T->ty;
    double cx = T->cx, cy = T->cy;

    T->M.a =  s * cos_w;
    T->M.b = -s * sin_w;
    T->M.d =  s * sin_w;
    T->M.e =  s * cos_w;
    T->M.c = cx - s * (cos_w * tx - sin_w * ty);
    T->M.f = cy - s * (sin_w * tx + cos_w * ty);

    return v2d( cos_w, sin_w );
}

vec2d update_TM_combined(Transform *T, double world_angle, double obj_angle, double obj_x, double obj_y) {
    double cos_w = SDL_cos(world_angle);
    double sin_w = SDL_sin(world_angle);
    double cos_o = SDL_cos(obj_angle);
    double sin_o = SDL_sin(obj_angle);
    double s = T->s;
    double tx = T->tx, ty = T->ty;
    double cx = T->cx, cy = T->cy;

    // world
    double a_c =  s * cos_w;
    double b_c = -s * sin_w;
    double d_c =  s * sin_w;
    double e_c =  s * cos_w;
    double c_c = cx - s * (cos_w * tx - sin_w * ty);
    double f_c = cy - s * (sin_w * tx + cos_w * ty);

    // object
    T->M.a =  a_c * cos_o + b_c * sin_o;
    T->M.b = -a_c * sin_o + b_c * cos_o;
    T->M.d =  d_c * cos_o + e_c * sin_o;
    T->M.e = -d_c * sin_o + e_c * cos_o;
    T->M.c =  a_c * obj_x + b_c * obj_y + c_c;
    T->M.f =  d_c * obj_x + e_c * obj_y + f_c;

    return v2d( cos_o, sin_o );
}


void set_scale( Transform *T, float s ){
	T->s = s;
	T->invs = 1 / s;
}


vec2d apply_transform_v2d( vec2d *vec, Transform *T ){
	return (vec2d){ T->cx + (T->s * (vec->x - T->tx)), T->cy + (T->s * (vec->y - T->ty)) };
}

vec2d reverse_transform_v2d( vec2d *vec, Transform *T ){
	return (vec2d){ ((vec->x - T->cx) * T->invs) + T->tx, ((vec->y - T->cy) * T->invs) + T->ty };
}

cpVect apply_transform_cpv( cpVect vec, Transform *T ){
	return (cpVect){ T->cx + (T->s * (vec.x - T->tx)), T->cy + (T->s * (vec.y - T->ty)) };
}

cpVect reverse_transform_cpv( cpVect vec, Transform *T ){
	return (cpVect){ ((vec.x - T->cx) * T->invs) + T->tx, ((vec.y - T->cy) * T->invs) + T->ty };
}
SDL_FPoint apply_transform_fp( SDL_FPoint p, Transform *T ){
	return (SDL_FPoint){ T->cx + (T->s * (p.x - T->tx)), T->cy + (T->s * (p.y - T->ty)) };
}

SDL_FPoint reverse_transform_fp( SDL_FPoint p, Transform *T ){
	return (SDL_FPoint){ ((p.x - T->cx) * T->invs) + T->tx, ((p.y - T->cy) * T->invs) + T->ty };
}

SDL_Rect apply_transform_rect( SDL_Rect *rct, Transform *T ){
	return (SDL_Rect){ T->cx + (T->s * (rct->x - T->tx)), T->cy + (T->s * (rct->y - T->ty)), 
					   rct->w * T->s, rct->h * T->s };
}

SDL_Rect reverse_transform_rect( SDL_Rect *rct, Transform *T ){
	return (SDL_Rect){ ((rct->x - T->cx) * T->invs) + T->tx, ((rct->y - T->cy) * T->invs) + T->ty,
						rct->w * T->invs, rct->h * T->invs };
}

SDL_FRect apply_transform_frect( SDL_FRect *rct, Transform *T ){
	return (SDL_FRect){ T->cx + (T->s * (rct->x - T->tx)), T->cy + (T->s * (rct->y - T->ty)), 
						rct->w * T->s, rct->h * T->s };
}

SDL_FRect reverse_transform_frect( SDL_FRect *rct, Transform *T ){
	return (SDL_FRect){ ((rct->x - T->cx) * T->invs) + T->tx, ((rct->y - T->cy) * T->invs) + T->ty,
						rct->w * T->invs, rct->h * T->invs };
}


void constrain_Transform( Transform *T, SDL_FRect window_rct, SDL_FRect bounds ){

    SDL_FRect dst = apply_transform_frect( &window_rct, T );
    float pw = dst.w;
    constrain_frect( &dst, bounds );

    T->s *= dst.w / pw;
    T->invs = 1.0 / T->s;
    T->tx = -(dst.x - T->cx) * T->invs;
    T->ty = -(dst.y - T->cy) * T->invs;
}



vec2d apply_Mat23_v2d( vec2d world, const Mat23 *M ) {
    return (vec2d){
        .x = M->a * world.x + M->b * world.y + M->c,
        .y = M->d * world.x + M->e * world.y + M->f
    };
}	


vec2d reverse_Mat23_v2d( vec2d screen, const Mat23 *M ){
    double det = M->a * M->e - M->b * M->d;
    double inv_det = 1.0 / det;

    double inv_a =  M->e * inv_det;
    double inv_b = -M->b * inv_det;
    double inv_d = -M->d * inv_det;
    double inv_e =  M->a * inv_det;
    double dx = screen.x - M->c;
    double dy = screen.y - M->f;

    return (vec2d){
        .x = inv_a * dx + inv_b * dy,
        .y = inv_d * dx + inv_e * dy
    };
}



