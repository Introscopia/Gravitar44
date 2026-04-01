#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL.h>
#include "vec2d.h"


typedef enum {
    INPUT_NULL,
    INPUT_KEYBOARD,
    INPUT_MOUSE_MOTION,
    INPUT_MOUSE_BUTTON,
    INPUT_MOUSE_WHEEL,
    INPUT_GAMEPAD_BUTTON,
    INPUT_GAMEPAD_AXIS,
} Input_Type;


typedef struct Input {
    
    Input_Type type;

    SDL_JoystickID which_gamepad;   // only for gamepad events; ignored otherwise

    union {
        SDL_Keycode key;
        Uint8 mouse_motion; // 1: X axis, 2: Y axis
        Uint8 mouse_pos;    // 1: X axis, 2: Y axis
        Uint8 mouse_button; // SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT, SDL_BUTTON_X1, SDL_BUTTON_X2
        Uint8 mouse_wheel;  // 1: X axis, 2: Y axis
        SDL_GamepadButton gamepad_button;
        SDL_GamepadAxis gamepad_axis;
    } detail;

    int current;

    float factor;
    /* gamepad axis : factor = 1 / 32767.0f;
       mouse_pos    : factor = window height 
       mouse_rel    : factor = some "sensitivity" constant
    */

} Input;

bool Event_Input_cmp( SDL_Event *E, Input *I );

// Returns 1 if event matches binding and fills *output, otherwise 0.
// cx, cy are the center for mouse-absolute-position directional controls.
int HandleEvent( SDL_Event *event, Input *binding, int cx, int cy );


typedef struct DirInput {

    int mode; // 4 (4 buttons), 2 (2 axis)

    Input bindings [4];
    /* mode 4: R L U D
       mode 2: X Y
    */
    
} DirInput;

int Dir_HandleEvent( SDL_Event *event, DirInput *dir, int cx, int cy );
vec2d DirInput_compute( DirInput *D );


DirInput Directional_Yawer_gamepad( SDL_JoystickID which );

// str shall be in "wasd" order, lower-case
DirInput Directional_from_String( char *str );

DirInput Directional_arrows();

#endif