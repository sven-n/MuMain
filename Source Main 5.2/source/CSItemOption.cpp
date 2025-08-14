/*+++++++++++++++++++++++++++++++++++++
    INCLUDE.
+++++++++++++++++++++++++++++++++++++*/
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "CSItemOption.h"
#include "CharacterManager.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "SkillManager.h"

extern	wchar_t TextList[50][100];
extern	int  TextListColor[50];
extern	int  TextBold[50];

constexpr BYTE EMPTY_OPTION = 0xFF;

static  CSItemOption csItemOption; // do not delete, required for singleton initialization.

struct ITEM_SET_OPTION_FILE
{
    char	strSetName[MAX_ITEM_SET_NAME];
    BYTE	byStandardOption[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT];
    BYTE	byStandardOptionValue[MAX_ITEM_SET_STANDARD_OPTION_COUNT][MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT];
    BYTE	byExtOption[MAX_ITEM_SET_EXT_OPTION_COUNT];
    BYTE	byExtOptionValue[MAX_ITEM_SET_EXT_OPTION_COUNT];
    BYTE	byOptionCount;
    BYTE	byFullOption[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE	byFullOptionValue[MAX_ITEM_SET_FULL_OPTION_COUNT];
    BYTE	byRequireClass[MAX_CLASS];
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
    FILE* fp = _wfopen(filename, L"rb");
    if (fp != nullptr)
    {
        const int Size = sizeof(ITEM_SET_TYPE);
        BYTE* Buffer = new BYTE[Size * MAX_ITEM];
        fread(Buffer, Size * MAX_ITEM, 1, fp);

        DWORD dwCheckSum;
        fread(&dwCheckSum, sizeof(DWORD), 1, fp);
        fclose(fp);

        if (dwCheckSum != GenerateCheckSum2(Buffer, Size * MAX_ITEM, 0xE5F1))
        {
            wchar_t Text[256];
            swprintf(Text, L"%s - File corrupted.", filename);
            g_ErrorReport.Write(Text);
            MessageBox(g_hWnd, Text, nullptr, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        }
        else
        {
            BYTE* pSeek = Buffer;
            for (int i = 0; i < MAX_ITEM; i++)
            {
                BuxConvert(pSeek, Size);
                memcpy(&m_ItemSetType[i], pSeek, Size);

                pSeek += Size;
            }
        }
        delete[] Buffer;
    }
    else
    {
        wchar_t Text[256];
        swprintf(Text, L"%s - File not exist.", filename);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }

    return true;
}

bool CSItemOption::OpenItemSetOption(const wchar_t* filename)
{
    FILE* fp = _wfopen(filename, L"rb");
    if (fp != nullptr)
    {
        const int Size = sizeof(ITEM_SET_OPTION_FILE);
        BYTE* Buffer = new BYTE[Size * MAX_SET_OPTION];
        fread(Buffer, Size * MAX_SET_OPTION, 1, fp);

        DWORD dwCheckSum;
        fread(&dwCheckSum, sizeof(DWORD), 1, fp);
        fclose(fp);

        if (dwCheckSum != GenerateCheckSum2(Buffer, Size * MAX_SET_OPTION, 0xA2F1))
        {
            wchar_t Text[256];
            swprintf(Text, L"%s - File corrupted.", filename);
            g_ErrorReport.Write(Text);
            MessageBox(g_hWnd, Text, nullptr, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        }
        else
        {
            BYTE* pSeek = Buffer;
            for (int i = 0; i < MAX_SET_OPTION; i++)
            {
                BuxConvert(pSeek, Size);

                ITEM_SET_OPTION_FILE current{ };
                memcpy(&current, pSeek, Size);
                const auto target = &m_ItemSetOption[i];

                CMultiLanguage::ConvertFromUtf8(target->strSetName, current.strSetName);
                target->byOptionCount = current.byOptionCount;
                target->bySetItemCount = 0; // Is calculated below
                for (int o = 0; o < MAX_ITEM_SET_STANDARD_OPTION_COUNT; ++o)
                {
                    for (int n = 0; n < MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT; n++)
                    {
                        target->byStandardOption[o][n] = current.byStandardOption[o][n];
                        target->byStandardOptionValue[o][n] = current.byStandardOptionValue[o][n];
                    }
                }

                for (int o = 0; o < MAX_ITEM_SET_EXT_OPTION_COUNT; ++o)
                {
                    target->byExtOption[o] = current.byExtOption[o];
                    target->byExtOptionValue[o] = current.byExtOptionValue[o];
                }

                for (int o = 0; o < MAX_ITEM_SET_FULL_OPTION_COUNT; ++o)
                {
                    target->byFullOption[o] = current.byFullOption[o];
                    target->byFullOptionValue[o] = current.byFullOptionValue[o];
                }

                for (int i = 0; i < MAX_CLASS; i++)
                {
                    target->byRequireClass[i] = current.byRequireClass[i];
                }

                pSeek += Size;
            }

            for (int j = 0; j < MAX_ITEM; j++)
            {
                const auto &temptype = m_ItemSetType[j];
                for (int k = 0; k < MAX_ITEM_SETS_PER_ITEM; k++)
                {
                    const auto optionNumber = temptype.byOption[k];
                    if (optionNumber < MAX_SET_OPTION)
                    {
                        m_ItemSetOption[optionNumber].bySetItemCount++;
                    }
                }
            }
        }
        delete[] Buffer;
    }
    else
    {
        wchar_t Text[256];
        swprintf(Text, L"%s - File does not exist.", filename);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }

    return true;
}

bool CSItemOption::IsDisableSkill(ActionSkillType Type, int Energy, int Charisma)
{
    int SkillEnergy = 20 + SkillAttribute[Type].Energy * (SkillAttribute[Type].Level) * 4 / 100;

    if (Type == AT_SKILL_SUMMON_EXPLOSION || Type == AT_SKILL_SUMMON_REQUIEM)
    {
        SkillEnergy = 20 + SkillAttribute[Type].Energy * (SkillAttribute[Type].Level) * 3 / 100;
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT)
    {
        SkillEnergy = 10 + SkillAttribute[Type].Energy * (SkillAttribute[Type].Level) * 4 / 100;
    }

    switch (Type)
    {
    case 17:SkillEnergy = 0; break;
    case 30:SkillEnergy = 30; break;
    case 31:SkillEnergy = 60; break;
    case 32:SkillEnergy = 90; break;
    case 33:SkillEnergy = 130; break;
    case 34:SkillEnergy = 170; break;
    case 35:SkillEnergy = 210; break;
    case 36:SkillEnergy = 300; break;
    case 37:SkillEnergy = 500; break;
    case 60:SkillEnergy = 15; break;
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
    case AT_SKILL_EARTHSHAKE:    SkillEnergy = 0; break;
    case AT_PET_COMMAND_DEFAULT: SkillEnergy = 0; break;
    case AT_PET_COMMAND_RANDOM:  SkillEnergy = 0; break;
    case AT_PET_COMMAND_OWNER:   SkillEnergy = 0; break;
    case AT_PET_COMMAND_TARGET:  SkillEnergy = 0; break;
    case AT_SKILL_PLASMA_STORM_FENRIR: SkillEnergy = 0; break;
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR: SkillEnergy = 0; break;
    case AT_SKILL_STRIKE_OF_DESTRUCTION: 
    case AT_SKILL_STRIKE_OF_DESTRUCTION_STR: SkillEnergy = 0; break;
    case AT_SKILL_RECOVER:
    case AT_SKILL_GAOTIC:
    case AT_SKILL_MULTI_SHOT:
    case AT_SKILL_FIRE_SCREAM_STR:
    case AT_SKILL_FIRE_SCREAM:
        SkillEnergy = 0;
        break;

    case AT_SKILL_EXPLODE:
        SkillEnergy = 0;
        break;
    }

    if (Type >= AT_SKILL_STUN && Type <= AT_SKILL_REMOVAL_BUFF)
    {
        SkillEnergy = 0;
    }
    else
        if ((Type >= 18 && Type <= 23) || (Type >= 41 && Type <= 43) || (Type >= 47 && Type <= 49) || Type == 24 || Type == 51 || Type == 52 || Type == 55 || Type == 56)
        {
            SkillEnergy = 0;
        }
        else if (Type == 44 || Type == 45 || Type == 46 || Type == 57 || Type == 73 || Type == 74)
        {
            SkillEnergy = 0;
        }

    if (Charisma > 0)
    {
        const int SkillCharisma = SkillAttribute[Type].Charisma;
        if (Charisma < SkillCharisma)
        {
            return true;
        }
    }

    if (Energy < SkillEnergy)
    {
        return true;
    }

    return false;
}

BYTE CSItemOption::IsChangeSetItem(const int Type, const int SubType = -1)
{
    const ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    if (SubType == -1)
    {
        return itemSType.byOption[0] == EMPTY_OPTION && itemSType.byOption[1] == EMPTY_OPTION ? 0 : 255;
    }

    return itemSType.byOption[SubType] == EMPTY_OPTION ? 0 : SubType + 1;
}

WORD CSItemOption::GetMixItemLevel(const int Type) const
{
    if (Type < 0) return 0;

    WORD MixLevel = 0;
    const ITEM_SET_TYPE& itemSType = m_ItemSetType[Type];

    MixLevel = MAKEWORD(itemSType.byMixItemLevel[0], itemSType.byMixItemLevel[1]);

    return MixLevel;
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

    const auto setTypeIndex = static_cast<BYTE>(ancientDiscriminator - 1);

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


bool CSItemOption::isClassRequirementFulfilled(const ITEM_SET_OPTION& setOptions, const int firstClass, int secondClass)
{
    bool RequireClass = false;
    for (int i = CLASS_WIZARD; i <= CLASS_RAGEFIGHTER; i++)
    {
        if (setOptions.byRequireClass[i] == 1 && firstClass == i)
        {
            RequireClass = true;
        }

        if (setOptions.byRequireClass[i] == 2 && firstClass == i && secondClass)
        {
            RequireClass = true;
        }
    }

    return RequireClass;
}

void CSItemOption::TryAddSetOption(BYTE option, int value, int optionIndex, SET_SEARCH_RESULT_OPT& set, const ITEM_SET_OPTION& setOptions, bool isThisSetComplete, bool isFullOption, bool isExtOption, bool fulfillsClassRequirement, int firstClass, int secondClass)
{
    if (option == EMPTY_OPTION
        || value == 0
        || (option >= MASTERY_OPTION 
            && (setOptions.byRequireClass[firstClass] && secondClass >= setOptions.byRequireClass[firstClass] - 1)))
    {
        return;
    }

    const auto setOption = &set.SetOption[set.SetOptionCount];
    setOption->OptionNumber = option;
    setOption->FulfillsClassRequirement = fulfillsClassRequirement;
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

        const ITEM_SET_OPTION& setOptions = m_ItemSetOption[set.SetNumber];
        bool isThisSetComplete = false;
        if (set.CompleteSetItemCount <= set.ItemCount)
        {
            Hero->ExtendState = 1;
            isThisSetComplete = true;
        }

        const auto requireClass = isClassRequirementFulfilled(setOptions, firstClass, secondClass);
        const auto standardOptionCount = min(set.CompleteSetItemCount - 1, MAX_ITEM_SET_STANDARD_OPTION_COUNT);
        for (int o = 0; o < standardOptionCount; ++o)
        {
            for (int n = 0; n < MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT; ++n)
            {
                TryAddSetOption(setOptions.byStandardOption[o][n], setOptions.byStandardOptionValue[o][n], o, set, setOptions, isThisSetComplete, false, false, requireClass, firstClass, secondClass);
            }
        }

        for (int o = 0; o < MAX_ITEM_SET_EXT_OPTION_COUNT; ++o)
        {
            TryAddSetOption(setOptions.byExtOption[o], setOptions.byExtOptionValue[o], 0, set, setOptions, isThisSetComplete, false, true, requireClass, firstClass, secondClass);
        }

        for (int o = 0; o < MAX_ITEM_SET_FULL_OPTION_COUNT; ++o)
        {
            TryAddSetOption(setOptions.byFullOption[o], setOptions.byFullOptionValue[o], 255, set, setOptions, isThisSetComplete, true, false, requireClass, firstClass, secondClass);
        }
    }
}

bool CSItemOption::getExplainText(wchar_t* text, const BYTE option, const int value)
{
    if (option == EMPTY_OPTION)
    {
        return false;
    }

    switch (option + AT_SET_OPTION_IMPROVE_STRENGTH)
    {
    case AT_SET_OPTION_IMPROVE_MAGIC_POWER:
        swprintf(text, GlobalText[632], value);
        return true;

    case AT_SET_OPTION_IMPROVE_STRENGTH:
    case AT_SET_OPTION_IMPROVE_DEXTERITY:
    case AT_SET_OPTION_IMPROVE_ENERGY:
    case AT_SET_OPTION_IMPROVE_VITALITY:
    case AT_SET_OPTION_IMPROVE_CHARISMA:
    case AT_SET_OPTION_IMPROVE_ATTACK_MIN:
    case AT_SET_OPTION_IMPROVE_ATTACK_MAX:
        swprintf(text, GlobalText[950 + option], value);
        return true;

    case AT_SET_OPTION_IMPROVE_DAMAGE:
    case AT_SET_OPTION_IMPROVE_ATTACKING_PERCENT:
    case AT_SET_OPTION_IMPROVE_DEFENCE:
    case AT_SET_OPTION_IMPROVE_MAX_LIFE:
    case AT_SET_OPTION_IMPROVE_MAX_MANA:
    case AT_SET_OPTION_IMPROVE_MAX_AG:
    case AT_SET_OPTION_IMPROVE_ADD_AG:
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE_PERCENT:
    case AT_SET_OPTION_IMPROVE_CRITICAL_DAMAGE:
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE_PERCENT:
    case AT_SET_OPTION_IMPROVE_EXCELLENT_DAMAGE:
    case AT_SET_OPTION_IMPROVE_SKILL_ATTACK:
    case AT_SET_OPTION_DOUBLE_DAMAGE:
        swprintf(text, GlobalText[949 + option], value);
        return true;

    case AT_SET_OPTION_DISABLE_DEFENCE:
        swprintf(text, GlobalText[970], value);
        return true;

    case AT_SET_OPTION_TWO_HAND_SWORD_IMPROVE_DAMAGE:
        swprintf(text, GlobalText[983], value);
        return true;

    case AT_SET_OPTION_IMPROVE_SHIELD_DEFENCE:
        swprintf(text, GlobalText[984], value);
        return true;

    case AT_SET_OPTION_IMPROVE_ATTACK_1:
    case AT_SET_OPTION_IMPROVE_ATTACK_2:
    case AT_SET_OPTION_IMPROVE_MAGIC:
        //	case AT_SET_OPTION_IMPROVE_DEFENCE_1:
        //	case AT_SET_OPTION_IMPROVE_DEFENCE_2:
    case AT_SET_OPTION_IMPROVE_DEFENCE_3:
    case AT_SET_OPTION_IMPROVE_DEFENCE_4:
    case AT_SET_OPTION_FIRE_MASTERY:
    case AT_SET_OPTION_ICE_MASTERY:
    case AT_SET_OPTION_THUNDER_MASTERY:
    case AT_SET_OPTION_POSION_MASTERY:
    case AT_SET_OPTION_WATER_MASTERY:
    case AT_SET_OPTION_WIND_MASTERY:
    case AT_SET_OPTION_EARTH_MASTERY:
        swprintf(text, GlobalText[971 + (option + AT_SET_OPTION_IMPROVE_STRENGTH - AT_SET_OPTION_IMPROVE_ATTACK_2)], value);
        return true;
    default:
        return false;
    }
}

int CSItemOption::AggregateOptionValue(int optionNumber) const
{
    WORD result = 0;
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

void CSItemOption::PlusSpecial(WORD* Value, int Special) const
{
    Special -= AT_SET_OPTION_IMPROVE_STRENGTH;

    if (const int optionValue = AggregateOptionValue(Special))
    {
        *Value += optionValue;
    }
}

void CSItemOption::PlusSpecialPercent(WORD* Value, int Special) const
{
    Special -= AT_SET_OPTION_IMPROVE_STRENGTH;

    if (const int optionValue = AggregateOptionValue(Special))
    {
        *Value += ((*Value) * optionValue) / 100;
    }
}

void CSItemOption::PlusSpecialLevel(WORD* Value, const WORD SrcValue, int Special) const
{
    Special -= AT_SET_OPTION_IMPROVE_STRENGTH;
    int optionValue = 0;
    int count = 0;

    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        const auto& set = m_SetSearchResult[i];
        for (int j = 0; j < set.SetOptionCount; j++)
        {
            const auto& option = set.SetOption[j];
            if (option.IsActive
                && option.OptionNumber == Special
                && option.Value != 0)
            {
                optionValue += option.Value;
                count++;
            }
        }
    }

    if (optionValue)
    {
        optionValue = SrcValue * optionValue / 100;
        *Value += (optionValue * count);
    }
}

void CSItemOption::PlusMastery(int* Value, const BYTE MasteryType) const
{
    int optionValue = 0;

    for (int i = 0; i < m_SetSearchResultCount; i++)
    {
        const auto& set = m_SetSearchResult[i];
        for (int j = 0; j < set.SetOptionCount; j++)
        {
            const auto& option = set.SetOption[j];
            if (option.IsActive
                && option.OptionNumber >= MASTERY_OPTION
                && (option.OptionNumber - MASTERY_OPTION - 5) == MasteryType
                && option.Value != 0)
            {
                optionValue += option.Value;
            }
        }
    }

    if (optionValue)
    {
        *Value += optionValue;
    }
}

int CSItemOption::GetDefaultOptionValue(ITEM* ip, WORD* Value)
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
        swprintf(Text, GlobalText[950], ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_DEXTERITY:
        swprintf(Text, GlobalText[951], ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_ENERGY:
        swprintf(Text, GlobalText[952], ip->AncientBonusOption * 5);
        break;

    case SET_OPTION_VITALITY:
        swprintf(Text, GlobalText[953], ip->AncientBonusOption * 5);
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
            swprintf(TextList[TNum], GlobalText[1165]); // "Increase Attribute Damage"
            TextListColor[TNum] = TEXT_COLOR_BLUE;
            TNum++;
        }
    }

    return TNum;
}

void CSItemOption::getAllAddState(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma) const
{
    for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
    {
        ITEM* item = &CharacterMachine->Equipment[i];

        if (item->Durability <= 0)
        {
            continue;
        }

        WORD Result = 0;
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

void    CSItemOption::AddStatsBySetOptions(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma) const
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

void CSItemOption::getAllAddStateOnlyAddValue(WORD* AddStrength, WORD* AddDexterity, WORD* AddEnergy, WORD* AddVitality, WORD* AddCharisma) const
{
    *AddStrength = *AddDexterity = *AddEnergy = *AddVitality = *AddCharisma = 0;
    getAllAddState(AddStrength, AddDexterity, AddEnergy, AddVitality, AddCharisma);
}

void CSItemOption::getAllAddOptionStatesbyCompare(WORD* Strength, WORD* Dexterity, WORD* Energy, WORD* Vitality, WORD* Charisma, WORD iCompareStrength, WORD iCompareDexterity, WORD iCompareEnergy, WORD iCompareVitality, WORD iCompareCharisma)
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

        WORD Result = 0;
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

    ZeroMemory(m_SetSearchResult, sizeof(SET_SEARCH_RESULT_OPT) * MAX_EQUIPPED_SETS);

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

    const WORD AllStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    const WORD AllDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    const WORD AllEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    WORD AllVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
    const WORD AllCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
    const WORD AllLevel = CharacterAttribute->Level;

    // And now we're doing all that again, just for checking the required stats?!
    // TODO: How can this be improved?

    ZeroMemory(m_SetSearchResult, sizeof(SET_SEARCH_RESULT_OPT) * MAX_EQUIPPED_SETS);
    ZeroMemory(byOptionList, sizeof byOptionList);

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
    swprintf(Text, L"[%s]", GlobalText[989]);
    g_pRenderText->RenderText(StartX + 96, (int)(y + 3), Text, 0, 0, RT3_WRITE_CENTER);

    g_pRenderText->SetTextColor(0xffffffff);
    if (m_SetSearchResultCount > 0)
        g_pRenderText->SetTextColor(255, 204, 25, 255);
    else
        g_pRenderText->SetTextColor(128, 128, 128, 255);
    g_pRenderText->RenderText(StartX + 95, (int)(y + 2), Text, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
}

void CSItemOption::RenderSetOptionList(const int StartX, const int StartY)
{
    if (m_bViewOptionList && m_SetSearchResultCount > 0)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(100, 0, 0, 0);

        int PosX, PosY;

        PosX = StartX + 95;
        PosY = StartY + 40;

        BYTE TextNum = 0;
        BYTE SkipNum = 0;
        BYTE setIndex = 0;

        swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;

        int		iCurSetItemTypeSequence = 0, iCurSetItemType = -1;

        for (int i = 0; i < m_SetSearchResultCount; i++)
        {
            const auto& set = m_SetSearchResult[i];

            // print set name:
            swprintf(TextList[TextNum], L"%s %s", set.SetName, GlobalText[1089]);
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextBold[TextNum] = true;
            TextNum++;

            TextNum = RenderSetOptionList(set, TextNum, true, false);
        }

        RenderTipTextList(PosX, PosY, TextNum, 120, RT3_SORT_CENTER);
        m_bViewOptionList = false;
    }
}

void CSItemOption::CheckRenderOptionHelper(const wchar_t* FilterName)
{
    wchar_t Name[256];

    if (FilterName[0] != '/') return;

    const auto Length1 = wcslen(FilterName);
    for (int i = 0; i < MAX_SET_OPTION; ++i)
    {
        ITEM_SET_OPTION& setOption = m_ItemSetOption[i];
        if (setOption.byOptionCount < 255)
        {
            swprintf(Name, L"/%s", setOption.strSetName);

            const auto Length2 = wcslen(Name);

            m_byRenderOptionList = 0;
            if (wcsncmp(FilterName, Name, Length1) == NULL && wcsncmp(FilterName, Name, Length2) == NULL)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
                g_pNewUISystem->Show(SEASON3B::INTERFACE_SETITEM_EXPLANATION);

                m_byRenderOptionList = i + 1;
                return;
            }
        }
    }
}

void CSItemOption::RenderOptionHelper(void)
{
    if (m_byRenderOptionList <= 0) return;

    int TextNum = 0;
    int sx = 0, sy = 0;
    ZeroMemory(TextListColor, sizeof TextListColor);
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = NULL;
    }

    ITEM_SET_OPTION& setOption = m_ItemSetOption[m_byRenderOptionList - 1];
    if (setOption.byOptionCount >= 255)
    {
        m_byRenderOptionList = 0;
        return;
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++;
    swprintf(TextList[TextNum], L"%s %s %s", setOption.strSetName, GlobalText[1089], GlobalText[159]);
    TextListColor[TextNum] = TEXT_COLOR_YELLOW;
    TextNum++;

    swprintf(TextList[TextNum], L"\n"); TextNum++;
    swprintf(TextList[TextNum], L"\n"); TextNum++;

    for (int o = 0; o < MAX_ITEM_SET_STANDARD_OPTION_COUNT; ++o)
    {
        for (int n = 0; n < MAX_ITEM_SET_STANDARD_OPTION_PER_ITEM_COUNT; ++n)
        {
            if (getExplainText(TextList[TextNum], setOption.byStandardOption[o][n], setOption.byStandardOptionValue[o][n]))
            {
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
    }

    for (int o = 0; o < MAX_ITEM_SET_EXT_OPTION_COUNT; ++o)
    {
        if (getExplainText(TextList[TextNum], setOption.byExtOption[o], setOption.byExtOption[o]))
        {
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    for (int o = 0; o < MAX_ITEM_SET_FULL_OPTION_COUNT; ++o)
    {
        if (getExplainText(TextList[TextNum], setOption.byFullOption[o], setOption.byFullOption[o]))
        {
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++;
    swprintf(TextList[TextNum], L"\n"); TextNum++;

    SIZE TextSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
    RenderTipTextList(sx, sy, TextNum, 0);
}

int CSItemOption::RenderSetOptionListInItem(const ITEM* ip, int TextNum, bool bIsEquippedItem)
{
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


    swprintf(TextList[TNum], L"\n"); TNum++;
    swprintf(TextList[TNum], L"%s %s", GlobalText[1089], GlobalText[159]);
    TextListColor[TNum] = TEXT_COLOR_YELLOW;
    TNum++;

    swprintf(TextList[TNum], L"\n"); TNum++;
    swprintf(TextList[TNum], L"\n"); TNum++;


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

    swprintf(TextList[TNum], L"\n"); TNum++;
    swprintf(TextList[TNum], L"\n"); TNum++;

    return TNum;
}

BYTE  CSItemOption::RenderSetOptionList(const SET_SEARCH_RESULT_OPT& set, BYTE textIndex, bool bIsEquippedItem, bool bShowInactive)
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
            else if (option.OptionNumber >= AT_SET_OPTION_IMPROVE_ATTACK_1 && !option.FulfillsClassRequirement)
            {
                // Mastery
                TextListColor[textIndex] = TEXT_COLOR_RED;
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

    swprintf(TextList[textIndex], L"\n");
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