#pragma once

#ifdef _EDITOR

#include "imgui.h"

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace MuEditor::Items
{
// The values of the selected item that a tool form edits. The widgets change
// the draft; a text or number field is stored (one undo step) when it is left
// after a change, a checkbox or list right when it changes. While the draft has
// no change that is not stored yet (no field being edited, or one only clicked
// into), it follows the selected item and the data.
// A selection that changes while a field is edited first stores that edit,
// to the item it belongs to; so does a form that was not drawn for a frame
// (its window hidden, the editor closed) or a field that was not drawn (its
// section closed) while it was being edited.
template <typename TValue> class ItemEditDraft
{
public:
    // The values of the item, nullopt when there is nothing to edit (no item,
    // no model).
    using Load = std::function<std::optional<TValue>(int itemType)>;
    // Stores the values under the undo label. It also gets an edit stored
    // without its field being left (the form hidden, the selection changed,
    // the field's section closed), so the form's checks of a field belong here: it stores nothing for an
    // edit it refuses, and the draft takes the item back as stored.
    using Store = std::function<void(const std::string& label, const TValue& value)>;

    ItemEditDraft(Load load, Store store) : m_load(std::move(load)), m_store(std::move(store)) {}

    // Call once a frame before the form. `dataVersion` changes with the data
    // the values come from. Returns the draft, or nullptr for nothing to edit.
    TValue* Begin(int itemType, long long dataVersion)
    {
        // A field stops being edited without being left when the form is not
        // drawn: ImGui lets go of it.
        const bool formWasHidden = m_active && m_lastEndFrame != ImGui::GetFrameCount() - 1;
        if (m_active && (itemType != m_itemType || formWasHidden) && m_changed && m_value)
        {
            m_store(m_activeLabel, *m_value);
            m_changed = false;
        }
        if (formWasHidden)
        {
            m_active = false;
            m_reload = true;
        }
        // Also while a field is active, as long as the draft has no change: a
        // field clicked into while a field of another tool was left (stored in
        // the same frame, after this Begin) would store that tool's old values
        // with its own edit. An active field keeps its own text.
        const bool follow = !m_active || !m_changed || itemType != m_itemType;
        if (follow && (itemType != m_itemType || dataVersion != m_version || m_reload))
        {
            m_value = m_load(itemType);
            m_itemType = itemType;
            m_version = dataVersion;
            m_reload = false;
            m_changed = false;
        }
        m_anyActive = false;
        return m_value ? &*m_value : nullptr;
    }

    // After a text or number field: stores the draft when the field was left
    // after a change.
    void AfterField(const std::string& label)
    {
        if (ImGui::IsItemActive())
        {
            m_anyActive = true;
            m_activeLabel = label;
        }
        if (ImGui::IsItemEdited())
        {
            m_changed = true;
        }
        // Not when the edit was stored already (the form was hidden meanwhile).
        if (ImGui::IsItemDeactivatedAfterEdit() && m_changed)
        {
            StoreNow(label);
        }
    }

    // For a widget that changes at once (checkbox, list, button): stores the
    // draft when it changed.
    void AfterChange(bool changed, const std::string& label)
    {
        if (changed)
        {
            StoreNow(label);
        }
    }

    // Stores the draft and takes the item back as stored: what the store
    // derived (a weapon kind that follows the classes) must be in the draft
    // before the next field is stored, also when that field is already
    // active. An active field keeps its own text meanwhile.
    void StoreNow(const std::string& label)
    {
        if (m_value)
        {
            m_store(label, *m_value);
            m_value = m_load(m_itemType);
        }
        m_changed = false;
        m_reload = true;
    }

    // Drops the changes of the active field: the draft is the item again.
    void Discard()
    {
        m_value = m_load(m_itemType);
        m_changed = false;
    }

    // Call after the form.
    void End()
    {
        // The field being edited was not drawn: a click on the arrow of its
        // section's header closes the section on mouse down, before ImGui
        // reports the field left, and the closed section does not draw it to
        // report it. Its edit is stored as when it is left.
        if (m_active && !m_anyActive && m_changed)
        {
            StoreNow(m_activeLabel);
        }
        m_active = m_anyActive;
        m_lastEndFrame = ImGui::GetFrameCount();
    }

private:
    Load m_load;
    Store m_store;
    std::optional<TValue> m_value;
    int m_itemType = -1;
    long long m_version = -1;
    std::string m_activeLabel;
    // A field of the form was active the frame before, or is this frame.
    bool m_active = false;
    bool m_anyActive = false;
    // The active field changed the draft since it was stored.
    bool m_changed = false;
    bool m_reload = false;
    // The ImGui frame of the last End.
    int m_lastEndFrame = -1;
};
} // namespace MuEditor::Items

#endif // _EDITOR
