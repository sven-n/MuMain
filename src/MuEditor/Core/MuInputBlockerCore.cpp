#include "stdafx.h"

#ifdef _EDITOR

#include "MuInputBlockerCore.h"

#include "imgui.h"
#include "NewUICommon.h"

CMuInputBlockerCore& CMuInputBlockerCore::GetInstance()
{
    static CMuInputBlockerCore instance;
    return instance;
}

void CMuInputBlockerCore::ProcessInputBlocking()
{
    // Block game input when hovering UI
    ImGuiIO& io = ImGui::GetIO();

    // Always clear game mouse input flags when hovering UI
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

    // Only clear keyboard input when ImGui wants to capture it (text fields, etc.)
    // WantCaptureKeyboard is true when a text input field has focus
    if (io.WantCaptureKeyboard)
    {
        SEASON3B::CNewKeyInput* pKeyInput = SEASON3B::CNewKeyInput::GetInstance();
        for (int key = 0; key < 256; key++)
        {
            pKeyInput->SetKeyState(key, SEASON3B::CNewKeyInput::KEY_NONE);
        }
    }
}

#endif // _EDITOR
