#include "stdafx.h"

#include "UI/Options/OptionUpdates.h"

#include "UI/Core/WindowSystem.h"
#include "UI/Options/OptionWindow.h"

namespace UI::Options
{
void ApplySaved(bool autoAttack, bool whisperSound, bool slideHelp)
{
    g_pOption->SetAutoAttack(autoAttack);
    g_pOption->SetWhisperSound(whisperSound);
    g_pOption->SetSlideHelp(slideHelp);
}

void ResetForNewGame()
{
    g_pOption->SetAutoAttack(true);
    g_pOption->SetWhisperSound(false);
}

bool IsWhisperSoundOn()
{
    return g_pOption->IsWhisperSound();
}
}
