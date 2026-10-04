#include "stdafx.h"

#include "Guild/GuildUpdates.h"

#include "Engine/Object/ZzzInventory.h"
#include "Guild/GuildInfoWindow.h"
#include "UI/Core/WindowSystem.h"

#include <array>
#include <string>

namespace UI::Guild
{
void ClearNotices()
{
    g_pGuildInfoWindow->NoticeClear();
}

void AddNotice(std::wstring_view text)
{
    std::wstring notice(text);
    g_pGuildInfoWindow->AddGuildNotice(notice.data());
}

void ResetMembers(std::wstring_view rivalGuild)
{
    std::wstring rival(rivalGuild);
    g_pGuildInfoWindow->GuildClear();
    g_pGuildInfoWindow->UnionGuildClear();
    g_pGuildInfoWindow->SetRivalGuildName(rival.data());
}

void AddMember(int memberIndex)
{
    g_pGuildInfoWindow->AddGuildMember(&GuildList[memberIndex]);
}

void ShowRelationshipRequest(std::uint32_t relationshipType, std::uint32_t requestType, std::uint8_t requesterKeyH,
                             std::uint8_t requesterKeyL)
{
    g_pGuildInfoWindow->ReceiveGuildRelationShip(static_cast<GuildRelationshipType>(relationshipType),
                                                 static_cast<GuildRequestType>(requestType), requesterKeyH,
                                                 requesterKeyL);
}

int AllianceGuildCount()
{
    return g_pGuildInfoWindow->GetUnionCount();
}

void ClearAllianceGuilds()
{
    g_pGuildInfoWindow->UnionGuildClear();
}

void AddAllianceGuild(std::span<const std::uint8_t, 64> mark, std::wstring_view name, int memberCount)
{
    std::array<BYTE, 64> cells{};
    std::copy(mark.begin(), mark.end(), cells.begin());
    std::wstring guildName(name);
    g_pGuildInfoWindow->AddUnionList(cells.data(), guildName.data(), memberCount);
}
}
