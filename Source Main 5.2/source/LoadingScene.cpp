//*****************************************************************************
// File: LoadingScene.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoadingScene.h"

#include "Input.h"

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