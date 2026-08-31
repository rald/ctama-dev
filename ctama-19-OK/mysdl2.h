/*
* mysdl2.h - Minimalist SDL2 Wrapper with GameController, Joystick & PixelSize Support
*/

#ifndef MYSDL2_H
#define MYSDL2_H

#define _USE_MATH_DEFINES // Ensures M_PI is defined on strict C99 builds
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MYSDL_DEFAULT_DEAD_ZONE 8000

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool is_running;
    int pixel_size;
    
    // Input states
    const Uint8* keyboard_state;
    int mouse_x, mouse_y;
    Uint32 mouse_buttons;
    SDL_Joystick* joystick;
    SDL_GameController* controller;
    Sint16 joy_axes[6];
    Uint8 joy_buttons[32];
    Uint8 joy_hats[4];
} MySDL;

bool mysdl_init(MySDL* app, char *gamecontrollerdb_path, const char* title, int width, int height, int pixelSize);
void mysdl_quit(MySDL* app);
bool mysdl_poll(MySDL* app);
void mysdl_clear(MySDL* app, Uint32 color);
void mysdl_present(MySDL* app);

// Graphics Primitives (Using Uint32 color format: 0xRRGGBBAA)
void mysdl_draw_pixel(MySDL* app, int x, int y, Uint32 color);
void mysdl_pset(MySDL* app, int x, int y, Uint32 color);
void mysdl_draw_line(MySDL* app, int x0, int y0, int x1, int y1, Uint32 color);
void mysdl_draw_rect(MySDL* app, int x, int y, int w, int h, Uint32 color);
void mysdl_fill_rect(MySDL* app, int x, int y, int w, int h, Uint32 color);
void mysdl_draw_circle(MySDL* app, int cx, int cy, int radius, Uint32 color);
void mysdl_fill_circle(MySDL* app, int cx, int cy, int radius, Uint32 color);

// Collision Helpers
bool mysdl_inrect(int x, int y, int rx, int ry, int rw, int rh);
bool mysdl_incircle(int x, int y, int cx, int cy, int radius);

// Handlers / Input Queries
bool mysdl_key_down(MySDL* app, SDL_Scancode scancode);
bool mysdl_mouse_button_down(MySDL* app, int button);
void mysdl_get_mouse_pos(MySDL* app, int* x, int* y);
float mysdl_joystick_axis(MySDL* app, int axis);
bool mysdl_joystick_button_down(MySDL* app, int button);
Uint8 mysdl_joystick_hat(MySDL* app, int hat);
bool mysdl_controller_button_down(MySDL* app, SDL_GameControllerButton button);

// Advanced Joystick Helpers
int mysdl_joystick_get_direction_x(MySDL* app, int axis, int dead_zone);
int mysdl_joystick_get_direction_y(MySDL* app, int axis, int dead_zone);
double mysdl_joystick_angle(MySDL* app, int axis_x, int axis_y, int dead_zone);

#ifdef __cplusplus
}
#endif

#ifdef MYSDL2_IMPLEMENTATION

#include <stdio.h>

// Color Component Unpacking Macros (Assuming 0xRRGGBBAA layout)
#define MYSDL_COLOR_A(c) ((Uint8)((c >> 24) & 0xFF))
#define MYSDL_COLOR_R(c) ((Uint8)((c >> 16) & 0xFF))
#define MYSDL_COLOR_G(c) ((Uint8)((c >> 8)  & 0xFF))
#define MYSDL_COLOR_B(c) ((Uint8)((c)       & 0xFF))

