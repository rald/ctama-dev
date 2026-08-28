#define MYSDL2_IMPLEMENTATION
#define SWEETIE_16_IMPLEMENTATION
#define FONT_IMPLEMENTATION
#define SPRITE_IMPLEMENTATION

#include "mysdl2.h"
#include "sweetie-16.h"
#include "font.h"
#include "sprite.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum {
    STAGE_EGG = 0,
    STAGE_CHICK,
    STAGE_ADULT,
    STAGE_OLD,
    STAGE_DEAD
} LifeStage;

typedef enum {
    MENU_FEED = 0,
    MENU_SNACK,
    MENU_GAME,
    MENU_LIGHT,
    MENU_MEDICINE,
    MENU_DISCIPLINE,
    MENU_COUNT
} MenuAction;

typedef enum {
    STATE_MAIN = 0,
    STATE_MINIGAME
} GameState;

typedef struct {
    LifeStage stage;
    float age_days;
    int hunger;
    int happiness;
    int discipline;
    bool is_sick;
    bool has_poop;
    bool lights_off;
    bool needs_attention;
    int selected_menu;
    GameState state;
    
    int mg_target;
    int mg_guess;
    int mg_message_timer;
} Tamagotchi;

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    MySDL app;
    if (!mysdl_init(&app, "Tamagotchi Hen", 128, 128, 4)) {
        return 1;
    }

    srand((unsigned int)time(NULL));

    Tamagotchi pet = {
        .stage = STAGE_EGG,
        .age_days = 0.0f,
        .hunger = 80,
        .happiness = 80,
        .discipline = 50,
        .is_sick = false,
        .has_poop = false,
        .lights_off = false,
        .needs_attention = false,
        .selected_menu = 0,
        .state = STATE_MAIN,
        .mg_target = 2,
        .mg_guess = 1,
        .mg_message_timer = 0
    };

    Uint32 last_tick = SDL_GetTicks();
    bool btn_a_prev = false;
    bool btn_b_prev = false;
    bool btn_c_prev = false;

    while (mysdl_poll(&app)) {
        Uint32 current_tick = SDL_GetTicks();
        float delta_time = (current_tick - last_tick) / 1000.0f;
        last_tick = current_tick;

        bool key_a = mysdl_key_down(&app, SDL_SCANCODE_A) || mysdl_key_down(&app, SDL_SCANCODE_LEFT);
        bool key_b = mysdl_key_down(&app, SDL_SCANCODE_RETURN) || mysdl_key_down(&app, SDL_SCANCODE_S) || mysdl_key_down(&app, SDL_SCANCODE_SPACE);
        bool key_c = mysdl_key_down(&app, SDL_SCANCODE_D) || mysdl_key_down(&app, SDL_SCANCODE_RIGHT);

        bool pressed_a = key_a && !btn_a_prev;
        bool pressed_b = key_b && !btn_b_prev;
        bool pressed_c = key_c && !btn_c_prev;

        btn_a_prev = key_a;
        btn_b_prev = key_b;
        btn_c_prev = key_c;

        if (pet.stage != STAGE_DEAD) {
            pet.age_days += delta_time * 1.0f;
            if (pet.stage == STAGE_EGG && pet.age_days > 1.0f) pet.stage = STAGE_CHICK;
            else if (pet.stage == STAGE_CHICK && pet.age_days > 4.0f) pet.stage = STAGE_ADULT;
            else if (pet.stage == STAGE_ADULT && pet.age_days > 10.0f) pet.stage = STAGE_OLD;
            else if (pet.stage == STAGE_OLD && pet.age_days > 16.0f) pet.stage = STAGE_DEAD;

            static float stat_timer = 0.0f;
            stat_timer += delta_time;
            if (stat_timer > 3.0f) {
                stat_timer = 0.0f;
                if (!pet.lights_off) {
                    if (pet.hunger > 0) pet.hunger -= 5;
                    if (pet.happiness > 0) pet.happiness -= 5;
                    if (rand() % 10 == 0) pet.has_poop = true;
                    if (rand() % 15 == 0 && !pet.is_sick) pet.is_sick = true;
                    if (pet.hunger < 20 && pet.happiness < 20) pet.needs_attention = true;
                }
            }

            if (pet.hunger <= 0 || pet.happiness <= 0 || (pet.is_sick && pet.age_days > 16.0f)) {
                pet.stage = STAGE_DEAD;
            }
        }

        if (pet.mg_message_timer > 0) {
            pet.mg_message_timer--;
        }

        if (pet.state == STATE_MAIN) {
            if (pressed_a) {
                pet.selected_menu = (pet.selected_menu + MENU_COUNT - 1) % MENU_COUNT;
            }
            if (pressed_c) {
                pet.selected_menu = (pet.selected_menu + 1) % MENU_COUNT;
            }
            if (pressed_b) {
                switch (pet.selected_menu) {
                    case MENU_FEED:
                        if (pet.hunger < 100) pet.hunger += 20;
                        if (pet.hunger > 100) pet.hunger = 100;
                        break;
                    case MENU_SNACK:
                        if (pet.hunger < 100) pet.hunger += 10;
                        if (pet.happiness < 100) pet.happiness += 15;
                        break;
                    case MENU_GAME:
                        if (pet.stage != STAGE_EGG && !pet.lights_off) {
                            pet.state = STATE_MINIGAME;
                            pet.mg_guess = 1;
                            pet.mg_target = (rand() % 3) + 1;
                        }
                        break;
                    case MENU_LIGHT:
                        pet.lights_off = !pet.lights_off;
                        break;
                    case MENU_MEDICINE:
                        if (pet.is_sick) {
                            pet.is_sick = false;
                            pet.happiness += 10;
                        }
                        break;
                    case MENU_DISCIPLINE:
                        if (pet.needs_attention && pet.hunger > 70 && pet.happiness > 70) {
                            pet.discipline = (pet.discipline < 100) ? pet.discipline + 20 : 100;
                            pet.needs_attention = false;
                        }
                        break;
                    default:
                        break;
                }
            }
        } else if (pet.state == STATE_MINIGAME) {
            if (pressed_a) {
                pet.mg_guess = (pet.mg_guess > 1) ? pet.mg_guess - 1 : 3;
            }
            if (pressed_c) {
                pet.mg_guess = (pet.mg_guess < 3) ? pet.mg_guess + 1 : 1;
            }
            if (pressed_b) {
                if (pet.mg_guess == pet.mg_target) {
                    pet.happiness = (pet.happiness <= 80) ? pet.happiness + 20 : 100;
                } else {
                    if (pet.happiness >= 10) pet.happiness -= 10;
                }
                pet.state = STATE_MAIN;
                pet.mg_message_timer = 30;
            }
        }

        Uint32 bg_color = pet.lights_off ? SWEETIE_16_PALETTE[0] : SWEETIE_16_PALETTE[12];
        mysdl_clear(&app, bg_color);

        if (!pet.lights_off) {
            mysdl_draw_line(&app, 0, 100, 128, 100, SWEETIE_16_PALETTE[14]);
            
            if (pet.has_poop) {
                mysdl_draw_sprite16(&app, 20, 84, SPRITE_POOP, SWEETIE_16_PALETTE);
            }

            if (pet.is_sick) {
                mysdl_draw_sprite16(&app, 80, 40, SPRITE_SKULL, SWEETIE_16_PALETTE);
            }

            const uint8_t* current_sprite = SPRITE_STAGE_EGG;
            if (pet.stage == STAGE_CHICK) current_sprite = SPRITE_STAGE_CHICK;
            else if (pet.stage == STAGE_ADULT) current_sprite = SPRITE_STAGE_ADULT;
            else if (pet.stage == STAGE_OLD) current_sprite = SPRITE_STAGE_OLD;
            
            if (pet.stage != STAGE_DEAD) {
                mysdl_draw_sprite16(&app, 56, 60, current_sprite, SWEETIE_16_PALETTE);
            } else {
                mysdl_draw_string(&app, 40, 68, "R.I.P", SWEETIE_16_PALETTE[2]);
            }

            mysdl_draw_sprite16(&app, 16, 4, SPRITE_ICON_FEED, SWEETIE_16_PALETTE);
            mysdl_draw_sprite16(&app, 56, 4, SPRITE_ICON_SNACK, SWEETIE_16_PALETTE);
            mysdl_draw_sprite16(&app, 96, 4, SPRITE_ICON_GAME, SWEETIE_16_PALETTE);

            mysdl_draw_sprite16(&app, 16, 108, SPRITE_ICON_LIGHT, SWEETIE_16_PALETTE);
            mysdl_draw_sprite16(&app, 56, 108, SPRITE_ICON_MEDICINE, SWEETIE_16_PALETTE);
            mysdl_draw_sprite16(&app, 96, 108, SPRITE_ICON_DISCIPLINE, SWEETIE_16_PALETTE);

            int cursor_x = 16 + (pet.selected_menu % 3) * 40;
            int cursor_y = (pet.selected_menu < 3) ? 2 : 106;
            mysdl_draw_rect(&app, cursor_x - 1, cursor_y - 1, 18, 18, SWEETIE_16_PALETTE[2]);

            if (pet.state == STATE_MINIGAME) {
                mysdl_fill_rect(&app, 14, 30, 100, 45, SWEETIE_16_PALETTE[15]);
                mysdl_draw_rect(&app, 14, 30, 100, 45, SWEETIE_16_PALETTE[12]);
                mysdl_draw_string(&app, 20, 36, "GUESS 1-3:", SWEETIE_16_PALETTE[12]);
                char guess_buf[16];
                sprintf(guess_buf, " > %d < ", pet.mg_guess);
                mysdl_draw_string(&app, 36, 52, guess_buf, SWEETIE_16_PALETTE[4]);
            }
        }

        mysdl_present(&app);
    }

    mysdl_quit(&app);
    return 0;
}
