#include "Core/Utilities/Log/MuLogger.h"

namespace
{
const auto g_earlyLogger = mu::log::Get("core");
}

std::shared_ptr<spdlog::logger> GetEarlyLogger()
{
    return g_earlyLogger;
}
