#pragma once

#include "Data/GameData/BuffData/BuffValue.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// The texts the skill tooltip and the buff tooltip build a buff's value lines from (Data/Buffs/BuffTexts.json, buffs
// design B7, B9, B26), in the display language, resolved when they load or the language changes, so drawing never
// looks a name up. The file has the format of Data/Items/Tooltips/OptionTexts.json: {value} and {divisor} are its
// placeholders.
//
//   - a buff attribute's name ("greaterDamageBonus"): its value line, "Damage +{value}";
//   - "term" + a term attribute's name with a capital ("termTotalEnergy"): a part's label, "energy {value}";
//   - the fixed texts of BuffLineText.
namespace Data::Buffs
{
enum class BuffLineText : std::uint8_t
{
    // "Duration: {value} seconds"
    Duration,
    // "base: {value}"
    Base,
    // "{value} s": a duration part's seconds
    Seconds,
    // "at most {value}"
    Maximum,
    // "1 per {divisor}": a term that divides
    RuleDivisor,
    // "x{value}": a term that multiplies
    RuleTimes,
    // "The value depends on its caster": the buff tooltip's note when the server sent no value (buffs design B8)
    DependsOnCaster,
    Count,
};

constexpr std::array<std::string_view, static_cast<size_t>(BuffLineText::Count)> BuffLineTextKeys = {
    "duration", "base", "seconds", "maximum", "ruleDivisor", "ruleTimes", "dependsOnCaster",
};

// The key of a term attribute's label: "term" and the name with a capital.
std::string BuffTermTextKey(std::string_view termAttribute);

class BuffLineTextTable
{
public:
    static BuffLineTextTable& GetInstance();

    // Replaces the texts and resolves them in `locale`.
    void SetTexts(Items::ItemOptionTexts texts, std::string_view locale);
    void SetDisplayLocale(std::string_view locale);
    const Items::ItemOptionTexts& GetTexts() const
    {
        return m_texts;
    }
    // Counts every change of the texts or their language, so a reader can keep lines it built from them.
    std::uint32_t Version() const
    {
        return m_version;
    }
    // The language the texts are in.
    const std::string& GetDisplayLocale() const
    {
        return m_locale;
    }

    // The texts in the display language; nullptr where the file has none.
    const std::wstring* ForAttribute(int attributeIndex) const;
    const std::wstring* ForTerm(BuffTermAttribute attribute) const;
    const std::wstring* Get(BuffLineText text) const;

private:
    BuffLineTextTable() = default;

    Items::ItemOptionTexts m_texts;
    std::string m_locale;
    std::uint32_t m_version = 0;
    // By BuffAttributes index, BuffTermAttribute and BuffLineText; empty for none.
    std::vector<std::wstring> m_attributes;
    std::array<std::wstring, BuffTermAttributeCount> m_terms;
    std::array<std::wstring, static_cast<size_t>(BuffLineText::Count)> m_fixed;
};
} // namespace Data::Buffs

#define g_BuffLineTexts Data::Buffs::BuffLineTextTable::GetInstance()
