//*****************************************************************************
// File: UIMng.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UIMng.h"
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/ServerMsgWin.h"
#include "UI/Windows/ServerSelWin.h"
#include "UI/Windows/SysMenuWin.h"
#include "UI/Windows/LoginWin.h"
#include "Character/CharSelMainWin.h"
#include "Character/CharMakeWin.h"
#include "Character/CharInfoBalloonMng.h"
#include "UI/Windows/LoginMainWin.h"
#include "Core/Globals/_enum.h"
#include "Core/Input/Input.h"
#include "Audio/DSPlaySound.h"
#include "Render/Sprites/Sprite.h"
#include "UI/Widgets/GaugeBar.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

#include "UIControls.h"
#include "Network/Server/ServerListManager.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "UI/Scaling/UITransform.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#endif

#define DOCK_EXTENT 10

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
bool InputDiagnosticsEnabled()
{
    static const bool enabled = std::getenv("MU_INPUT_DIAGNOSTICS") != nullptr;
    return enabled;
}

const char* WindowName(const CUIMng&, const CWin* window)
{
    return window == nullptr ? "none" : "unknown";
}
} // namespace

CUIMng::CUIMng()
{
    m_asprTitle = NULL;
    m_pgbLoding = NULL;
    m_pLoadingScene = NULL;
}

CUIMng::~CUIMng() {}

CUIMng& CUIMng::Instance()
{
    static CUIMng s_UIMng;
    return s_UIMng;
}

