// NewUISiegeWarObserver.cpp: implementation of the CSiegeWarObserver class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUISiegeWarObserver.h"

#include "UI/Widgets/UIControls.h"

#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CSiegeWarObserver::CSiegeWarObserver() {}

CSiegeWarObserver::~CSiegeWarObserver() {}

bool mu::ui::window::CSiegeWarObserver::OnCreate(int x, int y)
{
    return true;
}

void mu::ui::window::CSiegeWarObserver::OnRelease() {}

bool mu::ui::window::CSiegeWarObserver::OnUpdate()
{
    return true;
}

bool mu::ui::window::CSiegeWarObserver::OnRender()
{
    EnableAlphaTest();
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    RenderCharPosInMiniMap();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CSiegeWarObserver::OnSetPos(int x, int y) {}

void mu::ui::window::CSiegeWarObserver::RenderCharPosInMiniMap()
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

bool mu::ui::window::CSiegeWarObserver::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    return true;
}

bool mu::ui::window::CSiegeWarObserver::OnUpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CSiegeWarObserver::OnBtnProcess()
{
    return false;
}

void mu::ui::window::CSiegeWarObserver::OnLoadImages() {}

void mu::ui::window::CSiegeWarObserver::OnUnloadImages() {}