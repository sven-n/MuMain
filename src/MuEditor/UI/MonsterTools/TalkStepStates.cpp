#include "stdafx.h"

#ifdef _EDITOR

#include "TalkStepStates.h"

#include <algorithm>

namespace MuEditor::Monsters::SetupLists
{
std::vector<int> MatchStepIds(std::span<const StepRow> before, std::span<const unsigned int> signatures,
                              const std::function<int()>& newId)
{
    std::vector<bool> taken(before.size(), false);
    std::vector<int> ids(signatures.size(), 0);
    const auto take = [&](size_t row, size_t old)
    {
        ids[row] = before[old].id;
        taken[old] = true;
    };
    // The same step in the same row.
    for (size_t row = 0; row < signatures.size() && row < before.size(); ++row)
    {
        if (before[row].id != 0 && before[row].signature == signatures[row])
            take(row, row);
    }
    // A step that moved.
    for (size_t row = 0; row < signatures.size(); ++row)
    {
        for (size_t old = 0; old < before.size() && ids[row] == 0; ++old)
        {
            if (!taken[old] && before[old].id != 0 && before[old].signature == signatures[row])
                take(row, old);
        }
    }
    // A step changed in its row, else a new one.
    for (size_t row = 0; row < signatures.size(); ++row)
    {
        if (ids[row] == 0 && row < before.size() && !taken[row] && before[row].id != 0)
            take(row, row);
        if (ids[row] == 0)
            ids[row] = newId();
    }
    return ids;
}

TalkStepStates::TalkStepStates(size_t rows) : m_storage(ImGui::GetStateStorage())
{
    ImGui::PushID("##Steps");
    m_countKey = ImGui::GetID("count");
    m_nextKey = ImGui::GetID("next");
    const size_t keys = std::max(rows, KnownCount()) + 1;
    for (size_t row = 0; row < keys; ++row)
    {
        ImGui::PushID(static_cast<int>(row));
        m_idKeys.push_back(ImGui::GetID("id"));
        m_signatureKeys.push_back(ImGui::GetID("signature"));
        ImGui::PopID();
    }
    ImGui::PopID();
}

std::vector<StepRow> TalkStepStates::Before() const
{
    std::vector<StepRow> rows;
    for (size_t row = 0; row < KnownCount(); ++row)
        rows.push_back({static_cast<unsigned int>(m_storage->GetInt(m_signatureKeys[row], 0)), Id(row)});
    return rows;
}

int TalkStepStates::NewId()
{
    const int id = m_storage->GetInt(m_nextKey, 1);
    m_storage->SetInt(m_nextKey, id + 1);
    return id;
}

int TalkStepStates::Id(size_t row) const
{
    return m_storage->GetInt(m_idKeys[row], 0);
}

void TalkStepStates::SetId(size_t row, int id)
{
    m_storage->SetInt(m_idKeys[row], id);
}

void TalkStepStates::Leave(std::span<const unsigned int> signatures)
{
    m_storage->SetInt(m_countKey, static_cast<int>(signatures.size()));
    for (size_t row = 0; row < signatures.size(); ++row)
        m_storage->SetInt(m_signatureKeys[row], static_cast<int>(signatures[row]));
}

size_t TalkStepStates::KnownCount() const
{
    return static_cast<size_t>(std::max(m_storage->GetInt(m_countKey, 0), 0));
}
} // namespace MuEditor::Monsters::SetupLists

#endif // _EDITOR
