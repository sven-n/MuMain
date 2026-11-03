/*+++++++++++++++++++++++++++++++++++++
    INCLUDE.
+++++++++++++++++++++++++++++++++++++*/
#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Scenes/SceneCore.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Character/CharacterManager.h"
#include "UI/Social/SocialWindowBase.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <memory>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>


namespace
{
constexpr std::uint8_t EMPTY_OPTION = 0xFF;
constexpr int SET_OPTION_COUNT = AT_SET_OPTION_WATER_MASTERY - AT_SET_OPTION_IMPROVE_STRENGTH + 1;
// The first set option after AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE: 24 to 28 are not supported.
constexpr int FIRST_UNSUPPORTED_SET_OPTION =
    AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE - AT_SET_OPTION_IMPROVE_STRENGTH + 1;

// Whether the client applies set option `option` (counted from AT_SET_OPTION_IMPROVE_STRENGTH): 0 to
// 23 and the mastery options; 24 to 28 (class bonuses that grew with a stat) are not supported.
constexpr bool IsSupportedSetOption(int option)
{
    return (option >= 0 && option < FIRST_UNSUPPORTED_SET_OPTION) ||
           (option >= MASTERY_OPTION && option < SET_OPTION_COUNT);
}

// The format each set option's value is described with, in AT_SET_OPTION order from
// AT_SET_OPTION_IMPROVE_STRENGTH, so a set option's number is its index.
constexpr const wchar_t* const* kSetOptionFormats[] = {
    // AT_SET_OPTION_IMPROVE_STRENGTH to AT_SET_OPTION_IMPROVE_MAGIC_POWER
    &I18N::Game::IncreaseStrengthD,
    &I18N::Game::IncreaseAgilityD,
    &I18N::Game::IncreaseEnergyD,
    &I18N::Game::IncreaseStaminaD,
    &I18N::Game::IncreaseCommandD,
    &I18N::Game::IncreaseMinDamageD,
    &I18N::Game::IncreaseMaxDamageD,
    &I18N::Game::IncreaseWizardryDmgD,
    // AT_SET_OPTION_IMPROVE_DAMAGE to AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE
    &I18N::Game::IncreaseDamageD957,
    &I18N::Game::IncreaseDamageSuccessRateD,
    &I18N::Game::IncreaseDefensiveSkillD,
    &I18N::Game::IncreaseMaxLifeD,
    &I18N::Game::IncreaseMaxManaD,
    &I18N::Game::IncreaseMaxAGD,
    &I18N::Game::IncreaseAGIncreaseRateD,
    &I18N::Game::IncreaseCriticalDamageRateD,
    &I18N::Game::IncreaseCriticalDamageD,
    &I18N::Game::IncreaseExcellentDamageRateD,
    &I18N::Game::IncreaseExcellentDamageD,
    &I18N::Game::IncreaseSkillAttackingRateD,
    &I18N::Game::DoubleDamageRateD,
    &I18N::Game::IgnoreEnemiesDefensiveSkillD,
    &I18N::Game::IncreaseDefensiveSkillWhenUsingShieldWeaponsD,
    &I18N::Game::IncreaseDamageWhenUsingTwoHandedWeaponsD,
    // 24 to 28 are not supported (see AT_SET_OPTION_ICE_MASTERY)
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    // AT_SET_OPTION_ICE_MASTERY to AT_SET_OPTION_WATER_MASTERY
    &I18N::Game::IceAttributeSkillIncreaseDamageD,
    &I18N::Game::PoisonAttributeSkillIncreaseDamageD,
    &I18N::Game::LightningAttributeSkillIncreaseDamageD,
    &I18N::Game::FireAttributeSkillIncreaseDamageD,
    &I18N::Game::EarthAttributeSkillIncreaseDamageD,
    &I18N::Game::WindAttributeSkillIncreaseDamageD,
    &I18N::Game::WaterAttributeSkillIncreaseDamageD,
};
static_assert(std::ssize(kSetOptionFormats) == SET_OPTION_COUNT, "one slot per set option");

constexpr bool DescribesExactlyTheSupportedSetOptions()
{
    for (int option = 0; option < SET_OPTION_COUNT; ++option)
    {
        if ((kSetOptionFormats[option] != nullptr) != IsSupportedSetOption(option))
        {
            return false;
        }
    }
    return true;
}
static_assert(DescribesExactlyTheSupportedSetOptions(), "a description for each supported set option, none for others");
static_assert(MASTERY_OPTION == AT_SET_OPTION_ICE_MASTERY - AT_SET_OPTION_IMPROVE_STRENGTH,
              "the mastery options start at MASTERY_OPTION");

// Whether a wearer of base class `firstClass` gets the set's class-bound options (the mastery
// options), like in the original client: the set's byRequireClass names the class, 1 for the class
// as it is, 2 only once it has its second class (`secondClass`).
bool FulfillsClassRequirement(const ITEM_SET_OPTION& setOptions, int firstClass, int secondClass)
{
    const int requirement = setOptions.byRequireClass[firstClass];
    return requirement != 0 && secondClass >= requirement - 1;
}

using FilePtr = std::unique_ptr<FILE, decltype(&std::fclose)>;

FilePtr OpenBinaryFile(const wchar_t* filename)
{
    return FilePtr(_wfopen(filename, L"rb"), &std::fclose);
}

void ReportFileIssue(const wchar_t* filename, const wchar_t* issue)
{
    wchar_t text[256]{};
    std::swprintf(text, std::size(text), L"%ls - %ls.", filename, issue);
    g_ErrorReport.Write(text);
    MessageBox(g_hWnd, text, nullptr, MB_OK);
    SendMessage(g_hWnd, WM_DESTROY, 0, 0);
}
}

