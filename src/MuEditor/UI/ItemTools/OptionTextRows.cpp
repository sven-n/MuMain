#include "stdafx.h"

#ifdef _EDITOR

#include "OptionTextRows.h"

#include "../MuEditor/UI/Common/TextSearch.h"
#include "I18N/All.h"

#include <cstdio>

namespace
{
const std::string NoName;
} // namespace

void COptionTextRows::SetTexts(Data::Items::ItemOptionTexts* texts, const Data::Items::ItemOptionTexts* stored)
{
    m_texts = texts;
    m_stored = stored;
    UpdateNames();
}

const std::string& COptionTextRows::GetName(int id) const
{
    return id >= 0 && static_cast<size_t>(id) < m_idNames.size() ? m_idNames[static_cast<size_t>(id)] : NoName;
}

int COptionTextRows::IdOf(const std::string& name)
{
    const auto [found, added] = m_ids.try_emplace(name, static_cast<int>(m_idNames.size()));
    if (added)
    {
        m_idNames.push_back(name);
    }
    return found->second;
}

void COptionTextRows::UpdateNames()
{
    if (m_texts == nullptr || *m_texts == m_namesOf)
    {
        return;
    }
    m_rows.clear();
    for (const auto& [name, text] : m_texts->texts)
    {
        m_rows.push_back(IdOf(name));
    }
    m_namesOf = *m_texts;
    m_listed = false;
    ++m_version;
}

const std::vector<int>& COptionTextRows::List(std::string_view search, std::string_view missingLocale)
{
    if (m_listed && search == m_search && missingLocale == m_missingLocale)
    {
        return m_list;
    }
    m_search = std::string(search);
    m_missingLocale = std::string(missingLocale);
    m_list.clear();
    for (const int id : m_rows)
    {
        const Data::LocalizedString* names = FindNames(id);
        if (names == nullptr)
        {
            continue;
        }
        if (!missingLocale.empty() && !names->Get(missingLocale).empty())
        {
            continue;
        }
        if (!search.empty() && !MuEditor::TextSearch::ContainsIgnoringCase(GetName(id), search) &&
            !MuEditor::TextSearch::ContainsIgnoringCase(names->Get("en"), search))
        {
            continue;
        }
        m_list.push_back(id);
    }
    m_listed = true;
    return m_list;
}

void COptionTextRows::Refresh()
{
    m_listed = false;
}

int COptionTextRows::GetVersion() const
{
    return m_version;
}

std::vector<int> COptionTextRows::CountMissing(std::span<const char* const> locales) const
{
    std::vector<int> missing(locales.size(), 0);
    for (const int id : m_rows)
    {
        const Data::LocalizedString* names = FindNames(id);
        for (size_t locale = 0; names != nullptr && locale < locales.size(); ++locale)
        {
            missing[locale] += names->Get(locales[locale]).empty() ? 1 : 0;
        }
    }
    return missing;
}

const Data::LocalizedString* COptionTextRows::FindNames(int id) const
{
    return m_texts != nullptr ? m_texts->Find(GetName(id)) : nullptr;
}

void COptionTextRows::FormatId(int id, char* buffer, std::size_t size) const
{
    const Data::LocalizedString* text = FindNames(id);
    const Data::LocalizedString* stored = m_stored != nullptr ? m_stored->Find(GetName(id)) : nullptr;
    const bool changed = text != nullptr && (stored == nullptr || *stored != *text);
    // The mark goes first, so a long name cut at the buffer keeps it.
    std::snprintf(buffer, size, "%s%s", changed ? "* " : "", GetName(id).c_str());
}

float COptionTextRows::GetIdColumnWidth() const
{
    // The texts' names, the longest about 30 letters.
    return 220.0f;
}

void COptionTextRows::CommitName(int id, const char* locale, const char* text)
{
    if (m_texts == nullptr || GetName(id).empty())
    {
        return;
    }
    const auto found = m_texts->texts.find(GetName(id));
    if (found == m_texts->texts.end())
    {
        return;
    }
    // The English text is never removed: a text without one is left out (the reader's error).
    if (std::string_view(locale) == "en" && *text == '\0')
    {
        return;
    }
    found->second.Set(locale, text);
    UpdateNames();
}

int COptionTextRows::GetSelected() const
{
    return m_selected;
}

int COptionTextRows::GetSelectionSerial() const
{
    return m_selectionSerial;
}

void COptionTextRows::Select(int id)
{
    m_selected = id;
    ++m_selectionSerial;
}

void COptionTextRows::ShowBrowser() {}

const char* COptionTextRows::GetSelectTooltip() const
{
    return I18N::Editor::TheNameOptionsNameTheTextBy;
}

const char* COptionTextRows::GetAllRowsLabel() const
{
    return I18N::Editor::AllTexts;
}

#endif // _EDITOR
