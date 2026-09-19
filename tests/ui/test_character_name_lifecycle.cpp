#include "App/stdafx.h"

#include <doctest.h>

#include "Engine/Object/w_CharacterInfo.h"
#include "UI/Chat/Chat.h"

HWND g_hWnd = nullptr;

TEST_CASE("reused character slots clear the complete display name [ui][character_name]")
{
    CHARACTER character;
    std::fill(std::begin(character.ID), std::end(character.ID), L'x');

    character.ClearDisplayName();

    CHECK(std::all_of(std::begin(character.ID), std::end(character.ID), [](wchar_t value) { return value == L'\0'; }));
}

TEST_CASE("chat labels reject a reused owner slot with a different name [ui][character_name]")
{
    CHARACTER character;
    std::wcscpy(character.ID, L"Original NPC");

    CHECK(UI::Chat::HasCurrentOwnerName(L"Original NPC", character.ID));

    std::wcscpy(character.ID, L"Replacement NPC");

    CHECK_FALSE(UI::Chat::HasCurrentOwnerName(L"Original NPC", character.ID));
}
