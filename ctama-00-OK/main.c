/*
* main.c - Showcase application utilizing standardized SDL_GameController configuration layout with GUID printer
*/

#define MYSDL2_IMPLEMENTATION
#include "mysdl2.h"
#include <stdio.h>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 300
#define SCREEN_SCALE 2

// Sweetie-16 Color Palette (GrafxKid) - Updated for 0xAABBGGRR layout
const uint32_t SWEETIE_16_PALETTE[16] = {
    0xFF2C1C1A, // [0x00] Dark Chocolate Licorice (Swapped R and B)[cite: 2]
    0xFF5D275D, // [0x01] Blackberry Truffle[cite: 2]
    0xFF533EB1, // [0x02] Cherry Drop[cite: 2]
    0xFF577DEF, // [0x03] Spiced Peach[cite: 2]
    0xFF75CDFF, // [0x04] Butterscotch Crunch[cite: 2]
    0xFF70F0A7, // [0x05] Key Lime Jelly[cite: 2]
    0xFF64B738, // [0x06] Spearmint Leaf[cite: 2]
    0xFF797125, // [0x07] Blue Raspberry Frost[cite: 2]
    0xFF6F3629, // [0x08] Blueberry Hard Candy[cite: 2]
    0xFFC95D3B, // [0x09] Bubblegum Glaze[cite: 2]
    0xFFF6A641, // [0x0A] Cotton Candy Sky[cite: 2]
    0xFFF7EF73, // [0x0B] Iced Lemonade Fizz[cite: 2]
    0xFFF4F4F4, // [0x0C] Powdered Sugar[cite: 2]
    0xFFC2B094, // [0x0D] Frosting Mist[cite: 2]
    0xFF866C56, // [0x0E] Cool Mint Swirl[cite: 2]
    0xFF573C33  // [0x0F] Cocoa Twilight[cite: 2]
};

// Math Helpers
static inline float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    MySDL app;

    if (!mysdl_init(&app, "MySDL2 GameController Configuration Showcase", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE)) {
        printf("Failed to initialize MySDL2!\n");
        return 1;
    }

    if (SDL_NumJoysticks() > 0) {
        char guid_str[33];
        SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(0), guid_str, sizeof(guid_str));
        printf("Connected Joystick GUID: %s\n", guid_str);
    }

    int custom_circle_x = SCREEN_WIDTH / 2;
    int custom_circle_y = SCREEN_HEIGHT / 2;
    int custom_circle_r = 20;

    float player_x = (float)(SCREEN_WIDTH / 2);
    float player_y = (float)(SCREEN_HEIGHT - 75);
    float player_speed = 2.0f;

    while (mysdl_poll(&app)) {
        if (mysdl_key_down(&app, SDL_SCANCODE_ESCAPE)) {
            break;
        }

        bool a_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_A);
        bool b_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_B);
        bool x_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_X);
        bool y_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_Y);
        bool l_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bool r_pressed      = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        bool select_pressed = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_BACK);
        bool start_pressed  = mysdl_controller_button_down(&app, SDL_CONTROLLER_BUTTON_START);

        int xDir = mysdl_joystick_get_direction_x(&app, 0, 8000);
        int yDir = mysdl_joystick_get_direction_y(&app, 1, 8000);
        double joystickAngle = mysdl_joystick_angle(&app, 0, 1, 8000);

        Uint8 hat  = mysdl_joystick_hat(&app, 0);
        bool up    = mysdl_key_down(&app, SDL_SCANCODE_W) || mysdl_key_down(&app, SDL_SCANCODE_UP) || yDir < 0 || (hat & SDL_HAT_UP);
        bool down  = mysdl_key_down(&app, SDL_SCANCODE_S) || mysdl_key_down(&app, SDL_SCANCODE_DOWN) || yDir > 0 || (hat & SDL_HAT_DOWN);
        bool left  = mysdl_key_down(&app, SDL_SCANCODE_A) || mysdl_key_down(&app, SDL_SCANCODE_LEFT) || xDir < 0 || (hat & SDL_HAT_LEFT);
        bool right = mysdl_key_down(&app, SDL_SCANCODE_D) || mysdl_key_down(&app, SDL_SCANCODE_RIGHT) || xDir > 0 || (hat & SDL_HAT_RIGHT);

        if (up) player_y -= player_speed;
        if (down) player_y += player_speed;
        if (left) player_x -= player_speed;
        if (right) player_x += player_speed;

        player_x = clamp(player_x, 0.0f, (float)(SCREEN_WIDTH - 15));
        player_y = clamp(player_y, 0.0f, (float)(SCREEN_HEIGHT - 15));

        bool action = mysdl_key_down(&app, SDL_SCANCODE_SPACE) || 
                      a_pressed || b_pressed || x_pressed || y_pressed ||
                      r_pressed || l_pressed || start_pressed || select_pressed;

        int mx, my;
        mysdl_get_mouse_pos(&app, &mx, &my);
        bool mouse_clicked = mysdl_mouse_button_down(&app, SDL_BUTTON_LEFT);

        bool hovering_rect = mysdl_inrect(mx, my, 50, 50, 100, 50);
        bool hovering_circle = mysdl_incircle(mx, my, custom_circle_x, custom_circle_y, custom_circle_r);

        mysdl_clear(&app, SWEETIE_16_PALETTE[0x00]);

        for (int x = 0; x < SCREEN_WIDTH; x += 25) mysdl_draw_line(&app, x, 0, x, SCREEN_HEIGHT,  SWEETIE_16_PALETTE[0x01]);
        for (int y = 0; y < SCREEN_HEIGHT; y += 25) mysdl_draw_line(&app, 0, y, SCREEN_WIDTH, y,  SWEETIE_16_PALETTE[0x01]);

        Uint8 rect_g = hovering_rect ? 200 : 100;
        if (hovering_rect && mouse_clicked) rect_g = 255;
        mysdl_fill_rect(&app, 50, 50, 100, 50, (50 << 24) | (150 << 16) | (rect_g << 8) | 25);
        mysdl_draw_rect(&app, 50, 50, 100, 50, SWEETIE_16_PALETTE[0x0C]);

        Uint8 circ_b = hovering_circle ? 255 : 150;
        if (hovering_circle && mouse_clicked) circ_b = 255;
        mysdl_fill_circle(&app, custom_circle_x, custom_circle_y, custom_circle_r, (200 << 24) | (circ_b << 16) | (50 << 8) | 255);
        mysdl_draw_circle(&app, custom_circle_x, custom_circle_y, custom_circle_r,  SWEETIE_16_PALETTE[0x0C]);

        Uint8 player_r = action ? 255 : 0;
        Uint8 player_b = action ? 0 : 255;
        mysdl_fill_rect(&app, (int)player_x, (int)player_y, 15, 15, (255 << 24) | (player_b << 16) | (200 << 8) | player_r);
        mysdl_draw_rect(&app, (int)player_x, (int)player_y, 15, 15,  SWEETIE_16_PALETTE[0x0C]);

        if (xDir != 0 || yDir != 0) {
            (void)joystickAngle;
        }

        mysdl_present(&app);
    }

    mysdl_quit(&app);
    return 0;
}