static  CSItemOption csItemOption; // do not delete, required for singleton initialization.

struct ITEM_SET_OPTION_FILE
{
    char	strSetName[MAX_ITEM_SET_NAME];
    std::array<std::array<std::uint8_t, MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT>, MAX_ITEM_SET_STANDARD_OPTION_COUNT> byStandardOption{};
    std::array<std::array<std::uint8_t, MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT>, MAX_ITEM_SET_STANDARD_OPTION_COUNT> byStandardOptionValue{};
    std::array<std::uint8_t, MAX_ITEM_SET_EXT_OPTION_COUNT> byExtOption{};
    std::array<std::uint8_t, MAX_ITEM_SET_EXT_OPTION_COUNT> byExtOptionValue{};
    std::uint8_t byOptionCount;
    std::array<std::uint8_t, MAX_ITEM_SET_FULL_OPTION_COUNT> byFullOption{};
    std::array<std::uint8_t, MAX_ITEM_SET_FULL_OPTION_COUNT> byFullOptionValue{};
    std::array<std::uint8_t, MAX_CLASS> byRequireClass{};
};


bool CSItemOption::OpenItemSetScript()
{
    std::wstring strFileName = L"";
    const std::wstring strTest = L"";

    strFileName = L"Data\\Local\\ItemSetType" + strTest + L".bmd";
    if (!OpenItemSetType(strFileName.c_str()))		return false;

    strFileName = L"Data\\Local\\" + g_strSelectedML + L"\\ItemSetOption" + strTest + L"_" + g_strSelectedML + L".bmd";
    if (!OpenItemSetOption(strFileName.c_str()))	 	return false;
    return true;
}

bool CSItemOption::OpenItemSetType(const wchar_t* filename)
{
    auto file = OpenBinaryFile(filename);
    if (!file)
    {
        ReportFileIssue(filename, L"File does not exist.");
        return false;
    }

    const std::size_t entrySize = sizeof(ITEM_SET_TYPE);
    std::vector<std::uint8_t> buffer(entrySize * MAX_ITEM);
    if (std::fread(buffer.data(), buffer.size(), 1, file.get()) != 1)
    {
        ReportFileIssue(filename, L"Failed to read content.");
        return false;
    }

    std::uint32_t checksum{};
    if (std::fread(&checksum, sizeof(checksum), 1, file.get()) != 1)
    {
        ReportFileIssue(filename, L"Failed to read checksum.");
        return false;
    }

    if (checksum != GenerateCheckSum2(buffer.data(), static_cast<int>(buffer.size()), 0xE5F1))
    {
        ReportFileIssue(filename, L"File corrupted.");
        return false;
    }

    auto* pSeek = buffer.data();
    for (int i = 0; i < MAX_ITEM; ++i)
    {
        BuxConvert(pSeek, static_cast<int>(entrySize));
        std::memcpy(&m_ItemSetType[i], pSeek, entrySize);
        pSeek += entrySize;
    }

    return true;
}

bool CSItemOption::OpenItemSetOption(const wchar_t* filename)
{
    auto file = OpenBinaryFile(filename);
    if (!file)
    {
        ReportFileIssue(filename, L"File does not exist.");
        return false;
    }

    const std::size_t entrySize = sizeof(ITEM_SET_OPTION_FILE);
    std::vector<std::uint8_t> buffer(entrySize * MAX_SET_OPTION);
    if (std::fread(buffer.data(), buffer.size(), 1, file.get()) != 1)
    {
        ReportFileIssue(filename, L"Failed to read content.");
        return false;
    }

    std::uint32_t checksum{};
    if (std::fread(&checksum, sizeof(checksum), 1, file.get()) != 1)
    {
        ReportFileIssue(filename, L"Failed to read checksum.");
        return false;
    }

    if (checksum != GenerateCheckSum2(buffer.data(), static_cast<int>(buffer.size()), 0xA2F1))
    {
        ReportFileIssue(filename, L"File corrupted.");
        return false;
    }

    auto* pSeek = buffer.data();
    for (int i = 0; i < MAX_SET_OPTION; ++i)
    {
        BuxConvert(pSeek, static_cast<int>(entrySize));

        ITEM_SET_OPTION_FILE current{};
        std::memcpy(&current, pSeek, entrySize);
        auto* target = &m_ItemSetOption[i];

        CMultiLanguage::ConvertFromUtf8(target->strSetName, current.strSetName);
        target->byOptionCount = current.byOptionCount;
        target->bySetItemCount = 0; // Is calculated below
        target->byStandardOption = current.byStandardOption;
        target->byStandardOptionValue = current.byStandardOptionValue;
        target->byExtOption = current.byExtOption;
        target->byExtOptionValue = current.byExtOptionValue;
        target->byFullOption = current.byFullOption;
        target->byFullOptionValue = current.byFullOptionValue;
        target->byRequireClass = current.byRequireClass;

        pSeek += entrySize;
    }

    for (int j = 0; j < MAX_ITEM; ++j)
    {
        const auto& temptype = m_ItemSetType[j];
        for (int k = 0; k < MAX_ITEM_SETS_PER_ITEM; ++k)
        {
            const auto optionNumber = temptype.byOption[k];
            if (optionNumber < MAX_SET_OPTION)
            {
                m_ItemSetOption[optionNumber].bySetItemCount++;
            }
        }
    }

    return true;
}

