#include "stdafx.h"

#include "UI/Options/OptionUpdates.h"

#include "GameLogic/KeyConfiguration/ChatBoxConfiguration.h"
#include "UI/Core/WindowSystem.h"
#include "UI/HUD/ChatLogWindow.h"
#include "UI/Options/OptionWindow.h"

namespace UI::Options
{
void ApplySaved(bool autoAttack, bool whisperSound, bool slideHelp)
{
    g_pOption->SetAutoAttack(autoAttack);
    g_pOption->SetWhisperSound(whisperSound);
    g_pOption->SetSlideHelp(slideHelp);
}

bool ApplySavedChatBox(std::uint8_t chatBox)
{
    namespace ChatBox = GameLogic::KeyConfiguration::ChatBox;
    const int lines = ChatBox::Lines(chatBox);
    if (lines > 0)
    {
        g_pChatListBox->SetNumberOfShowingLines(lines);
    }
    const float alpha = ChatBox::Alpha(chatBox);
    if (alpha > 0.f)
    {
        g_pChatListBox->SetBackAlpha(alpha);
    }
    return lines > 0 && alpha > 0.f;
}

void ResetForNewCharacter()
{
    g_pOption->SetAutoAttack(true);
    g_pOption->SetWhisperSound(false);
    g_pOption->SetSlideHelp(true);
    g_pChatListBox->SetNumberOfShowingLines(mu::ui::window::CChatLogWindow::DefaultShowingLines);
    g_pChatListBox->SetBackAlpha(mu::ui::window::CChatLogWindow::DefaultBackAlpha);
}

bool IsWhisperSoundOn()
{
    return g_pOption->IsWhisperSound();
}
}