void CUIMng::CreateTitleSceneUI()
{
    ReleaseTitleSceneUI();

    CInput& rInput = CInput::Instance();
    float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
    float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

    m_asprTitle = new CSprite[UIM_TS_MAX];

    float _fScaleXTemp = (float)rInput.GetScreenWidth() / 1280.0f;
    float _fScaleYTemp = (float)rInput.GetScreenHeight() / 1024.0f;

    m_asprTitle[UIM_TS_BACK0].Create(400, 69, BITMAP_TITLE, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    m_asprTitle[UIM_TS_BACK0].SetPosition(0, 0);

    m_asprTitle[UIM_TS_BACK1].Create(400, 69, BITMAP_TITLE + 1, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    m_asprTitle[UIM_TS_BACK1].SetPosition(400, 0);

    m_asprTitle[UIM_TS_BACK2].Create(400, 100, BITMAP_TITLE + 6, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    m_asprTitle[UIM_TS_BACK2].SetPosition(0, 500);

    m_asprTitle[UIM_TS_BACK3].Create(400, 100, BITMAP_TITLE + 7, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                     fScaleY);
    m_asprTitle[UIM_TS_BACK3].SetPosition(400, 500);

    m_asprTitle[UIM_TS_BACK4].Create(512, 512, BITMAP_TITLE + 8, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK4].SetPosition(0, 119);

    m_asprTitle[UIM_TS_BACK5].Create(512, 512, BITMAP_TITLE + 9, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK5].SetPosition(512, 119);

    m_asprTitle[UIM_TS_BACK6].Create(256, 512, BITMAP_TITLE + 10, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK6].SetPosition(1024, 119);

    m_asprTitle[UIM_TS_BACK7].Create(512, 223, BITMAP_TITLE + 11, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK7].SetPosition(0, 512 + 119);

    m_asprTitle[UIM_TS_BACK8].Create(512, 223, BITMAP_TITLE + 12, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK8].SetPosition(512, 512 + 119);

    m_asprTitle[UIM_TS_BACK9].Create(256, 223, BITMAP_TITLE + 13, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT,
                                     _fScaleXTemp, _fScaleYTemp);
    m_asprTitle[UIM_TS_BACK9].SetPosition(1024, 512 + 119);

    m_asprTitle[UIM_TS_121518].Create(256, 206, BITMAP_TITLE + 3, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX,
                                      fScaleY);
    m_asprTitle[UIM_TS_121518].SetPosition(544, 60);

    m_pgbLoding = new CGaugeBar;

    RECT rc = {0, 0, 656, 15};
    m_pgbLoding->Create(4, 15, BITMAP_TITLE + 5, &rc, 0, 0, -1, true, fScaleX, fScaleY);

    m_pgbLoding->SetPosition(72, 540);
    for (int i = 0; i < UIM_TS_MAX; ++i)
    {
        m_asprTitle[i].Show();
    }
    m_pgbLoding->Show();
    m_asprTitle[UIM_TS_121518].Show(false);
    m_nScene = UIM_SCENE_TITLE;
}

void CUIMng::ReleaseTitleSceneUI()
{
    SAFE_DELETE_ARRAY(m_asprTitle);
    SAFE_DELETE(m_pgbLoding);

    m_nScene = UIM_SCENE_NONE;
}

void CUIMng::RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal)
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
        m_asprTitle[i].Render();
    }

    m_pgbLoding->SetValue(dwNow, dwTotal);
    m_pgbLoding->Render();

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

void CUIMng::Create()
{
    m_bCursorOnUI = false;
    m_bBlockCharMove = false;
    m_bWinActive = false;
    m_nScene = UIM_SCENE_NONE;

    return;
}

void CUIMng::RemoveWinList()
{
    CWin* pWin;
    while (m_WinList.GetCount())
    {
        pWin = (CWin*)m_WinList.RemoveHead();
        pWin->Release();
    }
}

void CUIMng::Release()
{
    RemoveWinList();

    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    m_nScene = UIM_SCENE_NONE;
}

void CUIMng::CreateLoginScene()
{
    RemoveWinList();

    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    // WindowWidth/WindowHeight (ZzzOpenglUtil.cpp), not CInput::Instance().GetScreenWidth()/
    // GetScreenHeight() -- see LoginWin.cpp's LoginUIScaleRatio() for why: CInput's own copy of
    // the screen size isn't guaranteed to already match WindowWidth/WindowHeight (the exact
    // values every migrated window's own Create()/SetPosition() now uses internally). This
    // function is the one place that positions g_LoginWin and g_LoginMainWin relative to each
    // other -- their hit-test boxes sit only ~11px apart vertically at the reference resolution.
    // A drift between this function's source of screen size and each window's own internal one
    // could close that gap into an overlap, but that's no longer a starvation risk either way
    // (docs/newui-legacy-merger.md, Phase 3): both windows' dispatch now goes through the same
    // depth-sorted CNewUIManager claim, not a first-checked-wins list walk.
    g_MsgWin.Create();
    g_MsgWin.SetPosition((static_cast<int>(WindowWidth) - 352) / 2, (static_cast<int>(WindowHeight) - 113) / 2);

    g_SysMenuWin.Create();

    g_LoginMainWin.Create();

    int nBaseY = int(567.0f / 600.0f * static_cast<float>(WindowHeight));
    g_LoginMainWin.SetPosition(30, nBaseY - g_LoginMainWin.GetHeight() - 11);

    g_ServerSelWin.Create();
    g_ServerSelWin.SetPosition((static_cast<int>(WindowWidth) - g_ServerSelWin.GetWidth()) / 2,
                               (static_cast<int>(WindowHeight) - g_ServerSelWin.GetHeight()) / 2);

    g_LoginWin.Create();
    g_LoginWin.SetPosition((static_cast<int>(WindowWidth) - g_LoginWin.GetWidth()) / 2,
                           (static_cast<int>(WindowHeight) - g_LoginWin.GetHeight()) * 2 / 3);

    g_CreditWin.Create();

    m_bSysMenuWinShow = false;
    m_nScene = UIM_SCENE_LOGIN;
}

void CUIMng::CreateCharacterScene()
{
    RemoveWinList();

    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    g_CharInfoBalloonMng.Create();

    CInput& rInput = CInput::Instance();

    g_MsgWin.Create();
    g_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2, (rInput.GetScreenHeight() - 113) / 2);

    g_ServerMsgWin.Create();
    int nBaseY = int(31.0f / 600.0f * (float)rInput.GetScreenHeight());
    g_ServerMsgWin.SetPosition(10, nBaseY + 10);

    g_SysMenuWin.Create();

    g_CharSelMainWin.Create();

    g_CharMakeWin.Create();
    g_CharMakeWin.SetPosition((rInput.GetScreenWidth() - 454) / 2, (rInput.GetScreenHeight() - 406) / 2);

    g_CharSelMainWin.UpdateDisplay();
    g_CharInfoBalloonMng.UpdateDisplay();

    g_CharSelMainWin.Show(true);

    m_bSysMenuWinShow = false;
    m_nScene = UIM_SCENE_CHARACTER;
}

