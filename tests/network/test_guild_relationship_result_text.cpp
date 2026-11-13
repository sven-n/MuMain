// Tests of the chat text for OpenMU's E6 guild relationship result.

#include "Guild/GuildRelationshipResultText.h"
#include "I18N/Game.h"
#include "Network/Server/GuildRelationshipCodes.h"

#include "doctest.h"

#include <cstdint>

namespace GR = Network::Packets::GuildRelationship;
using Network::Packets::GuildRelationshipResult;
using UI::Guild::RelationshipResultText;

namespace
{
GuildRelationshipResult Change(std::uint8_t relationshipType, std::uint8_t requestType, std::uint8_t result)
{
    return GuildRelationshipResult{relationshipType, requestType, result, 0};
}
} // namespace

TEST_CASE("a successful alliance change shows what changed [network][guild]")
{
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GR::Success)) ==
          I18N::Game::GuildAllianceRegistrationIsSuccessful);
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Leave, GR::Success)) ==
          I18N::Game::GuildAllianceWithdrawalIsSuccessful);
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::DisbandAlliance, GR::Success)) ==
          I18N::Game::DisbandOfAllianceGuildOrRequest);
}

TEST_CASE("a successful hostility change shows whether it started or ended [network][guild]")
{
    CHECK(RelationshipResultText(Change(GR::Hostility, GR::Join, GR::Success)) == I18N::Game::HostileGuildIsConnected);
    CHECK(RelationshipResultText(Change(GR::Hostility, GR::Leave, GR::Success)) ==
          I18N::Game::HostileGuildIsDisconnected);
}

TEST_CASE("a failed change shows the reason [network][guild]")
{
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GR::Failed)) == I18N::Game::Failed);
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GUILD_ANS_CANCEL_REQUEST)) ==
          I18N::Game::RequestHasBeenCancelled);
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GUILD_ANS_EXCEED_MAX_UNION_MEMBER)) ==
          I18N::Game::MaximumNoOfGuildAllianceIs7);
}

TEST_CASE("0x10 shows the Castle Siege text, though OpenMU sends it for a missing guild too [network][guild]")
{
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, 0x10)) ==
          I18N::Game::AllianceFunctionWillBeRestrictedDueToTheCastleSiege);
}

TEST_CASE("a result the client has no text for gives none [network][guild]")
{
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GUILD_ANS_NOTEXIST_EXTRA_STATUS)) == nullptr);
    CHECK(RelationshipResultText(Change(GR::Alliance, GR::Join, GUILD_ANS_NOTEXIST_EXTRA_TYPE)) == nullptr);
    CHECK(RelationshipResultText(Change(GR::Alliance, 0x03, GR::Success)) == nullptr);
}
