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
    if (!CanUndo() || !IsAsLeft(m_entries[m_done - 1], true))
    {
        return false;
    }
    Apply(m_entries[m_done - 1], true);
    --m_done;
    return true;
}

bool ItemEditHistory::Redo()
{
    if (!CanRedo() || !IsAsLeft(m_entries[m_done], false))
    {
        return false;
    }
    Apply(m_entries[m_done], false);
    ++m_done;
    return true;
}

void ItemEditHistory::Clear()
{
    m_entries.clear();
    m_done = 0;
    m_saved = 0;
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
            if (!m_hooks.readSlot(occupied).item.Exists() || m_hooks.readSlot(empty).item.Exists())
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
        }
    }
    return true;
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
    const auto firstItem = std::ranges::find_if(entry.steps, [](const ItemEditStep& step)
                                                { return step.kind != ItemEditStep::Kind::SharedModel; });
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
        if (undoing)
        {
            m_hooks.moveItem(step.otherItemType, step.itemType);
        }
        else
        {
            m_hooks.moveItem(step.itemType, step.otherItemType);
        }
        break;
    case ItemEditStep::Kind::SharedModel:
        m_hooks.restoreSharedModel(SharedModelName(step, undoing), undoing ? step.sharedBefore : step.sharedAfter);
        break;
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
