//*****************************************************************************
// File: NewUIBattleSoccerScore.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Events/NewUIBattleSoccerScore.h"
#include "UI/Widgets/UIControls.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Textures/ZzzTexture.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CBattleSoccerScore::CBattleSoccerScore()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

mu::ui::window::CBattleSoccerScore::~CBattleSoccerScore()
{
    Release();
}

bool mu::ui::window::CBattleSoccerScore::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BATTLE_SOCCER_SCORE, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void mu::ui::window::CBattleSoccerScore::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CBattleSoccerScore::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CBattleSoccerScore::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CBattleSoccerScore::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CBattleSoccerScore::Update()
{
    return true;
}

bool mu::ui::window::CBattleSoccerScore::Render()
{
    ::EnableAlphaTest();

    RenderBackImage();
    RenderContents();

    ::DisableAlphaBlend();

    return true;
}

void mu::ui::window::CBattleSoccerScore::RenderBackImage()
{
    RenderImage(IMAGE_BSS_BACK, m_Pos.x, m_Pos.y, float(BSS_WIDTH), float(BSS_HEIGHT));
}

void mu::ui::window::CBattleSoccerScore::RenderContents()
{
    wchar_t szTemp[128];
    int nX = m_Pos.x + 30;
    int nY = m_Pos.y + 33;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 128);

    if (EnableGuildWar && Hero->GuildMarkIndex != -1)
    {
        if (HeroSoccerTeam == 0)
            g_pRenderText->SetTextColor(255, 60, 0, 255);
        else
            g_pRenderText->SetTextColor(0, 150, 255, 255);

        mu_swprintf(szTemp, L"%d", GuildWarScore[0]);
        g_pRenderText->RenderText(nX, nY, szTemp); // ����
        ::CreateGuildMark(Hero->GuildMarkIndex);
        ::RenderBitmap(BITMAP_GUILD, float(nX + 21), float(nY), 8, 8);                     // ��� ��ũ
        g_pRenderText->RenderText(nX + 33, nY, GuildMark[Hero->GuildMarkIndex].GuildName); // ����

        if (HeroSoccerTeam == 0)
            g_pRenderText->SetTextColor(0, 150, 255, 255);
        else
            g_pRenderText->SetTextColor(255, 60, 0, 255);

        mu_swprintf(szTemp, L"%d", GuildWarScore[1]);
        g_pRenderText->RenderText(nX, nY + 22, szTemp); // ����
        ::CreateGuildMark(FindGuildMark(GuildWarName));
        ::RenderBitmap(BITMAP_GUILD, float(nX + 21), float(nY + 22), 8, 8); // ��� ��ũ
        g_pRenderText->RenderText(nX + 33, nY + 22, GuildWarName);          // ����
    }
    else if (SoccerObserver)
    {
        g_pRenderText->SetTextColor(255, 60, 0, 255);
        mu_swprintf(szTemp, L"%d", GuildWarScore[0]);
        g_pRenderText->RenderText(nX, nY, szTemp);
        ::CreateGuildMark(FindGuildMark(SoccerTeamName[0]));
        ::RenderBitmap(BITMAP_GUILD, float(nX + 21), float(nY), 8, 8);
        g_pRenderText->RenderText(nX + 33, nY, SoccerTeamName[0]);

        g_pRenderText->SetTextColor(0, 150, 255, 255);
        mu_swprintf(szTemp, L"%d", GuildWarScore[1]);
        g_pRenderText->RenderText(nX, nY + 22, szTemp);
        ::CreateGuildMark(FindGuildMark(SoccerTeamName[1]));
        ::RenderBitmap(BITMAP_GUILD, float(nX + 21), float(nY + 22), 8, 8);
        g_pRenderText->RenderText(nX + 33, nY + 22, SoccerTeamName[1]);
    }
}

int mu::ui::window::CBattleSoccerScore::FindGuildMark(wchar_t* pszGuildName)
{
    for (int i = 0; i < MARK_EDIT; ++i)
    {
        MARK_t* p = &GuildMark[i];
        if (wcscmp(p->GuildName, pszGuildName) == 0)
        {
            return i;
        }
    }
    return 0;
}

float mu::ui::window::CBattleSoccerScore::GetLayerDepth()
{
    return 1.8f;
}

void mu::ui::window::CBattleSoccerScore::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_ground.tga", IMAGE_BSS_BACK, GL_LINEAR);
}

void mu::ui::window::CBattleSoccerScore::UnloadImages()
{
    DeleteBitmap(IMAGE_BSS_BACK);
}