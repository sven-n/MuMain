#pragma once

namespace Rml
{
class DataModelConstructor;
}

// What the classic (0.97) HUD shows and the S6 one does not, added to CMainFrameWindow's data model
// ("main_frame"): the Party and Guild buttons on the bar (open state, click, hint), the hero's
// coordinates in its left box. Only the classic theme's main_frame.rml uses them; a theme that
// does not pays nothing.
namespace UI::Hud::MainFrameExtras
{
// Adds the bindings, while CMainFrameWindow builds its model.
void BindModel(Rml::DataModelConstructor& constructor);

// Per-frame update, from CMainFrameWindow's own model sync.
void SyncModel();
} // namespace UI::Hud::MainFrameExtras
