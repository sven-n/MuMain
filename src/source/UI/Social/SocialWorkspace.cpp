#include "stdafx.h"
#include "UI/Social/SocialWorkspace.h"

#include "UI/Placement/WindowPlacement.h"
#include "UI/Scaling/UITransform.h"

#include <algorithm>
#include <cmath>

namespace UI::Social
{
float DpRatio()
{
    return UI::Scaling::TypographyScale(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
}

int WorkspaceWidth()
{
    return std::max(static_cast<int>(std::floor(static_cast<float>(WindowWidth) / DpRatio())), 1);
}

int WorkspaceHeight()
{
    return std::max(static_cast<int>(std::floor(static_cast<float>(WindowHeight) / DpRatio())), 1);
}

float FreeAreaBottomPx()
{
    const float windowBottom = static_cast<float>(WindowHeight);
    UI::Placement::PlacementParticipant::Box hud;
    if (UI::Placement::SlotBox("main_hud", hud) && hud.top + hud.height >= windowBottom - 1.f)
        return std::max(hud.top, 0.f);
    return windowBottom;
}

int FreeAreaBottom()
{
    return static_cast<int>(FreeAreaBottomPx() / DpRatio());
}
} // namespace UI::Social
