#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterTalkTextRows.h"

#include "MonsterEdits.h"
#include "MonsterOwnValues.h"
#include "MonsterSelection.h"
#include "MonsterShownValues.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <optional>

namespace MuEditor::Monsters
{
namespace
{
using ShownValues::WholeMonsterKey;

int NumberOf(int key)
{
    return key >= WholeMonsterKey ? key - WholeMonsterKey : key;
}

std::string Lower(std::string_view text)
{
    std::string lower(text);
    std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

using TalkTexts = std::vector<std::pair<std::string, Data::LocalizedString>>;

// The texts of a key's talk with their fields, its map and state variants'
// included.
TalkTexts TextsOf(int key)
{
    TalkTexts texts;
    const std::optional<ShownValues::ShownMonster> shown = ShownValues::Load(key);
    if (!shown)
        return texts;
    Data::Monsters::ForEachMonsterTalkText(shown->monster,
                                           [&](const Data::LocalizedString& text, const std::string& field)
                                           { texts.emplace_back(field, text); });
    return texts;
}
} // namespace

std::vector<MonsterTalkTextRows::Row> MonsterTalkTextRows::CollectRows() const
{
    std::vector<Row> rows;
    const auto add = [&](int key, TalkTexts texts)
    {
        for (auto& [field, text] : texts)
            rows.push_back({static_cast<int>(rows.size()), key, std::move(field), std::move(text)});
    };
    TalkTexts shared;
    Data::Monsters::ForEachSharedTalkText(g_MonsterDatabase.GetTalkTexts(),
                                          [&](const Data::LocalizedString& text, const std::string& field)
                                          { shared.emplace_back(field, text); });
    add(SharedKey, std::move(shared));
    for (const std::vector<int>& numbers : g_MonsterEdits.ReadMonsters())
    {
        if (numbers.size() < 2)
        {
            add(numbers.front(), TextsOf(numbers.front()));
            continue;
        }
        // The monster's texts, then those of a number whose talk is its own.
        const int whole = numbers.front() + WholeMonsterKey;
        const TalkTexts shared = TextsOf(whole);
        add(whole, shared);
        for (const int member : numbers)
        {
            TalkTexts own = TextsOf(member);
            if (own != shared)
                add(member, std::move(own));
        }
    }
    return rows;
}

void MonsterTalkTextRows::Build() const
{
    if (m_builtVersion == GetVersion())
        return;
    m_all = CollectRows();
    m_builtVersion = GetVersion();
}

const std::vector<int>& MonsterTalkTextRows::List(std::string_view search, std::string_view missingLocale)
{
    Build();
    if (m_listed && m_listedVersion == m_builtVersion && search == m_search && missingLocale == m_missingLocale)
        return m_rows;
    m_search.assign(search);
    m_missingLocale.assign(missingLocale);
    m_rows.clear();
    const std::string needle = Lower(search);
    for (const Row& row : m_all)
    {
        if (!m_missingLocale.empty() && row.text.HasOwn(m_missingLocale))
            continue;
        char id[96];
        FormatId(row.id, id, sizeof(id));
        if (!needle.empty() && Lower(id).find(needle) == std::string::npos &&
            Lower(row.text.GetNeutral()).find(needle) == std::string::npos &&
            (row.key == SharedKey ||
             Lower(ShownValues::MonsterName(NumberOf(row.key))).find(needle) == std::string::npos))
            continue;
        m_rows.push_back(row.id);
    }
    m_listed = true;
    m_listedVersion = m_builtVersion;
    return m_rows;
}

void MonsterTalkTextRows::Refresh()
{
    m_listed = false;
}

int MonsterTalkTextRows::GetVersion() const
{
    return g_MonsterDatabase.GetVersion() + g_MonsterDataHandler.GetGroupsVersion() + OwnValues::GetUnlockVersion();
}

std::vector<int> MonsterTalkTextRows::CountMissing(std::span<const char* const> locales) const
{
    Build();
    std::vector<int> missing(locales.size(), 0);
    for (const Row& row : m_all)
    {
        for (size_t index = 0; index < locales.size(); ++index)
            missing[index] += row.text.HasOwn(locales[index]) ? 0 : 1;
    }
    return missing;
}

const Data::LocalizedString* MonsterTalkTextRows::FindNames(int id) const
{
    Build();
    return id >= 0 && id < static_cast<int>(m_all.size()) ? &m_all[static_cast<size_t>(id)].text : nullptr;
}

void MonsterTalkTextRows::FormatId(int id, char* buffer, std::size_t size) const
{
    Build();
    if (id < 0 || id >= static_cast<int>(m_all.size()))
    {
        std::snprintf(buffer, size, "?");
        return;
    }
    // "408 onTalk[1].dialog.lines[0]"; a monster with several numbers by its
    // first one and how many it has.
    const Row& row = m_all[static_cast<size_t>(id)];
    if (row.key == SharedKey)
        std::snprintf(buffer, size, "%s", row.field.c_str());
    else if (row.key >= WholeMonsterKey)
        std::snprintf(buffer, size, "%d (%d) %s", NumberOf(row.key),
                      g_MonsterEdits.CountMonsterNumbers(NumberOf(row.key)), row.field.c_str());
    else
        std::snprintf(buffer, size, "%d %s", row.key, row.field.c_str());
}

void MonsterTalkTextRows::CommitName(int id, const char* locale, const char* text)
{
    Build();
    if (id < 0 || id >= static_cast<int>(m_all.size()))
        return;
    const Row row = m_all[static_cast<size_t>(id)];
    if (row.key == SharedKey)
    {
        Data::Monsters::MonsterTalkTexts texts = g_MonsterDatabase.GetTalkTexts();
        bool changed = false;
        Data::Monsters::ForEachSharedTalkText(texts,
                                              [&](Data::LocalizedString& talkText, const std::string& field)
                                              {
                                                  if (field == row.field && !changed)
                                                  {
                                                      talkText.Set(locale, text);
                                                      changed = true;
                                                  }
                                              });
        const std::string label = I18N::Format(I18N::Editor::StepTalkText, {I18N::Editor::SharedTalkTexts, row.field,
                                                                            I18N::GetLanguageDisplayName(locale)});
        if (changed)
            g_MonsterEdits.SetTalkTexts(label, texts);
        return;
    }
    std::optional<ShownValues::ShownMonster> shown = ShownValues::Load(row.key);
    if (!shown)
        return;
    bool found = false;
    Data::Monsters::ForEachMonsterTalkText(shown->monster,
                                           [&](Data::LocalizedString& talkText, const std::string& field)
                                           {
                                               if (field == row.field && !found)
                                               {
                                                   talkText.Set(locale, text);
                                                   found = true;
                                               }
                                           });
    if (!found)
        return;
    const std::string label = I18N::Format(
        I18N::Editor::StepTalkText, {ShownValues::LogName(*shown), row.field, I18N::GetLanguageDisplayName(locale)});
    if (!ShownValues::Store(label, *shown))
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheTalkTextWasNotStored);
}

int MonsterTalkTextRows::GetSelected() const
{
    Build();
    // The first text of the selected monster's key.
    const int key = ShownValues::GetSelectedKey();
    if (key < 0)
        return -1;
    const auto found = std::ranges::find(m_all, key, &Row::key);
    return found != m_all.end() ? found->id : -1;
}

int MonsterTalkTextRows::GetSelectionSerial() const
{
    return g_MonsterSelection.GetSerial();
}

void MonsterTalkTextRows::Select(int id)
{
    Build();
    if (id < 0 || id >= static_cast<int>(m_all.size()))
        return;
    const int key = m_all[static_cast<size_t>(id)].key;
    if (key == SharedKey)
        return;
    if (key >= WholeMonsterKey)
        g_MonsterSelection.SelectMonster(NumberOf(key));
    else
        g_MonsterSelection.Select(key);
}

void MonsterTalkTextRows::ShowBrowser()
{
    g_MuEditorCore.GetToolWindows().monsterBrowser = true;
    ImGui::SetWindowFocus(WindowIds::MonsterBrowser);
}

const char* MonsterTalkTextRows::GetSelectTooltip() const
{
    return I18N::Editor::SelectTheMonsterAndShowTheMonsterBrowser;
}

const char* MonsterTalkTextRows::GetAllRowsLabel() const
{
    return I18N::Editor::AllTalkTexts;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
