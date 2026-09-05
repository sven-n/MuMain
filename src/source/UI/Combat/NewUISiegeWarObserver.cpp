// NewUISiegeWarObserver.cpp: implementation of the CNewUISiegeWarObserver class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUISiegeWarObserver.h"

#include "UI/Widgets/UIControls.h"

#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CNewUISiegeWarObserver::CNewUISiegeWarObserver() {}

CNewUISiegeWarObserver::~CNewUISiegeWarObserver() {}

bool mu::ui::window::CNewUISiegeWarObserver::OnCreate(int x, int y)
{
    return true;
}

void mu::ui::window::CNewUISiegeWarObserver::OnRelease() {}

bool mu::ui::window::CNewUISiegeWarObserver::OnUpdate()
{
    return true;
}

bool mu::ui::window::CNewUISiegeWarObserver::OnRender()
{
    EnableAlphaTest();
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    RenderCharPosInMiniMap();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CNewUISiegeWarObserver::OnSetPos(int x, int y) {}

void mu::ui::window::CNewUISiegeWarObserver::RenderCharPosInMiniMap()
{
    float fPosX, fPosY;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero &&
            (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            fPosX = ((c->PositionX)) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            fPosY = (256 - (c->PositionY)) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
            RenderColor(fPosX, fPosY, 3, 3);
        }
    }
}

bool mu::ui::window::CNewUISiegeWarObserver::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    return true;
}

bool mu::ui::window::CNewUISiegeWarObserver::OnUpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CNewUISiegeWarObserver::OnBtnProcess()
{
    return false;
}

void mu::ui::window::CNewUISiegeWarObserver::OnLoadImages() {}

void mu::ui::window::CNewUISiegeWarObserver::OnUnloadImages() {}