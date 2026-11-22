#include "stdafx.h"

#include "doctest.h"

// The editor's Warp List window, drawn and clicked: its buttons and fields as
// undo steps, the game's list following each change, Save and Revert, and the
// warps' Data Checks area.
#ifdef _EDITOR

#include "ShippedItemsScope.h"
#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/MapData/WarpStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/MapData/WarpDatabase.h"
#include "I18N/All.h"
#include "Network/MoveCommandData.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/DataChecks/FileCheckArea.h"
#include "UI/WarpList/MuWarpListUI.h"
#include "UI/WarpList/WarpListEdits.h"
#include "UI/WarpList/WarpNameRows.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <system_error>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::Warp;
using Data::Maps::WarpList;

namespace
{
std::filesystem::path ShippedFile()
{
    return std::filesystem::path(MU_TEST_DATA_DIR) / "Maps" / "Warps.json";
}

// A client folder whose Data/Maps/Warps.json is the shipped file; the working
// directory while it lives, as the window reads the file from there. After it
// the game's lists (the rules and the M window's warps with the language of
// their names) are as they were before.
class WarpsFolder
{
public:
    WarpsFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_warp_list_window_" + std::to_string(std::random_device{}()))),
          m_previousDirectory(std::filesystem::current_path()), m_gameList(g_WarpDatabase.GetList()),
          m_moveList(SEASON3B::CMoveCommandData::GetInstance()->GetDataFileWarps()),
          m_moveLocale(SEASON3B::CMoveCommandData::GetInstance()->GetWarpNameLocale())
    {
        mu::log::Init();
        std::filesystem::create_directories(m_directory / "Data" / "Maps");
        std::filesystem::copy_file(ShippedFile(), m_directory / Data::Maps::GetWarpsFile());
        std::filesystem::current_path(m_directory);
    }

    ~WarpsFolder()
    {
        g_WarpDatabase.Build(m_gameList);
        SEASON3B::CMoveCommandData::GetInstance()->SetDataFileWarps(m_moveList, m_moveLocale);
        std::error_code ignored;
        std::filesystem::current_path(m_previousDirectory, ignored);
        std::filesystem::remove_all(m_directory, ignored);
    }

    WarpsFolder(const WarpsFolder&) = delete;
    WarpsFolder& operator=(const WarpsFolder&) = delete;

    std::string ReadFile() const
    {
        const std::optional<std::string> text = Data::ReadTextFile(m_directory / Data::Maps::GetWarpsFile());
        REQUIRE(text);
        return *text;
    }

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_previousDirectory;
    WarpList m_gameList;
    WarpList m_moveList;
    std::string m_moveLocale;
};

// The shipped file with LF line ends, as a save writes it (a checkout may have CRLF).
std::string ReadShippedFile()
{
    const std::optional<std::string> text = Data::ReadTextFile(ShippedFile());
    REQUIRE(text);
    std::string lf = *text;
    lf.erase(std::remove(lf.begin(), lf.end(), '\r'), lf.end());
    return lf;
}

void RenderWarpList()
{
    bool open = true;
    ImGui::NewFrame();
    g_MuWarpListUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* WarpListWindow()
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::WarpList);
    REQUIRE(window != nullptr);
    return window;
}

// The id of a widget in the window's toolbar.
ImGuiID ToolbarId(const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, WarpListWindow()->ID);
}

// A child window of `parent` ("##Warps", the list, or "##Warp", the picked warp); ImGui names a child window
// "<parent>/<name>_<id>".
const ImGuiWindow* Child(const ImGuiWindow* parent, const char* name)
{
    const std::string infix = std::string("/") + name + "_";
    for (const ImGuiWindow* child : ImGui::GetCurrentContext()->Windows)
    {
        if (child->ParentWindow == parent && std::strstr(child->Name, infix.c_str()) != nullptr)
        {
            return child;
        }
    }
    FAIL("the window has no child " << name);
    return nullptr;
}

