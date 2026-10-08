#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
struct DuelWindowRmlModel
{
    float boldTextPx = 0.f; // native bold text size in physical px

    Rml::String heroName, heroScore, enemyName, enemyScore;
};
} // namespace mu::ui::window
