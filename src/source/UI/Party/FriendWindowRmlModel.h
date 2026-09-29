#pragma once

#include <RmlUi/Core/Types.h>

#include <string>
#include <vector>

// The RmlUi model of one window of the friends family (CUIWindowMgr's windows): the parts its
// native Render() drew, in the order it drew them, each named by what it is (the theme class) and
// placed in reference px relative to the window's top-left corner. See FriendWindowView.h.
struct FriendWindowPart
{
    enum Kind
    {
        Fill = 0,   // a flat rectangle (the frame lines, the backgrounds, the tab strip, a selected row)
        Sprite = 1, // an image (title bar, control buttons, buttons, scroll bar, check marks, letters)
        Text = 2,
    };

    int kind = Fill;
    Rml::String role; // theme class, e.g. "frame-outer", "tab-on", "button-down", "scroll-thumb"
    float left = 0.f;
    float top = 0.f;
    float width = 0.f; // a text's box width, 0 = none (auto width, never clipped)
    float height = 0.f;
    Rml::String text;
    float textPx = 0.f;
    int align = 0; // a text in a box: 0 left and clipped to the box, 1 centred
    bool bold = false;
    Rml::String color;

    // Not bound: what `text` and `color` were formatted from, so a frame that draws the same text
    // in the same colour formats (and allocates) nothing.
    std::wstring sourceText;
    unsigned int sourceColor = 0;
};

struct FriendWindowRmlModel
{
    // The window's top-left corner and scale under the FloatingWorkspace transform.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    std::vector<FriendWindowPart> parts;
};