// The id of a widget of the picked warp.
ImGuiID WarpId(const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, Child(WarpListWindow(), "##Warp")->ID);
}

// The id of a widget of a rule list ("##NeedsOneOf" or "##NotWith") of the picked warp.
ImGuiID RuleId(const char* list, const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, WarpId(list));
}

// The id of a row of the warp list (RenderWarpList pushes its position).
ImGuiID RowId(int position)
{
    const ImGuiWindow* rows = Child(Child(WarpListWindow(), "##Warps"), "##Rows");
    return ImGui::GetIDWithSeed("###Warp", nullptr, ImGui::GetIDWithSeed(position, rows->ID));
}

// As a click: pressed in the next frame, as the field that was active is left.
void Press(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderWarpList();
}

// Activation selects all of the field's text; the typed text replaces it.
void Type(ImGuiID id, const char* text)
{
    Press(id);
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderWarpList();
}

// The window with the folder's file read again and the first warp (Arena) picked; an earlier test of this process
// may have used the window.
void StartWithArena()
{
    RenderWarpList();
    Press(ToolbarId(I18N::Editor::Revert));
    Press(RowId(0));
    REQUIRE(g_MuWarpListUI.GetSelected() == 0);
    REQUIRE_FALSE(g_MuWarpListUI.CanUndo());
    REQUIRE(g_MuWarpListUI.GetEdits().GetList().warps[0].names.GetNeutral() == "Arena");
}

const WarpList& EditedList()
{
    return g_MuWarpListUI.GetEdits().GetList();
}

std::vector<int> Indexes(const WarpList& list)
{
    std::vector<int> indexes;
    for (const Warp& warp : list.warps)
    {
        indexes.push_back(warp.index);
    }
    return indexes;
}

// The indexes of the M window's list, in its order.
std::vector<int> MoveWindowIndexes()
{
    std::vector<int> indexes;
    for (const auto* entry : SEASON3B::CMoveCommandData::GetInstance()->GetMoveCommandDatalist())
    {
        indexes.push_back(entry->_ReqInfo.index);
    }
    return indexes;
}

// The game has the window's list: the warp table and the M window.
void CheckGameHasTheList()
{
    CHECK(g_WarpDatabase.GetList() == EditedList());
    CHECK(MoveWindowIndexes() == Indexes(EditedList()));
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}
} // namespace

// A copy of a warp whose names fill what the game holds keeps its index: the text before it is cut.
TEST_CASE("Warp List window: Copy cuts a long name so the index added fits in 31 characters [editor][warps]")
{
    WarpList list;
    list.warps.push_back(Warp{});
    list.warps[0].index = 7;
    list.warps[0].command = std::string(31, 'C');
    list.warps[0].names.Set("en", std::string(30, 'N'));
    list.warps[0].names.Set("de", "Kurz");
    MuEditor::Warps::WarpListEdits edits;
    edits.Reset(list);

    const size_t position = edits.CopyWarp(0);
    const Warp& copy = edits.GetList().warps[position];
    const std::string suffix = std::to_string(copy.index);
    CHECK(copy.command == std::string(31 - suffix.size(), 'C') + suffix);
    CHECK(copy.names.GetNeutral() == std::string(31 - suffix.size(), 'N') + suffix);
    CHECK(copy.names.Get("de") == "Kurz" + suffix);
    // A second copy of the cut text gets its own /move name too.
    const Warp& second = edits.GetList().warps[edits.CopyWarp(0)];
    CHECK(second.command != copy.command);
    CHECK(second.command.size() <= Data::Maps::MaxWarpNameLength);
}

