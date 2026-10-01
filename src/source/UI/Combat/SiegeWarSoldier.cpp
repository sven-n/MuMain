
#include "stdafx.h"
#include "UI/Combat/SiegeWarSoldier.h"


#include "Engine/Object/ZzzCharacter.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSiegeWarSoldier::CSiegeWarSoldier() {}

CSiegeWarSoldier::~CSiegeWarSoldier() {}

//---------------------------------------------------------------------------------------------
// OnCreate
bool mu::ui::window::CSiegeWarSoldier::OnCreate(int x, int y)
{
    return true;
}

//---------------------------------------------------------------------------------------------
// OnRelease
void mu::ui::window::CSiegeWarSoldier::OnRelease() {}

//---------------------------------------------------------------------------------------------
// OnUpdate
bool mu::ui::window::CSiegeWarSoldier::OnUpdate()
{
    return true;
}

void mu::ui::window::CSiegeWarSoldier::OnSetPos(int x, int y) {}

// Everyone else in view, as a dot, then the teams' commands (the original's OnRender(): its
// per-kind colour branches were empty, so every dot takes the same colour).
void mu::ui::window::CSiegeWarSoldier::OnFillRmlModel(SiegeWarfareRmlModel& model)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c != NULL && c->Object.Live && c != Hero &&
            (c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_MONSTER || c->Object.Kind == KIND_NPC))
        {
            const POINT pos = MiniMapPoint(c->PositionX, c->PositionY);
            model.dots.push_back({static_cast<float>(pos.x), static_cast<float>(pos.y)});
        }
    }

    FillCommands(model);
}

//---------------------------------------------------------------------------------------------
// OnUpdateMouseEvent
bool mu::ui::window::CSiegeWarSoldier::OnUpdateMouseEvent()
{
    if (OnBtnProcess())
        return false;

    return true;
}

//---------------------------------------------------------------------------------------------
// OnUpdateKeyEvent
bool mu::ui::window::CSiegeWarSoldier::OnUpdateKeyEvent()
{
    return true;
}
//---------------------------------------------------------------------------------------------
// OnBtnProcess
bool mu::ui::window::CSiegeWarSoldier::OnBtnProcess()
{
    return false;
}
