// NewUIDuelWindow.cpp: implementation of the CNewUIDuelWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUIDuelWindow.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "UI/Widgets/UIControls.h"
#include "GameLogic/Combat/DuelMgr.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CNewUIDuelWindow::CNewUIDuelWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

mu::ui::window::CNewUIDuelWindow::~CNewUIDuelWindow()
{
    Release();
}

bool mu::ui::window::CNewUIDuelWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUEL_WINDOW, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void mu::ui::window::CNewUIDuelWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CNewUIDuelWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CNewUIDuelWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CNewUIDuelWindow::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CNewUIDuelWindow::Update()
{
    return true;
}

bool mu::ui::window::CNewUIDuelWindow::Render()
{
    EnableAlphaTest();

    RenderFrame();
    RenderContents();

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CNewUIDuelWindow::RenderFrame()
{
    RenderImage(IMAGE_DUEL_BACK, m_Pos.x, m_Pos.y, 131, 70);
}

void mu::ui::window::CNewUIDuelWindow::RenderContents()
{
    wchar_t strMyScore[12];
    wchar_t strDuelScore[12];
    mu_swprintf(strMyScore, L"%d", g_DuelMgr.GetScore(DUEL_HERO));
    mu_swprintf(strDuelScore, L"%d", g_DuelMgr.GetScore(DUEL_ENEMY));

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

float mu::ui::window::CNewUIDuelWindow::GetLayerDepth()
{
    return 1.1f;
}

void mu::ui::window::CNewUIDuelWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_ground.tga", IMAGE_DUEL_BACK, GL_LINEAR);
}

void mu::ui::window::CNewUIDuelWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUEL_BACK);
}