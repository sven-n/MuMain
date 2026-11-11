// The client's entry point on every platform. SDL runs the loop and calls these once each: init, then an event or
// a frame at a time, then quit, even after a failed init. The browser and Android drive it the same way.
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "App/Platform/ClientMain.h"

#include <string>

SDL_AppResult SDL_AppInit(void** /*appState*/, int argc, char* argv[])
{
    std::string commandLine;
    for (int index = 1; index < argc; ++index)
    {
        if (!commandLine.empty())
            commandLine += ' ';
        commandLine += argv[index];
    }
    return StartClient(commandLine.data()) ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void* /*appState*/, SDL_Event* event)
{
    return HandleClientEvent(*event) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppIterate(void* /*appState*/)
{
    return IterateClient() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

void SDL_AppQuit(void* /*appState*/, SDL_AppResult /*result*/)
{
    ShutdownClient();
}
