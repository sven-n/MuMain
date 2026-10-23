#include "stdafx.h"

#include "LocalizedString.h"

#include <algorithm>
#include <cctype>

namespace Data
{
namespace
{
constexpr char LocaleSeparator = '=';

void AddPart(LocalizedString& result, std::string_view part)
{
    const size_t separator = part.find(LocaleSeparator);
    if (separator == std::string_view::npos)
    {
        return;
    }

    result.Set(part.substr(0, separator), std::string(part.substr(separator + 1)));
}
} // namespace

LocalizedString LocalizedString::Parse(std::string_view serialized)
{
    LocalizedString result;
    size_t partEnd = serialized.find(Separator);
    result.m_neutral = std::string(serialized.substr(0, partEnd));

    while (partEnd != std::string_view::npos)
    {
        const size_t partStart = partEnd + Separator.size();
        partEnd = serialized.find(Separator, partStart);
        AddPart(result, serialized.substr(partStart, partEnd - partStart));
    }

    return result;
}

std::string LocalizedString::Serialize() const
{
    std::string result = m_neutral;
    for (const auto& [locale, text] : m_translations)
    {
        result += Separator;
        result += locale;
        result += LocaleSeparator;
        result += text;
    }
    return result;
}

const std::string& LocalizedString::Get(std::string_view locale) const
{
    const auto translation = m_translations.find(locale);
    return translation != m_translations.end() ? translation->second : m_neutral;
}

const std::string& LocalizedString::GetOwn(std::string_view locale) const
{
    static const std::string none;
    if (locale == NeutralLocale)
    {
        return m_neutral;
    }
    const auto translation = m_translations.find(locale);
    return translation != m_translations.end() ? translation->second : none;
}

void LocalizedString::Set(std::string_view locale, std::string text)
{
    if (locale == NeutralLocale)
    {
        m_neutral = std::move(text);
        return;
    }

    if (text.empty())
    {
        const auto translation = m_translations.find(locale);
        if (translation != m_translations.end())
        {
            m_translations.erase(translation);
        }
        return;
    }

    m_translations.insert_or_assign(std::string(locale), std::move(text));
}

bool LocalizedString::ContainsSeparator(std::string_view text)
{
    return text.find(Separator) != std::string_view::npos;
}

bool LocalizedString::IsValidLocaleCode(std::string_view locale)
{
    return !locale.empty() &&
           std::all_of(locale.begin(), locale.end(), [](char character)
                       { return std::isalnum(static_cast<unsigned char>(character)) || character == '-'; });
}
} // namespace Data
