#include "App/stdafx.h"

#include <doctest.h>

#include <algorithm>

#include "I18N/All.h"
#include "UI/Dialogs/HelpPages.h"

namespace
{
long CountHeadings(const std::vector<UI::Help::PageLine>& lines)
{
    return std::count_if(lines.begin(), lines.end(), [](const UI::Help::PageLine& line) { return line.heading; });
}
} // namespace

TEST_CASE("help key page: heading, F1-F4, engine hotkeys, then the shipped keys [ui][help]")
{
    const auto lines = UI::Help::BuildPage(UI::Help::KeyFunctionPage);

    // Spacer, heading, spacer, 4 + 6 + 15 key rows, spacer -- the original's TextList.
    REQUIRE(lines.size() == 29);
    CHECK(lines.front().halfSpacer);
    CHECK(lines[2].halfSpacer);
    CHECK(lines.back().halfSpacer);
    CHECK(lines[1].heading);
    CHECK(lines[1].text == I18N::Game::KeyFunction);
    CHECK(CountHeadings(lines) == 1);

    CHECK(lines[3].text == I18N::Game::F1HelpOnOff);
    CHECK(lines[6].text == I18N::Game::F4AdjustChatWindowSize);
    CHECK(lines[7].text == I18N::Game::F8ToggleMonsterHPBar);
    CHECK(lines[12].text == I18N::Game::JToggleChatCommands);
    CHECK(lines[13].text == I18N::Game::EnterChattingMode);
    CHECK(lines[27].text == I18N::Game::PrintScreenSaveScreenshots);
}

TEST_CASE("help chat page: heading and the sixteen chat rows [ui][help]")
{
    const auto lines = UI::Help::BuildPage(UI::Help::ChattingPage);

    REQUIRE(lines.size() == 20);
    CHECK(lines[1].heading);
    CHECK(lines[1].text == I18N::Game::ChattingInstructions);
    CHECK(CountHeadings(lines) == 1);
    CHECK(lines[3].text == I18N::Game::TabSwitchToTheIDWindow);
    CHECK(lines[18].text == I18N::Game::MoveCursorDownAdjustChatWindow);
    CHECK(lines.back().halfSpacer);
    CHECK(lines.back().text.empty());
}
