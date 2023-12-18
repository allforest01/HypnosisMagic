#include "keycode_manager.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

int16_t SDLKeycodeToOSKeyCode(SDL_Keycode sdlKeycode) {
    switch (sdlKeycode) {
        case SDLK_BACKSPACE: return VK_BACK;
        case SDLK_TAB: return VK_TAB;
        case SDLK_CLEAR: return VK_CLEAR;
        case SDLK_RETURN: return VK_RETURN;
        case SDLK_PAUSE: return VK_PAUSE;  
        case SDLK_ESCAPE: return VK_ESCAPE;
        case SDLK_SPACE: return VK_SPACE;
        case SDLK_QUOTEDBL: return VK_OEM_7;
        case SDLK_MINUS: return VK_OEM_MINUS;
        case SDLK_EQUALS: return VK_OEM_PLUS;
        case SDLK_COMMA: return VK_OEM_COMMA; 
        case SDLK_PERIOD: return VK_OEM_PERIOD; 
        case SDLK_SLASH: return VK_OEM_2;
        case SDLK_0: return '0';
        case SDLK_1: return '1';  
        case SDLK_2: return '2';
        case SDLK_3: return '3';
        case SDLK_4: return '4';
        case SDLK_5: return '5';
        case SDLK_6: return '6';
        case SDLK_7: return '7';
        case SDLK_8: return '8';
        case SDLK_9: return '9';
        case SDLK_SEMICOLON: return VK_OEM_1;
        case SDLK_LEFTBRACKET: return VK_OEM_4; 
        case SDLK_BACKSLASH: return VK_OEM_5; 
        case SDLK_RIGHTBRACKET: return VK_OEM_6;
        case SDLK_a: return 'A';
        case SDLK_b: return 'B';
        case SDLK_c: return 'C';
        case SDLK_d: return 'D';
        case SDLK_e: return 'E';
        case SDLK_f: return 'F';
        case SDLK_g: return 'G';
        case SDLK_h: return 'H';
        case SDLK_i: return 'I';
        case SDLK_j: return 'J';
        case SDLK_k: return 'K';
        case SDLK_l: return 'L';
        case SDLK_m: return 'M';
        case SDLK_n: return 'N';
        case SDLK_o: return 'O';
        case SDLK_p: return 'P';
        case SDLK_q: return 'Q';
        case SDLK_r: return 'R';
        case SDLK_s: return 'S';
        case SDLK_t: return 'T';
        case SDLK_u: return 'U';
        case SDLK_v: return 'V';
        case SDLK_w: return 'W';
        case SDLK_x: return 'X';
        case SDLK_y: return 'Y'; 
        case SDLK_z: return 'Z';
        case SDLK_F1: return VK_F1;
        case SDLK_F2: return VK_F2;
        case SDLK_F3: return VK_F3;
        case SDLK_F4: return VK_F4;
        case SDLK_F5: return VK_F5;
        case SDLK_F6: return VK_F6; 
        case SDLK_F7: return VK_F7;
        case SDLK_F8: return VK_F8;
        case SDLK_F9: return VK_F9;
        case SDLK_F10: return VK_F10;
        case SDLK_F11: return VK_F11;
        case SDLK_F12: return VK_F12;
        case SDLK_PRINTSCREEN: return VK_SNAPSHOT; 
        case SDLK_SCROLLLOCK: return VK_SCROLL;
        case SDLK_INSERT: return VK_INSERT;  
        case SDLK_HOME: return VK_HOME;
        case SDLK_PAGEUP: return VK_PRIOR;  
        case SDLK_DELETE: return VK_DELETE;
        case SDLK_END: return VK_END;
        case SDLK_PAGEDOWN: return VK_NEXT;
        case SDLK_RIGHT: return VK_RIGHT;  
        case SDLK_LEFT: return VK_LEFT;
        case SDLK_DOWN: return VK_DOWN; 
        case SDLK_UP: return VK_UP; 
        case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK;  
        case SDLK_KP_DIVIDE: return VK_DIVIDE;  
        case SDLK_KP_MULTIPLY: return VK_MULTIPLY; 
        case SDLK_KP_MINUS: return VK_SUBTRACT;
        case SDLK_KP_PLUS: return VK_ADD;
        case SDLK_KP_ENTER: return VK_RETURN;  
        case SDLK_KP_1: return VK_NUMPAD1; 
        case SDLK_KP_2: return VK_NUMPAD2;
        case SDLK_KP_3: return VK_NUMPAD3;
        case SDLK_KP_4: return VK_NUMPAD4;
        case SDLK_KP_5: return VK_NUMPAD5;  
        case SDLK_KP_6: return VK_NUMPAD6;
        case SDLK_KP_7: return VK_NUMPAD7;
        case SDLK_KP_8: return VK_NUMPAD8;
        case SDLK_KP_9: return VK_NUMPAD9; 
        case SDLK_KP_0: return VK_NUMPAD0;
        case SDLK_KP_PERIOD: return VK_DECIMAL;
        case SDLK_LCTRL: return VK_LCONTROL;
        case SDLK_RCTRL: return VK_RCONTROL;
        case SDLK_LALT: return VK_LMENU;
        case SDLK_RALT: return VK_RMENU;  
        case SDLK_LSHIFT: return VK_LSHIFT; 
        case SDLK_RSHIFT: return VK_RSHIFT;
        case SDLK_LGUI: return VK_LWIN;
        case SDLK_RGUI: return VK_RWIN;
    }
    return -1;
}

