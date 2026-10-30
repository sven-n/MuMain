#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditHistory.h"

#include <algorithm>
#include <ranges>
#include <utility>

namespace MuEditor::Items
{
namespace
{
const std::string NoLabel;

std::string SharedModelName(const ItemEditStep& step, bool undoing)
{
    const std::optional<Data::Items::SharedItemModel>& current = undoing ? step.sharedAfter : step.sharedBefore;
    const std::optional<Data::Items::SharedItemModel>& other = undoing ? step.sharedBefore : step.sharedAfter;
    return current ? current->name : other->name;
}
} // namespace

bool KeepsShape(const Data::Items::ItemDefinition& from, const Data::Items::ItemDefinition& to)
{
    return from.Exists() == to.Exists() && from.width == to.width && from.height == to.height && from.slot == to.slot;
}

ItemEditHistory::ItemEditHistory(Hooks hooks) : m_hooks(std::move(hooks)) {}

void ItemEditHistory::Record(ItemEditEntry entry)
{
    if (entry.steps.empty())
    {
        return;
    }
    m_entries.erase(m_entries.begin() + static_cast<std::ptrdiff_t>(m_done), m_entries.end());
    if (m_saved && *m_saved > m_done)
    {
        m_saved.reset();
    }
    m_entries.push_back(std::move(entry));
    m_done = m_entries.size();
    if (m_entries.size() > MaxEntries)
    {
        m_entries.erase(m_entries.begin());
        --m_done;
        if (m_saved)
        {
            m_saved = *m_saved > 0 ? std::optional<size_t>(*m_saved - 1) : std::nullopt;
        }
    }
}

bool ItemEditHistory::CanUndo() const
{
    return m_done > 0;
}

bool ItemEditHistory::CanRedo() const
{
    return m_done < m_entries.size();
}

const std::string& ItemEditHistory::GetUndoLabel() const
{
    return CanUndo() ? m_entries[m_done - 1].label : NoLabel;
}

const std::string& ItemEditHistory::GetRedoLabel() const
{
    return CanRedo() ? m_entries[m_done].label : NoLabel;
}

bool ItemEditHistory::Undo()
{
    if (!CanUndo())
    {
        m_lastRefusal = Refusal::NothingToDo;
        return false;
    }
    if (!TryApply(m_entries[m_done - 1], true))
    {
        return false;
    }
    --m_done;
    return true;
}

bool ItemEditHistory::Redo()
{
    if (!CanRedo())
    {
        m_lastRefusal = Refusal::NothingToDo;
        return false;
    }
    if (!TryApply(m_entries[m_done], false))
    {
        return false;
    }
    ++m_done;
    return true;
}

bool ItemEditHistory::TryApply(const ItemEditEntry& entry, bool undoing)
{
    if (!IsAsLeft(entry, undoing))
    {
        m_lastRefusal = Refusal::ItemsChanged;
        return false;
    }
    if (ChangesHeldItem(entry, undoing))
    {
        m_lastRefusal = Refusal::ItemHeld;
        return false;
    }
    m_lastRefusal = Refusal::None;
    Apply(entry, undoing);
    return true;
}

void ItemEditHistory::Clear()
{
    m_entries.clear();
    m_done = 0;
    m_saved.reset();
    m_lastChangedItem.reset();
}

void ItemEditHistory::MarkSaved()
{
    m_saved = m_done;
}

bool ItemEditHistory::IsModifiedSinceSave() const
{
    return m_saved != m_done;
}

// Undoing: the items are as the entry's steps left them. Redoing: as they
// were before them.
bool ItemEditHistory::IsAsLeft(const ItemEditEntry& entry, bool undoing) const
{
    for (const ItemEditStep& step : entry.steps)
    {
        switch (step.kind)
        {
        case ItemEditStep::Kind::Slot:
            if (m_hooks.readSlot(step.itemType) != (undoing ? step.after : step.before))
            {
                return false;
            }
            break;
        case ItemEditStep::Kind::Move:
        {
            const int occupied = undoing ? step.otherItemType : step.itemType;
            const int empty = undoing ? step.itemType : step.otherItemType;
            if (m_hooks.readSlot(occupied) != (undoing ? step.after : step.before) ||
                m_hooks.readSlot(empty).item.HasEntry())
            {
                return false;
            }
            break;
        }
        case ItemEditStep::Kind::SharedModel:
        {
            const std::optional<Data::Items::SharedItemModel>& expected =
                undoing ? step.sharedAfter : step.sharedBefore;
            if (m_hooks.readSharedModel(SharedModelName(step, undoing)) != expected)
            {
                return false;
            }
            break;
        }
        case ItemEditStep::Kind::GlowColors:
            if (m_hooks.readGlowColors() != (undoing ? step.colorsAfter : step.colorsBefore))
            {
                return false;
            }
            break;
        case ItemEditStep::Kind::StatFile:
            if (m_hooks.readStatFile() != *(undoing ? step.statFileAfter : step.statFileBefore))
            {
                return false;
            }
            break;
        }
    }
    return true;
}

// Called after IsAsLeft, so the items are in the state the entry starts from.
bool ItemEditHistory::ChangesHeldItem(const ItemEditEntry& entry, bool undoing) const
{
    if (!m_hooks.isHeld)
    {
        return false;
    }
    for (const ItemEditStep& step : entry.steps)
    {
        switch (step.kind)
        {
        case ItemEditStep::Kind::Slot:
        {
            const ItemSlotState& from = undoing ? step.after : step.before;
            const ItemSlotState& to = undoing ? step.before : step.after;
            if (!KeepsShape(from.item, to.item) && m_hooks.isHeld(step.itemType))
            {
                return true;
            }
            break;
        }
        case ItemEditStep::Kind::Move:
            if (m_hooks.isHeld(undoing ? step.otherItemType : step.itemType))
            {
                return true;
            }
            break;
        case ItemEditStep::Kind::SharedModel:
        case ItemEditStep::Kind::GlowColors:
        case ItemEditStep::Kind::StatFile:
            break;
        }
    }
    return false;
}

void ItemEditHistory::Apply(const ItemEditEntry& entry, bool undoing)
{
    m_lastChangedItem.reset();
    if (undoing)
    {
        for (const ItemEditStep& step : std::views::reverse(entry.steps))
        {
            ApplyStep(step, true);
        }
    }
    else
    {
        for (const ItemEditStep& step : entry.steps)
        {
            ApplyStep(step, false);
        }
    }
    const auto firstItem = std::ranges::find_if(
        entry.steps, [](const ItemEditStep& step)
        { return step.kind == ItemEditStep::Kind::Slot || step.kind == ItemEditStep::Kind::Move; });
    if (firstItem != entry.steps.end())
    {
        m_lastChangedItem =
            undoing || firstItem->kind != ItemEditStep::Kind::Move ? firstItem->itemType : firstItem->otherItemType;
    }
}

void ItemEditHistory::ApplyStep(const ItemEditStep& step, bool undoing)
{
    switch (step.kind)
    {
    case ItemEditStep::Kind::Slot:
    {
        const ItemSlotState& state = undoing ? step.before : step.after;
        m_hooks.restoreSlots(std::span<const ItemSlotState>(&state, 1));
        break;
    }
    case ItemEditStep::Kind::Move:
    {
        if (undoing)
        {
            m_hooks.moveItem(step.otherItemType, step.itemType);
        }
        else
        {
            m_hooks.moveItem(step.itemType, step.otherItemType);
        }
        const ItemSlotState& state = undoing ? step.before : step.after;
        m_hooks.restoreSlots(std::span<const ItemSlotState>(&state, 1));
        break;
    }
    case ItemEditStep::Kind::SharedModel:
        m_hooks.restoreSharedModel(SharedModelName(step, undoing), undoing ? step.sharedBefore : step.sharedAfter);
        break;
    case ItemEditStep::Kind::GlowColors:
        m_hooks.restoreGlowColors(undoing ? step.colorsBefore : step.colorsAfter);
        break;
    case ItemEditStep::Kind::StatFile:
        m_hooks.restoreStatFile(*(undoing ? step.statFileBefore : step.statFileAfter));
        break;
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
