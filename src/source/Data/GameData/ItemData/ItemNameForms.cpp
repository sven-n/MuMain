#include "stdafx.h"

#include "Data/GameData/ItemData/ItemNameForms.h"

namespace Data::Items
{
const std::string& ItemNameForms::Find(std::string_view locale, std::string_view caseKey) const
{
    static const std::string None;
    const auto language = m_forms.find(locale);
    if (language == m_forms.end())
    {
        return None;
    }
    const auto form = language->second.find(caseKey);
    return form != language->second.end() ? form->second : None;
}

void ItemNameForms::Set(std::string_view locale, std::string_view caseKey, std::string text)
{
    if (!text.empty())
    {
        m_forms[std::string(locale)][std::string(caseKey)] = std::move(text);
        return;
    }
    const auto language = m_forms.find(locale);
    if (language == m_forms.end())
    {
        return;
    }
    if (const auto form = language->second.find(caseKey); form != language->second.end())
    {
        language->second.erase(form);
    }
    if (language->second.empty())
    {
        m_forms.erase(language);
    }
}
} // namespace Data::Items
