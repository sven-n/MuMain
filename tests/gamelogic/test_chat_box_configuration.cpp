// Tests of the chat box byte of the key configuration the server stores per character.

#include "doctest.h"

#include "GameLogic/KeyConfiguration/ChatBoxConfiguration.h"

#include <cstdint>

namespace ChatBox = GameLogic::KeyConfiguration::ChatBox;

TEST_CASE("the chat box byte stores the shown lines and the background alpha")
{
    const std::uint8_t chatBox = ChatBox::Encode(9, 0.6f);
    CHECK(chatBox == 0x36);
    CHECK(ChatBox::Lines(chatBox) == 9);
    CHECK(ChatBox::Alpha(chatBox) == doctest::Approx(0.6f));
    CHECK(ChatBox::Lines(ChatBox::Encode(15, 0.8f)) == 15);
    CHECK(ChatBox::Alpha(ChatBox::Encode(15, 0.2f + 0.2f)) == doctest::Approx(0.4f));
    // A zeroed byte: nothing to restore.
    CHECK(ChatBox::Lines(0) == 0);
    CHECK(ChatBox::Alpha(0) == 0.f);
}

TEST_CASE("the chat box a character starts with is stored as it is")
{
    // Entering the game resets the box to these (ResetKeyConfiguration); the first save must keep them.
    const std::uint8_t chatBox = ChatBox::Encode(ChatBox::DefaultLines, ChatBox::DefaultAlpha);
    CHECK(ChatBox::Lines(chatBox) == ChatBox::DefaultLines);
    CHECK(ChatBox::Alpha(chatBox) == doctest::Approx(ChatBox::DefaultAlpha));
}
