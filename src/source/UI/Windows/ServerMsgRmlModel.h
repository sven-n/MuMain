#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

// The character-list server message window (CServerMsgWin): up to SMW_MSG_LINE_MAX lines in the
// fixed font, the frame sized to the line count. Real pixels, like the original (Legacy layout).
struct ServerMsgRmlModel
{
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native fixed-font text size in physical px
    float sideHeight = 0.f; // the frame's side pieces: 4 px per step, five steps per line
    std::vector<Rml::String> lines;
};
