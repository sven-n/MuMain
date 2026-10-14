#include "doctest.h"

#include "GameLogic/Items/VaultZenAmount.h"

using GameLogic::Items::kMaxVaultZen;
using GameLogic::Items::MaxVaultDeposit;
using GameLogic::Items::MaxVaultWithdraw;
using GameLogic::Items::ParseVaultZenAmount;

TEST_CASE("Vault zen input parses up to the vault limit and clamps above it")
{
    CHECK(ParseVaultZenAmount(L"") == 0);
    CHECK(ParseVaultZenAmount(L"99999999") == 99'999'999);
    CHECK(ParseVaultZenAmount(L"2100000000") == kMaxVaultZen);
    CHECK(ParseVaultZenAmount(L"2100000001") == kMaxVaultZen);
    CHECK(ParseVaultZenAmount(L"9999999999") == kMaxVaultZen);
}

TEST_CASE("Vault moves are limited by the zen the target can take")
{
    CHECK(MaxVaultDeposit(500, 0) == 500);
    CHECK(MaxVaultDeposit(2'000'000'000, 2'099'000'000) == 1'000'000);
    CHECK(MaxVaultDeposit(500, kMaxVaultZen) == 0);
    CHECK(MaxVaultWithdraw(1'200, 0) == 1'200);
    CHECK(MaxVaultWithdraw(kMaxVaultZen, 2'099'999'000) == 1'000);
    CHECK(MaxVaultWithdraw(500, kMaxVaultZen) == 0);
}
