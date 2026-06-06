#include "stdafx.h"
#include "UI/Chat/Whisper.h"

#include "Engine/Object/ZzzInfomation.h"     // CharacterAttribute
#include "UI/NewUI/NewUISystem.h"            // g_pSystemLogBox
#include "UI/NewUI/HUD/NewUIChatLogWindow.h" // SEASON3B::TYPE_SYSTEM_MESSAGE
#include "UI/Legacy/UIControls.h"            // g_pRenderText
#include "I18N/All.h"

namespace
{
    constexpr int WHISPER_ID_SLOTS = 10;

    int     WhisperID_Num = 0;
    wchar_t WhisperRegistID[MAX_USERNAME_SIZE + 1][WHISPER_ID_SLOTS];
}

namespace UI::Chat::Whisper
{
    bool CheckLevel(int requiredLevel, wchar_t* targetId)
    {
        int level = CharacterAttribute->Level;

        if (level >= requiredLevel)
            return true;

        for (int i = 0; i < WHISPER_ID_SLOTS; ++i)
        {
            if (wcscmp(targetId, WhisperRegistID[i]) == 0)
            {
                return true;
            }
        }

        g_pSystemLogBox->AddText(I18N::Game::YouCanUseTheWhisperCommandAtCharacterLevel6, SEASON3B::TYPE_SYSTEM_MESSAGE);

        return false;
    }

    void Register(int requiredLevel, wchar_t* targetId)
    {
        int level = CharacterAttribute->Level;

        if (level < requiredLevel)
        {
            bool noMatch = true;
            for (int i = 0; i < WHISPER_ID_SLOTS; ++i)
            {
                if (wcscmp(targetId, WhisperRegistID[i]) == 0)
                {
                    noMatch = false;
                    break;
                }
            }

            if (noMatch)
            {
                wcscpy(WhisperRegistID[WhisperID_Num], targetId);
                WhisperID_Num++;

                if (WhisperID_Num >= WHISPER_ID_SLOTS)
                {
                    WhisperID_Num = 0;
                }
            }
        }
    }

    void Clear()
    {
        ZeroMemory(WhisperRegistID, sizeof(char) * (MAX_USERNAME_SIZE + 1) * WHISPER_ID_SLOTS);
    }

    void RenderList()
    {
        for (int i = 0; i < WHISPER_ID_SLOTS; ++i)
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->SetBgColor(0, 0, 0, 255);
            g_pRenderText->RenderText(100, 100 + (i * 10), WhisperRegistID[i]);
        }
    }
}
