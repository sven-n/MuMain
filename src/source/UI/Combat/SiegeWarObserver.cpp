
#include "stdafx.h"
#include "UI/Combat/SiegeWarObserver.h"

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

void mu::ui::window::CSiegeWarObserver::OnSetPos(int x, int y) {}

// Everyone else in view, as a dot (the original's RenderCharPosInMiniMap()).
void mu::ui::window::CSiegeWarObserver::OnFillRmlModel(SiegeWarfareRmlModel& model)
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
