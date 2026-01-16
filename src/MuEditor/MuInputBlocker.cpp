#include "stdafx.h"

#ifdef _EDITOR

#include "MuInputBlocker.h"
#include "imgui.h"

CMuInputBlocker& CMuInputBlocker::GetInstance()
{
    static CMuInputBlocker instance;
    return instance;
}

void CMuInputBlocker::ProcessInputBlocking()
{
    // Block game input when ImGui is capturing mouse/keyboard
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
    {
        // Clear game input flags
        extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
        extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;
        extern bool MouseMButton;

        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        MouseLButtonDBClick = false;
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseMButton = false;
    }
}

#endif // _EDITOR
