#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemEditHistory.h"
#include "UI/ItemTools/ItemSelection.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::MakeItemType;

namespace
{
// Items and shared models kept in maps, standing in for the item database.
class FakeItems
{
public:
    ItemSlotState& Slot(int itemType)
    {
        ItemSlotState& state = m_slots[itemType];
        state.item.group = Data::Items::GetItemGroup(itemType);
        state.item.number = Data::Items::GetItemNumber(itemType);
        return state;
    }

    ItemEditHistory::Hooks MakeHooks()
    {
        return {[this](int itemType) { return Slot(itemType); },
                [this](std::span<const ItemSlotState> states)
                {
                    for (const ItemSlotState& state : states)
                    {
                        Slot(MakeItemType(state.item.group, state.item.number)) = state;
                    }
                },
                [this](int from, int to)
                {
                    ItemSlotState moved = Slot(from);
                    Slot(from) = ItemSlotState{};
                    moved.item.group = Data::Items::GetItemGroup(to);
                    moved.item.number = Data::Items::GetItemNumber(to);
                    if (MoveDerives)
                    {
                        moved.item.weaponKind = Data::Items::WeaponKind::Sword;
                    }
                    Slot(to) = moved;
                    return true;
                },
                [this](const std::string& name)
                {
                    const auto found = m_shared.find(name);
                    return found != m_shared.end() ? std::optional(found->second) : std::nullopt;
                },
                [this](const std::string& name, const std::optional<Data::Items::SharedItemModel>& model)
                {
                    m_shared.erase(name);
                    if (model)
                    {
                        m_shared[model->name] = *model;
                    }
                },
                {},
                [this] { return m_colors; },
                [this](std::span<const Data::Effects::GlowColor> colors)
                { m_colors.assign(colors.begin(), colors.end()); },
                [this] { return m_statFile; },
                [this](const Data::Items::ItemStatFile& file)
                {
                    m_statFile = file;
                    ++m_statFileRestores;
                }};
    }

    std::map<std::string, Data::Items::SharedItemModel> m_shared;
    std::vector<Data::Effects::GlowColor> m_colors;
    Data::Items::ItemStatFile m_statFile;
    int m_statFileRestores = 0;
    // The move changes the weapon kind, as a move to another group may.
    static inline bool MoveDerives = false;

private:
    std::map<int, ItemSlotState> m_slots;
};

ItemEditEntry SlotEntry(const std::string& label, int itemType, const ItemSlotState& before, const ItemSlotState& after)
{
    return {label, {{ItemEditStep::Kind::Slot, itemType, 0, before, after, std::nullopt, std::nullopt}}};
}

const int Kris = MakeItemType(0, 0);
} // namespace

TEST_CASE("The item edit history undoes and redoes what the tools changed [editor][items]")
{
    FakeItems items;
    ItemEditHistory history(items.MakeHooks());
    items.Slot(Kris).item.names = Data::LocalizedString::Parse("Kris");
    const ItemSlotState before = items.Slot(Kris);
    items.Slot(Kris).item.width = 2;
    const ItemSlotState after = items.Slot(Kris);
    history.Record(SlotEntry("Width", Kris, before, after));

    REQUIRE(history.CanUndo());
    CHECK(history.GetUndoLabel() == "Width");
    CHECK(history.IsModifiedSinceSave());
    REQUIRE(history.Undo());
    CHECK(items.Slot(Kris).item.width == 0);
    CHECK(history.GetLastChangedItem() == Kris);
    CHECK_FALSE(history.IsModifiedSinceSave());
    CHECK(history.CanRedo());
    CHECK(history.GetRedoLabel() == "Width");

    REQUIRE(history.Redo());
    CHECK(items.Slot(Kris).item.width == 2);
    CHECK_FALSE(history.Redo());

    // A new entry drops the ones that were undone.
    history.Undo();
    history.Record(SlotEntry("Height", Kris, before, before));
    CHECK_FALSE(history.CanRedo());
}

TEST_CASE("The item edit history refuses an undo when the item changed past it [editor][items]")
{
    FakeItems items;
    ItemEditHistory history(items.MakeHooks());
    const ItemSlotState before = items.Slot(Kris);
    items.Slot(Kris).item.width = 2;
    history.Record(SlotEntry("Width", Kris, before, items.Slot(Kris)));

    // Changed by something that did not record it.
    items.Slot(Kris).item.width = 3;
    CHECK_FALSE(history.Undo());
    CHECK(items.Slot(Kris).item.width == 3);
    CHECK(history.CanUndo());
}

