#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorLayout.h"

#include "MuEditorUI.h"
#include "MuEditorWindowIds.h"
#include "ToolGroups.h"
#include "../MuEditor/Core/MuEditorPlatformWindows.h"
#include "imgui_internal.h"

#include <algorithm>
#include <string_view>

namespace
{
// The editor window's width when it first opens, before the editor UI scale.
constexpr float EditorWindowWidth = 900.0f;
// The share of the dock space's height the console takes in the first layout.
constexpr float ConsoleHeightShare = 0.25f;
// The share of the tools' width the item browser takes, beside the other tools.
constexpr float ItemBrowserWidthShare = 0.35f;
constexpr char DockSpaceName[] = "ToolWindows";
// The editor window's dock space, as last submitted; 0 before.
ImGuiID g_dockSpace = 0;
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
    const MuEditor::ToolGroup& itemTools = MuEditor::GetItemTools();
    ImGui::DockBuilderDockWindow(itemTools.browser.windowId, browser);
    // Also the windows that are closed: they open in their place.
    for (const MuEditor::ToolWindowEntry& tool : itemTools.tools)
    {
        ImGui::DockBuilderDockWindow(tool.windowId, otherTools);
    }
    for (const char* window :
         {MuEditor::WindowIds::SkillEditor, MuEditor::WindowIds::DevEditor, MuEditor::WindowIds::MapEditor,
          MuEditor::WindowIds::EffectBrowser, MuEditor::WindowIds::ServerList})
    {
        ImGui::DockBuilderDockWindow(window, otherTools);
    }
    // The monster browser as a tab beside the item browser, the monster tools
    // after the other tools.
    const MuEditor::ToolGroup& monsterTools = MuEditor::GetMonsterTools();
    ImGui::DockBuilderDockWindow(monsterTools.browser.windowId, browser);
    for (const MuEditor::ToolWindowEntry& tool : monsterTools.tools)
    {
        ImGui::DockBuilderDockWindow(tool.windowId, otherTools);
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

// The dock node a window is in; for a window that is closed or not open yet
// this session, the one imgui.ini keeps for it. nullptr while it floats.
ImGuiDockNode* FindDockNode(const char* windowId)
{
    ImGuiID dockId = 0;
    if (const ImGuiWindow* window = ImGui::FindWindowByName(windowId))
    {
        if (window->DockNode != nullptr)
        {
            return window->DockNode;
        }
        dockId = window->DockId;
    }
    else if (const ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByID(ImHashStr(windowId)))
    {
        dockId = settings->DockId;
    }
    return dockId != 0 ? ImGui::DockBuilderGetNode(dockId) : nullptr;
}

// Beside the item browser: the node next to the browser's, so the browser
// stays in view. In a layout from before the item tools the console is the
// browser's neighbour (the tools were above it): then the browser's own node.
ImGuiDockNode* BesideBrowser(ImGuiDockNode* browser)
{
    if (browser == nullptr || browser->ParentNode == nullptr)
    {
        return nullptr;
    }
    ImGuiDockNode* parent = browser->ParentNode;
    ImGuiDockNode* beside = FirstLeaf(parent->ChildNodes[0] == browser ? parent->ChildNodes[1] : parent->ChildNodes[0]);
    return beside == FindDockNode(MuEditor::WindowIds::Console) ? browser : beside;
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
    g_dockSpace = dockSpace;
    if (resetLayout || ImGui::DockBuilderGetNode(dockSpace) == nullptr)
    {
        // Never empty, also when the editor window is down to its menu bar.
        BuildDefaultLayout(dockSpace, ImMax(ImGui::GetContentRegionAvail(), ImVec2(1.0f, 1.0f)));
    }
    ImGui::DockSpace(dockSpace);
}

ImGuiWindowFlags ToolWindowBarFlags(const char* windowName)
{
    // Where the window is docked. Not from DockIsActive: the editor window's
    // dock space clears it each frame for a window alone in its node, until
    // the window's Begin(). On the frame a window docks, its node is found by
    // its DockId.
    ImGuiWindow* window = ImGui::FindWindowByName(windowName);
    if (window == nullptr)
    {
        return ImGuiWindowFlags_None;
    }
    ImGuiDockNode* dockNode =
        window->DockNode != nullptr ? window->DockNode : ImGui::DockBuilderGetNode(window->DockId);
    if (dockNode == nullptr)
    {
        return ImGuiWindowFlags_MenuBar;
    }
    ImGuiDockNode* node = ImGui::DockNodeGetRootNode(dockNode);
    if (g_dockSpace != 0 && node->ID == g_dockSpace)
    {
        return ImGuiWindowFlags_None;
    }
    // One bar for a floating window: on the tab shown at its top left (the
    // first child of a split is the left or the top one).
    while (!node->IsLeafNode())
    {
        ImGuiDockNode* first = node->ChildNodes[0];
        node = first != nullptr && first->IsVisible ? first : node->ChildNodes[1];
    }
    const bool shown =
        node->VisibleWindow != nullptr ? node->VisibleWindow == window : node->SelectedTabId == window->TabId;
    return shown ? ImGuiWindowFlags_MenuBar : ImGuiWindowFlags_None;
}

void RenderToolWindowBar()
{
    g_MuEditorUI.RenderToolWindowBar();
}

void PrepareToolWindow()
{
    ImGuiWindowClass toolWindow;
    toolWindow.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&toolWindow);
}

void DockBesideItemTools(const char* windowId)
{
    DockInGroup(windowId, GetItemTools());
}

void DockInGroup(const char* windowId, const ToolGroup& group)
{
    if (ImGui::FindWindowByName(windowId) != nullptr || ImGui::FindWindowSettingsByID(ImHashStr(windowId)) != nullptr)
    {
        return;
    }
    // The browser of a group that docks with another one goes beside that
    // group's browser, as in the first layout.
    if (group.dockFallback != nullptr && std::string_view(windowId) == group.browser.windowId)
    {
        if (const ImGuiDockNode* browser = FindDockNode(group.dockFallback->browser.windowId))
        {
            ImGui::SetNextWindowDockID(browser->ID, ImGuiCond_FirstUseEver);
            return;
        }
    }
    // A neighbour also by its place in imgui.ini, while it is not open.
    for (const ToolWindowEntry& neighbour : group.tools)
    {
        if (const ImGuiDockNode* node = FindDockNode(neighbour.windowId))
        {
            ImGui::SetNextWindowDockID(node->ID, ImGuiCond_FirstUseEver);
            return;
        }
    }
    if (const ImGuiDockNode* beside = BesideBrowser(FindDockNode(group.browser.windowId)))
    {
        ImGui::SetNextWindowDockID(beside->ID, ImGuiCond_FirstUseEver);
        return;
    }
    if (group.dockFallback != nullptr)
    {
        DockInGroup(windowId, *group.dockFallback);
    }
}
} // namespace MuEditor::Layout

#endif // _EDITOR
