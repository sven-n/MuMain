///////////////////////////////////////////////////////////////////////////////
// WebzenScene.cpp - Webzen title/intro scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebzenScene.h"
#include "../ZzzScene.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzTexture.h"
#include "../ZzzInterface.h"
#include "../ZzzOpenData.h"
#include "../UIMng.h"
#include "../NewUISystem.h"

// External declarations
extern EGameScene SceneFlag;
extern CErrorReport g_ErrorReport;

// Constants for background image selection
constexpr int BACKGROUND_SELECTION_PERCENTAGE = 100;
constexpr int CLASSIC_BACKGROUND_PROBABILITY = 70;

void WebzenScene(HDC hDC)
{
    CUIMng& rUIMng = CUIMng::Instance();

    if (!OpenFont())
    {
        return;
    }
    ClearInput();

    LoadBitmap(L"Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
    LoadBitmap(L"Interface\\New_lo_back_02.jpg", BITMAP_TITLE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_121518.tga", BITMAP_TITLE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_lo.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Interface\\lo_back_s5_03.jpg", BITMAP_TITLE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_back_s5_04.jpg", BITMAP_TITLE + 7, GL_LINEAR);
    if (rand() % BACKGROUND_SELECTION_PERCENTAGE <= CLASSIC_BACKGROUND_PROBABILITY)
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

    rUIMng.CreateTitleSceneUI();

    FogEnable = false;

    ::EnableAlphaTest();
    OpenBasicData(hDC);

    g_pNewUISystem->LoadMainSceneInterface();

    CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 11);

    rUIMng.ReleaseTitleSceneUI();
    DeleteBitmap(BITMAP_TITLE);
    DeleteBitmap(BITMAP_TITLE + 1);
    DeleteBitmap(BITMAP_TITLE + 3);
    DeleteBitmap(BITMAP_TITLE + 5);

    for (int i = 6; i < 14; ++i)
        DeleteBitmap(BITMAP_TITLE + i);

    g_ErrorReport.Write(L"> Loading ok.\r\n");

    SceneFlag = LOG_IN_SCENE;	//
}
