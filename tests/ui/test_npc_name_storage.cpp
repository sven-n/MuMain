#include <cwchar>
#include <iterator>

#include <doctest.h>

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Core/Text/WideString.h"

TEST_CASE("monster name storage preserves the longest shipped translation [ui][npc_name]")
{
    constexpr wchar_t longestShippedName[] = L"Tercia de Encargados del Gremio Mercenario";
    wchar_t destination[MAX_MONSTER_NAME + 1]{};

    CHECK(Core::Text::CopyWideString(destination, std::size(destination), longestShippedName));
    CHECK(std::wcscmp(destination, longestShippedName) == 0);
}

TEST_CASE("wide string copy truncates and terminates [ui][npc_name]")
{
    wchar_t destination[5] = {L'x', L'x', L'x', L'x', L'x'};

    CHECK_FALSE(Core::Text::CopyWideString(destination, std::size(destination), L"abcdef"));
    CHECK(destination[0] == L'a');
    CHECK(destination[3] == L'd');
    CHECK(destination[4] == L'\0');
}

TEST_CASE("wide string copy rejects zero capacity [ui][npc_name]")
{
    wchar_t destination = L'x';

    CHECK_FALSE(Core::Text::CopyWideString(&destination, 0, L"abc"));
    CHECK(destination == L'x');
}
