#pragma once

#include "Core/Globals/InterfaceList.h"

// Showing, hiding and querying windows by id, for code that must not depend on the window classes.
namespace UI::Windows
{
using WindowId = mu::ui::window::INTERFACE_LIST;

void Show(WindowId id);
void Hide(WindowId id);
bool IsVisible(WindowId id);
void HideAll();

// The older panel manager behind a few NPC windows.
void ResetLegacyPanels();
void OpenServerDivision();
}
