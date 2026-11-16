#pragma once

#include <spdlog/common.h>

#include <vector>

namespace mu::log
{
// Where the target's log is read besides MuError.log (a browser's console, Android's logcat), from the adapter its
// CMake picks (MU_TARGET_ADAPTERS). Desktop targets add none.
std::vector<spdlog::sink_ptr> TargetLogSinks();
} // namespace mu::log
