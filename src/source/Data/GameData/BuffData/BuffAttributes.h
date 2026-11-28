#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

// The attributes a buff's power-up can raise (BuffPowerUp::attribute): OpenMU's Stats names in camelCase where OpenMU
// has the attribute, and names of Webzen's item add options (ADD_OPTION_*) where it has none. A name the list lacks is
// an error when Buffs.json loads (ValidateBuffs). See docs/buff-data.md, "Values".
namespace Data::Buffs
{
// How an attribute's value reads (buffs design B17): a flat number ("defenseBase" 60), or a fraction OpenMU keeps as
// one, which a text shows in percent (0.30 is 30 %).
enum class BuffAttributeUnit : std::uint8_t
{
    Flat,
    Fraction,
};

struct BuffAttribute
{
    std::string_view name;
    BuffAttributeUnit unit;
    // OpenMU's id of the attribute (AttributeDefinition.Id in Stats.cs), with which the server names a value it sends
    // (buffs design B22); empty for attributes OpenMU doesn't have. A stand-in carries the id of the OpenMU attribute
    // it stands for.
    std::string_view openMuId;
    // What OpenMU's value is multiplied by to read in this attribute's unit: 100 where OpenMU keeps a fraction and the
    // client a percent (criticalDamageChance, as the Scroll of Battle has it).
    double openMuScale = 1.0;
};

// OpenMU's attribute id as the extended C1 07 sends it: the 16 bytes in the order of the id's text form.
using OpenMuAttributeId = std::array<std::uint8_t, 16>;

inline constexpr auto BuffAttributes = std::to_array<BuffAttribute>({
    // OpenMU's Stats the item buffs raise. Their values are Webzen's, flat (a rate in percent).
    {"attackSpeedAny", BuffAttributeUnit::Flat, "DA08473F-DF5B-444D-8651-9EDB65797922"},
    {"baseDamageBonus", BuffAttributeUnit::Flat, "BB6F0151-EAB2-4A9D-BFE3-51E145F36C52"},
    {"defenseBase", BuffAttributeUnit::Flat, "EB098C46-60D4-4CA6-BBD4-5B6270A1407B"},
    {"maximumHealth", BuffAttributeUnit::Flat, "A6C39A5C-295F-415E-A314-5E9F9A748D27"},
    {"maximumMana", BuffAttributeUnit::Flat, "17CB8826-0677-4C93-A0C9-C0E3D2DA7D73"},
    {"maximumAbility", BuffAttributeUnit::Flat, "466BBBBA-C1D8-45DC-8832-2EAA1130ACFD"},
    {"maximumShield", BuffAttributeUnit::Flat, "BC745471-6EC6-48BC-8C7A-C8AACA3A92D9"},
    {"experienceRate", BuffAttributeUnit::Flat, "1AD454D4-BEF9-416E-BC49-82A5B0277FC7"},
    {"bonusItemDropRate", BuffAttributeUnit::Flat, "EEDAFF4A-E9AA-4D28-B07C-A9120434DF97"},
    {"vulcanusExperienceRate", BuffAttributeUnit::Flat, "199015AC-3139-4E6B-A7C0-5439DD4F7052"},
    {"totalStrength", BuffAttributeUnit::Flat, "F59709A3-44B0-4147-AAEB-E90CEC251641"},
    {"totalAgility", BuffAttributeUnit::Flat, "364F1207-00F8-485B-9F2C-74E04CB78C73"},
    {"totalVitality", BuffAttributeUnit::Flat, "6A0076E4-69DC-42E7-A92B-C8711392EF82"},
    {"totalEnergy", BuffAttributeUnit::Flat, "12956B45-007C-453A-AE1F-36475B8CEBBF"},
    {"totalLeadership", BuffAttributeUnit::Flat, "35E04272-63F3-4EBB-8FB5-EF2128DDB9F6"},
    {"physicalBaseDmg", BuffAttributeUnit::Flat, "DD1E13E4-BFFD-45B5-9B91-9080710324B2"},
    {"wizardryBaseDmg", BuffAttributeUnit::Flat, "7F4F3646-33A6-40AC-8DA6-29A0A0F46016"},
    {"healthRecoveryMultiplier", BuffAttributeUnit::Flat, "0A427A13-3708-4125-BA83-A2DF7C0753B8"},
    {"manaRecoveryMultiplier", BuffAttributeUnit::Flat, "E4EC7913-5004-48FC-ACB1-E1764237A251"},
    {"criticalDamageChance", BuffAttributeUnit::Flat, "ADE8092E-870F-4968-B707-8BFD6CBF8FFC", 100.0},
    {"excellentDamageChance", BuffAttributeUnit::Flat, "577AE8D4-D9E5-4A1F-9A69-45D1A2519EEE"},
    {"deathPenaltyProtection", BuffAttributeUnit::Flat, "3CA30A73-E8C8-4201-9703-FB46E31E7FBD"},
    {"itemDurabilityProtection", BuffAttributeUnit::Flat, "7232B55F-9680-41F7-98AC-3172E66FCABE"},
    // Webzen's item add options which OpenMU has no attribute for.
    {"disableExperience", BuffAttributeUnit::Flat, {}},
    {"mobility", BuffAttributeUnit::Flat, {}},
    {"vulcanusItemDropRate", BuffAttributeUnit::Flat, {}},
    {"partyExperienceBonus", BuffAttributeUnit::Flat, {}},
    {"vitalityBonus", BuffAttributeUnit::Flat, {}},
    // OpenMU's Stats the skill buffs raise (PR 2b), in OpenMU's units; maximumWizBaseDmgIncrease and
    // defenseRatePvmIncrease stand for OpenMU's x1.0 placeholders a master skill raises. criticalDamageChance (above)
    // stays
    // in percent, as the Scroll of Battle has it, so its master skill parts add their percent as it is.
    {"greaterDamageBonus", BuffAttributeUnit::Flat, "5CFE3ED7-AF45-4790-BDD9-0DC55B981296"},
    {"greaterDefenseBonus", BuffAttributeUnit::Flat, "5A7C3E9B-D1F4-4B82-8A6D-2F3E1C9B7A4D"},
    {"soulBarrierReceiveDecrement", BuffAttributeUnit::Fraction, "CBC5404A-6232-4BF3-9B85-9AA0AE0F9BA4"},
    {"soulBarrierManaTollPerHit", BuffAttributeUnit::Flat, "25315C15-C884-4B45-9883-9A92693DC455"},
    {"criticalDamageBonus", BuffAttributeUnit::Flat, "33F53519-16F3-44C2-9D36-432C36329C78"},
    {"ammunitionConsumptionRate", BuffAttributeUnit::Fraction, "4CD0B1AE-3FE0-499B-B421-4E4078182090"},
    {"attackDamageIncrease", BuffAttributeUnit::Fraction, "0765CCD2-C70A-4338-BF49-0D652364C223"},
    {"swellLifeHealthIncrease", BuffAttributeUnit::Fraction, "9C4E7B2A-F1D6-4A3E-B8C5-1D7F2E9A3B4C"},
    {"swellLifeManaIncrease", BuffAttributeUnit::Fraction, "8B4F1C6D-9A2E-4F7B-A3D5-1E9C7F2A4B6D"},
    {"damageReflection", BuffAttributeUnit::Fraction, "1535A2FB-6094-48B8-8578-086BA166A0F7"},
    {"berserkerManaMultiplier", BuffAttributeUnit::Fraction, "9B6AA0DA-9ABA-4C14-B094-F1AC70BCB52D"},
    {"berserkerHealthDecrement", BuffAttributeUnit::Fraction, "D1948382-23F6-43A4-AD84-69227BF2ABA3"},
    {"berserkerMinPhysDmgBonus", BuffAttributeUnit::Flat, "81B5B942-A6D4-4343-BD4D-106CFFF34F4E"},
    {"berserkerMaxPhysDmgBonus", BuffAttributeUnit::Flat, "602D5FC4-AB85-423A-9C91-D23B0A28996C"},
    {"berserkerWizardryMultiplier", BuffAttributeUnit::Fraction, "8DA7B8C2-F28B-4472-AE8F-28F08D4E7E93"},
    {"berserkerCurseMultiplier", BuffAttributeUnit::Fraction, "AD1A3A3F-B461-4CAE-B3A3-D03DD0714A96"},
    {"berserkerProficiencyMultiplier", BuffAttributeUnit::Fraction, "55124C97-6EF3-4C46-AEC9-D01C11BE18A4"},
    {"minimumWizBaseDmg", BuffAttributeUnit::Flat, "65583A02-AB94-4A17-9B79-86ECC82DC835"},
    {"maximumWizBaseDmgIncrease", BuffAttributeUnit::Fraction, "44B8236A-BF5B-4082-BA8B-5DEDA1458D33"},
    {"defenseIgnoreChance", BuffAttributeUnit::Fraction, "56BDC72E-9D37-4EDA-8AC8-9E12473966FC"},
    {"increaseBlockBonus", BuffAttributeUnit::Flat, "8F2D1A7C-4E6B-4A9D-B1F3-5C7E8A2D9F0B"},
    {"defenseRatePvmIncrease", BuffAttributeUnit::Fraction, "C520DD2D-1B06-4392-95EE-3C41F33E68DA"},
    {"defenseFinal", BuffAttributeUnit::Flat, "0888AD48-0CC8-47CA-B6A3-99F3771AA5FC"},
});

// The index in BuffAttributes of a name the list lacks.
inline constexpr int NoBuffAttribute = -1;

// The name's index in BuffAttributes, or NoBuffAttribute.
constexpr int FindBuffAttribute(std::string_view name)
{
    const auto found = std::find_if(BuffAttributes.begin(), BuffAttributes.end(),
                                    [name](const BuffAttribute& attribute) { return attribute.name == name; });
    return found != BuffAttributes.end() ? static_cast<int>(found - BuffAttributes.begin()) : NoBuffAttribute;
}

namespace Detail
{
constexpr int HexDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}
} // namespace Detail