void CUIMng::CreateMainScene()
{
    RemoveWinList();

    g_CharInfoBalloonMng.Release();
    g_CreditWin.Release();
    g_ServerMsgWin.Release();
    g_ServerSelWin.Release();
    g_MsgWin.Release();
    g_SysMenuWin.Release();
    g_CharSelMainWin.Release();
    g_CharMakeWin.Release();
    g_LoginMainWin.Release();
    g_LoginWin.Release();

    m_nScene = UIM_SCENE_MAIN;
}

void CUIMng::RepositionSceneUI()
{
    // A lightweight SetPosition sweep isn't enough: CSprite caches
    // m_fScrHeight = WindowHeight at Create() time and uses it for the
    // Y-flipped coordinate math in SetPosition(). When the window resizes,
    // every sprite's cached screen height is stale, so a pure SetPosition
    // call lands the windows in the wrong place.
    //
    // The only clean way to refresh that cache is to re-Create the sprites,
    // which is exactly what the scene's Create*Scene() function does. But
    // that also resets each window's m_bShow flag, so we snapshot the
    // current visibility here and restore it right after.
    if (m_nScene == UIM_SCENE_LOGIN)
    {
        const bool wasShown_MsgWin = g_MsgWin.IsVisible();
        const bool wasShown_SysMenuWin = g_SysMenuWin.IsVisible();
        const bool wasShown_LoginMainWin = g_LoginMainWin.IsVisible();
        const bool wasShown_ServerSelWin = g_ServerSelWin.IsVisible();
        const bool wasShown_LoginWin = g_LoginWin.IsVisible();
        const bool wasShown_CreditWin = g_CreditWin.IsVisible();

        CreateLoginScene();

        // Restore visibility BEFORE re-populating dynamic windows: child
        // elements like server/group buttons read `CWin::m_bShow` of their
        // parent when `UpdateDisplay()` decides which sub-elements to show.
        // If the parent is still hidden at that moment, nothing renders.
        if (wasShown_MsgWin)
            g_MsgWin.Show(true);
        if (wasShown_SysMenuWin)
            g_SysMenuWin.Show(true);
        if (wasShown_LoginMainWin)
            g_LoginMainWin.Show(true);
        if (wasShown_ServerSelWin)
            g_ServerSelWin.Show(true);
        if (wasShown_LoginWin)
            g_LoginWin.Show(true);
        if (wasShown_CreditWin)
            g_CreditWin.Show(true);

        // Re-populate the server / server-group buttons from the existing
        // network-side data. Create() clears the button labels, so without
        // this the server list and groups render empty after a resolution
        // change.
        g_ServerSelWin.UpdateDisplay();
    }
    else if (m_nScene == UIM_SCENE_CHARACTER)
    {
        // CreateCharacterScene() ends with an explicit g_CharSelMainWin.Show(true)
        // so visibility of the main panel is already preserved. Other character-
        // scene windows (msg box, server msg, char make) are shown on demand
        // by game events, matching the fresh-scene state.
        CreateCharacterScene();
    }
    // MainScene uses the new UI system which resizes itself; nothing to do.
}

CWin* CUIMng::SetActiveWin(CWin* pWin)
{
    CWin* pBeforeActWin = (CWin*)m_WinList.GetHead();

    if (pBeforeActWin == NULL)
        return NULL;

    if (pBeforeActWin->IsActive())
        pBeforeActWin->Active(FALSE);
    else
        pBeforeActWin = NULL;

    if (pWin->IsShow())
    {
        if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
            return NULL;

        m_bWinActive = true;
        m_WinList.AddHead(pWin);

        if (InputDiagnosticsEnabled())
        {
            mu::log::Get("input")->info("[InputDiag] active window={} previous={}", WindowName(*this, pWin),
                                        WindowName(*this, pBeforeActWin));
        }
    }

    return pBeforeActWin;
}

