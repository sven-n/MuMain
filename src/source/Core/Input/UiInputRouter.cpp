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

    IUiInputConsumer* ActiveUiInputConsumer()
    {
        return g_pUiInputConsumer;
    }

    void CancelSyntheticMousePress(IUiInputConsumer* owner, unsigned char button, SDL_Window* window)
    {
        if (owner && owner == g_pUiInputConsumer)
            owner->CancelSyntheticMousePress(button, window);
    }

    bool IsMouseOverUI()
    {
        return g_pUiInputConsumer && g_pUiInputConsumer->IsMouseOverUI();
    }
}