// Whether text is an id in the form 33F53519-16F3-44C2-9D36-432C36329C78 (hex digits, dashes after 8, 12, 16 and 20).
constexpr bool IsOpenMuIdText(std::string_view text)
{
    if (text.size() != 36)
    {
        return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        const bool dash = i == 8 || i == 13 || i == 18 || i == 23;
        if (dash ? text[i] != '-' : Detail::HexDigit(text[i]) < 0)
        {
            return false;
        }
    }
    return true;
}

// The id's 16 bytes; all 0 when the text isn't an id.
constexpr OpenMuAttributeId ParseOpenMuId(std::string_view text)
{
    OpenMuAttributeId id{};
    if (!IsOpenMuIdText(text))
    {
        return id;
    }
    std::size_t byte = 0;
    for (std::size_t i = 0; i + 1 < text.size() && byte < id.size(); ++i)
    {
        if (text[i] == '-')
        {
            continue;
        }
        id[byte++] = static_cast<std::uint8_t>(Detail::HexDigit(text[i]) * 16 + Detail::HexDigit(text[i + 1]));
        ++i;
    }
    return id;
}

// The id in its text form, as Stats.cs writes it: 33F53519-16F3-44C2-9D36-432C36329C78.
inline std::wstring FormatOpenMuId(const OpenMuAttributeId& id)
{
    // A dash after the 4th, 6th, 8th and 10th byte.
    constexpr std::array<std::size_t, 4> DashAfter = {3, 5, 7, 9};
    constexpr wchar_t Digits[] = L"0123456789ABCDEF";
    constexpr int BitsPerDigit = 4;
    constexpr std::uint8_t LowDigit = 0x0F;
    std::wstring text;
    for (std::size_t index = 0; index < id.size(); ++index)
    {
        text += Digits[id[index] >> BitsPerDigit];
        text += Digits[id[index] & LowDigit];
        if (std::find(DashAfter.begin(), DashAfter.end(), index) != DashAfter.end())
        {
            text += L'-';
        }
    }
    return text;
}

