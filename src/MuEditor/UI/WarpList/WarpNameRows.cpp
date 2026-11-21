#include "stdafx.h"

#ifdef _EDITOR

#include "WarpNameRows.h"

#include "MuWarpListUI.h"

#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Core/Text/AsciiText.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/MapData/Warp.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Warps
{
namespace
{
// The warp's index, /move name or a name of any language has the text.
bool MatchesSearch(const Data::Maps::Warp& warp, const std::string& needle)
{
    if (needle.empty() || std::to_string(warp.index).find(needle) != std::string::npos ||
        Core::Text::ToLowerAscii(warp.command).find(needle) != std::string::npos)
    {
        return true;
    }
    for (const char* locale : I18N::GetAvailableLocales())
    {
        if (Core::Text::ToLowerAscii(warp.names.GetOwn(locale)).find(needle) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}
} // namespace

const std::vector<int>& WarpNameRows::List(std::string_view search, std::string_view missingLocale)
{
    // A name filled in here keeps its row until Refresh (m_builtVersion follows the own edit).
    if (m_built && m_builtVersion == GetVersion() && m_search == search && m_missingLocale == missingLocale)
    {
        return m_rows;
    }
    m_search.assign(search);
    m_missingLocale.assign(missingLocale);
    m_rows.clear();
    const std::string needle = Core::Text::ToLowerAscii(search);
    for (const Data::Maps::Warp& warp : g_MuWarpListUI.GetEdits().GetList().warps)
    {
        if (MatchesSearch(warp, needle) && (missingLocale.empty() || !warp.names.HasOwn(missingLocale)))
        {
            m_rows.push_back(warp.index);
        }
    }
    m_builtVersion = GetVersion();
    m_built = true;
    return m_rows;
}

void WarpNameRows::Refresh()
{
    m_built = false;
}

int WarpNameRows::GetVersion() const
{
    return g_MuWarpListUI.GetVersion();
}

std::vector<int> WarpNameRows::CountMissing(std::span<const char* const> locales) const
{
    std::vector<int> missing(locales.size(), 0);
    for (const Data::Maps::Warp& warp : g_MuWarpListUI.GetEdits().GetList().warps)
    {
        for (size_t i = 0; i < locales.size(); ++i)
        {
            missing[i] += warp.names.HasOwn(locales[i]) ? 0 : 1;
        }
    }
    return missing;
}

const Data::LocalizedString* WarpNameRows::FindNames(int id) const
{
    const std::vector<Data::Maps::Warp>& warps = g_MuWarpListUI.GetEdits().GetList().warps;
    const auto warp =
        std::find_if(warps.begin(), warps.end(), [id](const Data::Maps::Warp& entry) { return entry.index == id; });
    return warp != warps.end() ? &warp->names : nullptr;
}

void WarpNameRows::FormatId(int id, char* buffer, std::size_t size) const
{
    std::snprintf(buffer, size, "%d", id);
}

void WarpNameRows::CommitName(int id, const char* locale, const char* text)
{
    std::string name(text);
    Core::Text::CutToUtf16Length(name, Data::Maps::MaxWarpNameLength);
    if (locale == Data::LocalizedString::NeutralLocale && name.empty())
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheEnglishNameCannotBeEmpty);
        return;
    }
    if (g_MuWarpListUI.SetWarpName(id, locale, std::move(name)))
    {
        // The row stays listed although its name is no longer missing.
        m_builtVersion = GetVersion();
    }
}

int WarpNameRows::GetSelected() const
{
    return g_MuWarpListUI.GetSelectedIndex();
}

int WarpNameRows::GetSelectionSerial() const
{
    return g_MuWarpListUI.GetSelectionSerial();
}

void WarpNameRows::Select(int id)
{
    g_MuWarpListUI.SelectWarpIndex(id);
}

void WarpNameRows::ShowBrowser()
{
    g_MuWarpListUI.ShowWarpsTab();
}

const char* WarpNameRows::GetSelectTooltip() const
{
    return I18N::Editor::SelectTheWarpAndShowItInTheWarpsTab;
}

const char* WarpNameRows::GetAllRowsLabel() const
{
    return I18N::Editor::AllWarps;
}
} // namespace MuEditor::Warps

#endif // _EDITOR
