#include "Core/Utilities/Log/MuLogger.h"

#include <chrono>
#include <string>

namespace
{
// Unique per run, so a MuError.log an older run left in the working folder can't match it.
const std::string g_earlyLine =
    "logged before Init " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

const auto g_earlyLogger = []
{
    auto logger = mu::log::Get("core");
    MU_LOG_INFO(logger, "{}", g_earlyLine);
    return logger;
}();
} // namespace

std::shared_ptr<spdlog::logger> GetEarlyLogger()
{
    return g_earlyLogger;
}

std::string GetEarlyLine()
{
    return g_earlyLine;
}
