/*
* main.c - Showcase application utilizing standardized SDL_GameController configuration layout with GUID printer
*/

#define MYSDL2_IMPLEMENTATION
#define SWEETIE_16_IMPLEMENTATION
#define FONT_IMPLEMENTATION
#define CANVAS_IMPLEMENTATION

#include "mysdl2.h"
#include "sweetie-16.h"
#include "font.h"
#include "canvas.h"

#include <stdio.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_SCALE 4

typedef enum {
    GAME_SETUP=0,
    GAME_START,
    GAME_END,
} GameState;

Canvas *egg_canvas=NULL;

// Math Helpers
static inline float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Delta Time Helper
static float get_delta_time(Uint32* last_time) {
    Uint32 current_time = SDL_GetTicks();
    float delta_time = (float)(current_time - *last_time) / 1000.0f;
    *last_time = current_time;
    return delta_time;
}

// Setup Function
void setup(GameState* state) {
    (*state)=GAME_SETUP;
    egg_canvas=Canvas_Load("egg.cvs");
}

// Update Function
void update(MySDL* app, GameState* state, float delta_time) {
    mysdl_clear(app,SWEETIE_16_PALETTE[0x00]);
    Canvas_Draw(app,egg_canvas,0,0,0,1);
}

int main(void) {

    MySDL app;

    if (!mysdl_init(&app, "Game", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE)) {
        printf("Failed to initialize MySDL2!\n");
        return 1;
    }

    if (SDL_NumJoysticks() > 0) {
        char guid_str[33];
        SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(0), guid_str, sizeof(guid_str));
        printf("Connected Joystick GUID: %s\n", guid_str);
    }

    GameState state;
    setup(&state);

    Uint32 last_time = SDL_GetTicks();

    while (mysdl_poll(&app)) {
        if (mysdl_key_down(&app, SDL_SCANCODE_ESCAPE)) {
            break;
        }

        float delta_time = get_delta_time(&last_time);

        update(&app, &state, delta_time);

        mysdl_present(&app);
    }

    mysdl_quit(&app);
    return 0;
}
