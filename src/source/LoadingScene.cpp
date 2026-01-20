//*****************************************************************************
// File: LoadingScene.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoadingScene.h"

#include "Input.h"
#include "UIMng.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzScene.h"
#include "ZzzInterface.h"

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
extern bool InitLoading;
extern int LoadingWorld;
extern bool FogEnable;
extern EGameScene SceneFlag;

void LoadingScene(HDC hDC)
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_Start");

    CUIMng& rUIMng = CUIMng::Instance();
    if (!InitLoading)
    {
        LoadingWorld = 9999999;

        InitLoading = true;

        LoadBitmap(L"Interface\\LSBg01.JPG", BITMAP_TITLE, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg02.JPG", BITMAP_TITLE + 1, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg03.JPG", BITMAP_TITLE + 2, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg04.JPG", BITMAP_TITLE + 3, GL_LINEAR);

        ::StopMp3(MUSIC_LOGIN_THEME);

        rUIMng.m_pLoadingScene = new CLoadingScene;
        rUIMng.m_pLoadingScene->Create();
    }

    FogEnable = false;
    ::BeginOpengl();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ::BeginBitmap();

    rUIMng.m_pLoadingScene->Render();

    ::EndBitmap();
    ::EndOpengl();
    ::glFlush();
    ::SwapBuffers(hDC);

    SAFE_DELETE(rUIMng.m_pLoadingScene);

    SceneFlag = MAIN_SCENE;
    for (int i = 0; i < 4; ++i)
        ::DeleteBitmap(BITMAP_TITLE + i);

    ::ClearInput();

    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_End");
}