#pragma once

#include <functional>

namespace UI::Placement
{
// A component keeps ownership of its rendering and input while the workspace places it.
struct PlacementParticipant
{
    struct Size
    {
        float width = 0.f;
        float height = 0.f;
    };
    struct Box
    {
        float left = 0.f;
        float top = 0.f;
        float width = 0.f;
        float height = 0.f;
        float scale = 1.f;
    };

    std::function<bool()> visible;
    // Preferred size in component units, independent of the assigned box.
    std::function<Size()> measure;
    // Null restores the component's own placement when its theme has no slot.
    std::function<void(const Box*)> place;
};
}