void CUIMng::ShowWin(CWin* pWin)
{
    pWin->Show(TRUE);
    SetActiveWin(pWin);
}

void CUIMng::HideWin(CWin* pWin)
{
    if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
        return;

    pWin->Show(FALSE);
    pWin->Active(FALSE);
    m_WinList.AddTail(pWin);

    pWin = (CWin*)m_WinList.GetHead();
    if (pWin->IsShow())
        m_bWinActive = true;
}

void CUIMng::CheckDockWin()
{
    NODE* position = m_WinList.GetHeadPosition();
    if (NULL == position)
        return;

    CWin* pMovWin = (CWin*)m_WinList.GetNext(position);

    if (pMovWin->GetState() != WS_MOVE)
        return;

    pMovWin->SetDocking(false);

    RECT rcMovWin = {pMovWin->GetTempXPos(), pMovWin->GetTempYPos(), pMovWin->GetTempXPos() + pMovWin->GetWidth(),
                     pMovWin->GetTempYPos() + pMovWin->GetHeight()};

    RECT rcDock[4] = {{rcMovWin.left - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT, rcMovWin.left + DOCK_EXTENT,
                       rcMovWin.top + DOCK_EXTENT},
                      {rcMovWin.right - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT, rcMovWin.right + DOCK_EXTENT,
                       rcMovWin.top + DOCK_EXTENT},
                      {rcMovWin.left - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT, rcMovWin.left + DOCK_EXTENT,
                       rcMovWin.bottom + DOCK_EXTENT},
                      {rcMovWin.right - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT, rcMovWin.right + DOCK_EXTENT,
                       rcMovWin.bottom + DOCK_EXTENT}};

    CInput& rInput = CInput::Instance();

    POINT pt[4] = {{0, 0},
                   {static_cast<LONG>(rInput.GetScreenWidth()), 0},
                   {0, static_cast<LONG>(rInput.GetScreenHeight())},
                   {static_cast<LONG>(rInput.GetScreenWidth()), static_cast<LONG>(rInput.GetScreenHeight())}};

    if (::PtInRect(&rcDock[0], pt[0]))
    {
        pMovWin->SetPosition(pt[0].x, pt[0].y);
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[1], pt[1]))
    {
        pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), pt[1].y);
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[2], pt[2]))
    {
        pMovWin->SetPosition(pt[2].x, pt[2].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (::PtInRect(&rcDock[3], pt[3]))
    {
        pMovWin->SetPosition(pt[3].x - pMovWin->GetWidth(), pt[3].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (rcDock[0].top < 0 && rcDock[0].bottom > 0)
    {
        pMovWin->SetPosition(rcMovWin.left, 0);
        pMovWin->SetDocking(true);
    }
    else if (rcDock[2].top < pt[2].y && rcDock[2].bottom > pt[2].y)
    {
        pMovWin->SetPosition(rcMovWin.left, pt[2].y - pMovWin->GetHeight());
        pMovWin->SetDocking(true);
    }
    else if (rcDock[0].left < 0 && rcDock[0].right > 0)
    {
        pMovWin->SetPosition(0, rcMovWin.top);
        pMovWin->SetDocking(true);
    }
    else if (rcDock[1].left < pt[1].x && rcDock[1].right > pt[1].x)
    {
        pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), rcMovWin.top);
        pMovWin->SetDocking(true);
    }

    BOOL bEdgeDocking = FALSE;
    int i, j, nXCoord, nYCoord;
    CWin* pWin;

    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);
        if (!pWin->IsShow())
            continue;

        pt[0].x = pWin->GetXPos();
        pt[0].y = pWin->GetYPos();
        pt[1].x = pWin->GetXPos() + pWin->GetWidth();
        pt[1].y = pt[0].y;
        pt[2].x = pt[0].x;
        pt[2].y = pWin->GetYPos() + pWin->GetHeight();
        pt[3].x = pt[1].x;
        pt[3].y = pt[2].y;

        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
            {
                if (i != j && ::PtInRect(&rcDock[i], pt[j]))
                {
                    bEdgeDocking = TRUE;
                    goto DOCKING;
                }
            }
        }

        if (pt[0].x < rcDock[1].left && pt[1].x > rcDock[0].right)
        {
            nXCoord = rcMovWin.left;
            if (pt[2].y > rcDock[0].top && pt[2].y < rcDock[0].bottom)
            {
                if (SetDockWinPosition(pMovWin, nXCoord, pt[2].y))
                    continue;
            }
            else if (pt[0].y > rcDock[2].top && pt[0].y < rcDock[2].bottom)
            {
                if (SetDockWinPosition(pMovWin, nXCoord, pt[0].y - pMovWin->GetHeight()))
                    continue;
            }
        }
        else if (pt[0].y < rcDock[2].top && pt[2].y > rcDock[0].bottom)
        {
            nYCoord = rcMovWin.top;
            if (pt[1].x > rcDock[0].left && pt[1].x < rcDock[0].right)
            {
                if (SetDockWinPosition(pMovWin, pt[1].x, nYCoord))
                    continue;
            }
            else if (pt[0].x > rcDock[1].left && pt[0].x < rcDock[1].right)
            {
                if (SetDockWinPosition(pMovWin, pt[0].x - pMovWin->GetWidth(), nYCoord))
                    continue;
            }
        }
    }

