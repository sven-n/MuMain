#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorLayout.h"

#include "MuEditorWindowIds.h"
#include "imgui_internal.h"

#include <algorithm>

namespace
{
// The editor window's width when it first opens, before the editor UI scale.
constexpr float EditorWindowWidth = 900.0f;
// The share of the dock space's height the console takes in the first layout.
constexpr float ConsoleHeightShare = 0.25f;
constexpr char DockSpaceName[] = "ToolWindows";

void BuildDefaultLayout(ImGuiID dockSpace, ImVec2 size)
{
    ImGui::DockBuilderRemoveNode(dockSpace);
    ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpace, size);

    ImGuiID tools = 0;
    const ImGuiID console = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Down, ConsoleHeightShare, nullptr, &tools);
    ImGui::DockBuilderDockWindow(MuEditor::WindowIds::Console, console);
    for (const char* window :
         {MuEditor::WindowIds::ItemEditor, MuEditor::WindowIds::SkillEditor, MuEditor::WindowIds::DevEditor,
          MuEditor::WindowIds::MapEditor, MuEditor::WindowIds::EffectBrowser})
    {
        ImGui::DockBuilderDockWindow(window, tools);
    }
    ImGui::DockBuilderFinish(dockSpace);
}
} // namespace

namespace MuEditor::Layout
{
WindowRect FirstEditorWindowRect(float uiScale)
{
    ImGuiViewport* game = ImGui::GetMainViewport();
    const ImGuiPlatformMonitor* monitor = ImGui::GetViewportPlatformMonitor(game);
    const float width = std::min(EditorWindowWidth * uiScale, game->Size.x);
    const ImVec2 size(width, std::min(game->Size.y, monitor->WorkSize.y));

    const float gameRight = game->Pos.x + game->Size.x;
    float x = gameRight - width;
    // Inside the game window, the only place it can be, while the editor's
    // windows stay in it.
    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
    {
        return {ImVec2(x, game->Pos.y), ImVec2(width, game->Size.y)};
    }

    const float roomRight = monitor->WorkPos.x + monitor->WorkSize.x - gameRight;
    const float roomLeft = game->Pos.x - monitor->WorkPos.x;
    if (roomRight >= width)
    {
        x = gameRight;
    }
    else if (roomLeft >= width)
    {
        x = game->Pos.x - width;
    }
    return {ImVec2(x, game->Pos.y), size};
}

void SubmitDockSpace(bool resetLayout)
{
    const ImGuiID dockSpace = ImGui::GetID(DockSpaceName);
    if (resetLayout || ImGui::DockBuilderGetNode(dockSpace) == nullptr)
    {
        // Never empty, also when the editor window is down to its menu bar.
        BuildDefaultLayout(dockSpace, ImMax(ImGui::GetContentRegionAvail(), ImVec2(1.0f, 1.0f)));
    }
    ImGui::DockSpace(dockSpace);
}
} // namespace MuEditor::Layout

#endif // _EDITOR