std::uint8_t CSItemOption::IsChangeSetItem(const int Type, const int SubType)
{
    const ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    if (SubType == -1)
    {
        return itemSType.byOption[0] == EMPTY_OPTION && itemSType.byOption[1] == EMPTY_OPTION ? 0 : 255;
    }

    return itemSType.byOption[SubType] == EMPTY_OPTION ? 0 : static_cast<std::uint8_t>(SubType + 1);
}

std::uint16_t CSItemOption::GetMixItemLevel(const int Type) const
{
    if (Type < 0) return 0;

    const ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    return static_cast<std::uint16_t>(itemSType.byMixItemLevel[0] | (static_cast<std::uint16_t>(itemSType.byMixItemLevel[1]) << 8));
}

bool CSItemOption::GetSetItemName(wchar_t* strName, const int iType, const int setType) const
{
    const int setItemType = (setType % 0x04);

    if (setItemType > 0)
    {
        const ITEM_SET_TYPE& itemSType = m_ItemSetType[iType];
        if (itemSType.byOption[setItemType - 1] != 255 && itemSType.byOption[setItemType - 1] != 0)
        {
            const ITEM_SET_OPTION& itemOption = m_ItemSetOption[itemSType.byOption[setItemType - 1]];

            
            memcpy(strName, itemOption.strSetName, sizeof itemOption.strSetName);

            const int length = wcslen(strName);
            strName[length] = ' ';
            strName[length + 1] = 0;
            return true;
        }
    }

    return false;
}


void CSItemOption::checkItemType(SET_SEARCH_RESULT* optionList, const int iType, const int ancientDiscriminator) const
{
    if (ancientDiscriminator <= 0)
    {
        return;
    }

    const auto setTypeIndex = static_cast<std::uint8_t>(ancientDiscriminator - 1);

    const ITEM_SET_TYPE& itemSetType = m_ItemSetType[iType];
    const auto itemSetNumber = itemSetType.byOption[setTypeIndex];
    
    if (itemSetNumber != 255 && itemSetNumber != 0)
    {
        // add set item to list
        for (int i = 0; i < MAX_EQUIPPED_SET_ITEMS; ++i)
        {
            const auto current = &optionList[i];
            if (current->SetNumber == 0)
            {
                // The set wasn't found in another item yet, so add it
                current->SetNumber = itemSetNumber;
                current->CompleteSetItemCount = m_ItemSetOption[itemSetNumber].bySetItemCount;
                current->ItemCount++;
                current->SetTypeIndex = setTypeIndex;
                wcscpy(current->SetName, m_ItemSetOption[itemSetNumber].strSetName);
                break;
            }

            if (current->SetNumber == itemSetNumber)
            {
                current->ItemCount++;
                current->SetTypeIndex = setTypeIndex;
                break;
            }
        }
    }
}

void CSItemOption::TryAddSetOption(std::uint8_t option, int value, int optionIndex, SET_SEARCH_RESULT_OPT& set,
                                   const ITEM_SET_OPTION& setOptions, bool isThisSetComplete, bool isFullOption,
                                   bool isExtOption, int firstClass, int secondClass)
{
    // An empty slot (EMPTY_OPTION) or an option the client doesn't support adds nothing, and a
    // mastery option only counts for the class the set names.
    if (!IsSupportedSetOption(option) || value == 0 ||
        (option >= MASTERY_OPTION && !FulfillsClassRequirement(setOptions, firstClass, secondClass)))
    {
        return;
    }

    const auto setOption = &set.SetOption[set.SetOptionCount];
    setOption->OptionNumber = option;
    setOption->Value = value;
    setOption->IsActive = isThisSetComplete || optionIndex < set.ItemCount - 1;
    setOption->IsFullOption = isFullOption;
    setOption->IsExtOption = isExtOption;

    set.SetOptionCount++;
}

