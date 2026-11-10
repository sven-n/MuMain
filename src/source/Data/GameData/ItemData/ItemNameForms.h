#pragma once

#include <map>
#include <string>
#include <string_view>

namespace Data::Items
{
// The forms of a name in the languages that decline names ("nameForms" in the item files, docs/item-data.md): by
// language code, then by case key (Data::Translation::ItemCaseKeys). A text that names the item with a case,
// {item:14,25:acc}, shows the form; without one it shows the name itself.
class ItemNameForms
{
public:
    using CaseForms = std::map<std::string, std::string, std::less<>>;
    using LocaleForms = std::map<std::string, CaseForms, std::less<>>;

    // The form, or an empty text when the language has none for the case.
    const std::string& Find(std::string_view locale, std::string_view caseKey) const;

    // Sets the form; an empty text removes it, and a language without forms is removed with its last one.
    void Set(std::string_view locale, std::string_view caseKey, std::string text);

    bool IsEmpty() const
    {
        return m_forms.empty();
    }

    // The languages with forms and their forms, sorted by language code and case key.
    const LocaleForms& GetAll() const
    {
        return m_forms;
    }

    bool operator==(const ItemNameForms& other) const = default;

private:
    LocaleForms m_forms;
};
} // namespace Data::Items
