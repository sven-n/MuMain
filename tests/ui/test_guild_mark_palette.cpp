#include <doctest.h>

#include "Guild/GuildMarkPalette.h"

TEST_CASE("guild mark cells use CreateGuildMark's palette, index 0 transparent [ui][guild]")
{
    using Guild::MarkPalette::CellColor;
    CHECK(CellColor(0) == "#00000000");
    CHECK(CellColor(1) == "#000000ff");
    CHECK(CellColor(3) == "#ffffffff");
    CHECK(CellColor(4) == "#ff0000ff");
    CHECK(CellColor(11) == "#0080ffff");
    CHECK(CellColor(15) == "#ff0080ff");
    CHECK(CellColor(16) == "#00000000");
    CHECK(CellColor(-1) == "#00000000");
}
