#pragma once

#include <functional>
#include <optional>

namespace Rml
{
    class Event;
}

// A level gauge: native's fill-by-level bar (base.rcss's .level-gauge). The bar element carries a
// .level-gauge-hit child, which takes the pointer and binds mousedown, drag and mousescroll to the
// owner's callback; the callback passes each event here. The hit child has its own `drag`, so a
// MakeDraggable() window around it does not move while the gauge is dragged.
namespace UI::RmlBridge
{
    // Maps the pointer's position along the bar to a level: `x` is in the bar's own layout units
    // from its left border edge (negative left of it), `width` is the bar's width in those units.
    using LevelFromPointer = std::function<int(float x, float width)>;

    // Applies one gauge event to `level`: a left press or a drag sets it from the pointer, a wheel
    // step moves it by one, clamped to 0..maxLevel. Wheel events stop here, so a scroll pane around
    // the gauge does not scroll too. Returns the new level when it changed.
    std::optional<int> ApplyLevelGaugeEvent(Rml::Event& event, int level, int maxLevel,
                                            const LevelFromPointer& fromPointer);
}