// The index of the attribute OpenMU names with that id, or NoBuffAttribute.
constexpr int FindBuffAttributeByOpenMuId(const OpenMuAttributeId& id)
{
    for (std::size_t index = 0; index < BuffAttributes.size(); ++index)
    {
        if (!BuffAttributes[index].openMuId.empty() && ParseOpenMuId(BuffAttributes[index].openMuId) == id)
        {
            return static_cast<int>(index);
        }
    }
    return NoBuffAttribute;
}

constexpr bool IsBuffAttribute(std::string_view name)
{
    return FindBuffAttribute(name) != NoBuffAttribute;
}

// The unit of the attribute at that index; Flat for NoBuffAttribute.
constexpr BuffAttributeUnit BuffAttributeUnitAt(int index)
{
    return index >= 0 && index < static_cast<int>(BuffAttributes.size()) ? BuffAttributes[index].unit
                                                                         : BuffAttributeUnit::Flat;
}

// A name of the list, checked when the code compiles: the game code that reads an attribute names it this way, so a
// name the list lacks doesn't build. It holds the name's index, which the game compares with the index the buff table
// resolves for each power-up when it loads (BuffPowerUp::attributeIndex), never a name.
class BuffAttributeName
{
public:
    consteval BuffAttributeName(const char* name) : m_index(FindBuffAttribute(name))
    {
        if (m_index == NoBuffAttribute)
        {
            throw "not a name of BuffAttributes";
        }
    }

    constexpr int Index() const
    {
        return m_index;
    }

private:
    int m_index;
};
} // namespace Data::Buffs
