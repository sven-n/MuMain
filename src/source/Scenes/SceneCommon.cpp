///////////////////////////////////////////////////////////////////////////////
// SceneCommon.cpp - Shared utilities used by multiple scenes
// Extracted from ZzzScene.cpp as part of scene refactoring
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SceneCommon.h"
#include "SceneCore.h"
#include "Camera/CameraProjection.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Render/Renderer/MuRenderer.h"

//=============================================================================
// Character Selection State Implementation
//=============================================================================

// Global instance
CharacterSelectionState g_characterSelection;

// Legacy global reference (for backward compatibility)
int& SelectedHero = g_characterSelection.GetLegacyReference();

//=============================================================================
// Scene Initialization State Implementation
//=============================================================================

// Global instance
SceneInitializationState g_sceneInit;

// Legacy global references (for backward compatibility)
bool& InitLogIn = g_sceneInit.LegacyRefInitLogIn();
bool& InitLoading = g_sceneInit.LegacyRefInitLoading();
bool& InitCharacterScene = g_sceneInit.LegacyRefInitCharacterScene();
bool& InitMainScene = g_sceneInit.LegacyRefInitMainScene();
bool& EnableMainRender = g_sceneInit.LegacyRefEnableMainRender();

//=============================================================================
// Scene Common Utilities
//=============================================================================
#include "Engine/Object/ZzzInterface.h"
#include "UI/HUD/Notices.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzOpenData.h"
#include "UI/Core/UIManager.h"
#include "Audio/DSPlaySound.h"
#include "App/Platform/Windows/Local.h"
#include "I18N/All.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "UI/Core/SceneUICoordinator.h"

// External variable declarations (defined in ZzzScene.cpp or other files)
extern wchar_t AbuseFilter[][20];
extern wchar_t AbuseNameFilter[][20];
extern int AbuseFilterNumber;
extern int AbuseNameFilterNumber;
extern wchar_t InputText[12][256];
extern int AskYesOrNo;
extern ITEM PickItem;
extern ITEM TargetItem;
extern int ErrorMessage;
extern bool MouseLButtonPush;
extern int MouseUpdateTime;
extern int MouseUpdateTimeMax;
extern bool g_bEnterPressed;

// External variables from ZzzScene.cpp
extern int g_iMessageTextStart;
extern char g_cMessageTextCurrNum;
extern char g_cMessageTextNum;
extern bool g_bTimeCheck;
extern int g_iBackupTime;
extern EGameScene SceneFlag;

///////////////////////////////////////////////////////////////////////////////
// Abuse Filter Functions
///////////////////////////////////////////////////////////////////////////////

bool CheckAbuseFilter(wchar_t* Text, bool bCheckSlash)
{
    if (bCheckSlash == true)
    {
        if (Text[0] == '/')
        {
            return false;
        }
    }

    int icntText = 0;
    wchar_t TmpText[2048];
    int textLength = wcslen(Text);
    for (int i = 0; i < textLength; ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckAbuseNameFilter(wchar_t* Text)
{
    int icntText = 0;
    wchar_t TmpText[256];
    int textLength = wcslen(Text);
    for (int i = 0; i < textLength; ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseNameFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseNameFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckName()
{
    if (CheckAbuseNameFilter(InputText[0]) || CheckAbuseFilter(InputText[0]) ||
        FindText(InputText[0], L" ") || FindText(InputText[0], L"　") ||
        FindText(InputText[0], L".") || FindText(InputText[0], L"·") || FindText(InputText[0], L"∼") ||
        FindText(InputText[0], L"Webzen") || FindText(InputText[0], L"WebZen") || FindText(InputText[0], L"webzen") || FindText(InputText[0], L"WEBZEN") ||
        FindText(InputText[0], I18N::Game::Operation) || FindText(InputText[0], I18N::Game::WEBZEN))
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// UI Utility Functions
///////////////////////////////////////////////////////////////////////////////

// SeparateTextIntoLines lives in src/source/Core/Text/TextLineWrap.cpp so it can be
// unit-tested without dragging in the full scene/UI translation unit.

///////////////////////////////////////////////////////////////////////////////
// Audio and Viewport Functions
///////////////////////////////////////////////////////////////////////////////

void SetEffectVolumeLevel(int level)
{
    if (level > 10)
        level = 10;
    if (level < 0)
        level = 0;

    if (g_platformAudio != nullptr)
    {
        g_platformAudio->SetSFXVolume(static_cast<float>(level) / 10.0f);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Rendering Functions
///////////////////////////////////////////////////////////////////////////////

void RenderInfomation()
{
    UI::Notices::Render();

    CSceneUICoordinator::Instance().Render();

    // The login/character-scene cursor render that used to happen here moved to
    // Winmain.cpp's SetPostRmlUiCallback registration, which fires after RmlUi's own render
    // pass -- RmlUi always renders after this point in the frame, so drawing the cursor here
    // (before it) let an opaque RmlUi panel visually cover it.
}

///////////////////////////////////////////////////////////////////////////////
// Time Utility Functions
///////////////////////////////////////////////////////////////////////////////

bool GetTimeCheck(int DelayTime)
{
    int PresentTime = timeGetTime();

    if (g_bTimeCheck)
    {
        g_iBackupTime = PresentTime;
        g_bTimeCheck = false;
    }

    if (g_iBackupTime + DelayTime <= PresentTime)
    {
        g_bTimeCheck = true;
        return true;
    }
    return false;
}

bool IsEnterPressed() {
    return g_bEnterPressed;
}

void SetEnterPressed(bool enterpressed) {
    g_bEnterPressed = enterpressed;
}
