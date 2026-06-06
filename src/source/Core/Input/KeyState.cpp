#include "stdafx.h"
#include "Core/Input/KeyState.h"

#ifndef _WIN32
#include <SDL3/SDL.h>
#endif

namespace Core::Input
{
#ifndef _WIN32
    namespace
    {
        // Map a Win32 virtual-key code (or ASCII letter/digit) to an SDL
        // scancode. Returns SDL_SCANCODE_UNKNOWN for keys we don't translate.
        SDL_Scancode VkToScancode(int vk)
        {
            // ASCII letters and digits: VK codes equal their ASCII values.
            if (vk >= 'A' && vk <= 'Z') return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (vk - 'A'));
            if (vk >= '1' && vk <= '9') return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (vk - '1'));
            if (vk == '0') return SDL_SCANCODE_0;

            switch (vk)
            {
            case VK_UP:      return SDL_SCANCODE_UP;
            case VK_DOWN:    return SDL_SCANCODE_DOWN;
            case VK_LEFT:    return SDL_SCANCODE_LEFT;
            case VK_RIGHT:   return SDL_SCANCODE_RIGHT;
            case VK_INSERT:  return SDL_SCANCODE_INSERT;
            case VK_DELETE:  return SDL_SCANCODE_DELETE;
            case VK_HOME:    return SDL_SCANCODE_HOME;
            case VK_END:     return SDL_SCANCODE_END;
            case VK_PRIOR:   return SDL_SCANCODE_PAGEUP;
            case VK_NEXT:    return SDL_SCANCODE_PAGEDOWN;
            case VK_SPACE:   return SDL_SCANCODE_SPACE;
            case VK_RETURN:  return SDL_SCANCODE_RETURN;
            case VK_ESCAPE:  return SDL_SCANCODE_ESCAPE;
            case VK_TAB:     return SDL_SCANCODE_TAB;
            case VK_BACK:    return SDL_SCANCODE_BACKSPACE;
            case VK_F1:      return SDL_SCANCODE_F1;
            case VK_F2:      return SDL_SCANCODE_F2;
            case VK_F3:      return SDL_SCANCODE_F3;
            case VK_F4:      return SDL_SCANCODE_F4;
            case VK_F5:      return SDL_SCANCODE_F5;
            case VK_F6:      return SDL_SCANCODE_F6;
            case VK_F7:      return SDL_SCANCODE_F7;
            case VK_F8:      return SDL_SCANCODE_F8;
            case VK_F9:      return SDL_SCANCODE_F9;
            case VK_F10:     return SDL_SCANCODE_F10;
            case VK_F11:     return SDL_SCANCODE_F11;
            case VK_F12:     return SDL_SCANCODE_F12;
            default:         return SDL_SCANCODE_UNKNOWN;
            }
        }
    }
#endif // !_WIN32

    bool IsKeyDown(int virtualKey)
    {
#ifdef _WIN32
        // On Windows the legacy UI polls global key/mouse-button state that must
        // keep working while a Win32 child EDIT control holds keyboard focus.
        // SDL_GetKeyboardState only reflects keys delivered to the SDL window, so
        // it goes blind whenever an EDIT box is focused (e.g. the login screen).
        // Keep the global async query here; switch to the SDL path below once the
        // EDIT controls are replaced (issue #447). GetAsyncKeyState reports the
        // mouse buttons (VK_LBUTTON/...) and modifiers too, matching the original.
        return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
#else
        // Mouse buttons and modifiers come from the SDL mouse / mod state.
        switch (virtualKey)
        {
        case VK_LBUTTON: return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) != 0;
        case VK_RBUTTON: return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_RMASK) != 0;
        case VK_MBUTTON: return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MMASK) != 0;
        case VK_SHIFT:   return (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
        case VK_CONTROL: return (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
        case VK_MENU:    return (SDL_GetModState() & SDL_KMOD_ALT) != 0;
        default: break;
        }

        const SDL_Scancode sc = VkToScancode(virtualKey);
        if (sc == SDL_SCANCODE_UNKNOWN) return false;

        const bool* state = SDL_GetKeyboardState(nullptr);
        return state != nullptr && state[sc];
#endif // _WIN32
    }
}
