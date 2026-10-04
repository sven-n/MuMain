#pragma once

#include <string>

namespace UI::Theme
{
enum class Persistence
{
    Session,
    Saved,
};

// Selects an installed UI theme and refreshes open windows. Saved also writes config.ini.
bool Select(const std::string& name, Persistence persistence);
}
