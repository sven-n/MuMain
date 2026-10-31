#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "I18N/All.h"
#include "UI/ServerList/ServerListEdits.h"

#include <string>
#include <vector>

using Data::ServerList::ChannelSettings;
using MuEditor::ServerList::ServerGroup;
using MuEditor::ServerList::ServerListData;
using MuEditor::ServerList::ServerListEdits;

namespace
{
ServerGroup Group(int id, const std::string& name)
{
    ServerGroup group;
    group.id = id;
    group.name = name;
    return group;
}

ServerListData List(std::vector<ServerGroup> groups)
{
    ServerListData list;
    list.groups = std::move(groups);
    return list;
}

ServerListEdits ThreeGroups()
{
    ServerListEdits edits;
    edits.Reset(List({Group(0, "Valhalla"), Group(1, "Helheim"), Group(2, "Midgard")}));
    return edits;
}

ChannelSettings Gold()
{
    ChannelSettings channel;
    channel.pvp = false;
    channel.vip = "gold";
    return channel;
}

std::vector<std::string> Names(const ServerListEdits& edits)
{
    std::vector<std::string> names;
    for (const ServerGroup& group : edits.GetGroups())
    {
        names.push_back(group.name);
    }
    return names;
}
} // namespace

TEST_CASE("ServerListEdits: a reset has nothing to undo and nothing unsaved")
{
    const ServerListEdits edits = ThreeGroups();

    CHECK_FALSE(edits.CanUndo());
    CHECK_FALSE(edits.CanRedo());
    CHECK_FALSE(edits.HasUnsavedChanges());
}

TEST_CASE("ServerListEdits: what changed since the last step is one undo step")
{
    ServerListEdits edits = ThreeGroups();
    edits.EditGroups()[0].name = "V";
    edits.EditGroups()[0].name = "Val";
    edits.EditGroups()[0].channels[0] = Gold();
    edits.Commit();

    CHECK(edits.CanUndo());
    CHECK(edits.HasUnsavedChanges());

    edits.Undo();
    CHECK(edits.GetGroups()[0].name == "Valhalla");
    CHECK(edits.GetGroups()[0].channels[0] == ChannelSettings{});
    CHECK_FALSE(edits.CanUndo());
    CHECK_FALSE(edits.HasUnsavedChanges());

    edits.Redo();
    CHECK(edits.GetGroups()[0].name == "Val");
    CHECK(edits.GetGroups()[0].channels[0] == Gold());
}

TEST_CASE("ServerListEdits: undo also takes back a change not yet committed")
{
    ServerListEdits edits = ThreeGroups();
    edits.EditGroups()[1].name = "Hel";

    edits.Undo();

    CHECK(edits.GetGroups()[1].name == "Helheim");
    CHECK(edits.CanRedo());
}

TEST_CASE("ServerListEdits: a new change drops what could be redone")
{
    ServerListEdits edits = ThreeGroups();
    edits.RemoveGroup(2);
    edits.Commit();
    edits.Undo();
    REQUIRE(edits.CanRedo());

    edits.MoveGroup(0, 1);
    edits.Commit();

    CHECK_FALSE(edits.CanRedo());
}

TEST_CASE("ServerListEdits: committing without a change adds no step")
{
    ServerListEdits edits = ThreeGroups();
    edits.Commit();
    CHECK_FALSE(edits.CanUndo());

    edits.EditGroups()[0].name = "X";
    edits.EditGroups()[0].name = "Valhalla";
    edits.Commit();
    CHECK_FALSE(edits.CanUndo());
}

TEST_CASE("ServerListEdits: the undo list keeps the last steps")
{
    ServerListEdits edits = ThreeGroups();
    for (size_t step = 0; step < ServerListEdits::MaxUndoSteps + 10; ++step)
    {
        edits.EditGroups()[0].name = "Step " + std::to_string(step);
        edits.Commit();
    }
    size_t undone = 0;
    while (edits.CanUndo())
    {
        edits.Undo();
        ++undone;
    }
    CHECK(undone == ServerListEdits::MaxUndoSteps);
    CHECK(edits.GetGroups()[0].name == "Step 9");
}

