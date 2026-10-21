#define SDL_MAIN_HANDLED
#include <Windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>
#include <chrono>
#include <fstream>
#include <string>

static std::ofstream evidence;
static bool fixedHook;

static LRESULT LegacyWindowMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_SYSKEYDOWN:
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        return 0;
    }
    }
    return fixedHook ? 0 : DefWindowProc(window, message, wparam, lparam);
}

static bool SDLCALL WindowMessageHook(void*, MSG* message)
{
    if (message->message == WM_CLOSE)
        return true;
    const bool windowAction = message->message == WM_SYSCOMMAND
        || message->message == WM_NCLBUTTONDOWN || message->message == WM_NCLBUTTONUP;
    if (windowAction)
        evidence << "hook-enter " << std::hex << message->message << " " << message->wParam << std::dec << std::endl;
    LegacyWindowMessage(message->hwnd, message->message, message->wParam, message->lParam);
    if (windowAction)
        evidence << "hook-return" << std::endl;
    return true;
}

int main(int count, char** arguments)
{
    if (count != 3) return 2;
    fixedHook = std::string(arguments[1]) == "fixed";
    evidence.open(arguments[2]);
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO)) return 3;
    const char* title = fixedHook ? "MuMain SDL hook fixed hook" : "MuMain SDL hook original hook";
    SDL_Window* window = SDL_CreateWindow(title, 480, 240, 0);
    if (!window) return 4;
    SDL_SetWindowsMessageHook(WindowMessageHook, nullptr);
    auto nextHeartbeat = std::chrono::steady_clock::now();
    const auto began = nextHeartbeat;
    bool running = true;
    unsigned long long loops = 0;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                running = false;
        }
        ++loops;
        const auto now = std::chrono::steady_clock::now();
        if (now >= nextHeartbeat)
        {
            evidence << "heartbeat " << std::chrono::duration_cast<std::chrono::milliseconds>(now - began).count()
                     << " " << loops << " " << SDL_GetWindowFlags(window) << std::endl;
            nextHeartbeat = now + std::chrono::milliseconds(250);
        }
        SDL_Delay(10);
    }
    SDL_SetWindowsMessageHook(nullptr, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    evidence << "normal-exit" << std::endl;
}
