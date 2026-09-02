#pragma once

#include "Core/Platform/WinCompat.h"

#include <string_view>

namespace Core::Platform
{
[[nodiscard]] bool ParseServerPort(std::wstring_view value, WORD& port);
}
