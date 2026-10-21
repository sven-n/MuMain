#include "doctest.h"

#include "GameLogic/Items/ZenAmount.h"

using GameLogic::Items::DigitCount;
using GameLogic::Items::kMaxInventoryZen;
using GameLogic::Items::kMaxVaultZen;
using GameLogic::Items::MaxVaultDeposit;
using GameLogic::Items::MaxVaultWithdraw;
using GameLogic::Items::ParseVaultZenAmount;

TEST_CASE("Vault zen input parses the leading digits")
{
    CHECK(ParseVaultZenAmount(L"") == 0);
    CHECK(ParseVaultZenAmount(L"99999999") == 99'999'999);
    CHECK(ParseVaultZenAmount(L"2100000000") == kMaxVaultZen);
    CHECK(ParseVaultZenAmount(L"12a34") == 12);
    CHECK(ParseVaultZenAmount(L"-5") == 0);
}

TEST_CASE("Vault zen input above the vault limit stays above it without overflowing")
{
    CHECK(ParseVaultZenAmount(L"2100000001") > kMaxVaultZen);
    CHECK(ParseVaultZenAmount(L"99999999999999999999") > kMaxVaultZen);
}

TEST_CASE("Digit count sizes the vault zen input")
{
    CHECK(DigitCount(0) == 1);
    CHECK(DigitCount(9) == 1);
    CHECK(DigitCount(10) == 2);
    CHECK(DigitCount(kMaxVaultZen) == 10);
}

TEST_CASE("Vault moves are limited by the zen the target can take")
{
    CHECK(MaxVaultDeposit(500, 0) == 500);
    CHECK(MaxVaultDeposit(2'000'000'000, 2'099'000'000) == 1'000'000);
    CHECK(MaxVaultDeposit(500, kMaxVaultZen) == 0);
    CHECK(MaxVaultWithdraw(1'200, 0) == 1'200);
    CHECK(MaxVaultWithdraw(kMaxVaultZen, 1'999'999'000) == 1'000);
    CHECK(MaxVaultWithdraw(500, kMaxInventoryZen) == 0);
}
