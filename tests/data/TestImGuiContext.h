#pragma once

#ifdef _EDITOR

#include "imgui.h"

// An ImGui context of its own for a test, without a window or a renderer.
class TestImGuiContext
{
public:
    TestImGuiContext() : m_previous(ImGui::GetCurrentContext()), m_context(ImGui::CreateContext())
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(DisplayWidth, DisplayHeight);
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // Keys as the Windows and Linux backends deliver them on every
        // platform: on macOS ImGui would turn an injected Ctrl into Cmd, and a
        // test's Ctrl+Z would never be pressed.
        io.ConfigMacOSXBehaviors = false;
        // The font atlas is built without a renderer that takes its texture.
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    }

    ~TestImGuiContext()
    {
        ImGui::DestroyContext(m_context);
        ImGui::SetCurrentContext(m_previous);
    }

    TestImGuiContext(const TestImGuiContext&) = delete;
    TestImGuiContext& operator=(const TestImGuiContext&) = delete;

private:
    static constexpr float DisplayWidth = 1600.0f;
    static constexpr float DisplayHeight = 1000.0f;

    ImGuiContext* m_previous;
    ImGuiContext* m_context;
};

#endif // _EDITOR