TEST_CASE("The item edit history undoes moves and shared model changes [editor][items]")
{
    FakeItems items;
    ItemEditHistory history(items.MakeHooks());
    const int target = MakeItemType(0, 100);
    items.Slot(Kris).item.names = Data::LocalizedString::Parse("Kris");

    SUBCASE("a move")
    {
        const ItemSlotState before = items.Slot(Kris);
        items.MakeHooks().moveItem(Kris, target);
        const ItemSlotState after = items.Slot(target);
        history.Record({"Move", {{ItemEditStep::Kind::Move, Kris, target, before, after, std::nullopt, std::nullopt}}});
        REQUIRE(history.Undo());
        CHECK(items.Slot(Kris) == before);
        CHECK_FALSE(items.Slot(target).item.Exists());
        REQUIRE(history.Redo());
        CHECK(history.GetLastChangedItem() == target);
        CHECK(items.Slot(target) == after);
    }

    SUBCASE("a move puts back what the move derived")
    {
        items.Slot(Kris).item.weaponKind = Data::Items::WeaponKind::Axe;
        const ItemSlotState before = items.Slot(Kris);
        items.MakeHooks().moveItem(Kris, target);
        const ItemSlotState after = items.Slot(target);
        history.Record({"Move", {{ItemEditStep::Kind::Move, Kris, target, before, after, std::nullopt, std::nullopt}}});
        // A move that derives another weapon kind on the way back.
        FakeItems::MoveDerives = true;
        REQUIRE(history.Undo());
        FakeItems::MoveDerives = false;
        CHECK(items.Slot(Kris).item.weaponKind == Data::Items::WeaponKind::Axe);
    }

    SUBCASE("a move is not undone when the moved item changed since")
    {
        const ItemSlotState before = items.Slot(Kris);
        items.MakeHooks().moveItem(Kris, target);
        const ItemSlotState after = items.Slot(target);
        history.Record({"Move", {{ItemEditStep::Kind::Move, Kris, target, before, after, std::nullopt, std::nullopt}}});
        items.Slot(target).item.durability = 7;
        CHECK_FALSE(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::ItemsChanged);
    }

    SUBCASE("a renamed shared model")
    {
        const Data::Items::SharedItemModel oldModel{"old", "Data/Item/A.bmd", {"Item"}, {}};
        Data::Items::SharedItemModel newModel = oldModel;
        newModel.name = "new";
        items.m_shared["new"] = newModel;
        history.Record({"Rename", {{ItemEditStep::Kind::SharedModel, 0, 0, {}, {}, oldModel, newModel}}});
        REQUIRE(history.Undo());
        CHECK(items.m_shared.contains("old"));
        CHECK_FALSE(items.m_shared.contains("new"));
        CHECK_FALSE(history.GetLastChangedItem().has_value());
        REQUIRE(history.Redo());
        CHECK(items.m_shared.contains("new"));
    }

    SUBCASE("a new glow color")
    {
        const std::vector<Data::Effects::GlowColor> before{{"orange", {1, 0.5, 0}}};
        std::vector<Data::Effects::GlowColor> after = before;
        after.push_back({"teal", {0, 0.5, 0.5}});
        items.m_colors = after;
        ItemEditStep step{ItemEditStep::Kind::GlowColors};
        step.colorsBefore = before;
        step.colorsAfter = after;
        history.Record({"New glow color", {step}});
        REQUIRE(history.Undo());
        CHECK(items.m_colors == before);
        CHECK_FALSE(history.GetLastChangedItem().has_value());
        REQUIRE(history.Redo());
        CHECK(items.m_colors == after);

        // A list changed past the history is not overwritten.
        items.m_colors.push_back({"gold", {1, 0.7, 0.2}});
        CHECK_FALSE(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::ItemsChanged);
    }

    SUBCASE("the item stat file with the items it gave another profile")
    {
        Data::Items::ItemStatFile before;
        before.excellentDropLevelBonus = 25;
        Data::Items::ItemStatFile after = before;
        after.excellentDropLevelBonus = 26;
        items.m_statFile = after;
        ItemSlotState kris = items.Slot(Kris);
        ItemSlotState krisAfter = kris;
        krisAfter.item.statProfile = "blade";
        items.Slot(Kris) = krisAfter;

        ItemEditStep step{ItemEditStep::Kind::StatFile};
        step.statFileBefore = std::make_shared<const Data::Items::ItemStatFile>(before);
        step.statFileAfter = std::make_shared<const Data::Items::ItemStatFile>(after);
        ItemEditEntry entry = SlotEntry("Change the formulas", Kris, kris, krisAfter);
        entry.steps.insert(entry.steps.begin(), step);
        history.Record(entry);

        REQUIRE(history.Undo());
        CHECK(items.m_statFile == before);
        CHECK(items.Slot(Kris) == kris);
        // The item it changed is selected, not the file.
        CHECK(history.GetLastChangedItem() == Kris);
        REQUIRE(history.Redo());
        CHECK(items.m_statFile == after);
        CHECK(items.Slot(Kris) == krisAfter);

        // A file changed past the history is not overwritten.
        items.m_statFile.ancientDropLevelBonus = 31;
        const int restores = items.m_statFileRestores;
        CHECK_FALSE(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::ItemsChanged);
        CHECK(items.m_statFileRestores == restores);
    }
}

