#include <doctest.h>
#include "UI/Combat/GuardGuildLists.h"

using UI::Combat::GuardGuildLists;

TEST_CASE("guard declaration fixture preserves rank and own alliance filtering")
{
    GuardGuildLists lists;
    lists.AddDeclaration({L"Own", 4, false, 1});
    lists.AddDeclaration({L"Other", 9, false, 2});
    lists.AddDeclaration({L"Alliance", 6, true, 3});
    CHECK(lists.Declarations()[0].name == L"Own");
    lists.SortDeclarations();
    CHECK(lists.Declarations()[0].name == L"Other");
    CHECK(lists.Declarations()[1].name == L"Alliance");
    CHECK(lists.Declarations()[1].gaveUp);
    CHECK_FALSE(GuardGuildLists::IsOwnDeclaration(lists.Declarations()[0], L"Own", L"Alliance"));
    CHECK(GuardGuildLists::IsOwnDeclaration(lists.Declarations()[1], L"Own", L"Alliance"));
    CHECK(GuardGuildLists::IsOwnDeclaration(lists.Declarations()[2], L"Own", L"Alliance"));
    REQUIRE(lists.SelectDeclaration(L"Own"));
    CHECK(lists.SelectedDeclaration() == L"Own");
    lists.ClearDeclarations();
    CHECK_FALSE(lists.SelectDeclaration(L"Own"));
    CHECK(lists.SelectedDeclaration().empty());
}

TEST_CASE("guard siege fixture keeps score identity beyond a visible page")
{
    GuardGuildLists lists;
    for (int i = 0; i < 25; ++i)
        lists.AddSiegeGuild({L"Guild" + std::to_wstring(i), i == 0 ? 1 : 2, 1, i * 100});
    CHECK(lists.SiegeGuilds()[0].name == L"Guild0");
    CHECK(lists.SelectedSiegeGuild() == L"Guild0");
    CHECK(lists.ScoreGuild() == nullptr);
    REQUIRE(lists.SelectSiegeGuild(L"Guild20"));
    REQUIRE(lists.ScoreGuild());
    CHECK(lists.ScoreGuild()->score == 2000);
    CHECK(lists.SelectSiegeGuild(L"Guild20"));
    CHECK_FALSE(lists.SelectSiegeGuild(L"Absent"));
    REQUIRE(lists.ScoreGuild());
    CHECK(lists.ScoreGuild()->name == L"Guild20");
    REQUIRE(lists.SelectSiegeGuild(L"Guild0"));
    CHECK(lists.ScoreGuild()->joinSide == 1);
    lists.ClearSiegeGuilds();
    CHECK(lists.ScoreGuild() == nullptr);
    CHECK_FALSE(lists.SelectSiegeGuild(L"Guild20"));
}
