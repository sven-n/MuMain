#pragma once

// The item on the cursor (CInventoryCtrl's picked item), drawn into a window-sized image in a
// document of its own above every window, so it stays over the RmlUi windows it is dragged across.
namespace UI::Items
{
// Once a frame, after the windows: shows the layer while an item is picked up.
void SyncCursorItemLayer();
// Unloads the layer's document; from CSystem::Release().
void ReleaseCursorItemLayer();
} // namespace UI::Items
