#include "stdafx.h"
#include "UiInputRouter.h"

#include "Audio/SoundSource.h"

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
        // The sounds of the windows' clicks and keys are the interface's (Q-SM13).
        const Audio::ScopedSoundPlace ui(Audio::SoundKind::Interface, -1, nullptr);
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