// A new name that is another warp's name or /move name would shadow it, or be shadowed.
TEST_CASE("Warp List window: Add warp and Copy pick names no other warp has [editor][warps]")
{
    MuEditor::Warps::WarpListEdits edits;
    WarpList list;
    list.warps.push_back(Warp{});
    list.warps[0].index = 1;
    list.warps[0].names.Set("en", "Arena");
    list.warps[0].command = "Arena";
    edits.Reset(list);

    // Add gives index 2, Warp2; with that index changed, the next Add has index 2 again.
    const size_t added = edits.AddWarp(1);
    REQUIRE(edits.GetList().warps[added].command == "Warp2");
    edits.EditList().warps[added].index = 50;
    const Warp& second = edits.GetList().warps[edits.AddWarp(2)];
    CHECK(second.index == 2);
    CHECK(second.command == "Warp2_2");
    CHECK(second.names.GetNeutral() == "Warp2_2");

    // Arena's copy has index 3; "arena3" is another warp's name in Spanish.
    list.warps.push_back(Warp{});
    list.warps[1].index = 2;
    list.warps[1].names.Set("en", "Stadium");
    list.warps[1].names.Set("es", "arena3");
    list.warps[1].command = "Stadium";
    edits.Reset(list);
    const Warp& copy = edits.GetList().warps[edits.CopyWarp(0)];
    CHECK(copy.index == 3);
    CHECK(copy.command == "Arena3_2");
    CHECK(copy.names.GetNeutral() == "Arena3_2");
}

// The later test cases of this binary see the game's lists as they were.
TEST_CASE("Warp List window: the test folder puts the game's lists back [editor][warps]")
{
    I18N::SetLocale("en");
    const WarpList original = g_WarpDatabase.GetList();
    const WarpList originalMove = SEASON3B::CMoveCommandData::GetInstance()->GetDataFileWarps();
    const std::string originalLocale = SEASON3B::CMoveCommandData::GetInstance()->GetWarpNameLocale();
    WarpList before;
    before.warps.push_back(Warp{});
    before.warps[0].index = 7;
    before.warps[0].names.Set("en", "Before");
    before.warps[0].command = "Before";
    g_WarpDatabase.Build(before);
    SEASON3B::CMoveCommandData::GetInstance()->SetDataFileWarps(before, "de");
    {
        WarpsFolder folder;
        TestImGuiContext context;
        StartWithArena();
        Press(ToolbarId(I18N::Editor::AddWarp));
        REQUIRE(g_WarpDatabase.GetList() != before);
    }
    CHECK(g_WarpDatabase.GetList() == before);
    CHECK(SEASON3B::CMoveCommandData::GetInstance()->GetDataFileWarps() == before);
    CHECK(SEASON3B::CMoveCommandData::GetInstance()->GetWarpNameLocale() == "de");
    g_WarpDatabase.Build(original);
    SEASON3B::CMoveCommandData::GetInstance()->SetDataFileWarps(originalMove, originalLocale);
}

