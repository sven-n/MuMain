#pragma once

// MuLogger — unified leveled logging facade backed by spdlog.
// Story 7.10.1: Replaces CErrorReport, CmuConsoleDebug, LOG_CALL, fprintf(stderr).
// [VS0-CORE-MIGRATE-LOGGING]

#include <filesystem>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace mu::log
{

// Initialize the logging subsystem.
// Must be called once from MuMain() before any logging calls.
// Creates:
//   - Rotating file sink → logDir / "MuError.log" (512 KB per file, 3 backups)
//   - Colored stderr sink → warn+ messages
//   - Named loggers for each game subsystem
//   - Preserves g_errorReportFd for async-signal-safe crash handler writes
void Init(const std::filesystem::path& logDir);

// Shut down the logging subsystem (flush all sinks, drop all loggers).
void Shutdown();

// Retrieve a named logger. Returns the default logger if name is not registered.
[[nodiscard]] std::shared_ptr<spdlog::logger> Get(const std::string& name);

// Set the log level for a named logger at runtime.
// Returns true if the logger was found and updated.
bool SetLevel(const std::string& loggerName, spdlog::level::level_enum level);

// List all registered logger names and their current levels (for $loggers command).
// Returns pairs of (name, level_string).
[[nodiscard]] std::vector<std::pair<std::string, std::string>> ListLoggers();

// Async-signal-safe file descriptor for crash handler writes.
// Set in Init() after opening the same log file path with O_WRONLY|O_APPEND.
// Used by POSIX signal handlers (PosixSignalHandlers.cpp) via write(fd, ...).
// [VS0-QUAL-SIGNAL-HANDLERS]
extern volatile int g_errorReportFd;

} // namespace mu::log

// Convenience macros — compile to nothing when SPDLOG_ACTIVE_LEVEL is above the macro's level.
// Usage: MU_LOG_INFO(mu::log::Get("core"), "Loaded {} items", count);
#define MU_LOG_TRACE(logger, ...) SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define MU_LOG_DEBUG(logger, ...) SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define MU_LOG_INFO(logger, ...) SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
#define MU_LOG_WARN(logger, ...) SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define MU_LOG_ERROR(logger, ...) SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define MU_LOG_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)