DOCKING:
    if (bEdgeDocking)
    {
        switch (j)
        {
        case 0:
            switch (i)
            {
            case 1:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos();
                break;
            case 2:
                nXCoord = pWin->GetXPos();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
            }
            break;

        case 1:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos();
                break;
            case 2:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() + pWin->GetWidth() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
            }
            break;

        case 2:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 1:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 3:
                nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight() - pMovWin->GetHeight();
            }
            break;

        case 3:
            switch (i)
            {
            case 0:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 1:
                nXCoord = pWin->GetXPos() + pWin->GetWidth() - pMovWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight();
                break;
            case 2:
                nXCoord = pWin->GetXPos() + pWin->GetWidth();
                nYCoord = pWin->GetYPos() + pWin->GetHeight() - pMovWin->GetHeight();
            }
        }
        SetDockWinPosition(pMovWin, nXCoord, nYCoord);
    }
}

bool CUIMng::SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY)
{
    CInput& rInput = CInput::Instance();
    RECT rcDummy;
    RECT rcScreen = {0, 0, static_cast<LONG>(rInput.GetScreenWidth()), static_cast<LONG>(rInput.GetScreenHeight())};
    RECT rcMoveWin = {static_cast<LONG>(nDockX), static_cast<LONG>(nDockY),
                      static_cast<LONG>(nDockX + pMoveWin->GetWidth()),
                      static_cast<LONG>(nDockY + pMoveWin->GetHeight())};

    if (::IntersectRect(&rcDummy, &rcScreen, &rcMoveWin))
    {
        pMoveWin->SetPosition(nDockX, nDockY);
        pMoveWin->SetDocking(true);
        return true;
    }

    return false;
}

