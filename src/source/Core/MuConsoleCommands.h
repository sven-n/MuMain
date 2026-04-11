#pragma once

// MuConsoleCommands — handles in-game `$` commands from the chat input.
// Story 7.10.1: Extracted from CmuConsoleDebug::CheckCommand, which is being deleted.
// [VS0-CORE-MIGRATE-LOGGING]

#include <string>

namespace mu::console
{

// Process a `$` command entered in the chat input box.
// Returns true if the command was recognized and handled.
bool CheckCommand(const std::wstring& command);

} // namespace mu::console
