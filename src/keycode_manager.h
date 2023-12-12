#pragma once

#include <SDL.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <windows.h>
#else
    #include <Carbon/Carbon.h>
#endif

int16_t SDLKeycodeToOSKeyCode(SDL_Keycode sdlKeycode);