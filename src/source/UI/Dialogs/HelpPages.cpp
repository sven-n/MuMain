#include "stdafx.h"
#include "UI/Dialogs/HelpPages.h"

#include "I18N/All.h"

namespace
{
// F1..F4, then the rest of the shipped key list.
const wchar_t* const* const kShippedKeyTexts[] = {
    &I18N::Game::F1HelpOnOff,
    &I18N::Game::F2ChatWindowOnOff,
    &I18N::Game::F3WhisperModeWindowOnOff,
    &I18N::Game::F4AdjustChatWindowSize,
    &I18N::Game::EnterChattingMode,
    &I18N::Game::CCharacterInfo,
    &I18N::Game::IVInventory,
    &I18N::Game::PPartyWindow,
    &I18N::Game::GGuildWindow,
    &I18N::Game::QUseHealingPotion,
    &I18N::Game::WUseManaPotion,
    &I18N::Game::EUseAntidote,
    &I18N::Game::ShiftCharacterLockupKey,
    &I18N::Game::CtrlNumSetHotKeysForSkills,
    &I18N::Game::NumUseSkillHotKeys,
    &I18N::Game::AltShowDroppedItems,
    &I18N::Game::AltItemsSelectItemsInPrimaryOrder,
    &I18N::Game::CtrlClickPvPModeAttackOtherPlayers,
    &I18N::Game::PrintScreenSaveScreenshots,
};
constexpr int kFunctionKeyTextsBeforeExtras = 4;
constexpr int kShippedKeyTextCount = static_cast<int>(std::size(kShippedKeyTexts));

const wchar_t* const* const kChattingTexts[] = {
    &I18N::Game::TabSwitchToTheIDWindow,
    &I18N::Game::RightClickOnMsgWhisperingID,
    &I18N::Game::UpDownViewChattingHistory,
    &I18N::Game::PageUPPageDNScrollMessage,
    &I18N::Game::MsgMessageToPartyMembers,
    &I18N::Game::MsgMessageToGuildMembers,
    &I18N::Game::MsgPostingToGuildMembers,
    &I18N::Game::MsgMakeMessageAppearLonger,
    &I18N::Game::TradePointingPlayerTrade,
    &I18N::Game::PartyPointingPlayerFormAParty,
    &I18N::Game::GuildPointingPlayerFormAGuild,
    &I18N::Game::WarGuildNameDeclareGuildWar,
    &I18N::Game::ItemNameItemInfo,
    &I18N::Game::WarpWorldWarpToAnotherWorld,
    &I18N::Game::FilterWord1Word2ViewChatsWithWord,
    &I18N::Game::MoveCursorDownAdjustChatWindow,
};

void AddHalfSpacer(std::vector<UI::Help::PageLine>& lines)
{
    UI::Help::PageLine line;
    line.halfSpacer = true;
    lines.push_back(std::move(line));
}

void AddText(std::vector<UI::Help::PageLine>& lines, const wchar_t* text, bool heading = false)
{
    UI::Help::PageLine line;
    line.text = text;
    line.heading = heading;
    lines.push_back(std::move(line));
}

void AddKeyFunctionLines(std::vector<UI::Help::PageLine>& lines)
{
    for (int i = 0; i < kFunctionKeyTextsBeforeExtras; ++i)
        AddText(lines, *kShippedKeyTexts[i]);

    // Engine-added camera/MU Helper hotkey entries, inserted between F4 and the shipped entries.
    const wchar_t* const extraHelpLines[] = {
        I18N::Game::F8ToggleMonsterHPBar, I18N::Game::F9Toggle3DCamera,   I18N::Game::F10LockUnlockCameraZoom,
        I18N::Game::F11ResetCameraView,   I18N::Game::HomeToggleMUHelper, I18N::Game::JToggleChatCommands,
    };
    for (const wchar_t* line : extraHelpLines)
        AddText(lines, line);

    for (int i = kFunctionKeyTextsBeforeExtras; i < kShippedKeyTextCount; ++i)
        AddText(lines, *kShippedKeyTexts[i]);
}
} // namespace

std::vector<UI::Help::PageLine> UI::Help::BuildPage(int page)
{
    std::vector<PageLine> lines;
    AddHalfSpacer(lines);

    if (page == KeyFunctionPage)
    {
        AddText(lines, I18N::Game::KeyFunction, true);
        AddHalfSpacer(lines);
        AddKeyFunctionLines(lines);
    }
    else
    {
        AddText(lines, I18N::Game::ChattingInstructions, true);
        AddHalfSpacer(lines);
        for (const wchar_t* const* text : kChattingTexts)
            AddText(lines, *text);
    }

    AddHalfSpacer(lines);
    return lines;
}
