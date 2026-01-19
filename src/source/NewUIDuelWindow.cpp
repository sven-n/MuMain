// NewUIDuelWindow.cpp: implementation of the CNewUIDuelWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDuelWindow.h"
#include "ZzzTexture.h"
#include "ZzzInventory.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "UIControls.h"
#include "DuelMgr.h"

using namespace SEASON3B;

SEASON3B::CNewUIDuelWindow::CNewUIDuelWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIDuelWindow::~CNewUIDuelWindow()
{
    Release();
}

bool SEASON3B::CNewUIDuelWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DUEL_WINDOW, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void SEASON3B::CNewUIDuelWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIDuelWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIDuelWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUIDuelWindow::UpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUIDuelWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIDuelWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderContents();

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIDuelWindow::RenderFrame()
{
    RenderImage(IMAGE_DUEL_BACK, m_Pos.x, m_Pos.y, 131, 70);
}

void SEASON3B::CNewUIDuelWindow::RenderContents()
{
    wchar_t strMyScore[12];
    wchar_t strDuelScore[12];
    swprintf(strMyScore, L"%d", g_DuelMgr.GetScore(DUEL_HERO));
    swprintf(strDuelScore, L"%d", g_DuelMgr.GetScore(DUEL_ENEMY));

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0, 0, 0, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(0, 150, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + 55, m_Pos.y + 33, g_DuelMgr.GetDuelPlayerID(DUEL_HERO));
    g_pRenderText->RenderText(m_Pos.x + 31, m_Pos.y + 33, strMyScore);
    g_pRenderText->SetTextColor(255, 25, 25, 255);
    g_pRenderText->RenderText(m_Pos.x + 55, m_Pos.y + 56, g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
    g_pRenderText->RenderText(m_Pos.x + 31, m_Pos.y + 56, strDuelScore);
}

float SEASON3B::CNewUIDuelWindow::GetLayerDepth()
{
    return 1.1f;
}

void SEASON3B::CNewUIDuelWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_ground.tga", IMAGE_DUEL_BACK, GL_LINEAR);
}

void SEASON3B::CNewUIDuelWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUEL_BACK);
}