bool mysdl_init(MySDL* app, char *gamecontrollerdb_path, const char* title, int width, int height, int pixelSize) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }
    
    int mappings_added = SDL_GameControllerAddMappingsFromFile(gamecontrollerdb_path);
    if (mappings_added > 0) {
        printf("Loaded %d controller mapping(s) from gamecontrollerdb.txt\n", mappings_added);
    } else {
        printf("gamecontrollerdb.txt not found or empty. Using default/fallback system mappings.\n");
    }
    
    app->pixel_size = (pixelSize > 0) ? pixelSize : 1;
    app->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * app->pixel_size, height * app->pixel_size, SDL_WINDOW_SHOWN);
    if (!app->window) return false;
    
    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!app->renderer) return false;
    
    app->is_running = true;
    app->keyboard_state = SDL_GetKeyboardState(NULL);
    app->mouse_x = 0;
    app->mouse_y = 0;
    app->mouse_buttons = 0;
    app->joystick = NULL;
    app->controller = NULL;

    for (int i = 0; i < 6; i++) app->joy_axes[i] = 0;
    for (int i = 0; i < 32; i++) app->joy_buttons[i] = 0;
    for (int i = 0; i < 4; i++) app->joy_hats[i] = 0;

    SDL_PumpEvents();

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            app->controller = SDL_GameControllerOpen(i);
            if (app->controller) {
                app->joystick = SDL_GameControllerGetJoystick(app->controller);
                printf("Successfully opened GameController 0: %s\n", SDL_GameControllerName(app->controller));
                break;
            }
        }
    }

    if (!app->joystick && SDL_NumJoysticks() > 0) {
        app->joystick = SDL_JoystickOpen(0);
        if (app->joystick) {
            printf("Successfully opened raw Joystick 0: %s\n", SDL_JoystickName(app->joystick));
        }
    }

    return true;
}

void mysdl_quit(MySDL* app) {
    if (app->controller) SDL_GameControllerClose(app->controller);
    else if (app->joystick) SDL_JoystickClose(app->joystick);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    SDL_Quit();
}

bool mysdl_poll(MySDL* app) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) app->is_running = false;
    }
    
    int raw_x, raw_y;
    app->mouse_buttons = SDL_GetMouseState(&raw_x, &raw_y);
    app->mouse_x = raw_x / app->pixel_size;
    app->mouse_y = raw_y / app->pixel_size;
    app->keyboard_state = SDL_GetKeyboardState(NULL);

    if (app->joystick) {
        for (int a = 0; a < SDL_JoystickNumAxes(app->joystick) && a < 6; a++) {
            app->joy_axes[a] = SDL_JoystickGetAxis(app->joystick, a);
        }
        for (int b = 0; b < SDL_JoystickNumButtons(app->joystick) && b < 32; b++) {
            app->joy_buttons[b] = SDL_JoystickGetButton(app->joystick, b);
        }
        for (int h = 0; h < SDL_JoystickNumHats(app->joystick) && h < 4; h++) {
            app->joy_hats[h] = SDL_JoystickGetHat(app->joystick, h);
        }
    }

    return app->is_running;
}

void mysdl_clear(MySDL* app, Uint32 color) {
    SDL_SetRenderDrawColor(app->renderer, MYSDL_COLOR_R(color), MYSDL_COLOR_G(color), MYSDL_COLOR_B(color), MYSDL_COLOR_A(color));
    SDL_RenderClear(app->renderer);
}

void mysdl_present(MySDL* app) {
    SDL_RenderPresent(app->renderer);
}

void mysdl_draw_pixel(MySDL* app, int x, int y, Uint32 color) {
    SDL_SetRenderDrawColor(app->renderer, MYSDL_COLOR_R(color), MYSDL_COLOR_G(color), MYSDL_COLOR_B(color), MYSDL_COLOR_A(color));
    if (app->pixel_size <= 1) {
        SDL_RenderDrawPoint(app->renderer, x, y);
    } else {
        SDL_Rect rect = {x * app->pixel_size, y * app->pixel_size, app->pixel_size, app->pixel_size};
        SDL_RenderFillRect(app->renderer, &rect);
    }
}

void mysdl_pset(MySDL* app, int x, int y, Uint32 color) {
    mysdl_draw_pixel(app, x, y, color);
}

