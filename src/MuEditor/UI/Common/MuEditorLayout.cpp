#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorLayout.h"

#include "MuEditorWindowIds.h"
#include "../MuEditor/Core/MuEditorPlatformWindows.h"
#include "imgui_internal.h"

#include <algorithm>

namespace
{
// The editor window's width when it first opens, before the editor UI scale.
constexpr float EditorWindowWidth = 900.0f;
// The share of the dock space's height the console takes in the first layout.
constexpr float ConsoleHeightShare = 0.25f;
// The share of the tools' width the item browser takes, beside the other tools.
constexpr float ItemBrowserWidthShare = 0.35f;
constexpr char DockSpaceName[] = "ToolWindows";
// The tools that show the item the browser selects, in their tab order.
constexpr const char* ItemToolWindows[] = {MuEditor::WindowIds::ItemBasics, MuEditor::WindowIds::ItemRequirements,
                                           MuEditor::WindowIds::ItemRules,  MuEditor::WindowIds::ItemCategories,
                                           MuEditor::WindowIds::ItemModel,  MuEditor::WindowIds::ItemGlowColors,
                                           MuEditor::WindowIds::ItemChecks, MuEditor::WindowIds::ItemTranslations};

void BuildDefaultLayout(ImGuiID dockSpace, ImVec2 size)
{
    ImGui::DockBuilderRemoveNode(dockSpace);
    ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpace, size);

    ImGuiID tools = 0;
    const ImGuiID console = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Down, ConsoleHeightShare, nullptr, &tools);
    ImGui::DockBuilderDockWindow(MuEditor::WindowIds::Console, console);
    // The item browser beside the item tools, which show what it selects.
    ImGuiID otherTools = 0;
    const ImGuiID browser =
        ImGui::DockBuilderSplitNode(tools, ImGuiDir_Left, ItemBrowserWidthShare, nullptr, &otherTools);
    ImGui::DockBuilderDockWindow(MuEditor::WindowIds::ItemBrowser, browser);
    // Also the windows that are closed: they open in their place.
    for (const char* window : ItemToolWindows)
    {
        ImGui::DockBuilderDockWindow(window, otherTools);
    }
    for (const char* window : {MuEditor::WindowIds::SkillEditor, MuEditor::WindowIds::DevEditor,
                               MuEditor::WindowIds::MapEditor, MuEditor::WindowIds::EffectBrowser})
    {
        ImGui::DockBuilderDockWindow(window, otherTools);
    }
    ImGui::DockBuilderFinish(dockSpace);
}

// The first leaf of a dock node: the node itself, or the first of its splits.
ImGuiDockNode* FirstLeaf(ImGuiDockNode* node)
{
    while (node != nullptr && node->ChildNodes[0] != nullptr)
    {
        node = node->ChildNodes[0];
    }
    return node;
}

// Beside the item browser: the node next to the browser's, so the browser
// stays in view.
ImGuiDockNode* BesideBrowser(const ImGuiWindow& browser)
{
    const ImGuiDockNode* node = browser.DockNode;
    if (node == nullptr || node->ParentNode == nullptr)
    {
        return nullptr;
    }
    ImGuiDockNode* parent = node->ParentNode;
    return FirstLeaf(parent->ChildNodes[0] == node ? parent->ChildNodes[1] : parent->ChildNodes[0]);
}
} // namespace

namespace MuEditor::Layout
{
WindowRect FirstEditorWindowRect(float uiScale)
{
    ImGuiViewport* game = ImGui::GetMainViewport();
    const float width = std::min(EditorWindowWidth * uiScale, game->Size.x);
    const float gameRight = game->Pos.x + game->Size.x;
    // Inside the game window, the only place it can be, while the editor's
    // windows stay in it.
    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
    {
        return {ImVec2(gameRight - width, game->Pos.y), ImVec2(width, game->Size.y)};
    }

    // Positions are of the client area; the OS title bar comes on top of it,
    // as tall as the game window's.
    const float titleBar = MuEditor::PlatformWindows::GameTitleBarHeight();
    const ImGuiPlatformMonitor* monitor = ImGui::GetViewportPlatformMonitor(game);
    const float height = std::min(game->Size.y, monitor->WorkSize.y - titleBar);
    const float roomRight = monitor->WorkPos.x + monitor->WorkSize.x - gameRight;
    const float roomLeft = game->Pos.x - monitor->WorkPos.x;
    if (roomRight >= width)
    {
        return {ImVec2(gameRight, game->Pos.y), ImVec2(width, height)};
    }
    if (roomLeft >= width)
    {
        return {ImVec2(game->Pos.x - width, game->Pos.y), ImVec2(width, height)};
    }
    // Over the right of the game window, below the game's title bar and its
    // buttons.
    return {ImVec2(gameRight - width, game->Pos.y + titleBar), ImVec2(width, height - titleBar)};
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

void PrepareToolWindow()
{
    ImGuiWindowClass toolWindow;
    toolWindow.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&toolWindow);
}

void DockBesideItemTools(const char* windowId)
{
    if (ImGui::FindWindowByName(windowId) != nullptr || ImGui::FindWindowSettingsByID(ImHashStr(windowId)) != nullptr)
    {
        return;
    }
    for (const char* neighbour : ItemToolWindows)
    {
        const ImGuiWindow* window = ImGui::FindWindowByName(neighbour);
        if (window != nullptr && window->DockId != 0)
        {
            ImGui::SetNextWindowDockID(window->DockId, ImGuiCond_FirstUseEver);
            return;
        }
    }
    const ImGuiWindow* browser = ImGui::FindWindowByName(MuEditor::WindowIds::ItemBrowser);
    if (const ImGuiDockNode* beside = browser != nullptr ? BesideBrowser(*browser) : nullptr)
    {
        ImGui::SetNextWindowDockID(beside->ID, ImGuiCond_FirstUseEver);
    }
}
} // namespace MuEditor::Layout

#endif // _EDITOR
