#include "stdafx.h"

#include "doctest.h"

// What a click on a problem in Data Checks opens, by area: the items select
// the item and open the tool of its field (Item Formulas for the item stat
// file), the file areas open their window, the model file table nothing; an
// arrow key onto a problem (not a click) opens no window.
#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/DataChecks/FileCheckArea.h"
#include "UI/ItemTools/ItemCheckArea.h"
#include "UI/ItemTools/ItemCheckTargets.h"
#include "UI/ItemTools/ItemSelection.h"
#include "imgui.h"

#include <memory>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using MuEditor::ToolWindows;
using MuEditor::DataChecks::DataCheckArea;

namespace
{
ItemDataIssue ItemIssue(int group, int number, const char* field)
{
    return {ItemDataIssueSeverity::Fatal, "Group00_Sword.json", group, number, field, "must be a whole number"};
}

ItemDataIssue FileIssue()
{
    return {ItemDataIssueSeverity::Fatal, "file.json", ItemDataIssue::NoItem,
            ItemDataIssue::NoItem,        "field",     "missing or not text"};
}

// Opens the problem inside a frame, as a click (or an arrow key) in the table
// does.
void OpenInFrame(DataCheckArea& area, const ItemDataIssue& issue, bool clicked)
{
    ImGui::NewFrame();
    area.Open(issue, clicked);
    ImGui::Render();
}

bool AnyToolWindowOpen()
{
    const ToolWindows& windows = g_MuEditorCore.GetToolWindows();
    return windows.serverList || windows.effectBrowser || windows.devEditor || windows.itemBasics ||
           windows.itemFormulas || windows.itemTranslations || windows.itemBrowser;
}
} // namespace

TEST_CASE("Data Checks: a click on an item's problem selects the item and opens the tool of its field [editor]")
{
    const ToolWindowsScope windows;
    TestImGuiContext context;
    const std::unique_ptr<DataCheckArea> items = MuEditor::Items::CreateItemCheckArea();
    const ItemDataIssue width = ItemIssue(0, 5, "width");
    const MuEditor::Items::ItemToolWindowRef tool =
        MuEditor::Items::GetToolWindow(MuEditor::Items::GetToolForIssue(width));

    OpenInFrame(*items, width, false);
    CHECK(g_ItemSelection.GetItemType() == Data::Items::MakeItemType(0, 5));
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().*tool.open);

    OpenInFrame(*items, ItemIssue(0, 6, "width"), true);
    CHECK(g_ItemSelection.GetItemType() == Data::Items::MakeItemType(0, 6));
    CHECK(g_MuEditorCore.GetToolWindows().*tool.open);
    CHECK(std::string(items->GetName()) == I18N::Editor::Items);
}

TEST_CASE("Data Checks: a click on a problem of the item stat file opens Item Formulas [editor]")
{
    const ToolWindowsScope windows;
    TestImGuiContext context;
    const std::unique_ptr<DataCheckArea> items = MuEditor::Items::CreateItemCheckArea();
    const ItemDataIssue profile{ItemDataIssueSeverity::Warning,
                                Data::Items::GetItemStatsFile().generic_string(),
                                ItemDataIssue::NoItem,
                                ItemDataIssue::NoItem,
                                "profiles.shield",
                                "no item uses it"};

    OpenInFrame(*items, profile, true);

    CHECK(g_MuEditorCore.GetToolWindows().itemFormulas);
}

TEST_CASE("Data Checks: a click on a file area's problem opens its window, the model file table none [editor]")
{
    struct Case
    {
        std::unique_ptr<DataCheckArea> area;
        bool ToolWindows::* window;
        const char* name;
    };
    Case cases[] = {
        {MuEditor::DataChecks::CreateServerListCheckArea(), &ToolWindows::serverList, I18N::Editor::ServerList},
        {MuEditor::DataChecks::CreateEffectCheckArea(), &ToolWindows::effectBrowser, I18N::Editor::Effects},
        {MuEditor::DataChecks::CreateLookCheckArea(), &ToolWindows::devEditor, I18N::Editor::Looks},
        {MuEditor::DataChecks::CreateModelFileCheckArea(), nullptr, I18N::Editor::ModelFiles},
    };
    for (Case& each : cases)
    {
        CAPTURE(each.name);
        const ToolWindowsScope windows;
        TestImGuiContext context;
        CHECK(std::string(each.area->GetName()) == each.name);

        OpenInFrame(*each.area, FileIssue(), false);
        CHECK_FALSE(AnyToolWindowOpen());

        OpenInFrame(*each.area, FileIssue(), true);
        if (each.window == nullptr)
        {
            CHECK_FALSE(AnyToolWindowOpen());
        }
        else
        {
            CHECK(g_MuEditorCore.GetToolWindows().*each.window);
        }
    }
}

TEST_CASE("Data Checks: a file area appends the problems its files have [editor]")
{
    MuEditor::DataChecks::FileCheckArea area(
        &I18N::Editor::ServerList,
        []
        {
            ItemDataIssue second = FileIssue();
            second.field = "second";
            return std::vector<ItemDataIssue>{FileIssue(), second};
        },
        nullptr, nullptr);
    ItemDataIssue earlier = FileIssue();
    earlier.field = "earlier";
    std::vector<ItemDataIssue> issues{earlier};

    area.Check(issues);

    REQUIRE(issues.size() == 3);
    CHECK(issues[0].field == "earlier");
    CHECK(issues[1].field == "field");
    CHECK(issues[2].field == "second");
}

#endif // _EDITOR
