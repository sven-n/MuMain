#include "UI/Combat/MonsterHealthBar.h"

bool UI::Combat::HealthBar::ShouldRenderSelected(float healthStatus)
{
    return healthStatus != 0.0f;
}
