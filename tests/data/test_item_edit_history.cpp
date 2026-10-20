#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemEditHistory.h"
#include "UI/ItemTools/ItemSelection.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <map>
#include <optional>
#include <string>

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
                }};
    }

    std::map<std::string, Data::Items::SharedItemModel> m_shared;

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
        items.MakeHooks().moveItem(Kris, target);
        history.Record({"Move", {{ItemEditStep::Kind::Move, Kris, target, {}, {}, std::nullopt, std::nullopt}}});
        REQUIRE(history.Undo());
        CHECK(items.Slot(Kris).item.Exists());
        CHECK_FALSE(items.Slot(target).item.Exists());
        REQUIRE(history.Redo());
        CHECK(history.GetLastChangedItem() == target);
        CHECK(items.Slot(target).item.Exists());
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

    history.Clear();
    CHECK_FALSE(history.CanUndo());
    CHECK_FALSE(history.IsModifiedSinceSave());
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
