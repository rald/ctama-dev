/*
* test.c - Example usage of Canvas_Animate with test.cvs custom frame sequence and microsecond delays
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

#define GAME_CONTROLLER_DB_PATH "../gamecontrollerdb.txt"
#define GAME_TITLE "CANVAS ANIMATION TEST"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_SCALE 4

typedef enum {
    GAME_SETUP=0,
    GAME_START,
    GAME_END,
} GameState;

Canvas *test_canvas = NULL;
Uint64 anim_timer = 0;
int current_index = 0;

// Example animation sequence: looping through frames 0, 1, 2, 1
const int frame_list[] = {0, 1, 2, 1};
// Microsecond delays per frame (e.g., 250,000 microseconds = 250 milliseconds per frame)
const Uint64 delay_list[] = {500000, 500000, 500000, 500000};
const int num_frames = 4;

// Setup Function
void setup(GameState* state) {
    (*state) = GAME_SETUP;
    test_canvas = Canvas_Load("test.cvs");
}

// Update Function
void update(MySDL* app, GameState* state, float delta_time) {
    mysdl_clear(app, SWEETIE_16_PALETTE[0x00]);
    
    // Draw and update the animation at position x=48, y=48 with scale size=4
    Canvas_Animate(app, test_canvas, &anim_timer, &current_index, frame_list, delay_list, num_frames, 48, 48, 1);
}

int main(void) {
    MySDL app;

    if (!mysdl_init(&app, GAME_CONTROLLER_DB_PATH, GAME_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE)) {
        printf("Failed to initialize MySDL2!\n");
        return 1;
    }

    GameState state;
    setup(&state);

    Uint32 last_time = SDL_GetTicks();

    while (mysdl_poll(&app)) {
        if (mysdl_key_down(&app, SDL_SCANCODE_ESCAPE)) {
            break;
        }

        // Delta time helper calculation
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (float)(current_time - last_time) / 1000.0f;
        last_time = current_time;

        update(&app, &state, delta_time);

        mysdl_present(&app);
    }

    if (test_canvas) {
        Canvas_Free(&test_canvas);
    }

    mysdl_quit(&app);
    return 0;
}
