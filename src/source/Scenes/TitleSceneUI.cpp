//*****************************************************************************
// File: TitleSceneUI.cpp
//*****************************************************************************

#include "stdafx.h"
#include "TitleSceneUI.h"

#include "Core/Input/Input.h"
#include "Render/Sprites/Sprite.h"
#include "UI/Widgets/GaugeBar.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

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
    CGaugeBar* s_pgbLoding = nullptr;
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

    s_pgbLoding = new CGaugeBar;

    RECT rc = {0, 0, 656, 15};
    s_pgbLoding->Create(4, 15, BITMAP_TITLE + 5, &rc, 0, 0, -1, true, fScaleX, fScaleY);

    s_pgbLoding->SetPosition(72, 540);
    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        s_asprTitle[i].Show();
    }
    s_pgbLoding->Show();
    s_asprTitle[UIM_TS_121518].Show(false);
}

void TitleSceneUI::ReleaseSceneUI()
{
    SAFE_DELETE_ARRAY(s_asprTitle);
    SAFE_DELETE(s_pgbLoding);
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

    s_pgbLoding->SetValue(dwNow, dwTotal);
    s_pgbLoding->Render();

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
