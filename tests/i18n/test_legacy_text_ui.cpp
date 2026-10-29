#include "App/stdafx.h"

#include <doctest.h>

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "I18N/All.h"
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Core/WindowManager.h" // CManager, which MasterLevel.h names without including it
#include "UI/Dialogs/HelpPages.h"
#include "UI/Dialogs/WindowMenu.h"
#include "UI/Events/GoldBowmanLena.h"
#include "UI/HUD/ChatInputBox.h"
#include "UI/HUD/MasterLevel.h"
#include "UI/HUD/QuickCommandWindow.h"
#include "UI/HUD/Skills/SkillTooltipModel.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "LegacyTextComparison.h"

namespace
{
// Every value a BYTE-based field (CLASS_TYPE, a mastery type, a quest state) can hold.
constexpr int kByteValueCount = 256;

// SkillTooltipModel.cpp's GLOBAL_TEXT_MASTERY_TYPE_BASE, and the element texts' block behind it
// (Ice, Poison, Lightning, Fire, Earth, Wind, Water; the next text is an unrelated item option).
constexpr int kLegacyMasteryTypeBase = 1080;
constexpr int kLegacyMasteryTypeBlockSize = 7;

// MasterLevel.cpp's SetMasterType() switch: CategoryTextIndex (the first column's text, the next
// two columns follow it) and ClassNameTextIndex; false for a class it left unchanged.
struct LegacyMasterTreeTextIds
{
    int category;
    int className;
};

bool LegacyMasterTreeTextIdsFor(CLASS_TYPE Class, LegacyMasterTreeTextIds& ids)
{
    switch (Class)
    {
    case CLASS_WIZARD:
    case CLASS_SOULMASTER:
    case CLASS_GRANDMASTER:
        ids = {1751, 1669};
        return true;
    case CLASS_KNIGHT:
    case CLASS_BLADEKNIGHT:
    case CLASS_BLADEMASTER:
        ids = {1755, 1668};
        return true;
    case CLASS_ELF:
    case CLASS_MUSEELF:
    case CLASS_HIGHELF:
        ids = {1759, 1670};
        return true;
    case CLASS_DARK:
    case CLASS_DUELMASTER:
        ids = {1763, 1671};
        return true;
    case CLASS_DARK_LORD:
    case CLASS_LORDEMPEROR:
        ids = {1767, 1672};
        return true;
    case CLASS_SUMMONER:
    case CLASS_BLOODYSUMMONER:
    case CLASS_DIMENSIONMASTER:
        ids = {3136, 1689};
        return true;
    case CLASS_RAGEFIGHTER:
    case CLASS_TEMPLENIGHT:
        ids = {3330, 3151};
        return true;
    default:
        return false;
    }
}

// HelpPages.cpp's BuildPage() as it read the legacy ids, one text per line (a spacer is empty).
std::vector<std::wstring> LegacyHelpPageTexts(int page)
{
    constexpr int kFirstFunctionKeyText = 121;
    constexpr int kFunctionKeyTextsBeforeExtras = 4;
    constexpr int kShippedKeyTextCount = 19;
    constexpr int kFirstChattingText = 141;
    constexpr int kChattingTextCount = 16;

    std::vector<std::wstring> texts{L""};
    if (page == UI::Help::KeyFunctionPage)
    {
        texts.push_back(I18N::Game::KeyFunction);
        texts.push_back(L"");
        for (int i = 0; i < kFunctionKeyTextsBeforeExtras; ++i)
            texts.push_back(I18N::Game::Lookup(kFirstFunctionKeyText + i));

        const wchar_t* const extraHelpLines[] = {
            I18N::Game::F8ToggleMonsterHPBar, I18N::Game::F9Toggle3DCamera,   I18N::Game::F10LockUnlockCameraZoom,
            I18N::Game::F11ResetCameraView,   I18N::Game::HomeToggleMUHelper, I18N::Game::JToggleChatCommands,
        };
        for (const wchar_t* line : extraHelpLines)
            texts.push_back(line);

        for (int i = kFunctionKeyTextsBeforeExtras; i < kShippedKeyTextCount; ++i)
            texts.push_back(I18N::Game::Lookup(kFirstFunctionKeyText + i));
    }
    else
    {
        texts.push_back(I18N::Game::ChattingInstructions);
        texts.push_back(L"");
        for (int i = 0; i < kChattingTextCount; ++i)
            texts.push_back(I18N::Game::Lookup(kFirstChattingText + i));
    }

    texts.push_back(L"");
    return texts;
}

// WindowMenu.cpp's kMenuTextIds, in CWindowMenu::MenuEntry order.
constexpr std::array<int, mu::ui::window::CWindowMenu::MENU_MAX_INDEX> kLegacyMenuTextIds{
    1741, 1742, 364, 1743, 3055, 3103
};

// QuickCommandWindow.cpp's kQuickCommandTextIds: trade, buy, party, follow, duel.
constexpr std::array<int, 5> kLegacyQuickCommandTextIds{943, 1124, 944, 948, 949};

// ChatInputBox.cpp's kHintText, in the button row's order.
constexpr std::array<int, 10> kLegacyChatButtonHintTextIds{
    1681, 1682, 1683, 3321, 1684, 1685, 750, 1686, 751, 752
};

// GoldBowmanLena.cpp's SyncView() loops: Lookup(700 + i) for three notes, Lookup(703 + j) for two.
constexpr int kLegacyFirstIntroNoteText = 700;
constexpr int kLegacyIntroNoteCount = 3;
constexpr int kLegacyFirstClosingNoteText = 703;
constexpr int kLegacyClosingNoteCount = 2;

// QuestMng.cpp's two nTextIndex switches in GetRequestRewardText(); -1 for a request type neither
// case group handled, which never looked a text up.
int LegacyEventMapRequestTextIndex(DWORD dwRequestType)
{
    switch (dwRequestType)
    {
    case QUEST_REQUEST_EVENT_MAP_MON_KILL:
        return 3074;
    case QUEST_REQUEST_EVENT_MAP_BLOOD_GATE:
        return 3077;
    case QUEST_REQUEST_EVENT_MAP_USER_KILL:
        return 3075;
    case QUEST_REQUEST_EVENT_MAP_DEVIL_POINT:
        return 3079;
    case QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD:
        return 3078;
    case QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS:
        return 3076;
    case QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL:
        return 3080;
    case QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION:
        return 3081;
    default:
        return -1;
    }
}

// The request types checked: two whole 16-bit ranges, so a type that only matches in its low bits shows up.
constexpr DWORD kRequestTypeCheckCount = 0x20000;

// MyQuestInfoWindow.cpp's nStateTextIndex for CSQuest::getCurrQuestState().
int LegacyJobChangeStateTextIndex(BYTE byState)
{
    int nStateTextIndex = 930;
    if (byState == QUEST_ING)
        nStateTextIndex = 931;
    else if (byState == QUEST_END)
        nStateTextIndex = 932;
    return nStateTextIndex;
}
} // namespace

