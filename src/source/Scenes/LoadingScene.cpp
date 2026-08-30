//*****************************************************************************
// File: LoadingScene.cpp
//*****************************************************************************

#include "stdafx.h"

#include "LoadingScene.h"

#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "SceneCore.h"
#include "Engine/Object/ZzzInterface.h"
#include "SceneCommon.h"
#include "UI/NewUI/Dialogs/ReconnectDialog.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include <RmlUi/Core/ElementDocument.h>


#ifdef _EDITOR
#include "Core/MuEditorCore.h"
#endif

CLoadingScene::CLoadingScene()
{
}

CLoadingScene::~CLoadingScene()
{
}

void CLoadingScene::Create()
{
    CInput rInput = CInput::Instance();
    float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
    float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

    int anHeight[LDS_BACK_MAX] = { 512, 512, 88, 88 };
    for (int i = 0; i < LDS_BACK_MAX; ++i)
    {
        m_asprBack[i].Create(400, anHeight[i], BITMAP_TITLE + i, 0, NULL, 0, 0, false,
            SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
        m_asprBack[i].Show(true);
    }

    m_asprBack[1].SetPosition(400, 0, X);
    m_asprBack[2].SetPosition(0, 512, Y);
    m_asprBack[3].SetPosition(400, 512);
}

void CLoadingScene::Release()
{
    for (int i = 0; i < LDS_BACK_MAX; ++i)
        m_asprBack[i].Release();
}

void CLoadingScene::Render()
{
    for (int i = 0; i < LDS_BACK_MAX; ++i)
    {
        m_asprBack[i].Render();
    }
}

// External variables
extern int LoadingWorld;
extern bool FogEnable;
extern EGameScene SceneFlag;

void LoadingScene(HDC hDC)
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_Start");

    // RmlUi migration plan Phase 1 pilot: replaces CLoadingScene's 4-tile CSprite rendering
    // (the class above is left completely untouched -- not deleted, per the plan's retirement
    // criteria: don't remove legacy code until parity is confirmed against a real build) with an
    // equivalent RmlUi document, Data/Interface/RmlUi/loading.rml + loading.rcss. Loaded and
    // closed within this single call, matching this function's own one-shot-per-flash lifecycle
    // (renders exactly one frame, then flips SceneFlag away below). No explicit
    // RmlUiRuntime::Update()/Render() call needed here, unlike the RHI-based port this was ported
    // from -- this branch's IMuRenderer::SetPreSubmitCallback (see RmlUiRuntime::Create()) already
    // fires automatically once per frame, for every scene, right before EndOpengl()'s underlying
    // SDL_GPU submit -- so simply Show()-ing the document before EndOpengl() runs (below) is
    // enough for it to render this one frame.
    Rml::ElementDocument* rmlLoadingDoc = nullptr;

    if (!InitLoading)
    {
        LoadingWorld = 9999999;
        InitLoading = true;

        ::StopMp3(MUSIC_LOGIN_THEME);

        if (RmlUiRuntime::Instance().IsCreated())
        {
            rmlLoadingDoc = RmlUiRuntime::Instance().GetContext()->LoadDocument("Data/Interface/RmlUi/loading.rml");
            if (rmlLoadingDoc)
                rmlLoadingDoc->Show();
        }
    }

    FogEnable = false;
    ::BeginOpengl();
    mu::GetRenderer().ClearScreen();

    ::EndOpengl();
#ifdef _EDITOR
    // Always render ImGui (shows "Open Editor" button when closed, or full UI when open)
    g_MuEditorCore.Render();

    // Render game cursor on top of ImGui if not hovering UI
    extern bool g_bRenderGameCursor;
    if (g_bRenderGameCursor)
    {
        BeginBitmap();
        RenderCursor();
        EndBitmap();
    }
#endif
    UI::Reconnect::RenderDialog();

    if (rmlLoadingDoc)
        rmlLoadingDoc->Close();

    SceneFlag = MAIN_SCENE;

    ::ClearInput();

    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_End");
}
