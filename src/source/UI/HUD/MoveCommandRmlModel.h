#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
    // One warp destination. The four text cells are pre-formatted by C++ (the required level is
    // the class-adjusted one, not _ReqInfo.iReqLevel); the four flags are the meaning behind
    // native's per-cell SetTextColor() calls, so each theme owns the colour.
    struct MoveCommandRowEntry
    {
        Rml::String strifeText; // the battle-zone marker, empty on an ordinary map
        Rml::String mapName;
        Rml::String reqLevel;
        Rml::String reqZen;

        bool canMove = false;
        bool levelUnmet = false; // only meaningful while !canMove, same as native
        bool zenUnmet = false;

        int index = 0; // position in m_listMoveInfoData, not the map index
    };

    struct MoveCommandRmlModel
    {
        // Left-docked window -- sourced from UI::Scaling::GetActiveTransform(), which CManager
        // scopes to LayoutMode::DockLeft around every call into this window.
        float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

        // Reference-space geometry CalculateLayout() derives from the dock height and the measured
        // row height; #panel and its children are authored in the same reference units.
        float panelHeight = 0.f;
        float panelWidth = 230.f; // #panel's content width
        float listHeight = 0.f;
        float listTail = 0.f; // listHeight modulo rowHeight
        float listWidth = 0.f;
        float rowWidth = 0.f;
        float rowHeight = 0.f;
        float closeTop = 0.f;

        std::vector<MoveCommandRowEntry> rows;

        // Set once at BuildRmlUi() time -- static I18N strings, same "set once after modelCreated"
        // pattern QuestProgressRmlModel's own labels use.
        Rml::String titleText;
        Rml::String headStrife;
        Rml::String headMap;
        Rml::String headLevel;
        Rml::String headZen;
        Rml::String closeText;
    };
}
