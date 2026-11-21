// doctest unit tests for the reply to a pick up request (C1 22): every reply,
// also a refusal without the zen amount, lets the next pick up be sent.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "pickup-reply"

#include "stdafx.h"
#include "doctest.h"

#include "Network/Server/WSclient.h"

#include <array>
#include <memory>
#include <span>

void ReceiveGetItem(std::span<const BYTE> ReceiveBuffer);

namespace
{
constexpr BYTE PickupCode = 0x22;
constexpr BYTE ZenReply = 0xFE;
constexpr BYTE RefusedReply = 0xFF;
constexpr int PendingDrop = 7;
constexpr int Gold = 0x01020304;

struct PickupScope
{
    std::unique_ptr<CHARACTER_MACHINE> machine = std::make_unique<CHARACTER_MACHINE>();
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    int previousSendGetItem = SendGetItem;

    PickupScope()
    {
        CharacterMachine = machine.get();
        CharacterMachine->Gold = Gold;
        SendGetItem = PendingDrop;
    }

    ~PickupScope()
    {
        CharacterMachine = previousMachine;
        SendGetItem = previousSendGetItem;
    }

    PickupScope(const PickupScope&) = delete;
    PickupScope& operator=(const PickupScope&) = delete;
};
} // namespace

TEST_CASE("A zen refusal without the amount lets the next pick up be sent [network][pickup-reply]")
{
    PickupScope scope;
    constexpr std::array<BYTE, 4> reply{0xC1, 4, PickupCode, ZenReply};

    ReceiveGetItem(reply);

    CHECK(SendGetItem == -1);
    CHECK(CharacterMachine->Gold == Gold);
}

TEST_CASE("The general refusal lets the next pick up be sent [network][pickup-reply]")
{
    PickupScope scope;
    constexpr std::array<BYTE, 4> reply{0xC1, 4, PickupCode, RefusedReply};

    ReceiveGetItem(reply);

    CHECK(SendGetItem == -1);
    CHECK(CharacterMachine->Gold == Gold);
}

TEST_CASE("A zen update sets the inventory money and lets the next pick up be sent [network][pickup-reply]")
{
    PickupScope scope;
    // Less than before, so nothing is announced and no chat or sound is needed.
    constexpr std::array<BYTE, 8> reply{0xC1, 8, PickupCode, ZenReply, 0x01, 0x02, 0x03, 0x00};

    ReceiveGetItem(reply);

    CHECK(SendGetItem == -1);
    CHECK(CharacterMachine->Gold == 0x01020300);
}