void CSItemOption::calcSetOptionList(const SET_SEARCH_RESULT* optionList)
{
    const int firstClass = gCharacterManager.GetBaseClass(Hero->Class);
    const int secondClass = gCharacterManager.IsSecondClass(Hero->Class);

    int setCount = 0;
    for (int i = 0; i < MAX_EQUIPPED_SET_ITEMS; ++i)
    {
        if (optionList[i].SetNumber == 0)
        {
            break;
        }

        if (optionList[i].ItemCount >= 2)
        {
            memcpy(&m_SetSearchResult[setCount], &optionList[i], sizeof(SET_SEARCH_RESULT));
            setCount++;
        }
    }

    m_SetSearchResultCount = setCount;

    // now we have all our equipped sets together and can continue to
    // build up the corresponding option list

    m_bySameSetItem = 0;

    Hero->ExtendState = 0;
    for (int i = 0; i < m_SetSearchResultCount; ++i)
    {
        auto& set = m_SetSearchResult[i];
        if (set.CompleteSetItemCount <= set.ItemCount)
        {
            Hero->ExtendState = 1;
        }

        AddSetOptions(set, m_ItemSetOption[set.SetNumber], firstClass, secondClass);
    }
}

void CSItemOption::AddSetOptions(SET_SEARCH_RESULT_OPT& set, const ITEM_SET_OPTION& setOptions, int firstClass,
                                 int secondClass)
{
    const bool isThisSetComplete = set.CompleteSetItemCount <= set.ItemCount;

    const auto standardOptionCount = std::min<int>(set.CompleteSetItemCount - 1, MAX_ITEM_SET_STANDARD_OPTION_COUNT);
    for (int o = 0; o < standardOptionCount; ++o)
    {
        for (int n = 0; n < MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT; ++n)
        {
            TryAddSetOption(setOptions.byStandardOption[o][n], setOptions.byStandardOptionValue[o][n], o, set,
                            setOptions, isThisSetComplete, false, false, firstClass, secondClass);
        }
    }

    for (int o = 0; o < MAX_ITEM_SET_EXT_OPTION_COUNT; ++o)
    {
        TryAddSetOption(setOptions.byExtOption[o], setOptions.byExtOptionValue[o], 0, set, setOptions,
                        isThisSetComplete, false, true, firstClass, secondClass);
    }

    for (int o = 0; o < MAX_ITEM_SET_FULL_OPTION_COUNT; ++o)
    {
        TryAddSetOption(setOptions.byFullOption[o], setOptions.byFullOptionValue[o], 255, set, setOptions,
                        isThisSetComplete, true, false, firstClass, secondClass);
    }
}

const wchar_t* CSItemOption::GetSetOptionFormat(std::uint8_t option)
{
    if (!IsSupportedSetOption(option))
    {
        return nullptr;
    }

    return *kSetOptionFormats[option];
}

bool CSItemOption::getExplainText(wchar_t* text, std::uint8_t option, int value)
{
    const wchar_t* format = GetSetOptionFormat(option);
    if (format == nullptr)
    {
        return false;
    }

    mu_swprintf(text, format, value);
    return true;
}

int CSItemOption::AggregateOptionValue(int optionNumber) const
{
    int result = 0;
    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        const auto& set = m_SetSearchResult[i];
        for (int j = 0; j < set.SetOptionCount; j++)
        {
            const auto& option = set.SetOption[j];

            if (option.IsActive
                && option.OptionNumber == optionNumber
                && option.Value != 0)
            {
                result += option.Value;
            }
        }
    }

    return result;
}

void CSItemOption::PlusSpecial(std::uint16_t* Value, int Special) const
{
    Special -= AT_SET_OPTION_IMPROVE_STRENGTH;

    if (const int optionValue = AggregateOptionValue(Special))
    {
        *Value += optionValue;
    }
}

void CSItemOption::PlusSpecialPercent(std::uint16_t* Value, int Special) const
{
    Special -= AT_SET_OPTION_IMPROVE_STRENGTH;

    if (const int optionValue = AggregateOptionValue(Special))
    {
        *Value += ((*Value) * optionValue) / 100;
    }
}

void CSItemOption::PlusMastery(int* Value, std::uint8_t MasteryType) const
{
    int optionValue = 0;

    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        optionValue += GetMasteryBonus(m_SetSearchResult[i], MasteryType);
    }

    if (optionValue)
    {
        *Value += optionValue;
    }
}

int CSItemOption::GetMasteryBonus(const SET_SEARCH_RESULT_OPT& set, std::uint8_t masteryType)
{
    int bonus = 0;
    for (int j = 0; j < set.SetOptionCount; j++)
    {
        const auto& option = set.SetOption[j];
        if (option.IsActive && option.OptionNumber >= MASTERY_OPTION &&
            (option.OptionNumber - MASTERY_OPTION) == masteryType && option.Value != 0)
        {
            bonus += option.Value;
        }
    }
    return bonus;
}

int CSItemOption::GetDefaultOptionValue(ITEM* ip, std::uint16_t* Value)
{
    if (ip->Type > MAX_ITEM)
    {
        return -1;
    }

    *Value = ip->AncientBonusOption;

    const ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

    return p->AttType;
}

