#include "Core/Utilities/Log/TargetLogSinks.h"

namespace mu::log
{
// The log file is where this target's log is read.
std::vector<spdlog::sink_ptr> TargetLogSinks()
{
    return {};
}
} // namespace mu::log
