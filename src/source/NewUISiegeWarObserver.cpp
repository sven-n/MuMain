// NewUISiegeWarObserver.cpp: implementation of the CNewUISiegeWarObserver class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISiegeWarObserver.h"

#include "UIControls.h"

#include "ZzzCharacter.h"

using namespace SEASON3B;

CNewUISiegeWarObserver::CNewUISiegeWarObserver()
{
}

CNewUISiegeWarObserver::~CNewUISiegeWarObserver()
{
}

bool SEASON3B::CNewUISiegeWarObserver::OnCreate(int x, int y)
{
    return true;
}

void SEASON3B::CNewUISiegeWarObserver::OnRelease()
{
}

bool SEASON3B::CNewUISiegeWarObserver::OnUpdate()
{
    return true;
}

bool SEASON3B::CNewUISiegeWarObserver::OnRender()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, m_fMiniMapAlpha);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    RenderCharPosInMiniMap();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUISiegeWarObserver::OnSetPos(int x, int y)
{
}

void SEASON3B::CNewUISiegeWarObserver::RenderCharPosInMiniMap()
{
    float fPosX, fPosY;

    glColor4f(0.8f, 0.8f, 0.8f, m_fMiniMapAlpha);

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero && (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            fPosX = ((c->PositionX)) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            fPosY = (256 - (c->PositionY)) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
            RenderColor(fPosX, fPosY, 3, 3);
        }
    }
}

bool SEASON3B::CNewUISiegeWarObserver::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    return true;
}

bool SEASON3B::CNewUISiegeWarObserver::OnUpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUISiegeWarObserver::OnBtnProcess()
{
    return false;
}

void SEASON3B::CNewUISiegeWarObserver::OnLoadImages()
{
}

void SEASON3B::CNewUISiegeWarObserver::OnUnloadImages()
{
}