void CUIMng::Update(double dDeltaTick)
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    // New-style (CNewUIObj-tier) windows migrating off CWin/m_WinList (docs/rmlui-ui-system's
    // CUIMng/CNewUIManager merger) -- UpdateMouseEvent()/UpdateKeyEvent() dispatched first, and
    // independent of m_WinList's own emptiness below, since this must keep running even once
    // every CWin window has migrated away and m_WinList is permanently empty.
    // bNewStyleConsumedClick lets the legacy click-walk further down skip itself when a
    // new-style window already claimed the click -- reproducing the same "full-screen exclusive
    // window blocks everything behind it" behavior CCreditWin had as m_WinList's own head entry
    // before migrating off it. m_bCursorOnUI folds it in too (docs/newui-legacy-merger.md), so a
    // migrated modal like CMsgWin still blocks CharacterScene.cpp's world-click/rotation gating
    // the same way its old full-screen CWin::CursorInWin(WA_ALL) rect used to.
    m_NewStyleMng.UpdateMouseEvent();
    m_NewStyleMng.UpdateKeyEvent();
    const bool bNewStyleConsumedClick = m_NewStyleMng.GetActiveMouseUIObj() != nullptr;
    m_bCursorOnUI = bNewStyleConsumedClick;

    CInput& rInput = CInput::Instance();

    // ESC toggles system menu in login/character scenes. Checked before m_NewStyleMng.Update()
    // (below) runs -- a migrated window's own ESC handling (e.g. CMsgWin closing itself on ESC)
    // must not also flip g_MsgWin.IsVisible() to false in time to fool this same frame's check,
    // same relative ordering the legacy m_WinList per-window Update() walk further down already
    // has with this block.
    //
    // Unconditional -- must run even now that m_WinList is permanently empty (g_LoginWin, Phase 3,
    // was the last CWin ever added to it). This used to sit after an `if (m_WinList.IsEmpty()) {
    // m_NewStyleMng.Update(); return; }` early-out, which silently stopped this whole block (and
    // therefore ESC-driven system-menu open/close) from ever running the moment that list emptied
    // for good -- found via live testing, not caught by the build. The legacy per-window
    // bookkeeping below (m_bWinActive, the click walk, docking) has no such requirement, so it
    // still exits early when the list is empty.
    //
    // Also resets/sets m_bSysMenuToggledByEscThisFrame (own comment) -- found via live testing:
    // closing the menu here, then letting m_NewStyleMng.Update() (below) run g_LoginWin's own
    // Escape-cancel gate in the same frame, made a single ESC press both close the menu AND cancel
    // the login form behind it (a synchronous CreateSocket() reconnect, visible as a multi-second
    // hang). Unlike CCreditWin (depth 100) and CMsgWin (depth 50), both handled entirely inside
    // m_NewStyleMng's own depth-sorted dispatch (so a lower-depth g_LoginWin's Update() always runs
    // BEFORE theirs, seeing pre-close state for free), g_SysMenuWin's ESC close happens here,
    // completely outside that dispatch and unconditionally before it -- g_LoginWin has no ordering
    // protection against it on its own.
    m_bSysMenuToggledByEscThisFrame = false;
    if (rInput.IsKeyDown(VK_ESCAPE))
    {
        extern EGameScene SceneFlag;
        if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
        {
            if (g_SysMenuWin.IsVisible())
            {
                g_SysMenuWin.Show(false);
                m_bSysMenuToggledByEscThisFrame = true;
            }
            else if (!g_MsgWin.IsVisible() && !g_LoginWin.IsVisible() && !g_CreditWin.IsVisible() &&
                     !g_CharMakeWin.IsVisible())
            {
                ::PlayBuffer(SOUND_CLICK01);
                g_SysMenuWin.Show(true);
                m_bSysMenuToggledByEscThisFrame = true;
            }
        }
    }

    m_NewStyleMng.Update();

    if (m_WinList.IsEmpty())
        return;

    if (m_bWinActive)
    {
        CWin* pWin = (CWin*)m_WinList.GetHead();
        if (pWin->IsShow())
        {
            pWin->Active(true);
            m_bWinActive = false;
        }
    }

    CWin* pWin;
    NODE* position;

    if (rInput.IsLBtnDn() && !bNewStyleConsumedClick)
    {
        bool bWinClick = false;
        position = m_WinList.GetHeadPosition();
        while (position)
        {
            pWin = (CWin*)m_WinList.GetNext(position);

            if (pWin->CursorInWin(WA_ALL))
            {
                // IsLBtnDn() is level-triggered (true every frame the button stays physically
                // held, Input.cpp), so without this guard, holding a click on a window that's
                // already the active head re-enters SetActiveWin() every single frame. That call
                // always deactivates "whatever's currently head" first (its own pBeforeActWin
                // step) even when that's the very window being re-clicked, then only re-arms
                // activation for the NEXT frame via the deferred m_bWinActive flag -- so between
                // this frame's redundant deactivation and next frame's deferred reactivation,
                // m_bActive was false for every frame of a held click. Update(), called later this
                // same frame below, gates UpdateWhileActive() on m_bActive -- so click/keyboard
                // consumption for the clicked window (every migrated window's RmlUi companion
                // state, real CUITextInputBox keystroke polling) was silently starved for the
                // click's entire held duration, only catching up once the button was released and
                // this loop stopped re-triggering the cycle. Skipping the call when the window is
                // already active and already head makes it a true no-op instead of a bug.
                if (m_WinList.GetHead() != pWin || !pWin->IsActive())
                    SetActiveWin(pWin);
                bWinClick = true;
                break;
            }
        }

        if (!bWinClick)
        {
            pWin = (CWin*)m_WinList.GetHead();
            pWin->Active(false);
        }
    }
    else if (rInput.IsLBtnUp())
    {
        m_bBlockCharMove = false;
    }
    int nlist = m_WinList.GetCount();
    std::vector<CWin*> apTempWin(nlist);

    position = m_WinList.GetHeadPosition();
    for (int i = 0; i < nlist; ++i)
    {
        apTempWin[i] = (CWin*)m_WinList.GetNext(position);
        apTempWin[i]->ActiveBtns(false);
    }

    position = m_WinList.GetHeadPosition();
    CWin* hoveredWindow = nullptr;
    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);
        if (pWin->CursorInWin(WA_ALL))
        {
            pWin->ActiveBtns(true);
            hoveredWindow = pWin;
            break;
        }
    }

    if (InputDiagnosticsEnabled())
    {
        static CWin* previousHoveredWindow = nullptr;
        if (hoveredWindow != previousHoveredWindow)
        {
            mu::log::Get("input")->info("[InputDiag] hover window={} cursor=({},{})", WindowName(*this, hoveredWindow),
                                        rInput.GetCursorX(), rInput.GetCursorY());
            previousHoveredWindow = hoveredWindow;
        }
    }

    for (int i = 0; i < nlist; ++i)
    {
        apTempWin[i]->Update(dDeltaTick);
    }

    //	CheckKey();
    CheckDockWin();

    position = m_WinList.GetHeadPosition();
    while (position)
    {
        pWin = (CWin*)m_WinList.GetNext(position);

        switch (pWin->GetState())
        {
        case WS_ETC:
            m_bCursorOnUI = true;
            break;

        case WS_MOVE:
            //			eCursorActType = CURSOR_M;
            m_bCursorOnUI = true;
            break;

        case WS_EXTEND_UP:
            //			eCursorActType = CURSOR_V;
            m_bCursorOnUI = true;
            break;

        case WS_EXTEND_DN:
            //			eCursorActType = CURSOR_V;
            m_bCursorOnUI = true;
            break;
        }

        if (m_bCursorOnUI)
            break;

        if (pWin->CursorInWin(WA_ALL))
        {
            m_bCursorOnUI = true;
            break;
        }
    }
}

