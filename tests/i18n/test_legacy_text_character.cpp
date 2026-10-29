#include "App/stdafx.h"

#include <doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "I18N/All.h"
#include "Character/CharInfoBalloon.h"
#include "Character/CharMakeWin.h"
#include "Character/CharacterManager.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Items/CComGem.h"
#include "LegacyTextComparison.h"

namespace
{
// CharMakeWin.cpp's kClassButtonTextIds, indexed by CLASS_*.
constexpr std::array<int, MAX_CLASS> kLegacyClassButtonTextIds{
    20, 21, 22, 23, 24, 1687, 3150
};

// CharMakeWin.cpp's ResolveDescriptionTextId().
int LegacyDescriptionTextId(CLASS_TYPE selectedClass)
{
    constexpr int kSummonerDescriptionTextId = 1690;
    constexpr int kRageFighterDescriptionTextId = 3152;
    constexpr int kDefaultDescriptionBase = 1705;

    if (selectedClass == CLASS_SUMMONER)
        return kSummonerDescriptionTextId;
    if (selectedClass == CLASS_RAGEFIGHTER)
        return kRageFighterDescriptionTextId;
    return kDefaultDescriptionBase + selectedClass;
}

// CharacterManager.cpp's kClassTextEntries and kDefaultClassTextIndex, as GetCharacterClassText() read them.
int LegacyClassTextIndex(CLASS_TYPE byCharacterClass)
{
    struct ClassTextEntry
    {
        CLASS_TYPE type;
        int textIndex;
    };

    constexpr int kDefaultClassTextIndex = 2305;

    constexpr std::array<ClassTextEntry, 18> kClassTextEntries{{
        { CLASS_WIZARD, 20 },
        { CLASS_SOULMASTER, 25 },
        { CLASS_GRANDMASTER, 1669 },
        { CLASS_KNIGHT, 21 },
        { CLASS_BLADEKNIGHT, 26 },
        { CLASS_BLADEMASTER, 1668 },
        { CLASS_ELF, 22 },
        { CLASS_MUSEELF, 27 },
        { CLASS_HIGHELF, 1670 },
        { CLASS_DARK, 23 },
        { CLASS_DUELMASTER, 1671 },
        { CLASS_DARK_LORD, 24 },
        { CLASS_LORDEMPEROR, 1672 },
        { CLASS_SUMMONER, 1687 },
        { CLASS_BLOODYSUMMONER, 1688 },
        { CLASS_DIMENSIONMASTER, 1689 },
        { CLASS_RAGEFIGHTER, 3150 },
        { CLASS_TEMPLENIGHT, 3151 },
    }};

    const auto it = std::find_if(kClassTextEntries.begin(), kClassTextEntries.end(),
        [byCharacterClass](const ClassTextEntry& entry) { return entry.type == byCharacterClass; });
    return (it != kClassTextEntries.end()) ? it->textIndex : kDefaultClassTextIndex;
}

// CharInfoBalloon.cpp's ResolveGuildTextIndex().
int LegacyGuildTextIndex(std::uint8_t guildStatus)
{
    struct GuildStatusText
    {
        std::uint8_t status;
        int textIndex;
    };

    constexpr std::array<GuildStatusText, 5> kGuildStatusTexts{ {
        {   0, 1330 },
        {  32, 1302 },
        {  64, 1301 },
        { 128, 1300 },
        { 255, 488 },
    } };

    const auto it = std::lower_bound(
        kGuildStatusTexts.begin(),
        kGuildStatusTexts.end(),
        guildStatus,
        [](const GuildStatusText& entry, std::uint8_t status) { return entry.status < status; });

    return (it != kGuildStatusTexts.end() && it->status == guildStatus) ? it->textIndex : 0;
}

// PlayerNpcStopAnimationSetting()'s iTextIndices for the world (none outside Devias and Lorencia).
std::vector<int> LegacyPlayerNpcChatTextIndices(int world)
{
    if (world == WD_2DEVIAS)
        return { 904, 905 };
    if (world == WD_0LORENCIA)
        return { 823 };
    return {};
}

// COMGEM::GetJewelIndex(_nJewel, eGEM_NAME).
int LegacyJewelNameIndex(int _nJewel)
{
    switch (_nJewel)
    {
    case COMGEM::eBLESS:
        return 1806;
    case COMGEM::eBLESS_C:
        return 1806;
    case COMGEM::eSOUL:
        return 1807;
    case COMGEM::eSOUL_C:
        return 1807;
    case COMGEM::eLIFE:
        return 3312;
    case COMGEM::eLIFE_C:
        return 3312;
    case COMGEM::eCREATE:
        return 3313;
    case COMGEM::eCREATE_C:
        return 3313;
    case COMGEM::ePROTECT:
        return 3314;
    case COMGEM::ePROTECT_C:
        return 3314;
    case COMGEM::eGEMSTONE:
        return 2081;
    case COMGEM::eGEMSTONE_C:
        return 2081;
    case COMGEM::eHARMONY:
        return 3315;
    case COMGEM::eHARMONY_C:
        return 3315;
    case COMGEM::eCHAOS:
        return 3316;
    case COMGEM::eCHAOS_C:
        return 3316;
    case COMGEM::eLOW:
        return 3317;
    case COMGEM::eLOW_C:
        return 3317;
    case COMGEM::eUPPER:
        return 3318;
    case COMGEM::eUPPER_C:
        return 3318;
    }

    return -1;
}

// ShowGemIntegrationJewelDialog()'s nBtnIndex, one button per plain jewel type.
constexpr std::array<int, COMGEM::eGEMTYPE_END> kLegacyJewelButtonTextIds{
    1806, 1807, 3312, 3313, 3314, 2081, 3315, 3316, 3317, 3318
};

// Every value a BYTE-based CLASS_TYPE or a guild status byte can hold.
constexpr int kByteValueCount = 256;
} // namespace