TEST_CASE("skill tooltip mastery tags match the legacy element block, empty outside it [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int masteryType = 0; masteryType < kByteValueCount; ++masteryType)
        {
            const wchar_t* actual = UI::Skills::Tooltip::MasteryTypeText(static_cast<std::uint8_t>(masteryType));
            const wchar_t* expected = masteryType < kLegacyMasteryTypeBlockSize
                ? I18N::Game::Lookup(kLegacyMasteryTypeBase + masteryType)
                : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(actual, expected), locale << " " << masteryType);
        }
    });
}

TEST_CASE("master skill tree header texts match the legacy ids for every class value [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int classValue = 0; classValue < kByteValueCount; ++classValue)
        {
            const auto Class = static_cast<CLASS_TYPE>(classValue);
            const MasterTreeHeaderTexts* texts = mu::ui::window::CMasterLevel::GetHeaderTexts(Class);
            const bool hasTexts = texts != nullptr;
            LegacyMasterTreeTextIds ids{};
            if (!LegacyMasterTreeTextIdsFor(Class, ids))
            {
                CHECK_MESSAGE(!hasTexts, locale << " " << classValue);
                continue;
            }

            CHECK_MESSAGE(hasTexts, locale << " " << classValue);
            if (!hasTexts)
                continue;

            CHECK_MESSAGE(LegacyTextComparison::SameText(*texts->className, I18N::Game::Lookup(ids.className)),
                          locale << " " << classValue);
            for (int column = 0; column < MAX_MASTER_SKILL_CATEGORY; ++column)
            {
                CHECK_MESSAGE(LegacyTextComparison::SameText(*texts->categoryPoints[column],
                                                             I18N::Game::Lookup(ids.category + column)),
                              locale << " " << classValue << " column " << column);
            }
        }
    });
}