void mysdl_draw_line(MySDL* app, int x0, int y0, int x1, int y1, Uint32 color) {
    SDL_SetRenderDrawColor(app->renderer, MYSDL_COLOR_R(color), MYSDL_COLOR_G(color), MYSDL_COLOR_B(color), MYSDL_COLOR_A(color));
    if (app->pixel_size <= 1) {
        SDL_RenderDrawLine(app->renderer, x0, y0, x1, y1);
    } else {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (1) {
            mysdl_pset(app, x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

void mysdl_draw_rect(MySDL* app, int x, int y, int w, int h, Uint32 color) {
    for (int i = 0; i < w; i++) {
        mysdl_pset(app, x + i, y, color);
        mysdl_pset(app, x + i, y + h - 1, color);
    }
    for (int j = 0; j < h; j++) {
        mysdl_pset(app, x, y + j, color);
        mysdl_pset(app, x + w - 1, y + j, color);
    }
}

void mysdl_fill_rect(MySDL* app, int x, int y, int w, int h, Uint32 color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            mysdl_pset(app, x + i, y + j, color);
        }
    }
}

void mysdl_draw_circle(MySDL* app, int cx, int cy, int radius, Uint32 color) {
    int dx = radius;
    int dy = 0;
    int err = 0;

    while (dx >= dy) {
        mysdl_pset(app, cx + dx, cy + dy, color);
        mysdl_pset(app, cx - dx, cy + dy, color);
        mysdl_pset(app, cx + dx, cy - dy, color);
        mysdl_pset(app, cx - dx, cy - dy, color);
        mysdl_pset(app, cx + dy, cy + dx, color);
        mysdl_pset(app, cx - dy, cy + dx, color);
        mysdl_pset(app, cx + dy, cy - dx, color);
        mysdl_pset(app, cx - dy, cy - dx, color);

        dy += 1;
        err += 1 + 2 * dy;
        if (2 * (err - dx) + 1 > 0) {
            dx -= 1;
            err += 1 - 2 * dx;
        }
    }
}

void mysdl_fill_circle(MySDL* app, int cx, int cy, int radius, Uint32 color) {
    int dx = radius;
    int dy = 0;
    int err = 0;

    while (dx >= dy) {
        for (int px = cx - dx; px <= cx + dx; px++) {
            mysdl_pset(app, px, cy + dy, color);
            mysdl_pset(app, px, cy - dy, color);
        }
        for (int px = cx - dy; px <= cx + dy; px++) {
            mysdl_pset(app, px, cy + dx, color);
            mysdl_pset(app, px, cy - dx, color);
        }

        dy += 1;
        err += 1 + 2 * dy;
        if (2 * (err - dx) + 1 > 0) {
            dx -= 1;
            err += 1 - 2 * dx;
        }
    }
}

bool mysdl_inrect(int x, int y, int rx, int ry, int rw, int rh) {
    return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
}

bool mysdl_incircle(int x, int y, int cx, int cy, int radius) {
    int dx = x - cx;
    int dy = y - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool mysdl_key_down(MySDL* app, SDL_Scancode scancode) {
    return app->keyboard_state[scancode] ? true : false;
}

bool mysdl_mouse_button_down(MySDL* app, int button) {
    return (app->mouse_buttons & SDL_BUTTON(button)) != 0;
}

void mysdl_get_mouse_pos(MySDL* app, int* x, int* y) {
    if (x) *x = app->mouse_x;
    if (y) *y = app->mouse_y;
}

float mysdl_joystick_axis(MySDL* app, int axis) {
    if (!app->joystick || axis < 0 || axis >= 6) return 0.0f;
    return (float)app->joy_axes[axis] / 32767.0f;
}

bool mysdl_joystick_button_down(MySDL* app, int button) {
    if (!app->joystick || button < 0 || button >= 32) return false;
    return app->joy_buttons[button] ? true : false;
}

Uint8 mysdl_joystick_hat(MySDL* app, int hat) {
    if (!app->joystick || hat < 0 || hat >= 4) return 0;
    return app->joy_hats[hat];
}

bool mysdl_controller_button_down(MySDL* app, SDL_GameControllerButton button) {
    if (!app->controller) return false;
    return SDL_GameControllerGetButton(app->controller, button) ? true : false;
}

int mysdl_joystick_get_direction_x(MySDL* app, int axis, int dead_zone) {
    if (!app->joystick || axis < 0 || axis >= 6) return 0;
    Sint16 val = app->joy_axes[axis];
    if (val < -dead_zone) return -1;
    if (val > dead_zone) return 1;
    return 0;
}

int mysdl_joystick_get_direction_y(MySDL* app, int axis, int dead_zone) {
    if (!app->joystick || axis < 0 || axis >= 6) return 0;
    Sint16 val = app->joy_axes[axis];
    if (val < -dead_zone) return -1;
    if (val > dead_zone) return 1;
    return 0;
}

double mysdl_joystick_angle(MySDL* app, int axis_x, int axis_y, int dead_zone) {
    int xDir = mysdl_joystick_get_direction_x(app, axis_x, dead_zone);
    int yDir = mysdl_joystick_get_direction_y(app, axis_y, dead_zone);

    double angle = atan2((double)yDir, (double)xDir) * (180.0 / M_PI);
    if (xDir == 0 && yDir == 0) {
        angle = 0.0;
    }
    return angle;
}

#endif // MYSDL2_IMPLEMENTATION
#endif // MYSDL2_H