void CUIMng::Render()
{
    if (UIM_SCENE_NONE == m_nScene)
        return;

    const auto previousTransform = UI::Scaling::GetActiveTransform();
    UI::Scaling::SetActiveTransform(UI::Scaling::LegacyUiTransform(WindowWidth, WindowHeight));

    // g_CharInfoBalloonMng no longer rendered directly here (docs/newui-legacy-merger.md, Phase 3)
    // -- it's registered with m_NewStyleMng below like every other migrated window, which calls
    // its Render() under its own LayoutMode::Legacy-derived transform (equivalent to the
    // LegacyUiTransform set just above) as part of the generic per-object walk.

    CWin* pWin;
    NODE* position = m_WinList.GetTailPosition();
    while (position)
    {
        pWin = (CWin*)m_WinList.GetPrev(position);
        pWin->Render();
    }

    // New-style (CNewUIObj-tier) windows migrating off CWin/m_WinList -- rendered last so they
    // paint on top of whatever legacy CWin content remains, same as CCreditWin's old position at
    // m_WinList's own head (Render()'s tail-to-head walk paints the head last/on top).
    m_NewStyleMng.Render();

    UI::Scaling::SetActiveTransform(previousTransform);
}

void CUIMng::PopUpMsgWin(int nMsgCode, wchar_t* pszMsg)
{
    if (UIM_SCENE_NONE == m_nScene || UIM_SCENE_TITLE == m_nScene || UIM_SCENE_LOADING == m_nScene)
        return;

    if (UIM_SCENE_MAIN == m_nScene)
        return;

    g_MsgWin.PopUp(nMsgCode, pszMsg);
}

void CUIMng::AddServerMsg(wchar_t* pszMsg)
{
    if (UIM_SCENE_CHARACTER != m_nScene)
        return;

    g_ServerMsgWin.AddMsg(pszMsg);
}
