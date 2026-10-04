#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

// Places windows where the active theme's workspace.rml puts their slots. Each theme lays out
// regions and slots in RML/RCSS; this lays the workspace out, reads back each open window's slot
// and moves the window there, in the window's own layout space.
namespace UI::Placement
{
using SetPosition = std::function<void(int x, int y)>;

// `slotName` is the slot's data-window value. A window with no slot in the workspace keeps the
// position it was created at.
void RegisterWindow(std::uint32_t windowId, std::string_view slotName, SetPosition setPosition);

// Re-places every open window that has a slot. Call after a window opens or closes.
void Arrange();
// Once a frame: re-places windows when the screen size, UI scale or HUD position changed.
void Update();
// Right edge, in screen pixels, of the world the open windows leave uncovered: the leftmost open
// slot in a region marked data-covers-world, or the screen width.
float UncoveredWorldRight();
void Release();
}