#else

int16_t SDLKeycodeToOSKeyCode(SDL_Keycode sdlKeycode) {
    switch (sdlKeycode) {
        case SDLK_RETURN: return kVK_Return;
        case SDLK_ESCAPE: return kVK_Escape;
        case SDLK_BACKSPACE: return kVK_Delete;
        case SDLK_TAB: return kVK_Tab;
        case SDLK_SPACE: return kVK_Space;
        case SDLK_EXCLAIM: return kVK_ANSI_1;
        case SDLK_QUOTEDBL: return kVK_ANSI_Quote;
        case SDLK_HASH: return kVK_ANSI_3; 
        case SDLK_DOLLAR: return kVK_ANSI_4;
        case SDLK_AMPERSAND: return kVK_ANSI_7;
        case SDLK_QUOTE: return kVK_ANSI_Quote;
        case SDLK_LEFTPAREN: return kVK_ANSI_9;
        case SDLK_RIGHTPAREN: return kVK_ANSI_0;
        case SDLK_ASTERISK: return kVK_ANSI_8;
        case SDLK_PLUS: return kVK_ANSI_Equal;
        case SDLK_COMMA: return kVK_ANSI_Comma;
        case SDLK_MINUS: return kVK_ANSI_Minus;
        case SDLK_PERIOD: return kVK_ANSI_Period;
        case SDLK_SLASH: return kVK_ANSI_Slash;
        case SDLK_0: return kVK_ANSI_0;
        case SDLK_1: return kVK_ANSI_1;
        case SDLK_2: return kVK_ANSI_2;
        case SDLK_3: return kVK_ANSI_3;
        case SDLK_4: return kVK_ANSI_4;
        case SDLK_5: return kVK_ANSI_5;
        case SDLK_6: return kVK_ANSI_6;
        case SDLK_7: return kVK_ANSI_7;
        case SDLK_8: return kVK_ANSI_8;
        case SDLK_9: return kVK_ANSI_9;
        case SDLK_COLON: return kVK_ANSI_Semicolon;
        case SDLK_SEMICOLON: return kVK_ANSI_Semicolon;
        case SDLK_EQUALS: return kVK_ANSI_Equal;
        case SDLK_QUESTION: return kVK_ANSI_Slash;
        case SDLK_LEFTBRACKET: return kVK_ANSI_LeftBracket;
        case SDLK_BACKSLASH: return kVK_ANSI_Backslash;
        case SDLK_RIGHTBRACKET: return kVK_ANSI_RightBracket;
        case SDLK_BACKQUOTE: return kVK_ANSI_Grave;
        case SDLK_a: return kVK_ANSI_A;
        case SDLK_b: return kVK_ANSI_B;
        case SDLK_c: return kVK_ANSI_C;
        case SDLK_d: return kVK_ANSI_D;
        case SDLK_e: return kVK_ANSI_E;
        case SDLK_f: return kVK_ANSI_F;
        case SDLK_g: return kVK_ANSI_G;
        case SDLK_h: return kVK_ANSI_H;
        case SDLK_i: return kVK_ANSI_I;
        case SDLK_j: return kVK_ANSI_J;
        case SDLK_k: return kVK_ANSI_K;
        case SDLK_l: return kVK_ANSI_L;
        case SDLK_m: return kVK_ANSI_M;
        case SDLK_n: return kVK_ANSI_N;
        case SDLK_o: return kVK_ANSI_O;
        case SDLK_p: return kVK_ANSI_P;
        case SDLK_q: return kVK_ANSI_Q;
        case SDLK_r: return kVK_ANSI_R;
        case SDLK_s: return kVK_ANSI_S;
        case SDLK_t: return kVK_ANSI_T;
        case SDLK_u: return kVK_ANSI_U;
        case SDLK_v: return kVK_ANSI_V;
        case SDLK_w: return kVK_ANSI_W;
        case SDLK_x: return kVK_ANSI_X;
        case SDLK_y: return kVK_ANSI_Y;
        case SDLK_z: return kVK_ANSI_Z;
        case SDLK_CAPSLOCK: return kVK_CapsLock;
        case SDLK_F1: return kVK_F1;
        case SDLK_F2: return kVK_F2;
        case SDLK_F3: return kVK_F3;
        case SDLK_F4: return kVK_F4;
        case SDLK_F5: return kVK_F5;
        case SDLK_F6: return kVK_F6;
        case SDLK_F7: return kVK_F7;
        case SDLK_F8: return kVK_F8;
        case SDLK_F9: return kVK_F9;
        case SDLK_F10: return kVK_F10;
        case SDLK_F11: return kVK_F11;
        case SDLK_F12: return kVK_F12;
        case SDLK_PRINTSCREEN: return kVK_F13;
        case SDLK_SCROLLLOCK: return kVK_F14;
        case SDLK_PAUSE: return kVK_F15;
        case SDLK_INSERT: return kVK_Help;
        case SDLK_HOME: return kVK_Home;
        case SDLK_PAGEUP: return kVK_PageUp;
        case SDLK_DELETE: return kVK_ForwardDelete;
        case SDLK_END: return kVK_End;
        case SDLK_PAGEDOWN: return kVK_PageDown;
        case SDLK_RIGHT: return kVK_RightArrow;
        case SDLK_LEFT: return kVK_LeftArrow;
        case SDLK_DOWN: return kVK_DownArrow;
        case SDLK_UP: return kVK_UpArrow; 
        case SDLK_KP_MULTIPLY: return kVK_ANSI_KeypadMultiply;
        case SDLK_KP_PLUS: return kVK_ANSI_KeypadPlus;  
        case SDLK_KP_MINUS: return kVK_ANSI_KeypadMinus;
        case SDLK_KP_PERIOD: return kVK_ANSI_KeypadDecimal;
        case SDLK_KP_DIVIDE: return kVK_ANSI_KeypadDivide;
        case SDLK_KP_ENTER: return kVK_ANSI_KeypadEnter;
        case SDLK_KP_EQUALS: return kVK_ANSI_KeypadEquals;
        case SDLK_LCTRL: return kVK_Control;
        case SDLK_LSHIFT: return kVK_Shift; 
        case SDLK_RSHIFT: return kVK_RightShift;
        case SDLK_LGUI: return kVK_Command;
        case SDLK_RGUI: return kVK_RightCommand;
    }
    return -1;
}

#endif
