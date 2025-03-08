// NewUINameWindow.cpp: implementation of the CNewUINameWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUINameWindow.h"
#include "ZzzBmd.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "CSChaosCastle.h"
#include "PersonalShopTitleImp.h"
#include "MatchEvent.h"
#include "MapManager.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUINameWindow::CNewUINameWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_bShowItemName = false;
}

SEASON3B::CNewUINameWindow::~CNewUINameWindow()
{
    Release();
}

bool SEASON3B::CNewUINameWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NAME_WINDOW, this);

    SetPos(x, y);

    Show(true);

    return true;
}

void SEASON3B::CNewUINameWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUINameWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUINameWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUINameWindow::UpdateKeyEvent()
{
    if (SEASON3B::IsPress(VK_MENU) == true)
    {
        m_bShowItemName = !m_bShowItemName;
    }

    return true;
}

bool SEASON3B::CNewUINameWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUINameWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderName();
    RenderTimes();
    matchEvent::RenderMatchTimes();
    RenderBooleans();
    DrawPersonalShopTitleImp();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUINameWindow::RenderName()
{
    if (g_bGMObservation == true)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER* c = &CharactersClient[i];
            OBJECT* o = &c->Object;
            if (o->Live && o->Kind == KIND_PLAYER)
            {
                if (IsShopTitleVisible(c) == false)
                {
                    CreateChat(c->ID, L"", c);
                }
            }
        }
    }

#ifndef GUILD_WAR_EVENT
    if (gMapManager.InChaosCastle() == true && (SelectedNpc != -1 || SelectedCharacter != -1))
    {
        return;
    }
#endif//GUILD_WAR_EVENT

    if (SelectedItem != -1 || SelectedNpc != -1 || SelectedCharacter != -1)
    {
        if (SelectedNpc != -1)
        {
            CHARACTER* c = &CharactersClient[SelectedNpc];
            OBJECT* o = &c->Object;
            CreateChat(c->ID, L"", c);
        }
        else if (SelectedCharacter != -1)
        {
            CHARACTER* c = &CharactersClient[SelectedCharacter];

            OBJECT* o = &c->Object;
            if (o->Kind == KIND_MONSTER)
            {
                g_pRenderText->SetTextColor(255, 230, 200, 255);
                g_pRenderText->SetBgColor(100, 0, 0, 255);
                g_pRenderText->RenderText(320, 2, c->ID, 0, 0, RT3_WRITE_CENTER);

                if (c->HealthStatus > 0)
                {
                    const auto steps = 20;
                    const auto borderWidth = 2.f;
                    const auto widthPerStep = 4;
                    const auto stepSeparatorWidth = 1;
                    const auto stepsWidth = steps * widthPerStep - 2 * stepSeparatorWidth; //38.f;
                    const auto totalWidth = stepsWidth + borderWidth * 2;
                    
                    auto ScreenX = 320 - totalWidth / 2;
                    auto ScreenY = 15;
                    

                    EnableAlphaTest();
                    glColor4f(0.f, 0.f, 0.f, 0.5f);
                    RenderColor((float)(ScreenX + 1), (float)(ScreenY + 1), totalWidth, 5.f);

                    EnableAlphaBlend();
                    glColor3f(0.2f, 0.0f, 0.0f);
                    RenderColor((float)ScreenX, (float)ScreenY, totalWidth, 5.f);

                    glColor3f(50.f / 255.f, 10 / 255.f, 0.f);
                    RenderColor((float)(ScreenX + borderWidth), (float)(ScreenY + borderWidth), stepsWidth, 1.f);

                    int stepHP = (int)(c->HealthStatus * steps);

                    glColor3f(250.f / 255.f, 10 / 255.f, 0.f);
                    for (int k = 0; k < stepHP; ++k)
                    {
                        RenderColor(
                            (float)(ScreenX + borderWidth + (k * widthPerStep)),
                            (float)(ScreenY + borderWidth),
                            widthPerStep - stepSeparatorWidth,
                            2.f);
                    }
                    DisableAlphaBlend();
                }
            }
            else
#ifdef ASG_ADD_GENS_SYSTEM
#ifndef PBG_MOD_STRIFE_GENSMARKRENDER
                if (!::IsStrifeMap(World) || Hero->m_byGensInfluence == c->m_byGensInfluence)
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
#endif	// ASG_ADD_GENS_SYSTEM
                {
                    if (IsShopTitleVisible(c) == false)
                    {
                        CreateChat(c->ID, L"", c);
                    }
                }
        }
        else if (SelectedItem != -1)
        {
            RenderItemName(SelectedItem, &Items[SelectedItem].Object, &Items[SelectedItem].Item, false);
        }
    }

    if (m_bShowItemName || SEASON3B::IsRepeat(VK_MENU))
    {
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            OBJECT* o = &Items[i].Object;
            if (o->Live)
            {
                if (o->Visible && i != SelectedItem)
                {
                    RenderItemName(i, o, &Items[i].Item, true);
                }
            }
        }
    }
}

float SEASON3B::CNewUINameWindow::GetLayerDepth()
{
    return 1.0f;
}