// NewUISiegeWarSoldier.cpp: implementation of the NewUISiegeWarSoldier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUISiegeWarSoldier.h"

#include "UI/Widgets/UIControls.h"

#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUISiegeWarSoldier::CNewUISiegeWarSoldier() {}

CNewUISiegeWarSoldier::~CNewUISiegeWarSoldier() {}

//---------------------------------------------------------------------------------------------
// OnCreate
bool mu::ui::window::CNewUISiegeWarSoldier::OnCreate(int x, int y)
{
    return true;
}

//---------------------------------------------------------------------------------------------
// OnRelease
void mu::ui::window::CNewUISiegeWarSoldier::OnRelease() {}

//---------------------------------------------------------------------------------------------
// OnUpdate
bool mu::ui::window::CNewUISiegeWarSoldier::OnUpdate()
{
    return true;
}

//---------------------------------------------------------------------------------------------
// OnRender
bool mu::ui::window::CNewUISiegeWarSoldier::OnRender()
{
    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    // 모든 캐릭터의 위치
    RenderCharPosInMiniMap();

    DisableAlphaBlend();

    EnableAlphaTest();

    // 지도상의 명령 Icon
    RenderCmdIconInMiniMap();

    DisableAlphaBlend();

    return true;
}

//---------------------------------------------------------------------------------------------
// OnCreate
void mu::ui::window::CNewUISiegeWarSoldier::OnSetPos(int x, int y) {}

//---------------------------------------------------------------------------------------------
// RenderCharPosInMiniMap
// 미니맵에 모든 캐릭터를 렌더
void mu::ui::window::CNewUISiegeWarSoldier::RenderCharPosInMiniMap()
{
    float fPosX, fPosY;

    // 미니멥에 플레이어 렌더
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero &&
            (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            OBJECT* o = &c->Object;

            if (g_isCharacterBuff(o, static_cast<eBuffState>(m_dwBuffState)))
            {
            }
            else
            {
            }
            if (o->Kind == KIND_NPC)
            {
            }
            else if (o->Kind == KIND_MONSTER && o->Type == MODEL_LIFE_STONE)
            {
            }

            fPosX = ((c->PositionX)) / m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
            fPosY = (256 - (c->PositionY)) / m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
            RenderColor(fPosX, fPosY, 3, 3);
        }
    }
}

//---------------------------------------------------------------------------------------------
// OnUpdateMouseEvent
bool mu::ui::window::CNewUISiegeWarSoldier::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    return true;
}

//---------------------------------------------------------------------------------------------
// OnUpdateKeyEvent
bool mu::ui::window::CNewUISiegeWarSoldier::OnUpdateKeyEvent()
{
    return true;
}
//---------------------------------------------------------------------------------------------
// OnBtnProcess
bool mu::ui::window::CNewUISiegeWarSoldier::OnBtnProcess()
{
    return false;
}

//---------------------------------------------------------------------------------------------
// OnLoadImages
void mu::ui::window::CNewUISiegeWarSoldier::OnLoadImages() {}

//---------------------------------------------------------------------------------------------
// OnUnloadImages
void mu::ui::window::CNewUISiegeWarSoldier::OnUnloadImages() {}