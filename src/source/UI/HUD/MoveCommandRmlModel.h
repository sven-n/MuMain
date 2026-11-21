#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace Rml
{
class DataModelConstructor;
}

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
        Rml::String reqMasterLevelAndResets; // a second line under the level, empty for none
        Rml::String reqZen;

        bool canMove = false;
        bool levelUnmet = false; // only meaningful while !canMove, same as native
        bool masterLevelOrResetsUnmet = false; // the second line's, as levelUnmet is the level's
        bool zenUnmet = false;

        int index = 0; // position in m_listMoveInfoData, not the map index
    };

    struct MoveCommandRmlModel
    {
        float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

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

    // The model's fields as move_command.rml binds them (the window adds its event callbacks).
    void BindMoveCommandFields(Rml::DataModelConstructor& c, MoveCommandRmlModel& model);
}
