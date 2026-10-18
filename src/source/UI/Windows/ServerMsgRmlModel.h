#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

// The character-list server message window (CServerMsgWin): up to SMW_MSG_LINE_MAX lines in the
// fixed font, the frame sized to the line count. Real pixels, like the original.
struct ServerMsgRmlModel
{
    float panelX = 0.f, panelY = 0.f; // window pixels
    float textPx = 0.f;     // native fixed-font text size in physical px
    float sideHeight = 0.f; // the frame's side pieces: 4 px per step, five steps per line
    std::vector<Rml::String> lines;
};
