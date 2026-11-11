#include "doctest.h"

#include "Core/Utilities/Log/WindowsConsole.h"

TEST_CASE("Console wrapper keeps portable state [core][console]")
{
    CHECK(leaf::OpenConsoleWindow(L"MuMain Test"));
    CHECK(leaf::OpenConsoleWindow(L"Ignored While Open"));
    CHECK(leaf::IsConsoleVisible());
    CHECK(leaf::GetConsoleTitle() == L"MuMain Test");

    leaf::SetConsoleTextColor(leaf::COLOR_YELLOW, leaf::COLOR_DARKBLUE);
    WORD background = leaf::COLOR_ERROR;
    CHECK(leaf::GetConsoleTextColorIndex(&background) == leaf::COLOR_YELLOW);
    CHECK(background == leaf::COLOR_DARKBLUE);
    CHECK_FALSE(leaf::SaveConsoleScreenBuffer(L"unsupported.txt"));

    leaf::CloseConsoleWindow();
    CHECK_FALSE(leaf::IsConsoleVisible());
}
