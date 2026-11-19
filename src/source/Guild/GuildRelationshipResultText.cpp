#include "Guild/GuildRelationshipResultText.h"

#include "Core/Globals/Defined_Global.h"
#include "I18N/Game.h"
#include "Network/Server/GuildRelationshipCodes.h"

namespace UI::Guild
{
namespace
{
namespace GR = Network::Packets::GuildRelationship;

const wchar_t* SuccessText(const Network::Packets::GuildRelationshipResult& change)
{
    if (change.relationshipType != GR::Alliance)
    {
        return change.requestType == GR::Join ? I18N::Game::HostileGuildIsConnected
                                              : I18N::Game::HostileGuildIsDisconnected;
    }

    switch (change.requestType)
    {
    case GR::Join:
        return I18N::Game::GuildAllianceRegistrationIsSuccessful;
    case GR::Leave:
        return I18N::Game::GuildAllianceWithdrawalIsSuccessful;
    case GR::DisbandAlliance:
        return I18N::Game::DisbandOfAllianceGuildOrRequest;
    default:
        return nullptr;
    }
}

const wchar_t* FailureText(std::uint8_t result)
{
    switch (result)
    {
    case GR::Failed:
        return I18N::Game::Failed;
    case GUILD_ANS_UNIONFAIL_BY_CASTLE:
        return I18N::Game::AllianceFunctionWillBeRestrictedDueToTheCastleSiege;
    case GUILD_ANS_NOTEXIST_PERMISSION:
        return I18N::Game::NoAuthorization;
    case GUILD_ANS_EXIST_RELATIONSHIP_UNION:
        return I18N::Game::AllianceGuild;
    case GUILD_ANS_EXIST_RELATIONSHIP_RIVAL:
        return I18N::Game::HostileGuild;
    case GUILD_ANS_EXIST_UNION:
        return I18N::Game::GuildAllianceExists;
    case GUILD_ANS_EXIST_RIVAL:
        return I18N::Game::HostileGuildExists;
    case GUILD_ANS_NOTEXIST_UNION:
        return I18N::Game::GuildAllianceDoesNotExist;
    case GUILD_ANS_NOTEXIST_RIVAL:
        return I18N::Game::HostileGuildDoesNotExist;
    case GUILD_ANS_NOT_UNION_MASTER:
        return I18N::Game::NotAMasterOfGuildAlliance;
    case GUILD_ANS_NOT_GUILD_RIVAL:
        return I18N::Game::Income;
    case GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD:
        return I18N::Game::IncompleteRequirementsForCreatingAGuildAlliance;
    case GUILD_ANS_EXCEED_MAX_UNION_MEMBER:
        return I18N::Game::MaximumNoOfGuildAllianceIs7;
    case GUILD_ANS_CANCEL_REQUEST:
        return I18N::Game::RequestHasBeenCancelled;
#ifdef ASG_ADD_GENS_SYSTEM
    case GUILD_ANS_UNION_MASTER_NOT_GENS:
        return I18N::Game::TheAllianceMasterHasNotJoinedTheGens;
    case GUILD_ANS_GUILD_MASTER_NOT_GENS:
        return I18N::Game::TheGuildMasterHasNotJoinedTheGens;
    case GUILD_ANS_UNION_MASTER_DISAGREE_GENS:
        return I18N::Game::YouAreWithADifferentGensThanTheAllianceMaster;
#endif // ASG_ADD_GENS_SYSTEM
    default:
        return nullptr;
    }
}
} // namespace

const wchar_t* RelationshipResultText(const Network::Packets::GuildRelationshipResult& change)
{
    return change.result == GR::Success ? SuccessText(change) : FailureText(change.result);
}
} // namespace UI::Guild
