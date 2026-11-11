#pragma once

#include "Network/Server/GuildRelationshipResult.h"

namespace UI::Guild
{
// The chat text for an E6 guild relationship result, or nullptr when the client has none for it.
[[nodiscard]] const wchar_t* RelationshipResultText(const Network::Packets::GuildRelationshipResult& change);
} // namespace UI::Guild
