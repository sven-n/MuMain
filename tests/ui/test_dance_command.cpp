#include <doctest.h>

#include "I18N/All.h"
#include "UI/Chat/ChatInput.h"

#include <string>

TEST_CASE("the dance command is a case-insensitive token [ui][chat]")
{
    using UI::Chat::IsDanceCommand;

    CHECK(IsDanceCommand(L"/dance"));
    CHECK(IsDanceCommand(L"/DANCE"));
    CHECK(IsDanceCommand(L"/dAnce"));
    CHECK(IsDanceCommand(L" /dance"));
    CHECK(IsDanceCommand(L"/dance   "));
    CHECK(IsDanceCommand(L"/dance hi"));

    CHECK_FALSE(IsDanceCommand(L"/dancer"));
    CHECK_FALSE(IsDanceCommand(L"/dancefoo"));
    CHECK_FALSE(IsDanceCommand(L"come /dance"));
    CHECK_FALSE(IsDanceCommand(L""));
}

TEST_CASE("the dance word is a whole word and not a slash command [ui][chat]")
{
    using UI::Chat::ContainsDanceWord;

    const std::wstring word = I18N::Game::Dance;
    REQUIRE_FALSE(word.empty());

    CHECK(ContainsDanceWord(word));
    CHECK(ContainsDanceWord(std::wstring(L"I ") + word + L" today"));
    CHECK_FALSE(ContainsDanceWord(L"/" + word));
    CHECK_FALSE(ContainsDanceWord(word + L"r"));
}

TEST_CASE("the dance word matches inside kana and CJK sentences [ui][chat]")
{
    using UI::Chat::ContainsWord;

    CHECK(ContainsWord(L"一緒にダンス", L"ダンス"));
    CHECK(ContainsWord(L"ダンスしよう", L"ダンス"));
    CHECK(ContainsWord(L"一起舞蹈吧", L"舞蹈"));
    CHECK(ContainsWord(L"Танец", L"Танец"));
    CHECK_FALSE(ContainsWord(L"Танцевать", L"Танец"));
    CHECK_FALSE(ContainsWord(L"Dancer", L"Dance"));
}
