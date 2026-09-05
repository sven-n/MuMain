#include "stdafx.h"

#ifdef _EDITOR

#include "MuInputBlockerCore.h"

#include "imgui.h"
#include "UI/Core/NewUICommon.h"

CMuInputBlockerCore& CMuInputBlockerCore::GetInstance()
{
    static CMuInputBlockerCore instance;
    return instance;
}

void CMuInputBlockerCore::ProcessInputBlocking()
{
    // Block game input ONLY when hovering UI
    ImGuiIO& io = ImGui::GetIO();

    // Only block when hovering or when keyboard input is wanted
    bool shouldBlockInput = io.WantCaptureMouse || io.WantCaptureKeyboard;
    
    if (!shouldBlockInput)
        return;

    // Clear game mouse input flags when hovering UI
    extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
    extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;
    extern bool MouseMButton;
    extern int MouseWheel;

    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
    MouseLButtonDBClick = false;
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseRButtonPush = false;
    MouseMButton = false;
    MouseWheel = 0;  // Block mouse wheel when ImGui wants mouse input

    // Block Enter key when hovering UI or when keyboard input is wanted
    // This prevents chat from opening when pressing Enter in input fields
    // We need to set EnterPressed flag to true to prevent ScanAsyncKeyState from clearing it
    mu::ui::window::CNewKeyInput* pKeyInput = mu::ui::window::CNewKeyInput::GetInstance();
    pKeyInput->SetKeyState(VK_RETURN, mu::ui::window::CNewKeyInput::KEY_NONE);
    SetEnterPressed(true); // Prevent ScanAsyncKeyState from clearing VK_RETURN state

    // Only clear all keyboard input when ImGui wants to capture it (text fields, etc.)
    // WantCaptureKeyboard is true when a text input field has focus
    if (io.WantCaptureKeyboard)
    {
        for (int key = 0; key < 256; key++)
        {
            pKeyInput->SetKeyState(key, mu::ui::window::CNewKeyInput::KEY_NONE);
        }
    }
}

#endif // _EDITOR
