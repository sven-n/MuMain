#pragma once

#include <map>
#include <string>
#include <string_view>

namespace Data
{
// A text with translations, stored in OpenMU's LocalizedString format:
// "English text||pt=Texto em português||es=Texto en español".
// The first part is the neutral (English) text; the other parts are
// "<locale>=<text>" pairs using the same locale codes as the UI translations
// (en, de, pt, es, ...). All texts are UTF-8.
class LocalizedString
{
public:
    static constexpr std::string_view Separator = "||";
    static constexpr std::string_view NeutralLocale = "en";

    static LocalizedString Parse(std::string_view serialized);

    // Neutral text first, then the translations sorted by locale, so the
    // result is the same for the same content.
    std::string Serialize() const;

    const std::string& GetNeutral() const
    {
        return m_neutral;
    }

    // The text for the locale, or the neutral text if it has no translation.
    const std::string& Get(std::string_view locale) const;

    // The text of the locale itself: the neutral text for the neutral locale,
    // else its translation, also one written like the neutral text. Empty
    // when it has none (Get then gives the neutral text).
    const std::string& GetOwn(std::string_view locale) const;
    // Whether the locale has a text of its own (GetOwn).
    bool HasOwn(std::string_view locale) const
    {
        // A translation is never stored empty (Set removes it).
        return !GetOwn(locale).empty();
    }

    // Setting the neutral locale sets the neutral text. An empty text removes
    // the translation.
    void Set(std::string_view locale, std::string text);

    bool IsEmpty() const
    {
        return m_neutral.empty() && m_translations.empty();
    }

    const std::map<std::string, std::string, std::less<>>& GetTranslations() const
    {
        return m_translations;
    }

    bool operator==(const LocalizedString& other) const = default;

    // A text holding the separator would split into two parts when parsed.
    static bool ContainsSeparator(std::string_view text);

    // Locale codes like "pt" or "zh-TW". Anything else (empty, "=", "||")
    // would break the format.
    static bool IsValidLocaleCode(std::string_view locale);

private:
    std::string m_neutral;
    std::map<std::string, std::string, std::less<>> m_translations;
};
} // namespace Data
