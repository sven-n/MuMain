#include "stdafx.h"

#ifdef _EDITOR

#include "WarpListEdits.h"

#include "Core/Text/AsciiText.h"
#include "Core/Text/Utf8.h"
#include "I18N/All.h"

#include <algorithm>
#include <utility>

namespace MuEditor::Warps
{
namespace
{
// The label of the first field of the warp that differs; nullptr when none does.
const char* FindChangedField(const Warp& before, const Warp& after)
{
    if (before.index != after.index)
        return I18N::Editor::Index;
    if (before.command != after.command)
        return I18N::Editor::MoveCommandName;
    if (before.level != after.level)
        return I18N::Editor::Level;
    if (before.zen != after.zen)
        return I18N::Editor::Zen;
    if (before.gate != after.gate)
        return I18N::Editor::Gate;
    if (before.noClassDiscount != after.noClassDiscount)
        return I18N::Editor::NoClassDiscount;
    if (before.minMasterLevel != after.minMasterLevel)
        return I18N::Editor::MasterLevel;
    if (before.minResets != after.minResets)
        return I18N::Editor::Resets;
    if (before.pvpServersOnly != after.pvpServersOnly)
        return I18N::Editor::PvPServersOnly;
    if (before.gensOnly != after.gensOnly)
        return I18N::Editor::GensMembersOnly;
    if (before.needsOneOf != after.needsOneOf)
        return I18N::Editor::NeedsOneOf;
    if (before.notWith != after.notWith)
        return I18N::Editor::NotWith;
    if (before.blockedMessage != after.blockedMessage)
        return I18N::Editor::BlockedMessage;
    return nullptr;
}
} // namespace

std::string GetStepName(const Warp& warp)
{
    return warp.names.GetNeutral().empty() ? "#" + std::to_string(warp.index) : warp.names.GetNeutral();
}

std::string DescribeFieldChange(const WarpList& before, const WarpList& after)
{
    const size_t count = std::min(before.warps.size(), after.warps.size());
    for (size_t position = 0; position < count; ++position)
    {
        const Warp& old = before.warps[position];
        const Warp& now = after.warps[position];
        if (old == now)
        {
            continue;
        }
        const std::string name = GetStepName(old.names.GetNeutral().empty() ? now : old);
        if (old.names != now.names)
        {
            return I18N::Format(I18N::Editor::StepName, {name});
        }
        const char* field = FindChangedField(old, now);
        return I18N::Format(I18N::Editor::StepWarpField, {name, field != nullptr ? field : ""});
    }
    return I18N::Editor::ClassLevelDiscount;
}

void WarpListEdits::Reset(WarpList list, bool complete)
{
    m_complete = complete;
    m_list = std::move(list);
    m_committed = m_list;
    m_saved = m_list;
    m_undo.clear();
    m_redo.clear();
    m_pendingLabel.clear();
}

void WarpListEdits::Commit()
{
    if (m_list == m_committed)
    {
        m_pendingLabel.clear();
        return;
    }
    std::string label = GetPendingLabel();
    m_undo.push_back({std::move(m_committed), std::move(label)});
    if (m_undo.size() > MaxUndoSteps)
    {
        m_undo.erase(m_undo.begin());
    }
    m_redo.clear();
    m_committed = m_list;
    m_pendingLabel.clear();
}

void WarpListEdits::Undo()
{
    Commit();
    if (m_undo.empty())
    {
        return;
    }
    Step step = std::move(m_undo.back());
    m_undo.pop_back();
    m_redo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
}

void WarpListEdits::Redo()
{
    Commit();
    if (m_redo.empty())
    {
        return;
    }
    Step step = std::move(m_redo.back());
    m_redo.pop_back();
    m_undo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
}

std::string WarpListEdits::GetUndoLabel() const
{
    if (m_list != m_committed)
    {
        return GetPendingLabel();
    }
    return m_undo.empty() ? std::string() : m_undo.back().label;
}

std::string WarpListEdits::GetRedoLabel() const
{
    return m_redo.empty() ? std::string() : m_redo.back().label;
}

std::string WarpListEdits::GetPendingLabel() const
{
    return m_pendingLabel.empty() ? DescribeFieldChange(m_committed, m_list) : m_pendingLabel;
}

size_t WarpListEdits::AddWarp(size_t position)
{
    Warp warp;
    warp.index = FindFreeIndex();
    // Its own name: another warp can have "Warp<index>" after an index change.
    std::string name = "Warp" + std::to_string(warp.index);
    for (int number = 2; IsNameUsed(name); ++number)
    {
        name = "Warp" + std::to_string(warp.index) + "_" + std::to_string(number);
    }
    warp.names.Set(Data::LocalizedString::NeutralLocale, name);
    warp.command = name;
    position = std::min(position, m_list.warps.size());
    m_list.warps.insert(m_list.warps.begin() + static_cast<std::ptrdiff_t>(position), std::move(warp));
    m_pendingLabel = I18N::Format(I18N::Editor::StepAdd, {name});
    return position;
}

bool WarpListEdits::IsNameUsed(const std::string& text) const
{
    const auto same = [&](const std::string& other) { return Core::Text::EqualsIgnoringAsciiCase(other, text); };
    return std::any_of(m_list.warps.begin(), m_list.warps.end(),
                       [&](const Warp& warp)
                       {
                           if (same(warp.command) || same(warp.names.GetNeutral()))
                           {
                               return true;
                           }
                           const auto& translations = warp.names.GetTranslations();
                           return std::any_of(translations.begin(), translations.end(),
                                              [&](const auto& translation) { return same(translation.second); });
                       });
}

size_t WarpListEdits::CopyWarp(size_t position)
{
    if (position >= m_list.warps.size())
    {
        return AddWarp(position);
    }
    Warp copy = m_list.warps[position];
    m_pendingLabel = I18N::Format(I18N::Editor::StepCopy, {GetStepName(copy)});
    copy.index = FindFreeIndex();
    // Its own /move name and names, or /move and the M window would always find
    // the first of the two: the index added (Arena7). The text before it is cut so
    // the whole fits in what the game holds, or the game would cut the index off.
    const auto withSuffix = [](const std::string& text, const std::string& suffix)
    {
        std::string base = text;
        Core::Text::CutToUtf16Length(base, Data::Maps::MaxWarpNameLength - suffix.size());
        return base + suffix;
    };
    // None of the new texts may be another warp's /move name or name.
    const auto anyUsed = [&](const std::string& suffix)
    {
        if (IsNameUsed(withSuffix(copy.command, suffix)) || IsNameUsed(withSuffix(copy.names.GetNeutral(), suffix)))
        {
            return true;
        }
        const auto& translations = copy.names.GetTranslations();
        return std::any_of(translations.begin(), translations.end(), [&](const auto& translation)
                           { return IsNameUsed(withSuffix(translation.second, suffix)); });
    };
    std::string suffix = std::to_string(copy.index);
    for (int number = 2; anyUsed(suffix); ++number)
    {
        suffix = std::to_string(copy.index) + "_" + std::to_string(number);
    }
    copy.command = withSuffix(copy.command, suffix);
    copy.names.Set(Data::LocalizedString::NeutralLocale, withSuffix(copy.names.GetNeutral(), suffix));
    const auto translations = copy.names.GetTranslations();
    for (const auto& [locale, name] : translations)
    {
        copy.names.Set(locale, withSuffix(name, suffix));
    }
    m_list.warps.insert(m_list.warps.begin() + static_cast<std::ptrdiff_t>(position) + 1, std::move(copy));
    return position + 1;
}

void WarpListEdits::RemoveWarp(size_t position)
{
    if (position >= m_list.warps.size())
    {
        return;
    }
    m_pendingLabel = I18N::Format(I18N::Editor::StepRemove, {GetStepName(m_list.warps[position])});
    m_list.warps.erase(m_list.warps.begin() + static_cast<std::ptrdiff_t>(position));
}

bool WarpListEdits::MoveWarp(size_t position, int direction)
{
    const long long target = static_cast<long long>(position) + direction;
    if (position >= m_list.warps.size() || target < 0 || target >= static_cast<long long>(m_list.warps.size()))
    {
        return false;
    }
    m_pendingLabel = I18N::Format(I18N::Editor::StepMove, {GetStepName(m_list.warps[position])});
    std::swap(m_list.warps[position], m_list.warps[static_cast<size_t>(target)]);
    return true;
}

bool WarpListEdits::IsIndexUsed(int index) const
{
    return std::any_of(m_list.warps.begin(), m_list.warps.end(),
                       [index](const Warp& warp) { return warp.index == index; });
}

int WarpListEdits::FindFreeIndex() const
{
    int index = 1;
    while (IsIndexUsed(index))
    {
        ++index;
    }
    return index;
}
} // namespace MuEditor::Warps

#endif // _EDITOR
