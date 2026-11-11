#pragma once

// The friend family's windows keep their positions and sizes in CUIWindowMgr in RmlUi dp, the
// units their documents lay out in, so a view hands them over unconverted.
namespace UI::Social
{
// The RmlUi dp ratio: screen pixels per dp.
float DpRatio();
// The window in dp.
int WorkspaceWidth();
int WorkspaceHeight();
// The bottom of the area above the HUD: in screen pixels, and in dp. The window's bottom when the
// theme does not stand the HUD on it.
float FreeAreaBottomPx();
int FreeAreaBottom();
} // namespace UI::Social