TEST_CASE("Warp List window: Add warp, Copy, Remove and the moves are undo steps the game follows [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const std::vector<int> shipped = Indexes(EditedList());
    const int freeIndex = g_MuWarpListUI.GetEdits().FindFreeIndex();
    const std::string newName = "Warp" + std::to_string(freeIndex);

    Press(ToolbarId(I18N::Editor::AddWarp));
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Add " + newName);
    REQUIRE(EditedList().warps.size() == shipped.size() + 1);
    // After the picked warp, and picked.
    CHECK(EditedList().warps[1].index == freeIndex);
    CHECK(EditedList().warps[1].names.GetNeutral() == newName);
    CHECK(EditedList().warps[1].command == newName);
    CHECK(g_MuWarpListUI.GetSelected() == 1);
    CheckGameHasTheList();

    Press(ToolbarId(I18N::Editor::Copy));
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Copy " + newName);
    REQUIRE(EditedList().warps.size() == shipped.size() + 2);
    CHECK(EditedList().warps[2].index != freeIndex);
    // Its own /move name and name, which /move and the M window tell apart.
    const std::string copySuffix = std::to_string(EditedList().warps[2].index);
    CHECK(EditedList().warps[2].names.GetNeutral() == newName + copySuffix);
    CHECK(EditedList().warps[2].command == newName + copySuffix);
    CHECK(g_MuWarpListUI.GetSelected() == 2);

    Press(ToolbarId(I18N::Editor::MoveUp));
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Move " + newName + copySuffix);
    CHECK(g_MuWarpListUI.GetSelected() == 1);
    CHECK(EditedList().warps[2].index == freeIndex);
    CheckGameHasTheList();

    Press(RowId(0));
    Press(ToolbarId(I18N::Editor::MoveDown));
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Move Arena");
    CHECK(EditedList().warps[1].names.GetNeutral() == "Arena");

    Press(ToolbarId(I18N::Editor::Remove));
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Remove Arena");
    CHECK(EditedList().warps.size() == shipped.size() + 1);
    CheckGameHasTheList();

    // Each step undone, the last first.
    for (const char* label : {"Remove Arena", "Move Arena", "Move Warp", "Copy Warp", "Add Warp"})
    {
        CAPTURE(label);
        CHECK(g_MuWarpListUI.GetUndoLabel().rfind(label, 0) == 0);
        g_MuWarpListUI.Undo();
        RenderWarpList();
        CheckGameHasTheList();
    }
    CHECK_FALSE(g_MuWarpListUI.CanUndo());
    CHECK(Indexes(EditedList()) == shipped);
    CHECK(g_MuWarpListUI.GetRedoLabel() == "Add " + newName);
    g_MuWarpListUI.Redo();
    RenderWarpList();
    CHECK(EditedList().warps.size() == shipped.size() + 1);
    CheckGameHasTheList();
}

TEST_CASE("Warp List window: a typed level is one step, and the M window shows it at once [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const int arena = EditedList().warps[0].index;
    const int shippedLevel = EditedList().warps[0].level;

    Type(WarpId(I18N::Editor::Level), "77");
    // The field is still active: the M window shows the level already.
    const auto* entry = SEASON3B::CMoveCommandData::GetInstance()->GetMoveCommandDataByIndex(arena);
    REQUIRE(entry != nullptr);
    CHECK(entry->_ReqInfo.iReqLevel == 77);
    // Leaving it for the zen field ends the step.
    Type(WarpId(I18N::Editor::Zen), "1234");
    RenderWarpList();

    CHECK(g_MuWarpListUI.GetUndoLabel() == "Arena: Zen");
    CHECK(g_WarpDatabase.GetList().warps[0].zen == 1234);
    g_MuWarpListUI.Undo();
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Arena: Level");
    CHECK(EditedList().warps[0].level == 77);
    g_MuWarpListUI.Undo();
    CHECK(EditedList().warps[0].level == shippedLevel);
    CHECK(SEASON3B::CMoveCommandData::GetInstance()->GetMoveCommandDataByIndex(arena)->_ReqInfo.iReqLevel ==
          shippedLevel);
    CHECK_FALSE(g_MuWarpListUI.CanUndo());
}

TEST_CASE("Warp List window: a typed name is a step of its own before a box or a rule [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const std::string english = std::string(I18N::GetLanguageDisplayName("en")) + "##Name_en";

    Type(WarpId(english.c_str()), "Colosseum");
    // The name field is still active when the box is clicked.
    Press(WarpId(I18N::Editor::PvPServersOnly));
    RenderWarpList();
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Colosseum: " + std::string(I18N::Editor::PvPServersOnly));
    CHECK(EditedList().warps[0].pvpServersOnly);
    CHECK(g_WarpDatabase.GetList().warps[0].pvpServersOnly);
    // The M window names the warp so.
    CHECK(std::wstring(
              SEASON3B::CMoveCommandData::GetInstance()->GetMoveCommandDatalist().front()->_ReqInfo.szMainMapName) ==
          L"Colosseum");

    Press(RuleId("##NeedsOneOf", I18N::Editor::Add));
    RenderWarpList();
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Colosseum: " + std::string(I18N::Editor::NeedsOneOf));
    REQUIRE(EditedList().warps[0].needsOneOf.size() == 1);
    CHECK(EditedList().warps[0].needsOneOf[0].kind == Data::Maps::WarpItemMatch::Kind::Wings);
    CHECK(g_WarpDatabase.GetList().warps[0].needsOneOf.size() == 1);

    g_MuWarpListUI.Undo();
    g_MuWarpListUI.Undo();
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Arena: name");
    g_MuWarpListUI.Undo();
    CHECK(EditedList().warps[0].names.GetNeutral() == "Arena");
}