bool CSItemOption::GetDefaultOptionText(const ITEM* ip, wchar_t* Text)
{
    if (ip->Type > MAX_ITEM)
    {
        return false;
    }

    if (ip->AncientBonusOption <= 0)
    {
        return false;
    }

    switch (ItemAttribute[ip->Type].AttType)
    {
    case SET_OPTION_STRENGTH:
        mu_swprintf(Text, I18N::Game::IncreaseStrengthD, ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_DEXTERITY:
        mu_swprintf(Text, I18N::Game::IncreaseAgilityD, ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_ENERGY:
        mu_swprintf(Text, I18N::Game::IncreaseEnergyD, ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_VITALITY:
        mu_swprintf(Text, I18N::Game::IncreaseStaminaD, ip->AncientBonusOption * 5);
        break;

    default:
        return false;
    }
    return true;
}

bool CSItemOption::IsNonWeaponSkillOrIsSkillEquipped(ActionSkillType skill)
{
    bool checkForWeaponSkill;
    auto baseSkill = gSkillManager.MasterSkillToBaseSkillIndex(skill);
    switch (baseSkill)
    {
    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_TRIPLE_SHOT:
    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_LUNGE:
    case AT_SKILL_UPPERCUT:
    case AT_SKILL_CYCLONE:
    case AT_SKILL_SLASH:
        checkForWeaponSkill = true;
        break;
    default:
        checkForWeaponSkill = false;
        break;
    }

    if (!checkForWeaponSkill)
    {
        return true;
    }

    for (int i = 0; i < 2; i++)
    {
        const ITEM* item = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i];
        if (item == nullptr || item->Type <= -1)
        {
            continue;
        }

        for (int j = 0; j < item->SpecialNum; j++)
        {
            if (item->Special[j] == baseSkill)
            {
                return true;
            }
        }
    }

    return false;
}

int CSItemOption::RenderDefaultOptionText(const ITEM* ip, int TextNum)
{
    int TNum = TextNum;
    if (GetDefaultOptionText(ip, TextList[TNum]))
    {
        TextListColor[TNum] = TEXT_COLOR_BLUE;
        TNum++;

        if ((ip->Type >= ITEM_RING_OF_ICE && ip->Type <= ITEM_RING_OF_POISON) || (ip->Type >= ITEM_PENDANT_OF_LIGHTING && ip->Type <= ITEM_PENDANT_OF_FIRE) || (ip->Type >= ITEM_RING_OF_FIRE && ip->Type <= ITEM_PENDANT_OF_WATER))
        {
            mu_swprintf(TextList[TNum], I18N::Game::IncreaseAttributeDamage); // "Increase Attribute Damage"
            TextListColor[TNum] = TEXT_COLOR_BLUE;
            TNum++;
        }
    }

    return TNum;
}

void CSItemOption::getAllAddState(std::uint16_t* Strength, std::uint16_t* Dexterity, std::uint16_t* Energy, std::uint16_t* Vitality, std::uint16_t* Charisma) const
{
    for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
    {
        ITEM* item = &CharacterMachine->Equipment[i];

        if (item->Durability <= 0)
        {
            continue;
        }

        std::uint16_t Result = 0;
        switch (GetDefaultOptionValue(item, &Result))
        {
        case SET_OPTION_STRENGTH:
            *Strength += Result * 5;
            break;

        case SET_OPTION_DEXTERITY:
            *Dexterity += Result * 5;
            break;

        case SET_OPTION_ENERGY:
            *Energy += Result * 5;
            break;

        case SET_OPTION_VITALITY:
            *Vitality += Result * 5;
            break;
        }
    }

    AddStatsBySetOptions(Strength, Dexterity, Energy, Vitality, Charisma);
}

void    CSItemOption::AddStatsBySetOptions(std::uint16_t* Strength, std::uint16_t* Dexterity, std::uint16_t* Energy, std::uint16_t* Vitality, std::uint16_t* Charisma) const
{
    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        auto& set = m_SetSearchResult[i];
        for (int j = 0; j < set.SetOptionCount; j++)
        {
            auto& option = set.SetOption[j];
            if (!option.IsActive)
            {
                continue;
            }

            switch (option.OptionNumber)
            {
            case AT_SET_OPTION_IMPROVE_STRENGTH:
                *Strength += option.Value;
                break;

            case AT_SET_OPTION_IMPROVE_DEXTERITY:
                *Dexterity += option.Value;
                break;

            case AT_SET_OPTION_IMPROVE_ENERGY:
                *Energy += option.Value;
                break;

            case AT_SET_OPTION_IMPROVE_VITALITY:
                *Vitality += option.Value;
                break;

            case AT_SET_OPTION_IMPROVE_CHARISMA:
                *Charisma += option.Value;
                break;
            default:
                // other options are not handled here.
                break;
            }
        }
    }
}

void CSItemOption::getAllAddStateOnlyAddValue(std::uint16_t* AddStrength, std::uint16_t* AddDexterity, std::uint16_t* AddEnergy, std::uint16_t* AddVitality, std::uint16_t* AddCharisma) const
{
    *AddStrength = *AddDexterity = *AddEnergy = *AddVitality = *AddCharisma = 0;
    getAllAddState(AddStrength, AddDexterity, AddEnergy, AddVitality, AddCharisma);
}

void CSItemOption::getAllAddOptionStatesbyCompare(std::uint16_t* Strength, std::uint16_t* Dexterity, std::uint16_t* Energy, std::uint16_t* Vitality, std::uint16_t* Charisma, std::uint16_t iCompareStrength, std::uint16_t iCompareDexterity, std::uint16_t iCompareEnergy, std::uint16_t iCompareVitality, std::uint16_t iCompareCharisma)
{
    for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
    {
        ITEM* item = &CharacterMachine->Equipment[i];

        if (item->RequireStrength > iCompareStrength ||
            item->RequireDexterity > iCompareDexterity ||
            item->RequireEnergy > iCompareEnergy)
        {
            continue;
        }

        if (item->Durability <= 0)
        {
            continue;
        }

        std::uint16_t Result = 0;
        switch (GetDefaultOptionValue(item, &Result))
        {
        case SET_OPTION_STRENGTH:
            *Strength += Result * 5;
            break;

        case SET_OPTION_DEXTERITY:
            *Dexterity += Result * 5;
            break;

        case SET_OPTION_ENERGY:
            *Energy += Result * 5;
            break;

        case SET_OPTION_VITALITY:
            *Vitality += Result * 5;
            break;
        }
    }

    AddStatsBySetOptions(Strength, Dexterity, Energy, Vitality, Charisma);
}

void CSItemOption::CheckItemSetOptions()
{
    SET_SEARCH_RESULT byOptionList[MAX_EQUIPPED_SET_ITEMS] = { };

    const ITEM* itemRight = nullptr;

    std::fill(std::begin(m_SetSearchResult), std::end(m_SetSearchResult), SET_SEARCH_RESULT_OPT{});

    for (int i = 0; i < MAX_EQUIPMENT_INDEX; ++i)
    {
        if (i == EQUIPMENT_WING || i == EQUIPMENT_HELPER)
        {
            continue;
        }

        const ITEM* item = &CharacterMachine->Equipment[i];

        if (item->Durability <= 0)
        {
            continue;
        }

        if ((i == EQUIPMENT_WEAPON_LEFT || i == EQUIPMENT_RING_LEFT)
            && itemRight != nullptr && itemRight->Type == item->Type
            && (itemRight->AncientDiscriminator == item->AncientDiscriminator))
        {
            // same item of a set should only count once
            continue;
        }

        if (item->Type > -1)
        {
            checkItemType(byOptionList, item->Type, item->AncientDiscriminator);
        }

        if (i == EQUIPMENT_WEAPON_RIGHT || i == EQUIPMENT_RING_RIGHT)
        {
            itemRight = item;
        }
    }

    calcSetOptionList(byOptionList);
    getAllAddStateOnlyAddValue(&CharacterAttribute->AddStrength, &CharacterAttribute->AddDexterity, &CharacterAttribute->AddEnergy, &CharacterAttribute->AddVitality, &CharacterAttribute->AddCharisma);

    const auto AllStrength = static_cast<std::uint16_t>(CharacterAttribute->Strength + CharacterAttribute->AddStrength);
    const auto AllDexterity = static_cast<std::uint16_t>(CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity);
    const auto AllEnergy = static_cast<std::uint16_t>(CharacterAttribute->Energy + CharacterAttribute->AddEnergy);
    auto AllVitality = static_cast<std::uint16_t>(CharacterAttribute->Vitality + CharacterAttribute->AddVitality);
    const auto AllCharisma = static_cast<std::uint16_t>(CharacterAttribute->Charisma + CharacterAttribute->AddCharisma);
    const auto AllLevel = static_cast<std::uint16_t>(CharacterAttribute->Level);

    // And now we're doing all that again, just for checking the required stats?!
    // TODO: How can this be improved?

    std::fill(std::begin(m_SetSearchResult), std::end(m_SetSearchResult), SET_SEARCH_RESULT_OPT{});
    std::fill(std::begin(byOptionList), std::end(byOptionList), SET_SEARCH_RESULT{});

    for (int i = 0; i < MAX_EQUIPMENT_INDEX; ++i)
    {
        if (i == EQUIPMENT_WING || i == EQUIPMENT_HELPER)
        {
            continue;
        }

        ITEM* ip = &CharacterMachine->Equipment[i];

        if (ip->RequireDexterity > AllDexterity || ip->RequireEnergy > AllEnergy || ip->RequireStrength > AllStrength || ip->RequireLevel > AllLevel || ip->RequireCharisma > AllCharisma || ip->Durability <= 0 || (IsRequireEquipItem(ip) == false)) {
            continue;
        }

        if ((i == EQUIPMENT_WEAPON_LEFT || i == EQUIPMENT_RING_LEFT)
            && itemRight != nullptr && itemRight->Type == ip->Type &&
            itemRight->AncientDiscriminator == ip->AncientDiscriminator)
        {
            continue;
        }

        if (ip->Type > -1)
        {
            checkItemType(byOptionList, ip->Type, ip->AncientDiscriminator);
        }

        if (i == EQUIPMENT_WEAPON_RIGHT || i == EQUIPMENT_RING_RIGHT)
        {
            itemRight = ip;
        }
    }

    calcSetOptionList(byOptionList);
}

void CSItemOption::MoveSetOptionList(const int StartX, const int StartY)
{
    int x, y, Width, Height;

    Width = 162; Height = 20; x = StartX + 14; y = StartY + 22;
    if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
    {
        m_bViewOptionList = true;

        MouseLButtonPush = false;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
    }
}

void CSItemOption::RenderSetOptionButton(const int StartX, const int StartY)
{
    float x, y, Width, Height;
    wchar_t  Text[100];

    Width = 162.f; Height = 20.f; x = (float)StartX + 14; y = (float)StartY + 22;
    RenderBitmap(BITMAP_INTERFACE_EX + 21, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 32.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0, 0, 0, 255);
    g_pRenderText->SetBgColor(100, 0, 0, 0);
    mu_swprintf(Text, L"[%ls]", I18N::Game::SetOption);
    g_pRenderText->RenderText(StartX + 96, (int)(y + 3), Text, 0, 0, RT3_WRITE_CENTER);

    g_pRenderText->SetTextColor(0xffffffff);
    if (m_SetSearchResultCount > 0)
        g_pRenderText->SetTextColor(255, 204, 25, 255);
    else
        g_pRenderText->SetTextColor(128, 128, 128, 255);
    g_pRenderText->RenderText(StartX + 95, (int)(y + 2), Text, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
}

bool CSItemOption::BuildSetOptionTooltipModel(UI::Inventory::Tooltip::Model& outModel)
{
    // Hover gating is the caller's responsibility now (CMyInventory::SyncRmlModel(), driven by
    // RmlUi's own mouseover/mouseout on the Set Option label) -- this only needs to know whether
    // there's anything to show.
    if (m_SetSearchResultCount == 0)
        return false;

    std::uint8_t TextNum = 0;

    mu_swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;
    mu_swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;
    mu_swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++;

    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        const auto& set = m_SetSearchResult[i];

        // print set name:
        mu_swprintf(TextList[TextNum], L"%ls %ls", set.SetName, I18N::Game::Set);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = true;
        TextNum++;

        TextNum = RenderSetOptionList(set, TextNum, true, false);
    }

    outModel.Reset();
    outModel.count = std::min<int>(TextNum, UI::Inventory::Tooltip::MAX_TOOLTIP_LINES);
    for (int i = 0; i < outModel.count; ++i)
    {
        UI::Inventory::Tooltip::Line& line = outModel.lines[i];
        wcsncpy(line.text, TextList[i], UI::Inventory::Tooltip::MAX_TOOLTIP_LINE_TEXT - 1);
        line.text[UI::Inventory::Tooltip::MAX_TOOLTIP_LINE_TEXT - 1] = L'\0';
        switch (TextListColor[i])
        {
        case TEXT_COLOR_BLUE:   line.color = UI::Inventory::Tooltip::LineColor::Blue;   break;
        case TEXT_COLOR_YELLOW: line.color = UI::Inventory::Tooltip::LineColor::Yellow; break;
        case TEXT_COLOR_GREEN:  line.color = UI::Inventory::Tooltip::LineColor::Green;  break;
        case TEXT_COLOR_PURPLE: line.color = UI::Inventory::Tooltip::LineColor::Purple; break;
        default:                line.color = UI::Inventory::Tooltip::LineColor::White;  break;
        }
        line.isBold = TextBold[i] != 0;
    }

    return true;
}

namespace
{
// Some English set names write their possessive with the typographic apostrophe U+2019 (Chrono, Semeden).
constexpr wchar_t kTypographicApostrophe = L'\u2019';
constexpr std::wstring_view kPossessiveSuffix = L"'s";

// `text` in lower case, with U+2019 as '.
std::wstring FoldSetName(std::wstring_view text)
{
    std::wstring folded;
    folded.reserve(text.size());
    for (const wchar_t c : text)
    {
        folded.push_back(c == kTypographicApostrophe ? L'\'' : static_cast<wchar_t>(std::towlower(c)));
    }
    return folded;
}

// `folded` without its apostrophes.
std::wstring WithoutApostrophes(std::wstring_view folded)
{
    std::wstring result;
    result.reserve(folded.size());
    for (const wchar_t c : folded)
    {
        if (c != L'\'')
        {
            result.push_back(c);
        }
    }
    return result;
}

enum class SetNameMatch
{
    None,
    // The name as it is, apart from capitals and the apostrophe's form.
    Exact,
    // Also without the apostrophe, with or without the name's trailing s.
    Loose,
};

SetNameMatch MatchSetName(std::wstring_view typed, std::wstring_view setName)
{
    const std::wstring typedFolded = FoldSetName(typed);
    if (typedFolded.empty())
    {
        return SetNameMatch::None;
    }

    const std::wstring nameFolded = FoldSetName(setName);
    if (typedFolded == nameFolded)
    {
        return SetNameMatch::Exact;
    }

    std::wstring_view nameBase = nameFolded;
    if (nameBase.size() > kPossessiveSuffix.size() && nameBase.ends_with(kPossessiveSuffix))
    {
        nameBase.remove_suffix(kPossessiveSuffix.size());
    }
    const std::wstring typedLoose = WithoutApostrophes(typedFolded);
    const std::wstring baseLoose = WithoutApostrophes(nameBase);
    return (typedLoose == baseLoose || typedLoose == baseLoose + L's') ? SetNameMatch::Loose : SetNameMatch::None;
}
} // namespace

int CSItemOption::FindSetByName(const wchar_t* typed, std::span<const ITEM_SET_OPTION> sets)
{
    int looseMatch = -1;
    for (int i = 0; i < static_cast<int>(sets.size()); ++i)
    {
        if (sets[i].byOptionCount >= 255)
        {
            continue;
        }

        const SetNameMatch match = MatchSetName(typed, sets[i].strSetName);
        if (match == SetNameMatch::Exact)
        {
            return i;
        }
        if (match == SetNameMatch::Loose && looseMatch < 0)
        {
            looseMatch = i;
        }
    }
    return looseMatch;
}

void CSItemOption::CheckRenderOptionHelper(const wchar_t* FilterName)
{
    if (FilterName[0] != '/')
        return;

    m_byRenderOptionList = 0;
    const int setIndex = FindSetByName(FilterName + 1, m_ItemSetOption);
    if (setIndex < 0)
    {
        return;
    }

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_ITEM_EXPLANATION);
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_HELP);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_SETITEM_EXPLANATION);

    m_byRenderOptionList = static_cast<std::uint8_t>(setIndex + 1);
}

