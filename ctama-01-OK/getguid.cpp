#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    int num_joysticks = SDL_NumJoysticks();
    if (num_joysticks < 1) {
        std::cout << "No joysticks/controllers connected!\n";
    } else {
        for (int i = 0; i < num_joysticks; ++i) {
            SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
            char guid_str[33];
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            
            const char* name = SDL_JoystickNameForIndex(i);
            
            std::cout << "Controller #" << i << ": " << (name ? name : "Unknown") << "\n";
            std::cout << "  GUID: " << guid_str << "\n\n";
        }
    }

    SDL_Quit();
    return 0;
}