TEST_CASE("ServerListEdits: saved groups are not unsaved changes")
{
    ServerListEdits edits = ThreeGroups();
    edits.EditGroups()[2].testServer = true;
    edits.Commit();
    edits.MarkSaved();

    CHECK_FALSE(edits.HasUnsavedChanges());
    // Undo stays across a save; the file then differs again.
    edits.Undo();
    CHECK(edits.HasUnsavedChanges());
}

TEST_CASE("ServerListEdits: groups from a read with errors are not complete until a clean read")
{
    ServerListEdits edits;
    edits.Reset(List({Group(0, "Valhalla")}), false);
    CHECK_FALSE(edits.IsComplete());

    edits.EditGroups()[0].name = "Asgard";
    edits.Commit();
    edits.MarkSaved();
    CHECK_FALSE(edits.IsComplete());

    edits.Reset(List({Group(0, "Valhalla"), Group(1, "Helheim")}));
    CHECK(edits.IsComplete());
}

TEST_CASE("ServerListEdits: a new group gets the smallest free ID at the end")
{
    ServerListEdits edits;
    edits.Reset(List({Group(0, "A"), Group(2, "B")}));

    const std::optional<size_t> added = edits.AddGroup("New");

    REQUIRE(added == size_t{2});
    CHECK(edits.GetGroups()[2].id == 1);
    CHECK(edits.GetGroups()[2].name == "New");
}

TEST_CASE("ServerListEdits: a new group gets the wanted ID when it is free")
{
    ServerListEdits edits = ThreeGroups();

    REQUIRE(edits.AddGroup("Sent", 7).has_value());
    CHECK(edits.GetGroups().back().id == 7);

    REQUIRE(edits.AddGroup("Taken", 1).has_value());
    CHECK(edits.GetGroups().back().id == 3);
}

TEST_CASE("ServerListEdits: remove and move")
{
    ServerListEdits edits = ThreeGroups();

    CHECK(edits.MoveGroup(2, -1));
    CHECK(Names(edits) == std::vector<std::string>{"Valhalla", "Midgard", "Helheim"});
    CHECK_FALSE(edits.MoveGroup(0, -1));
    CHECK_FALSE(edits.MoveGroup(2, 1));
    CHECK_FALSE(edits.MoveGroup(5, 1));

    edits.RemoveGroup(0);
    CHECK(Names(edits) == std::vector<std::string>{"Midgard", "Helheim"});
    edits.RemoveGroup(9);
    CHECK(edits.GetGroups().size() == 2);
}

TEST_CASE("ServerListEdits: each step is named for the editor bar")
{
    I18N::SetLocale("en");
    ServerListEdits edits = ThreeGroups();
    CHECK(edits.GetUndoLabel().empty());

    edits.EditGroups()[0].channels[2] = Gold();
    // Not committed yet (a field being typed in): named all the same.
    CHECK(edits.GetUndoLabel() == "Valhalla: channel 3");
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Valhalla: channel 3");

    edits.EditGroups()[1].name = "Asgard";
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Helheim: name");

    edits.MoveGroup(2, -1);
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Move Midgard");

    edits.RemoveGroup(0);
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Remove Valhalla");

    edits.AddGroup("New Group");
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Add New Group");

    // A step keeps its name when it is undone and redone.
    edits.Undo();
    CHECK(edits.GetRedoLabel() == "Add New Group");
    CHECK(edits.GetUndoLabel() == "Remove Valhalla");
    edits.Redo();
    CHECK(edits.GetUndoLabel() == "Add New Group");
    CHECK(edits.GetRedoLabel().empty());
}

TEST_CASE("ServerListEdits: field changes are named by the group as it was")
{
    I18N::SetLocale("en");
    ServerListEdits edits = ThreeGroups();

    edits.EditGroups()[2].id = 7;
    CHECK(edits.GetUndoLabel() == "Midgard: ID");
    edits.Commit();
    edits.EditGroups()[2].description = "New";
    CHECK(edits.GetUndoLabel() == "Midgard: description");
    edits.Commit();
    edits.EditGroups()[2].testServer = true;
    CHECK(edits.GetUndoLabel() == "Midgard: test server");
}

TEST_CASE("ServerListEdits: IDs in use")
{
    const ServerListEdits edits = ThreeGroups();

    CHECK(edits.IsIdUsed(1));
    CHECK_FALSE(edits.IsIdUsed(3));
    CHECK(edits.FindFreeId() == 3);
}

