#pragma once

#include <cstdint>
#include <functional>

namespace Rml
{
class DataModelConstructor;
}

namespace UI::RmlBridge
{
// Binds "window_close" on a window's model, so its markup closes it the same way everywhere: the
// frame's corner X (`<div id="frame_corner_close" data-event-click="window_close">`, drawn by
// docked_panel_frame.rcss) and any exit button. The click is RmlUi's, so no native hit test and no
// units. The first form hides `windowId`; the second runs a window's own close (a trade cancels).
void BindWindowClose(Rml::DataModelConstructor& constructor, std::uint32_t windowId);
void BindWindowClose(Rml::DataModelConstructor& constructor, std::function<void()> close);
} // namespace UI::RmlBridge