TEST_CASE("help pages match the legacy ids line by line [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int page : {UI::Help::KeyFunctionPage, UI::Help::ChattingPage})
        {
            const std::vector<UI::Help::PageLine> lines = UI::Help::BuildPage(page);
            const std::vector<std::wstring> expected = LegacyHelpPageTexts(page);
            CHECK_MESSAGE(lines.size() == expected.size(), locale << " page " << page);
            if (lines.size() != expected.size())
                continue;

            for (size_t line = 0; line < lines.size(); ++line)
            {
                CHECK_MESSAGE(LegacyTextComparison::SameText(lines[line].text.c_str(), expected[line].c_str()),
                              locale << " page " << page << " line " << line);
            }
        }
    });
}

TEST_CASE("window menu labels match the legacy ids [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int entry = 0; entry < mu::ui::window::CWindowMenu::MENU_MAX_INDEX; ++entry)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(mu::ui::window::CWindowMenu::GetMenuLabel(entry),
                                                         I18N::Game::Lookup(kLegacyMenuTextIds[entry])),
                          locale << " " << entry);
        }
    });
}

TEST_CASE("quick command labels match the legacy ids [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int index = 0; index < static_cast<int>(kLegacyQuickCommandTextIds.size()); ++index)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(mu::ui::window::CQuickCommandWindow::GetCommandLabel(index),
                                                         I18N::Game::Lookup(kLegacyQuickCommandTextIds[index])),
                          locale << " " << index);
        }
    });
}

TEST_CASE("chat input button hints match the legacy ids [i18n][legacy]")
{
    // SyncRmlModel() walks every button hint the model holds.
    constexpr size_t kButtonHintCount = std::tuple_size_v<decltype(mu::ui::window::ChatInputRmlModel::buttonHints)>;
    REQUIRE(kLegacyChatButtonHintTextIds.size() == kButtonHintCount);

    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (size_t index = 0; index < kButtonHintCount; ++index)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(mu::ui::window::CChatInputBox::GetButtonHint(index),
                                                         I18N::Game::Lookup(kLegacyChatButtonHintTextIds[index])),
                          locale << " " << index);
        }
    });
}

TEST_CASE("golden archer Rena notes match the legacy ids [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int i = 0; i < kLegacyIntroNoteCount; ++i)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(mu::ui::window::CGoldBowmanLena::GetIntroNote(i),
                                                         I18N::Game::Lookup(kLegacyFirstIntroNoteText + i)),
                          locale << " intro " << i);
        }
        for (int j = 0; j < kLegacyClosingNoteCount; ++j)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(mu::ui::window::CGoldBowmanLena::GetClosingNote(j),
                                                         I18N::Game::Lookup(kLegacyFirstClosingNoteText + j)),
                          locale << " closing " << j);
        }
    });
}

TEST_CASE("quest event map request formats match the legacy ids, empty for other types [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (DWORD type = 0; type < kRequestTypeCheckCount; ++type)
        {
            const int legacyId = LegacyEventMapRequestTextIndex(type);
            const wchar_t* expected = legacyId >= 0 ? I18N::Game::Lookup(legacyId) : L"";
            CHECK_MESSAGE(LegacyTextComparison::SameText(CQuestMng::GetEventMapRequestFormat(type), expected),
                          locale << " " << type);
        }
        CHECK_MESSAGE(LegacyTextComparison::SameText(CQuestMng::GetEventMapRequestFormat(UINT32_MAX), L""), locale);
    });
}

TEST_CASE("job change quest state lines match the legacy ids for every state value [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int state = 0; state < kByteValueCount; ++state)
        {
            const auto byState = static_cast<BYTE>(state);
            CHECK_MESSAGE(
                LegacyTextComparison::SameText(mu::ui::window::CMyQuestInfoWindow::GetJobChangeStateText(byState),
                                               I18N::Game::Lookup(LegacyJobChangeStateTextIndex(byState))),
                locale << " " << state);
        }
    });
}
