#pragma once

#include <cstdint>

#include "UI/Scaling/UITransform.h"

namespace UI::Layout
{
    inline constexpr float ForegroundPanelLayerDepth = 10.65f;

    Scaling::LayoutMode ForInterface(std::uint32_t interfaceKey);
}
