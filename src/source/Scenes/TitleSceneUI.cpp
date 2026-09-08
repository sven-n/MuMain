//*****************************************************************************
// File: TitleSceneUI.cpp
//*****************************************************************************

#include "stdafx.h"
#include "TitleSceneUI.h"

#include "Core/Input/Input.h"
#include "Render/Sprites/Sprite.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementProgress.h>

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#endif

// #define	UIM_TS_BG_BLACK		0
#define UIM_TS_BACK0 0
#define UIM_TS_BACK1 1
#define UIM_TS_121518 3
#define UIM_TS_BACK2 5
#define UIM_TS_BACK3 6
#define UIM_TS_BACK4 7
#define UIM_TS_BACK5 8
#define UIM_TS_BACK6 9
#define UIM_TS_BACK7 10
#define UIM_TS_BACK8 11
#define UIM_TS_BACK9 12
#define UIM_TS_MAX 13

namespace
{
    CSprite* s_asprTitle = nullptr;

    // Replaces CGaugeBar (the last consumer of that class -- see this file's own comment at
    // CreateSceneUI()'s gauge setup) -- free statics, not members, since TitleSceneUI has no
    // owning class, same shape LoadingScene.cpp's own s_rmlLoadingDoc uses. s_pGaugeFill is cached
    // once so RenderSceneUI() can call SetValue()/SetMax() directly -- no data-model binding
    // needed, RmlUi's own <progress> element already owns that state.
    Rml::ElementDocument* s_rmlDoc = nullptr;
    Rml::ElementProgress* s_pGaugeFill = nullptr;
}

