#pragma once

union SDL_Event;

// The client's life as the platform drives it: SDL's main callbacks call these (AppMain.cpp).
bool StartClient(char* commandLine);
bool HandleClientEvent(SDL_Event& event);
bool IterateClient();
void ShutdownClient();
