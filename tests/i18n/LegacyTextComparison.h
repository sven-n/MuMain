#pragma once

#include <string_view>

#include "I18N/All.h"

namespace LegacyTextComparison
{
// Runs `check` once per locale this build ships, with that locale active, then switches back to en.
template <typename Check>
void ForEachLocale(Check&& check)
{
    for (const char* locale : I18N::GetAvailableLocales())
    {
        I18N::SetLocale(locale);
        check(locale);
    }
    I18N::SetLocale("en");
}

// Text equality; a null text counts as empty, as Lookup's "unknown id" answer is an empty string.
inline bool SameText(const wchar_t* actual, const wchar_t* expected)
{
    return std::wstring_view(actual != nullptr ? actual : L"") == std::wstring_view(expected != nullptr ? expected : L"");
}
} // namespace LegacyTextComparison
