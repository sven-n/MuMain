#pragma once

#include "UI/Placement/PlacementParticipant.h"

#include <cstdint>
#include <functional>
#include <string_view>

namespace mu::ui::window
{
class CObject;
}

namespace UI::Scaling
{
struct Transform;
}

// Places windows where the active theme's workspace.rml puts their slots. Each theme lays out
// regions and slots in RML/RCSS; this lays the workspace out, reads back each open window's slot
// and maps the window onto it (LayoutMode::Slot).
namespace UI::Placement
{
void RegisterParticipant(std::string_view name, PlacementParticipant participant);
void UnregisterParticipant(std::string_view name);
// Marks placement stale; the next Update() re-places once (a theme reload, a measured size change).
void Invalidate();
// Last resolved slot rectangle in screen pixels; false for a closed or absent slot.
bool SlotBox(std::string_view name, PlacementParticipant::Box& box);

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

// For a window that places itself: where the theme's slot puts a `width` x `height` box (screen
// pixels), as the window's first position. False when the theme has no such slot.
bool InitialPosition(std::string_view slotName, float width, float height, float& x, float& y);

// A window drawn from its own position in HUD space (LayoutMode::HudFrame) takes its place from a slot:
// the slot's box, sized width x height HUD units, becomes its position. Without a slot it returns
// to (homeX, homeY).
void RegisterHudWindow(std::string_view name, std::uint32_t windowId, SetPosition setPosition, float width,
                       float height, int homeX, int homeY);

// Re-places every open component now. Call after a window opens or closes.
void Arrange();
// Once a frame: re-places windows when the screen size, UI scale or HUD position changed.
void Update();
// A registered window's #panel size in its layout units; false without a document to measure.
bool PanelSizeOf(std::uint32_t windowId, float& width, float& height);
// Edges, in screen pixels, of the world the open windows leave uncovered. An open slot in a region
// marked data-covers-world narrows it from the side of the screen the slot is on.
float UncoveredWorldLeft();
float UncoveredWorldRight();
// The same edges in `transform`'s units, for a widget that follows them in its own layout.
float UncoveredWorldLeftIn(const UI::Scaling::Transform& transform);
float UncoveredWorldRightIn(const UI::Scaling::Transform& transform);
void Release();
}