int CSItemOption::RenderSetOptionListInItem(const ITEM* ip, int TextNum, bool bIsEquippedItem)
{
    if (ip->AncientDiscriminator == 0)
    {
        return TextNum;
    }

    const ITEM_SET_TYPE& itemSType = m_ItemSetType[ip->Type];

    m_bySelectedItemOption = itemSType.byOption[ip->AncientDiscriminator - 1];

    if (m_bySelectedItemOption <= 0 || m_bySelectedItemOption == 255) return TextNum;

    int TNum = TextNum;

    const ITEM_SET_OPTION& setOption = m_ItemSetOption[m_bySelectedItemOption];
    if (setOption.byOptionCount >= 255)
    {
        m_bySelectedItemOption = 0;
        return TNum;
    }


    mu_swprintf(TextList[TNum], L"\n"); TNum++;
    mu_swprintf(TextList[TNum], L"%ls %ls", I18N::Game::Set, I18N::Game::ItemOptionInfo);
    TextListColor[TNum] = TEXT_COLOR_YELLOW;
    TNum++;

    mu_swprintf(TextList[TNum], L"\n"); TNum++;
    mu_swprintf(TextList[TNum], L"\n"); TNum++;


    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        const auto& set = m_SetSearchResult[i];
        if (wcscmp(set.SetName, setOption.strSetName) == 0)
        {
            // Set Found.
            TNum = RenderSetOptionList(set, TNum, bIsEquippedItem, true);
            break;
        }
    }

    mu_swprintf(TextList[TNum], L"\n"); TNum++;
    mu_swprintf(TextList[TNum], L"\n"); TNum++;

    return TNum;
}

std::uint8_t  CSItemOption::RenderSetOptionList(const SET_SEARCH_RESULT_OPT& set, std::uint8_t textIndex, bool bIsEquippedItem, bool bShowInactive)
{
    for (int j = 0; j < set.SetOptionCount; j++)
    {
        const auto option = set.SetOption[j];
        if (!bShowInactive && !option.IsActive)
        {
            break;
        }

        if (getExplainText(TextList[textIndex], option.OptionNumber, option.Value))
        {
            if (!bIsEquippedItem || !option.IsActive)
            {
                TextListColor[textIndex] = TEXT_COLOR_GRAY;
            }
            else
            {
                TextListColor[textIndex] = option.IsFullOption ? TEXT_COLOR_YELLOW
                                            : option.IsExtOption ? TEXT_COLOR_GREEN
                                            : TEXT_COLOR_BLUE;
            }

            TextBold[textIndex] = false;
            textIndex++;
        }
    }

    mu_swprintf(TextList[textIndex], L"\n");
    textIndex++;

    return textIndex;
}

void CSItemOption::SetViewOptionList(bool bView)
{
    m_bViewOptionList = bView;
}

bool CSItemOption::IsViewOptionList()
{
    return m_bViewOptionList;
}
