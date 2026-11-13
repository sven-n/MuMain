///////////////////////////////////////////////////////////////////////////////
// WebzenScene.cpp - Webzen title/intro scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Utilities/Random.h"
#include "WebzenScene.h"
#include "SceneCore.h"
#include "Core/Platform/ScopedSystemCursor.h"
#include "Core/Utilities/StagedLoad.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Scenes/TitleSceneUI.h"
#include "UI/Core/WindowSystem.h"

#include <SDL3/SDL_timer.h>

#include <algorithm>
#include <chrono>
#include <optional>

// External declarations
extern EGameScene SceneFlag;
extern CErrorReport g_ErrorReport;

// Constants for background image selection
constexpr int BACKGROUND_SELECTION_PERCENTAGE = 100;
constexpr int CLASSIC_BACKGROUND_PROBABILITY = 70;

// Bitmap indices for title scene resources
constexpr int TITLE_BITMAP_BASE = BITMAP_TITLE;
constexpr int TITLE_BITMAP_BACK_02 = BITMAP_TITLE + 1;
constexpr int TITLE_BITMAP_LOGO = BITMAP_TITLE + 3;
constexpr int TITLE_BITMAP_PATTERN = BITMAP_TITLE + 5;
constexpr int TITLE_BITMAP_DYNAMIC_START = BITMAP_TITLE + 6;
constexpr int TITLE_BITMAP_DYNAMIC_END = BITMAP_TITLE + 14;

enum class BackgroundTheme
{
    Classic,
    Season5
};

/**
 * @brief Determines which background theme to use based on configured probability.
 * @return BackgroundTheme::Classic or BackgroundTheme::Season5
 */
static BackgroundTheme SelectBackgroundTheme()
{
    int roll = Random::Rand() % BACKGROUND_SELECTION_PERCENTAGE;
    return (roll <= CLASSIC_BACKGROUND_PROBABILITY) ? BackgroundTheme::Classic : BackgroundTheme::Season5;
}

/**
 * @brief Loads the appropriate background image set based on theme.
 * @param theme The background theme to load (Classic or Season5)
 */
static void LoadBackgroundTheme(BackgroundTheme theme)
{
    if (theme == BackgroundTheme::Classic)
    {
        LoadBitmap(L"Interface\\lo_back_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }
    else
    {
        LoadBitmap(L"Interface\\lo_back_s5_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }
}

/**
 * @brief Loads all common title scene bitmaps.
 */
static void LoadCommonTitleBitmaps()
{
    LoadBitmap(L"Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
    LoadBitmap(L"Interface\\New_lo_back_02.jpg", BITMAP_TITLE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_121518.tga", BITMAP_TITLE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_lo.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Interface\\lo_back_s5_03.jpg", BITMAP_TITLE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_back_s5_04.jpg", BITMAP_TITLE + 7, GL_LINEAR);
}

/**
 * @brief Unloads all title scene bitmaps.
 */
static void UnloadTitleBitmaps()
{
    DeleteBitmap(TITLE_BITMAP_BASE);
    DeleteBitmap(TITLE_BITMAP_BACK_02);
    DeleteBitmap(TITLE_BITMAP_LOGO);
    DeleteBitmap(TITLE_BITMAP_PATTERN);

    for (int i = TITLE_BITMAP_DYNAMIC_START; i < TITLE_BITMAP_DYNAMIC_END; ++i)
    {
        DeleteBitmap(i);
    }
}

namespace
{
// Load work per frame: the title screen redraws at about 30 fps while the data loads.
constexpr std::chrono::milliseconds StartupStepBudget{33};

// The startup load, across frames; it holds what the load needs for as long as it runs.
struct StartupLoad
{
    explicit StartupLoad(std::vector<Core::StagedLoad::Step> steps) : steps(std::move(steps)) {}

    // The game draws no cursor while it loads.
    Core::Platform::ScopedSystemCursor cursor;
    TextureLoadBatch textureBatch;
    Core::StagedLoad steps;
    Uint64 longestPassMs = 0;
};

std::optional<StartupLoad> s_load;
// The title screen is up; it goes before the first frame that no longer draws it.
bool s_titleShown = false;
} // namespace

// Loads the startup data a few steps at a time, before the frame begins, then hands over to login.
void PumpWebzenScene()
{
    if (SceneFlag != WEBZEN_SCENE)
    {
        return;
    }

    if (!s_load)
    {
        if (!OpenFont())
        {
            return;
        }
        ClearInput();

        LoadCommonTitleBitmaps();
        BackgroundTheme theme = SelectBackgroundTheme();
        LoadBackgroundTheme(theme);

        TitleSceneUI::CreateSceneUI();
        s_titleShown = true;

        FogEnable = false;

        ::EnableAlphaTest();

        std::vector<Core::StagedLoad::Step> steps = OpenBasicDataSteps();
        steps.emplace_back([] { g_pNewUISystem->LoadMainSceneInterface(); });
        s_load.emplace(std::move(steps));
        // The title draws once before the loading starts, as soon as the window shows.
        return;
    }

    const Uint64 passStart = SDL_GetTicks();
    const bool done = s_load->steps.Pump(StartupStepBudget);
    s_load->longestPassMs = std::max(s_load->longestPassMs, SDL_GetTicks() - passStart);
    if (!done)
    {
        return;
    }

    const Uint64 longestPassMs = s_load->longestPassMs;
    // Ends the texture batch and gives the cursor back.
    s_load.reset();
    Bitmaps.LogTextureFilteringSummary("startup");
    mu::log::Get("render")->info(
        "Startup data loaded {} ms after start; the longest frame of loading spent {} ms on it", SDL_GetTicks(),
        longestPassMs);
    g_ErrorReport.Write(L"> Loading ok.\r\n");
    // Login draws from this frame on, with no empty frame between; the title is released before it begins.
    SceneFlag = LOG_IN_SCENE;
}

/**
 * @brief Webzen title/intro scene - draws the startup load's progress; PumpWebzenScene does the loading.
 * @param hDC Device context for rendering
 */
void WebzenScene(HDC hDC)
{
    if (s_load)
    {
        TitleSceneUI::RenderSceneUI(hDC, static_cast<DWORD>(s_load->steps.Done()),
                                    static_cast<DWORD>(s_load->steps.Total()));
    }
}

void EndWebzenScene()
{
    s_load.reset();
    if (s_titleShown)
    {
        s_titleShown = false;
        TitleSceneUI::ReleaseSceneUI();
        UnloadTitleBitmaps();
    }
}
