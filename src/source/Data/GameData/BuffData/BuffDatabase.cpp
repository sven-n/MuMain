#include "stdafx.h"

#include "BuffDatabase.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/GameData/ItemData/ItemOptionEffects.h"

#include <algorithm>
#include <map>

namespace Data::Buffs
{
namespace
{
constexpr wchar_t LineSeparator = L'\n';

std::vector<std::wstring> SplitLines(const std::wstring& text)
{
    std::vector<std::wstring> lines;
    size_t start = 0;
    while (start < text.size())
    {
        const size_t end = text.find(LineSeparator, start);
        const size_t stop = end == std::wstring::npos ? text.size() : end;
        lines.push_back(text.substr(start, stop - start));
        start = stop + 1;
    }
    return lines;
}
} // namespace

BuffDatabase& BuffDatabase::GetInstance()
{
    static BuffDatabase instance;
    return instance;
}

BuffDatabase::BuffDatabase()
{
    m_indices.fill(NoBuff);
}

void BuffDatabase::Build(std::span<const BuffDefinition> buffs, std::string_view locale)
{
    m_buffs.assign(buffs.begin(), buffs.end());
    for (BuffDefinition& buff : m_buffs)
    {
        // The values computed every frame read the attributes' indices, never their names.
        if (buff.duration)
        {
            Items::ResolveTermAttributes(*buff.duration);
        }
        for (BuffPowerUp& powerUp : buff.powerUps)
        {
            powerUp.attributeIndex = FindBuffAttribute(powerUp.attribute);
            Items::ResolveTermAttributes(powerUp.value);
        }
    }
    m_indices.fill(NoBuff);
    m_itemBuffs.clear();
    for (size_t index = 0; index < m_buffs.size(); ++index)
    {
        const BuffDefinition& buff = m_buffs[index];
        if (buff.number >= 0 && buff.number <= MaxBuffNumber)
        {
            m_indices[buff.number] = static_cast<int>(index);
        }
        if (buff.item)
        {
            const auto [entry, added] =
                m_itemBuffs.emplace(ItemKey(buff.item->group, buff.item->number), static_cast<int>(index));
            if (!added && buff.number < m_buffs[entry->second].number)
            {
                entry->second = static_cast<int>(index);
            }
        }
    }
    std::map<std::string_view, int> lowestNumbers;
    for (const BuffDefinition& buff : m_buffs)
    {
        if (!buff.family.empty() && Find(buff.number) != nullptr)
        {
            const auto entry = lowestNumbers.emplace(buff.family, buff.number).first;
            entry->second = std::min(entry->second, buff.number);
        }
    }
    m_families.assign(m_buffs.size(), NoBuff);
    for (size_t index = 0; index < m_buffs.size(); ++index)
    {
        const auto lowest = lowestNumbers.find(m_buffs[index].family);
        if (lowest != lowestNumbers.end())
        {
            m_families[index] = lowest->second;
        }
    }
    SetDisplayLocale(locale);
}

void BuffDatabase::SetDisplayLocale(std::string_view locale)
{
    m_locale = locale;
    m_texts.clear();
    m_texts.reserve(m_buffs.size());
    for (const BuffDefinition& buff : m_buffs)
    {
        BuffDisplayTexts texts;
        texts.name = StringUtils::NarrowToWide(buff.displayName.Get(locale));
        if (!buff.description.IsEmpty())
        {
            texts.descriptionLines = SplitLines(StringUtils::NarrowToWide(buff.description.Get(locale)));
        }
        m_texts.push_back(std::move(texts));
    }
}

const BuffDefinition* BuffDatabase::FindByItem(int group, int number) const
{
    const auto found = m_itemBuffs.find(ItemKey(group, number));
    return found != m_itemBuffs.end() ? &m_buffs[found->second] : nullptr;
}

const BuffDefinition* BuffDatabase::Find(int number) const
{
    if (number < 0 || number > MaxBuffNumber || m_indices[number] == NoBuff)
    {
        return nullptr;
    }
    return &m_buffs[m_indices[number]];
}

const BuffDisplayTexts* BuffDatabase::FindTexts(int number) const
{
    if (number < 0 || number > MaxBuffNumber || m_indices[number] == NoBuff)
    {
        return nullptr;
    }
    return &m_texts[m_indices[number]];
}

void BuffDatabase::PickShownOnStrip(std::span<const int> active, std::span<bool> shown) const
{
    // The buff's index in m_buffs if the strip may show it at all, else NoBuff.
    const auto showable = [this](int number)
    {
        const int index = number >= 0 && number <= MaxBuffNumber ? m_indices[number] : NoBuff;
        return index != NoBuff && !m_buffs[index].hidden ? index : NoBuff;
    };
    const size_t count = std::min(active.size(), shown.size());
    // Per family (by its lowest buff number), the position in `active` of its highest tier.
    std::array<int, MaxBuffNumber + 1> familyBest;
    familyBest.fill(NoBuff);
    for (size_t position = 0; position < count; ++position)
    {
        const int index = showable(active[position]);
        const int family = index == NoBuff ? NoBuff : m_families[index];
        if (family == NoBuff)
        {
            continue;
        }
        int& best = familyBest[family];
        if (best == NoBuff || m_buffs[m_indices[active[best]]].tier < m_buffs[index].tier)
        {
            best = static_cast<int>(position);
        }
    }
    for (size_t position = 0; position < count; ++position)
    {
        const int index = showable(active[position]);
        const int family = index == NoBuff ? NoBuff : m_families[index];
        shown[position] = index != NoBuff && (family == NoBuff || familyBest[family] == static_cast<int>(position));
    }
}
} // namespace Data::Buffs
