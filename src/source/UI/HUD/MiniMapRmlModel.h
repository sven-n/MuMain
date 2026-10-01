#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A rectangle of the window, physical px: one of the regions the map may paint (see
// CMiniMap::SyncClips()).
struct MiniMapClipEntry
{
    float left = 0.f, top = 0.f, width = 0.f, height = 0.f;
    // -left / -top: .world's offset back to window coordinates (the data expressions have no
    // unary minus).
    float worldLeft = 0.f, worldTop = 0.f;
};

struct MiniMapMarkerEntry
{
    bool portal = false;   // MINI_MAP::Kind 2 (a gate), else an NPC
    float size = 15.f;     // the icon's side, physical px (the original never scaled it)
    Rml::String transform; // CSS matrix() placing the icon element
};

struct MiniMapRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs this window).
    float scaleX = 1.f, scaleY = 1.f;
    float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

    std::vector<MiniMapClipEntry> clips;

    Rml::String mapSource;    // the world's mini_map texture, relative to the document
    Rml::String mapTransform; // CSS matrix() placing the 800x800 map element
    std::vector<MiniMapMarkerEntry> markers;
    std::vector<Rml::String> sideLines; // CSS matrix() of each left/right border tile

    // The hovered marker's name (CMiniMap::Check_Btn()), physical px.
    bool hintVisible = false;
    Rml::String hintText;
    float hintLeft = 0.f, hintTop = 0.f, hintWidth = 0.f, hintHeight = 0.f;
};
} // namespace mu::ui::window
