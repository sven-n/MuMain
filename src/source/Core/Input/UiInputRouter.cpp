#include "stdafx.h"
#include "UiInputRouter.h"

namespace Core::Input
{
    namespace
    {
        IUiInputConsumer* g_pUiInputConsumer = nullptr;
    }

    void SetUiInputConsumer(IUiInputConsumer* consumer)
    {
        g_pUiInputConsumer = consumer;
    }

    bool RouteToUi(SDL_Event& event, SDL_Window* window)
    {
        return g_pUiInputConsumer ? g_pUiInputConsumer->ProcessSdlEvent(event, window) : true;
    }

    bool IsMouseOverUI()
    {
        return g_pUiInputConsumer && g_pUiInputConsumer->IsMouseOverUI();
    }
}
