
#include "input.h"


bool Event_Input_cmp( SDL_Event *E, Input *I ){
    switch (E->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if( I->type == INPUT_KEYBOARD ) return true;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if( I->type == INPUT_MOUSE_MOTION ) return true;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if( I->type == INPUT_MOUSE_BUTTON ) return true;
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            if( I->type == INPUT_MOUSE_WHEEL ) return true;
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if( I->type == INPUT_GAMEPAD_BUTTON ) return true;
            break;

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if( I->type == INPUT_GAMEPAD_AXIS ) return true;
            break;
    }
    return false;
}

int HandleEvent( SDL_Event *event, Input *binding, int cx, int cy ){
    if( binding->type == INPUT_NULL || !Event_Input_cmp( event, binding ) ){
        return 0;
    }

    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (event->key.key == binding->detail.key) {
                binding->current = (event->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
                return 1;
            } break;

        case SDL_EVENT_MOUSE_MOTION:{
            int prev = binding->current;
                 if( binding->detail.mouse_motion == 1 ) binding->current = event->motion.xrel;
            else if( binding->detail.mouse_motion == 2 ) binding->current = event->motion.yrel;
            else if( binding->detail.mouse_pos == 1 ) binding->current = event->motion.x - cx;
            else if( binding->detail.mouse_pos == 2 ) binding->current = event->motion.y - cy;
            if( prev != binding->current ) return 1;
            } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event->button.button == binding->detail.mouse_button) {
                binding->current = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
                return 1;
            } break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (event->gbutton.which == binding->which_gamepad &&
                event->gbutton.button == binding->detail.gamepad_button) {
                binding->current = event->gbutton.down ? 1 : 0;
                return 1;
            } break;

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if (event->gaxis.which == binding->which_gamepad &&
                event->gaxis.axis == binding->detail.gamepad_axis) {
                binding->current = event->gaxis.value;
                /*SDL_Log( "SDL_EVENT_GAMEPAD_AXIS_MOTION! axis: %d, value: %d", event->gaxis.axis, event->gaxis.value );
                if( event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || 
                    event->gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER ){
                    binding->current = (binding->current + 32767) / 2;
                    SDL_Log( "binding->current biased! now: %d", binding->current );
                }*/
                return 1;
            } break;

        case SDL_EVENT_MOUSE_WHEEL:
                 if( binding->detail.mouse_wheel == 1 ) binding->current = event->wheel.x;
            else if( binding->detail.mouse_wheel == 2 ) binding->current = event->wheel.y;
            return 1;
    }
    return 0;
}

int Dir_HandleEvent( SDL_Event *event, DirInput *dir, int cx, int cy ){

    int captured = 0;
    for (int i = 0; i < dir->mode; ++i ){
        captured += HandleEvent( event, dir->bindings + i, cx, cy );
    }
    return captured;
}

vec2d DirInput_compute( DirInput *dir ){
    vec2d out = v2dzero;
    switch (dir->mode) {
        case 4: {
            out.x += dir->bindings[0].current;
            out.x -= dir->bindings[1].current;
            out.y -= dir->bindings[2].current;
            out.y += dir->bindings[3].current;
            } break;
        case 2: {
            out.x = dir->bindings[0].current * dir->bindings[0].factor;
            out.y = dir->bindings[1].current * dir->bindings[1].factor;
            } break;
    }
    /*if( v2d_magsq( out ) > 1.0 ){
        out = v2d_normalize( out );
    }*/
    return out;
}


DirInput Directional_Yawer_gamepad( SDL_JoystickID which ){
    DirInput out;
    out.mode = 2;
    out.bindings[0] = (Input){ .type = INPUT_GAMEPAD_AXIS,
                               .which_gamepad = which,
                               .detail.gamepad_axis = SDL_GAMEPAD_AXIS_LEFTX,
                               .factor = 1 / 32767.0f };
    out.bindings[1] = (Input){ .type = INPUT_GAMEPAD_AXIS,
                               .which_gamepad = which,
                               .detail.gamepad_axis = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
                               .factor = -1 / 32767.0f };
    /*out.bindings[1] = (Input){ .type = INPUT_GAMEPAD_AXIS, 
                               .which_gamepad = which,
                               .detail.gamepad_axis = SDL_GAMEPAD_AXIS_LEFTY };*/
    return out;
}

DirInput Directional_from_String( char *str ){
    DirInput out;
    out.mode = 4;
    out.bindings[2] = (Input){ .type = INPUT_KEYBOARD, .detail.key = str[0] };
    out.bindings[1] = (Input){ .type = INPUT_KEYBOARD, .detail.key = str[1] };
    out.bindings[3] = (Input){ .type = INPUT_KEYBOARD, .detail.key = str[2] };
    out.bindings[0] = (Input){ .type = INPUT_KEYBOARD, .detail.key = str[3] };
    return out;
}

DirInput Directional_arrows(){
    DirInput out;
    out.mode = 4;
    out.bindings[0] = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_RIGHT };
    out.bindings[1] = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_LEFT  };
    out.bindings[2] = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_UP    };
    out.bindings[3] = (Input){ .type = INPUT_KEYBOARD, .detail.key = SDLK_DOWN  };
    return out;
}