TEST_CASE("The item edit history keeps a limited number of entries and knows the saved state [editor][items]")
{
    FakeItems items;
    ItemEditHistory history(items.MakeHooks());
    ItemSlotState state = items.Slot(Kris);
    for (size_t entry = 0; entry < ItemEditHistory::MaxEntries + 10; ++entry)
    {
        ItemSlotState next = state;
        next.item.durability = static_cast<BYTE>(entry % 200);
        items.Slot(Kris) = next;
        history.Record(SlotEntry("Durability", Kris, state, next));
        state = next;
        if (entry == 5)
        {
            history.MarkSaved();
        }
    }
    size_t undone = 0;
    while (history.Undo())
    {
        ++undone;
    }
    CHECK(undone == ItemEditHistory::MaxEntries);
    // The saved state was among the entries dropped first.
    CHECK(history.IsModifiedSinceSave());

    // Cleared for data that replaced all items: it is not saved yet.
    history.Clear();
    CHECK_FALSE(history.CanUndo());
    CHECK(history.IsModifiedSinceSave());
    history.MarkSaved();
    CHECK_FALSE(history.IsModifiedSinceSave());
}

TEST_CASE("The item edit history refuses a step that would change an item the game holds [editor][items]")
{
    FakeItems items;
    int heldItemType = -1;
    ItemEditHistory::Hooks hooks = items.MakeHooks();
    hooks.isHeld = [&](int itemType) { return itemType == heldItemType; };
    ItemEditHistory history(hooks);
    items.Slot(Kris).item.names = Data::LocalizedString::Parse("Kris");
    const ItemSlotState before = items.Slot(Kris);

    SUBCASE("its size")
    {
        items.Slot(Kris).item.height = 3;
        history.Record(SlotEntry("Height", Kris, before, items.Slot(Kris)));
        heldItemType = Kris;
        CHECK_FALSE(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::ItemHeld);
        CHECK(items.Slot(Kris).item.height == 3);
        heldItemType = -1;
        CHECK(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::None);
        CHECK(items.Slot(Kris).item.height == 0);
    }
    SUBCASE("another field")
    {
        items.Slot(Kris).item.durability = 30;
        history.Record(SlotEntry("Durability", Kris, before, items.Slot(Kris)));
        heldItemType = Kris;
        CHECK(history.Undo());
        CHECK(items.Slot(Kris).item.durability == before.item.durability);
    }
    SUBCASE("a move")
    {
        const int moved = MakeItemType(0, 40);
        items.Slot(moved) = before;
        items.Slot(moved).item.number = 40;
        items.Slot(Kris) = ItemSlotState{};
        history.Record(
            {"Move", {{ItemEditStep::Kind::Move, Kris, moved, before, items.Slot(moved), std::nullopt, std::nullopt}}});
        heldItemType = moved;
        CHECK_FALSE(history.Undo());
        CHECK(history.GetLastRefusal() == ItemEditHistory::Refusal::ItemHeld);
        heldItemType = -1;
        CHECK(history.Undo());
        CHECK(items.Slot(Kris).item.Exists());
    }
}

TEST_CASE("Selecting an item counts, also the same item again [editor][items]")
{
    ItemSelection& selection = ItemSelection::GetInstance();
    const int serial = selection.GetSerial();
    selection.Select(Kris);
    CHECK(selection.GetItemType() == Kris);
    selection.Select(Kris);
    CHECK(selection.GetSerial() == serial + 2);
}

#endif // _EDITOR