TEST_CASE("character creation job names match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // The dialog only shows the MAX_CLASS creatable classes.
        for (int classIndex = 0; classIndex < MAX_CLASS; ++classIndex)
        {
            const auto classType = static_cast<CLASS_TYPE>(classIndex);
            CHECK_MESSAGE(LegacyTextComparison::SameText(CCharMakeWin::GetJobName(classType),
                                                         I18N::Game::Lookup(kLegacyClassButtonTextIds[classIndex])),
                          locale << " " << classIndex);
        }
    });
}

TEST_CASE("character creation job descriptions match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // The dialog only selects the MAX_CLASS creatable classes.
        for (int classIndex = 0; classIndex < MAX_CLASS; ++classIndex)
        {
            const auto classType = static_cast<CLASS_TYPE>(classIndex);
            CHECK_MESSAGE(LegacyTextComparison::SameText(CCharMakeWin::GetJobDescription(classType),
                                                         I18N::Game::Lookup(LegacyDescriptionTextId(classType))),
                          locale << " " << classIndex);
        }
    });
}

TEST_CASE("character class texts match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int value = 0; value < kByteValueCount; ++value)
        {
            const auto classType = static_cast<CLASS_TYPE>(value);
            CHECK_MESSAGE(LegacyTextComparison::SameText(gCharacterManager.GetCharacterClassText(classType),
                                                         I18N::Game::Lookup(LegacyClassTextIndex(classType))),
                          locale << " " << value);
        }
    });
}

TEST_CASE("character balloon guild statuses match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int value = 0; value < kByteValueCount; ++value)
        {
            const auto guildStatus = static_cast<std::uint8_t>(value);
            CHECK_MESSAGE(LegacyTextComparison::SameText(CCharInfoBalloon::GetGuildStatusText(guildStatus),
                                                         I18N::Game::Lookup(LegacyGuildTextIndex(guildStatus))),
                          locale << " " << value);
        }
    });
}

TEST_CASE("player NPC chat lines match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int world = -1; world <= NUM_WD + 1; ++world)
        {
            const std::span<const wchar_t* const* const> texts = GetPlayerNpcChatTexts(world);
            const std::vector<int> legacyIndices = LegacyPlayerNpcChatTextIndices(world);
            CHECK_MESSAGE(texts.size() == legacyIndices.size(), locale << " " << world);
            for (std::size_t i = 0; i < std::min(texts.size(), legacyIndices.size()); ++i)
            {
                CHECK_MESSAGE(LegacyTextComparison::SameText(*texts[i], I18N::Game::Lookup(legacyIndices[i])),
                              locale << " " << world << "/" << i);
            }
        }
    });
}

TEST_CASE("jewel names match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // COMGEM::m_cGemType is a char; Check_Jewel() answers an eGEMTYPE or NOGEM.
        for (int jewel = -128; jewel < kByteValueCount; ++jewel)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(COMGEM::GetJewelName(jewel),
                                                         I18N::Game::Lookup(LegacyJewelNameIndex(jewel))),
                          locale << " " << jewel);
        }
    });
}

TEST_CASE("jewel combination buttons match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // ShowGemIntegrationJewelDialog(): button i selects the plain jewel type i * 2.
        for (int i = 0; i < COMGEM::eGEMTYPE_END; ++i)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(COMGEM::GetJewelName(i * 2),
                                                         I18N::Game::Lookup(kLegacyJewelButtonTextIds[i])),
                          locale << " " << i);
        }
    });
}
