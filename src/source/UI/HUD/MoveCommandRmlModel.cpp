#include "stdafx.h"
#include "UI/HUD/MoveCommandRmlModel.h"

#include <RmlUi/Core/DataModelHandle.h>

void mu::ui::window::BindMoveCommandFields(Rml::DataModelConstructor& c, MoveCommandRmlModel& model)
{
    c.Bind("text_px", &model.textPx);

    auto row = c.RegisterStruct<MoveCommandRowEntry>();
    row.RegisterMember("strife_text", &MoveCommandRowEntry::strifeText);
    row.RegisterMember("map_name", &MoveCommandRowEntry::mapName);
    row.RegisterMember("req_level", &MoveCommandRowEntry::reqLevel);
    row.RegisterMember("req_ml_resets", &MoveCommandRowEntry::reqMasterLevelAndResets);
    row.RegisterMember("req_zen", &MoveCommandRowEntry::reqZen);
    row.RegisterMember("can_move", &MoveCommandRowEntry::canMove);
    row.RegisterMember("level_unmet", &MoveCommandRowEntry::levelUnmet);
    row.RegisterMember("ml_resets_unmet", &MoveCommandRowEntry::masterLevelOrResetsUnmet);
    row.RegisterMember("zen_unmet", &MoveCommandRowEntry::zenUnmet);
    row.RegisterMember("index", &MoveCommandRowEntry::index);
    c.RegisterArray<std::vector<MoveCommandRowEntry>>();
    c.Bind("rows", &model.rows);

    c.Bind("title_text", &model.titleText);
    c.Bind("head_strife", &model.headStrife);
    c.Bind("head_map", &model.headMap);
    c.Bind("head_level", &model.headLevel);
    c.Bind("head_zen", &model.headZen);
    c.Bind("close_text", &model.closeText);
}
