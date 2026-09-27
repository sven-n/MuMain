#include "stdafx.h"
#include "UI/MuHelper/MuHelperShared.h"

#include "Core/Globals/_enum.h"

#include <cwchar>

namespace UI::MuHelper
{
    MUHelper::ConfigData& StagedConfig()
    {
        static MUHelper::ConfigData config;
        return config;
    }

    int ParseIntInput(const wchar_t* text)
    {
        if (text == nullptr)
            return 0;

        wchar_t* end = nullptr;
        const int value = static_cast<int>(std::wcstol(text, &end, 10));
        return (end != nullptr && *end == L'\0') ? value : 0;
    }

    ClassFeatures ResolveClassFeatures(int baseClass)
    {
        ClassFeatures f;
        f.potionPage = SUB_PAGE_POTION_CONFIG;

        switch (baseClass)
        {
        case CLASS_KNIGHT:
            f.skill3 = true;
            f.combo = true;
            break;
        case CLASS_WIZARD:
            f.skill3 = true;
            f.party = true;
            f.partyPage = SUB_PAGE_PARTY_CONFIG;
            break;
        case CLASS_ELF:
            f.skill3 = true;
            f.party = true;
            f.partyPage = SUB_PAGE_PARTY_CONFIG_ELF;
            f.autoHeal = true;
            f.potionPage = SUB_PAGE_POTION_CONFIG_ELF;
            break;
        case CLASS_DARK:
        case CLASS_RAGEFIGHTER:
            f.skill3 = true;
            break;
        case CLASS_DARK_LORD:
            f.pet = true;
            break;
        case CLASS_SUMMONER:
            f.skill3 = true;
            f.drainLife = true;
            f.potionSummoner = true;
            f.potionPage = SUB_PAGE_POTION_CONFIG_SUMMY;
            break;
        default:
            break;
        }
        return f;
    }

    void EnsureConditionDefaults(uint32_t& conditionBits)
    {
        using namespace MUHelper;

        if ((conditionBits & ~MUHELPER_SKILL_PRECON_CLEAR) == 0)
            conditionBits |= static_cast<uint32_t>(ON_MOBS_NEARBY);
        if ((conditionBits & ~MUHELPER_SKILL_SUBCON_CLEAR) == 0)
            conditionBits |= static_cast<uint32_t>(ON_MORE_THAN_TWO_MOBS);
    }
}
