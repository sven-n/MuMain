#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ItemModelDatabaseScope.h"
#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "I18N/All.h"
#include "UI/Colors/MuColorsUI.h"
#include "UI/Common/MuEditorLayout.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/ToolGroups.h"
#include "UI/ItemTools/ItemEdits.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <string_view>
#include <vector>

using Data::Effects::GlowColor;
using Data::Items::ItemGlow;
using Data::Items::ItemModelDefinition;
using MuEditor::Colors::ColorsTab;
namespace WindowIds = MuEditor::WindowIds;

namespace
{
constexpr ImVec2 EditorWindowSize(1200.0f, 800.0f);

const MuEditor::ToolWindowEntry* FindItemTool(const char* const* label)
{
    for (const MuEditor::ToolWindowEntry& tool : MuEditor::GetItemTools().tools)
    {
        if (tool.label == label)
        {
            return &tool;
        }
    }
    return nullptr;
}

// The editor window, which holds the dock space of the tool windows.
void BeginEditorWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(EditorWindowSize);
    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoDocking);
}

ImGuiID DockIdOf(const char* windowId)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(windowId);
    REQUIRE(window != nullptr);
    return window->DockId;
}

void RenderColors()
{
    ImGui::NewFrame();
    bool open = true;
    g_MuColorsUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// The id of a widget of the Glow tab, outside its two panes.
ImGuiID GlowTabWidgetId(const char* label)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::Colors);
    REQUIRE(window != nullptr);
    const ImGuiID tabBar = ImGui::GetIDWithSeed("##Tabs", nullptr, window->ID);
    const ImGuiID tab = ImGui::GetIDWithSeed("###Glow", nullptr, tabBar);
    return ImGui::GetIDWithSeed(label, nullptr, tab);
}

ItemModelDefinition Model(int number, const char* color)
{
    ItemModelDefinition model;
    model.number = number;
    model.file = "Data/Item/Sword01.bmd";
    model.glow.color = color;
    return model;
}

// The glow colors of a test, and the ones before it again after it.
class GlowColorsScope
{
public:
    explicit GlowColorsScope(const std::vector<GlowColor>& colors)
        : m_previous(g_GlowColors.GetColors().begin(), g_GlowColors.GetColors().end())
    {
        g_GlowColors.Build(colors);
    }
    ~GlowColorsScope()
    {
        g_GlowColors.Build(m_previous);
        g_ItemEdits.GetHistory().Clear();
    }
    GlowColorsScope(const GlowColorsScope&) = delete;
    GlowColorsScope& operator=(const GlowColorsScope&) = delete;

private:
    std::vector<GlowColor> m_previous;
};

bool HasGlowColor(const char* name)
{
    return g_GlowColors.Find(name) != nullptr;
}
} // namespace

// Items > Glow Colors is the Colors window's Glow tab: it opens the window there.
TEST_CASE("Items > Glow Colors opens the Colors window on its Glow tab [editor][items]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    const MuEditor::ToolWindowEntry* glow = FindItemTool(&I18N::Editor::GlowColors);
    REQUIRE(glow != nullptr);
    CHECK(std::string_view(glow->windowId) == WindowIds::Colors);
    CHECK(glow->open == &MuEditor::ToolWindows::colors);
    REQUIRE(glow->showTab != nullptr);
    REQUIRE(glow->isTabShown != nullptr);
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().colors);

    glow->showTab();
    CHECK(g_MuEditorCore.GetToolWindows().colors);
    CHECK(glow->isTabShown());
    RenderColors();
    RenderColors();
    CHECK(g_MuColorsUI.GetShownTab() == ColorsTab::Glow);
    CHECK(glow->isTabShown());
}

TEST_CASE("The first layout docks the Colors window with the item tools [editor][items]")
{
    TestImGuiContext context;
    const auto frame = []
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics, WindowIds::Colors})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::Colors) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::Colors) != DockIdOf(WindowIds::ItemBrowser));
}

// A layout saved before the Colors window, which had the Glow Colors window in it: the Colors window opened the first
// time docks with the item tools.
TEST_CASE("The Colors window opened the first time in a layout from before it docks with the item tools "
          "[editor][items]")
{
    TestImGuiContext context;
    ImGuiID tools = 0;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        const ImGuiID dockSpace = ImGui::GetID("ToolWindows");
        if (ImGui::DockBuilderGetNode(dockSpace) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpace, ImVec2(EditorWindowSize.x, EditorWindowSize.y - 20.0f));
            const ImGuiID browser = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Left, 0.35f, nullptr, &tools);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBrowser, browser);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBasics, tools);
            ImGui::DockBuilderDockWindow("###ItemGlowColors", tools);
            ImGui::DockBuilderFinish(dockSpace);
        }
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        bool open = true;
        g_MuColorsUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::Colors) == tools);
}

// D26 on the shared panel: Remove them takes every color no item uses, as one undo step; the defaults stay although no
// item names them, and so does a color an item uses.
TEST_CASE("The Glow tab removes the glow colors no item uses as one undo step and keeps the defaults [editor][items]")
{
    TestImGuiContext context;
    ItemModelDatabaseScope restoreModels;
    GlowColorsScope restoreColors({{ItemGlow::DefaultColor, {1.0, 0.5, 0.0}},
                                   {ItemGlow::DefaultShineColor, {1.0, 1.0, 1.0}},
                                   {ItemGlow::DefaultAncientColor, {0.0, 0.5, 1.0}},
                                   {"red", {1.0, 0.0, 0.0}},
                                   {"gold", {1.0, 0.7, 0.2}},
                                   {"teal", {0.0, 0.6, 0.6}}});
    const std::vector<ItemModelDefinition> models = {Model(0, "red")};
    g_ItemModelDatabase.Build(models, g_GlowColors);
    g_ItemEdits.GetHistory().Clear();

    RenderColors();
    ImGui::ActivateItemByID(GlowTabWidgetId(I18N::Editor::RemoveThem));
    RenderColors();

    CHECK(HasGlowColor("red"));
    CHECK_FALSE(HasGlowColor("gold"));
    CHECK_FALSE(HasGlowColor("teal"));
    for (const char* kept : {ItemGlow::DefaultColor, ItemGlow::DefaultShineColor, ItemGlow::DefaultAncientColor})
    {
        CHECK(HasGlowColor(kept));
    }
    CHECK(g_ItemEdits.GetHistory().GetUndoLabel() == I18N::Editor::RemoveUnusedGlowColors);

    REQUIRE(g_ItemEdits.Undo());
    CHECK(HasGlowColor("gold"));
    CHECK(HasGlowColor("teal"));
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());
}

#endif // _EDITOR
