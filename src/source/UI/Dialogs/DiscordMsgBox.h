// The Discord dialog: the state of the account link with its actions, and
// the one-time code to link the account. Shown when the server has the
// Discord integration (GameLogic::Discord::ServerIntegration), on the generic
// menu dialog (UI/Dialogs/GenericMenuDialog.h).
#pragma once

#include "Network/Discord/DiscordPackets.h"

namespace UI::Discord
{
// Opens the account dialog: whether the account is linked, which chats are
// mirrored; Link or Unlink, Join, Close.
void ShowAccount();

// Shows the code the server sent, copied to the clipboard; Copy, Join, Close.
// Says so instead when linking isn't available.
void ShowLinkCode(const Network::Discord::LinkCode& linkCode);
} // namespace UI::Discord
