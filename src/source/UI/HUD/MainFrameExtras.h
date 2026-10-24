#pragma once

namespace Rml
{
class DataModelConstructor;
}

// What the classic HUD shows and the S6 one does not, added to CMainFrameWindow's data model
// ("main_frame"): the Party and Guild buttons on the bar (0.97) and the command window button
// (Season 1 / 2) (open state, click, hint), and the hero's coordinates in its left box. Only the
// classic theme's main_frame.rml uses them; a theme that does not pays nothing.
namespace UI::Hud::MainFrameExtras
{
// Adds the bindings, while CMainFrameWindow builds its model.
void BindModel(Rml::DataModelConstructor& constructor);

// Per-frame update, from CMainFrameWindow's own model sync.
void SyncModel();
} // namespace UI::Hud::MainFrameExtras