TEST_CASE("Warp List window: an index another warp has stays with it [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const int arena = EditedList().warps[0].index;
    const int other = EditedList().warps[1].index;

    Type(WarpId(I18N::Editor::Index), std::to_string(other).c_str());
    RenderWarpList();
    CHECK(EditedList().warps[0].index == arena);
    CHECK_FALSE(g_MuWarpListUI.CanUndo());
}

TEST_CASE("Warp List window: Save writes the file as it was when nothing changed, and the changes [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const std::string shipped = ReadShippedFile();

    Press(ToolbarId(I18N::Editor::Save));
    CHECK(folder.ReadFile() == shipped);

    Type(WarpId(I18N::Editor::Level), "77");
    RenderWarpList();
    CHECK(g_MuWarpListUI.GetEdits().HasUnsavedChanges());
    Press(ToolbarId(I18N::Editor::Save));
    CHECK_FALSE(g_MuWarpListUI.GetEdits().HasUnsavedChanges());
    const Data::Maps::WarpsLoadResult saved = Data::Maps::LoadWarpsFile(Data::Maps::GetWarpsFile());
    CHECK(saved.issues.empty());
    CHECK(saved.list == EditedList());
    CHECK(saved.list.warps[0].level == 77);

    // Back as it was: the file is the shipped one again, byte for byte.
    g_MuWarpListUI.Undo();
    Press(ToolbarId(I18N::Editor::Save));
    CHECK(folder.ReadFile() == shipped);
}

TEST_CASE("Warp List window: Revert reads the file again, into the game too [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    const int shippedLevel = EditedList().warps[0].level;

    Type(WarpId(I18N::Editor::Level), "77");
    Press(ToolbarId(I18N::Editor::Remove));
    REQUIRE(g_WarpDatabase.GetList().warps[0].names.GetNeutral() != "Arena");

    Press(ToolbarId(I18N::Editor::Revert));
    CHECK_FALSE(g_MuWarpListUI.CanUndo());
    CHECK_FALSE(g_MuWarpListUI.GetEdits().HasUnsavedChanges());
    CHECK(EditedList() == Data::Maps::LoadWarpsFile(ShippedFile()).list);
    CHECK(EditedList().warps[0].level == shippedLevel);
    CheckGameHasTheList();
}

TEST_CASE("Warp List window: the Data Checks area finds the problems of the game's list [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    ShippedItemsScope items;
    std::vector<ItemDataIssue> readIssues;
    REQUIRE(Data::Maps::RevertWarps(readIssues));

    std::unique_ptr<MuEditor::DataChecks::DataCheckArea> area = MuEditor::DataChecks::CreateWarpCheckArea();
    CHECK(std::string(area->GetName()) == I18N::Editor::WarpList);
    {
        std::vector<ItemDataIssue> issues;
        area->Check(issues);
        for (const ItemDataIssue& issue : issues)
        {
            INFO(issue.ToString());
            CHECK(false);
        }
    }

    WarpList list = g_WarpDatabase.GetList();
    REQUIRE(list.warps.size() > 3);
    Warp& first = list.warps[0];
    Data::Maps::WarpItemMatch unknownItem;
    unknownItem.group = 13;
    unknownItem.number = 511;
    first.notWith.push_back(unknownItem);
    Data::Maps::WarpItemMatch unknownTag;
    unknownTag.kind = Data::Maps::WarpItemMatch::Kind::Tag;
    unknownTag.tag = "dragonish";
    first.needsOneOf.push_back(unknownTag);
    first.blockedMessage = "NoSuchText";
    first.command = "Arena One";
    list.warps[1].index = list.warps[2].index;
    list.warps[3].names.Set("en", "");
    g_WarpDatabase.Build(list);

    std::vector<ItemDataIssue> issues;
    area->Check(issues);
    std::string found;
    for (const ItemDataIssue& issue : issues)
    {
        found += issue.ToString() + "\n";
    }
    INFO(found);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "warps[0].notWith[0].item"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "warps[0].needsOneOf[0].tag"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "warps[0].blockedMessage"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "warps[0].command"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "warps[2].index"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "warps[3].name"));
}

