#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace mu::ui::window
{
class CObject;
}

// Places windows where the active theme's workspace.rml puts their slots. Each theme lays out
// regions and slots in RML/RCSS; this lays the workspace out, reads back each open window's slot
// and maps the window onto it (LayoutMode::Slot).
namespace UI::Placement
{
using GetWindow = std::function<mu::ui::window::CObject*()>;
using SetPosition = std::function<void(int x, int y)>;

// `slotName` is the slot's data-window value. `document` is the window's RML file (for example
// "character_info.rml"), whose #panel sizes a content slot; without one the slot takes the docked
// windows' size. A window with no slot in the workspace keeps its own layout mode and position.
void RegisterWindow(std::uint32_t windowId, std::string_view slotName, GetWindow getWindow, SetPosition setPosition,
                    const char* document = nullptr);
// A window that can only be named in data-closes.
void RegisterName(std::uint32_t windowId, std::string_view name);

// Before a window opens: closes the open windows its slot lists in data-closes, the ones the
// theme gives no room beside it.
void CloseForOpening(std::uint32_t windowId);

// Re-places every open window that has a slot. Call after a window opens or closes.
void Arrange();
// Once a frame: re-places windows when the screen size, UI scale or HUD position changed.
void Update();
// Edges, in screen pixels, of the world the open windows leave uncovered. An open slot in a region
// marked data-covers-world narrows it from the side of the screen the slot is on.
float UncoveredWorldLeft();
float UncoveredWorldRight();
void Release();
}