void TitleSceneUI::CreateSceneUI()
{
    ReleaseSceneUI();

    CInput& rInput = CInput::Instance();
    float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
    float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

    s_asprTitle = new CSprite[UIM_TS_MAX];

    float _fScaleXTemp = (float)rInput.GetScreenWidth() / 1280.0f;
    float _fScaleYTemp = (float)rInput.GetScreenHeight() / 1024.0f;

    s_asprTitle[UIM_TS_BACK0].Create(400, 69, BITMAP_TITLE, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    s_asprTitle[UIM_TS_BACK0].SetPosition(0, 0);

    s_asprTitle[UIM_TS_BACK1].Create(400, 69, BITMAP_TITLE + 1, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    s_asprTitle[UIM_TS_BACK1].SetPosition(400, 0);

    s_asprTitle[UIM_TS_BACK2].Create(400, 100, BITMAP_TITLE + 6, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    s_asprTitle[UIM_TS_BACK2].SetPosition(0, 500);

    s_asprTitle[UIM_TS_BACK3].Create(400, 100, BITMAP_TITLE + 7, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    s_asprTitle[UIM_TS_BACK3].SetPosition(400, 500);

    s_asprTitle[UIM_TS_BACK4].Create(512, 512, BITMAP_TITLE + 8, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK4].SetPosition(0, 119);

    s_asprTitle[UIM_TS_BACK5].Create(512, 512, BITMAP_TITLE + 9, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK5].SetPosition(512, 119);

    s_asprTitle[UIM_TS_BACK6].Create(256, 512, BITMAP_TITLE + 10, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK6].SetPosition(1024, 119);

    s_asprTitle[UIM_TS_BACK7].Create(512, 223, BITMAP_TITLE + 11, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK7].SetPosition(0, 512 + 119);

    s_asprTitle[UIM_TS_BACK8].Create(512, 223, BITMAP_TITLE + 12, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK8].SetPosition(512, 512 + 119);

    s_asprTitle[UIM_TS_BACK9].Create(256, 223, BITMAP_TITLE + 13, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    s_asprTitle[UIM_TS_BACK9].SetPosition(1024, 512 + 119);

    s_asprTitle[UIM_TS_121518].Create(256, 206, BITMAP_TITLE + 3, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                      fScaleY);
    s_asprTitle[UIM_TS_121518].SetPosition(544, 60);

    // Loading bar, RmlUi's own <progress> element now (title_scene.rml/.rcss) -- was CGaugeBar,
    // the last consumer of that class. IsCreated() guards against RmlUiRuntime not being up yet,
    // matching LoadingScene.cpp's own guard around its structurally identical sprite->RmlUi port.
    if (RmlUiRuntime::Instance().IsCreated())
    {
        s_rmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
            "Data/Interface/RmlUi/title_scene.rml");
        if (s_rmlDoc)
        {
            s_rmlDoc->Show();
            s_pGaugeFill = rmlui_dynamic_cast<Rml::ElementProgress*>(s_rmlDoc->GetElementById("gauge_fill"));

            // Pushed as real px, not dp -- dp only matches this scene's still-native background
            // sprites' own fScaleX/fScaleY (800x600-reference, independent per axis, unclamped)
            // scaling at exactly the 640x480 reference size (RmlUi's dp unit is a different,
            // uniform/clamped/damped formula against a 640x480 reference -- confirmed live: the
            // gauge only lined up at 640x480 before this, misplaced everywhere else). Recomputing
            // the identical fScaleX/fScaleY math CGaugeBar::Create()/SetPosition() used keeps this
            // pixel-exact with the sprites at any resolution, same reasoning login_main.rcss's own
            // #panel comment gives for pushing its geometry from C++ instead of static dp.
            if (s_pGaugeFill)
            {
                s_pGaugeFill->SetProperty("left", std::to_string(static_cast<int>(72 * fScaleX)) + "px");
                s_pGaugeFill->SetProperty("top", std::to_string(static_cast<int>(540 * fScaleY)) + "px");
                s_pGaugeFill->SetProperty("width", std::to_string(static_cast<int>(656 * fScaleX)) + "px");
                s_pGaugeFill->SetProperty("height", std::to_string(static_cast<int>(15 * fScaleY)) + "px");
            }
        }
    }

    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        s_asprTitle[i].Show();
    }
    s_asprTitle[UIM_TS_121518].Show(false);
}

void TitleSceneUI::ReleaseSceneUI()
{
    SAFE_DELETE_ARRAY(s_asprTitle);

    s_pGaugeFill = nullptr;
    if (s_rmlDoc)
    {
        s_rmlDoc->Close();
        s_rmlDoc = nullptr;
    }
}

void TitleSceneUI::RenderSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal)
{
    // Each loading update gets its own frame so the progress bar is visible.
    // When called inside the game loop, temporarily close the caller frame,
    // present this loading update, then reopen the caller frame.
    const bool wasFrameActive = mu::GetRenderer().IsFrameActive();
    if (wasFrameActive)
    {
        mu::GetRenderer().EndFrame();
    }

    mu::GetRenderer().BeginFrame();

    ::BeginOpengl();
    mu::GetRenderer().ClearScreen();
    ::BeginBitmap();

    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        if (i == 2)
            continue;
        s_asprTitle[i].Render();
    }

    // Not rendered here -- RmlUiRuntime's SetPreSubmitCallback fires automatically before this
    // function's own EndFrame() submits below, same as every other RmlUi document (see this
    // file's own comment at CreateSceneUI()'s gauge setup). SetMax()/SetValue() own the
    // normalization (clamped, divide-by-zero-safe -- ElementProgress::GetMax() floors to 1
    // internally), no manual fraction math needed.
    if (s_pGaugeFill)
    {
        s_pGaugeFill->SetMax(static_cast<float>(dwTotal));
        s_pGaugeFill->SetValue(static_cast<float>(dwNow));
    }

    ::EndBitmap();
    ::EndOpengl();
#ifdef _EDITOR
    // Always render ImGui (shows "Open Editor" button when closed, or full UI when open)
    g_MuEditorCore.Render();
#endif

    mu::GetRenderer().EndFrame();

    if (wasFrameActive)
    {
        mu::GetRenderer().BeginFrame();
    }
}