TEST_CASE("ServerListEdits: a new VIP type gets an id from its name that no other type has")
{
    ServerListEdits edits = ThreeGroups();

    CHECK(edits.AddVipType("Gold") == 0);
    CHECK(edits.AddVipType("Gold") == 1);
    CHECK(edits.AddVipType("Gold Plus") == 2);

    const std::vector<MuEditor::ServerList::VipType>& vipTypes = edits.GetList().vipTypes;
    REQUIRE(vipTypes.size() == 3);
    CHECK(vipTypes[0].id == "gold");
    CHECK(vipTypes[0].name.GetNeutral() == "Gold");
    CHECK(vipTypes[1].id == "gold2");
    CHECK(vipTypes[2].id == "goldPlus");
}

TEST_CASE("ServerListEdits: a removed VIP type leaves its channels without one")
{
    I18N::SetLocale("en");
    ServerListEdits edits = ThreeGroups();
    edits.AddVipType("Gold");
    edits.EditGroups()[0].channels[0] = Gold();
    edits.EditGroups()[2].channels[4] = Gold();
    edits.Commit();

    CHECK(edits.FindChannelsWithVipType("gold") == std::vector<std::string>{"Valhalla-1", "Midgard-5"});
    edits.RemoveVipType(0);
    edits.Commit();

    CHECK(edits.GetList().vipTypes.empty());
    CHECK(edits.GetGroups()[0].channels[0].vip.empty());
    // PvP stays as it was.
    CHECK_FALSE(edits.GetGroups()[0].channels[0].pvp);
    CHECK(edits.GetUndoLabel() == "Remove VIP type Gold");

    edits.Undo();
    CHECK(edits.GetGroups()[0].channels[0] == Gold());
}

TEST_CASE("ServerListEdits: a VIP type's new id goes to its channels too")
{
    ServerListEdits edits = ThreeGroups();
    edits.AddVipType("Gold");
    edits.AddVipType("Silver");
    edits.EditGroups()[1].channels[0] = Gold();
    edits.Commit();

    CHECK(edits.ChangeVipTypeId(0, "golden"));
    CHECK(edits.GetList().vipTypes[0].id == "golden");
    CHECK(edits.GetGroups()[1].channels[0].vip == "golden");

    // Not a VIP type id, or one another type has: nothing changes.
    CHECK_FALSE(edits.ChangeVipTypeId(0, "Golden"));
    CHECK_FALSE(edits.ChangeVipTypeId(0, "silver"));
    CHECK_FALSE(edits.ChangeVipTypeId(5, "bronze"));
    CHECK(edits.GetList().vipTypes[0].id == "golden");
    CHECK(edits.ChangeVipTypeId(0, "golden"));
}

TEST_CASE("ServerListEdits: VIP types move and their changes are named")
{
    I18N::SetLocale("en");
    ServerListEdits edits = ThreeGroups();
    edits.AddVipType("Gold");
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Add VIP type Gold");
    edits.AddVipType("Silver");
    edits.Commit();

    CHECK(edits.MoveVipType(1, -1));
    edits.Commit();
    CHECK(edits.GetUndoLabel() == "Move VIP type Silver");
    CHECK(edits.GetList().vipTypes[0].id == "silver");
    CHECK_FALSE(edits.MoveVipType(0, -1));
    CHECK_FALSE(edits.MoveVipType(1, 1));

    edits.EditList().vipTypes[1].themeColor = Data::ServerList::ThemeColor::Orange;
    CHECK(edits.GetUndoLabel() == "Gold: theme color");
    edits.Commit();
    edits.EditList().vipTypes[1].name.Set("es", "Oro");
    CHECK(edits.GetUndoLabel() == "Gold: name");
    edits.Commit();
    edits.EditList().names.Template(Data::ServerList::ChannelNamePlace::Login).Set("en", "[{name}]");
    CHECK(edits.GetUndoLabel() == "Text: Login window");
    edits.Commit();
    edits.EditList().names.nonPvp.Set("de", "Nicht-PvP");
    CHECK(edits.GetUndoLabel() == "Text: Non-PvP text");
    CHECK(edits.HasUnsavedChanges());
}

#endif // _EDITOR