TEST_CASE("Warp List window: a click on a problem of the warps opens the window [editor][warps]")
{
    const ToolWindowsScope windows;
    TestImGuiContext context;
    std::unique_ptr<MuEditor::DataChecks::DataCheckArea> area = MuEditor::DataChecks::CreateWarpCheckArea();
    const ItemDataIssue issue{ItemDataIssueSeverity::Warning, "Data/Maps/Warps.json", ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem,          "warps[0].command",     "has a space"};
    ImGui::NewFrame();
    area->Open(issue, false);
    ImGui::Render();
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().warpList);
    ImGui::NewFrame();
    area->Open(issue, true);
    ImGui::Render();
    CHECK(g_MuEditorCore.GetToolWindows().warpList);
}

TEST_CASE("Warp List window: the Translations tab changes names as undo steps the game follows [editor][warps]")
{
    I18N::SetLocale("en");
    WarpsFolder folder;
    TestImGuiContext context;
    StartWithArena();
    MuEditor::Warps::WarpNameRows rows;
    const int arena = EditedList().warps[0].index;
    const size_t count = EditedList().warps.size();
    // The shipped file has no German names.
    REQUIRE(rows.List("", "de").size() == count);
    CHECK(rows.CountMissing(std::array<const char*, 2>{"en", "de"}) == std::vector<int>{0, static_cast<int>(count)});

    rows.CommitName(arena, "de", "Arena-DE");
    CHECK(EditedList().warps[0].names.GetOwn("de") == "Arena-DE");
    CHECK(g_WarpDatabase.GetList().warps[0].names.GetOwn("de") == "Arena-DE");
    CHECK(g_MuWarpListUI.GetUndoLabel() == "Arena: name");
    CHECK(rows.CountMissing(std::array<const char*, 1>{"de"}) == std::vector<int>{static_cast<int>(count) - 1});
    // The row stays listed until Refresh, so it doesn't jump away while being worked on.
    CHECK(rows.List("", "de").size() == count);
    rows.Refresh();
    CHECK(rows.List("", "de").size() == count - 1);

    // An empty English name is refused.
    rows.CommitName(arena, "en", "");
    CHECK(EditedList().warps[0].names.GetNeutral() == "Arena");

    // A search finds a warp by a name of any language; a click on it picks it in the Warps tab.
    CHECK(rows.List("calabozo2", "") == std::vector<int>{9});
    rows.Select(9);
    CHECK(g_MuWarpListUI.GetSelectedIndex() == 9);
    CHECK(rows.GetSelected() == 9);

    // A warp added in the Warps tab is listed.
    REQUIRE(rows.List("", "").size() == count);
    Press(ToolbarId(I18N::Editor::AddWarp));
    CHECK(rows.List("", "").size() == count + 1);

    g_MuWarpListUI.Undo();
    g_MuWarpListUI.Undo();
    CHECK_FALSE(EditedList().warps[0].names.HasOwn("de"));
    CHECK_FALSE(g_WarpDatabase.GetList().warps[0].names.HasOwn("de"));
    RenderWarpList();
}

#endif // _EDITOR
