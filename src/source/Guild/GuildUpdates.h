#pragma once

#include <cstdint>
#include <span>
#include <string_view>

// Guild changes the server reports, applied to the guild window. Strings and spans are borrowed
// for the call.
namespace UI::Guild
{
void ClearNotices();
void AddNotice(std::wstring_view text);

// Empties the member and alliance lists before a fresh member list arrives.
void ResetMembers(std::wstring_view rivalGuild);
// GuildList[memberIndex], already filled in.
void AddMember(int memberIndex);

// Another guild master asks for an alliance or hostility; the values are the protocol's
// GuildRelationshipType and GuildRequestType.
void ShowRelationshipRequest(std::uint32_t relationshipType, std::uint32_t requestType, std::uint8_t requesterKeyH,
                             std::uint8_t requesterKeyL);

int AllianceGuildCount();
void ClearAllianceGuilds();
// `mark` is the guild's 8x8 emblem, one colour index per cell.
void AddAllianceGuild(std::span<const std::uint8_t, 64> mark, std::wstring_view name, int memberCount);
}
