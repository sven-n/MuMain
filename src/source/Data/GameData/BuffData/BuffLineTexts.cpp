#include "stdafx.h"

#include "BuffLineTexts.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/GameData/BuffData/BuffAttributes.h"

#include <cctype>

namespace Data::Buffs
{
namespace
{
const std::wstring* NonEmpty(const std::wstring& text)
{
    return text.empty() ? nullptr : &text;
}
} // namespace

std::string BuffTermTextKey(std::string_view termAttribute)
{
    std::string key = "term";
    key += termAttribute;
    if (key.size() > 4)
    {
        key[4] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[4])));
    }
    return key;
}

BuffLineTextTable& BuffLineTextTable::GetInstance()
{
    static BuffLineTextTable instance;
    return instance;
}

void BuffLineTextTable::SetTexts(Items::ItemOptionTexts texts, std::string_view locale)
{
    m_texts = std::move(texts);
    SetDisplayLocale(locale);
}

void BuffLineTextTable::SetDisplayLocale(std::string_view locale)
{
    m_locale = std::string(locale);
    const auto resolve = [&](std::string_view key)
    {
        const LocalizedString* text = m_texts.Find(key);
        return text != nullptr ? StringUtils::NarrowToWide(text->Get(locale)) : std::wstring();
    };
    m_attributes.assign(BuffAttributes.size(), std::wstring());
    for (size_t index = 0; index < BuffAttributes.size(); ++index)
    {
        m_attributes[index] = resolve(BuffAttributes[index].name);
    }
    m_terms.fill(std::wstring());
    for (const BuffTermAttributeName& term : BuffTermAttributes)
    {
        m_terms[static_cast<size_t>(term.attribute)] = resolve(BuffTermTextKey(term.name));
    }
    for (size_t index = 0; index < m_fixed.size(); ++index)
    {
        m_fixed[index] = resolve(BuffLineTextKeys[index]);
    }
}

const std::wstring* BuffLineTextTable::ForAttribute(int attributeIndex) const
{
    if (attributeIndex < 0 || static_cast<size_t>(attributeIndex) >= m_attributes.size())
    {
        return nullptr;
    }
    return NonEmpty(m_attributes[attributeIndex]);
}

const std::wstring* BuffLineTextTable::ForTerm(BuffTermAttribute attribute) const
{
    return NonEmpty(m_terms[static_cast<size_t>(attribute)]);
}

const std::wstring* BuffLineTextTable::Get(BuffLineText text) const
{
    return NonEmpty(m_fixed[static_cast<size_t>(text)]);
}
} // namespace Data::Buffs
