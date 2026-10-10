// Dispatches the Discord integration messages of the server (F5 group) to
// the parts of the client which show them.
#pragma once

#include "Core/Platform/WinCompat.h"

#include <span>

namespace Network::Discord
{
// Returns false for a sub code which isn't one of the Discord messages.
bool HandleMessage(BYTE subCode, std::span<const BYTE> packet);
} // namespace Network::Discord
