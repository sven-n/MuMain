#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace mu::log
{
void Init(const std::filesystem::path& logDirectory = {});
void Shutdown();
std::shared_ptr<spdlog::logger> Get(const std::string& name);

inline volatile int g_errorReportFd = -1;
} // namespace mu::log

#define MU_LOG_TRACE(logger, ...) SPDLOG_LOGGER_TRACE((logger), __VA_ARGS__)
#define MU_LOG_DEBUG(logger, ...) SPDLOG_LOGGER_DEBUG((logger), __VA_ARGS__)
#define MU_LOG_INFO(logger, ...) SPDLOG_LOGGER_INFO((logger), __VA_ARGS__)
#define MU_LOG_WARN(logger, ...) SPDLOG_LOGGER_WARN((logger), __VA_ARGS__)
#define MU_LOG_ERROR(logger, ...) SPDLOG_LOGGER_ERROR((logger), __VA_ARGS__)
#define MU_LOG_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL((logger), __VA_ARGS__)
