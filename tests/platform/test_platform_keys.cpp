// Story 1.2.1: Platform Abstraction Headers — PlatformKeys VK_* Constants
// RED PHASE: These tests will FAIL until PlatformKeys.h is implemented.

#include "PlatformKeys.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("AC-2: PlatformKeys mouse button constants", "[platform][keys]")
{
    REQUIRE(VK_LBUTTON == 0x01);
    REQUIRE(VK_RBUTTON == 0x02);
    REQUIRE(VK_MBUTTON == 0x04);
}

TEST_CASE("AC-2: PlatformKeys navigation constants", "[platform][keys]")
{
    REQUIRE(VK_ESCAPE == 0x1B);
    REQUIRE(VK_RETURN == 0x0D);
    REQUIRE(VK_SPACE == 0x20);
    REQUIRE(VK_TAB == 0x09);
    REQUIRE(VK_BACK == 0x08);
}

TEST_CASE("AC-2: PlatformKeys arrow and page constants", "[platform][keys]")
{
    REQUIRE(VK_LEFT == 0x25);
    REQUIRE(VK_UP == 0x26);
    REQUIRE(VK_RIGHT == 0x27);
    REQUIRE(VK_DOWN == 0x28);
    REQUIRE(VK_PRIOR == 0x21);
    REQUIRE(VK_NEXT == 0x22);
    REQUIRE(VK_END == 0x23);
    REQUIRE(VK_HOME == 0x24);
    REQUIRE(VK_INSERT == 0x2D);
    REQUIRE(VK_DELETE == 0x2E);
}

TEST_CASE("AC-2: PlatformKeys function key constants", "[platform][keys]")
{
    REQUIRE(VK_F1 == 0x70);
    REQUIRE(VK_F2 == 0x71);
    REQUIRE(VK_F3 == 0x72);
    REQUIRE(VK_F4 == 0x73);
    REQUIRE(VK_F5 == 0x74);
    REQUIRE(VK_F6 == 0x75);
    REQUIRE(VK_F7 == 0x76);
    REQUIRE(VK_F8 == 0x77);
    REQUIRE(VK_F9 == 0x78);
    REQUIRE(VK_F10 == 0x79);
    REQUIRE(VK_F11 == 0x7A);
    REQUIRE(VK_F12 == 0x7B);
}

TEST_CASE("AC-2: PlatformKeys modifier constants", "[platform][keys]")
{
    REQUIRE(VK_CONTROL == 0x11);
    REQUIRE(VK_SHIFT == 0x10);
    REQUIRE(VK_MENU == 0x12);
}

TEST_CASE("AC-2: PlatformKeys numpad constants", "[platform][keys]")
{
    REQUIRE(VK_NUMPAD0 == 0x60);
    REQUIRE(VK_NUMPAD1 == 0x61);
    REQUIRE(VK_NUMPAD2 == 0x62);
    REQUIRE(VK_NUMPAD3 == 0x63);
    REQUIRE(VK_NUMPAD4 == 0x64);
    REQUIRE(VK_NUMPAD5 == 0x65);
    REQUIRE(VK_NUMPAD6 == 0x66);
    REQUIRE(VK_NUMPAD7 == 0x67);
    REQUIRE(VK_NUMPAD8 == 0x68);
    REQUIRE(VK_NUMPAD9 == 0x69);
}
