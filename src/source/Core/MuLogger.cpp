// MuLogger.cpp: unified leveled logging backed by spdlog.
// Story 7.10.1 [VS0-CORE-MIGRATE-LOGGING]

#include "stdafx.h"
#include "MuLogger.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifndef _MSC_VER
#include <fcntl.h>
#include <unistd.h>
#endif

namespace mu::log
{

// Async-signal-safe file descriptor for crash handler writes.
// Initialized to -1 (disabled). Set in Init() after the file sink is created.
// [VS0-QUAL-SIGNAL-HANDLERS]
volatile int g_errorReportFd = -1;

// All named loggers created during Init(). Order matches game subsystems.
static constexpr std::array kLoggerNames = {
    "core", "network", "render", "data", "gameplay", "ui", "audio", "platform", "dotnet", "gameshop", "scenes",
};

void Init(const std::filesystem::path& logDir)
{
    // Build the log file path (same filename as legacy CErrorReport).
    auto logPath = logDir / "MuError.log";

    // Rotating file sink: 512 KB per file, 3 rotated backups.
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.string(), 512 * 1024, 3);
    fileSink->set_level(spdlog::level::trace);

    // Colored stderr sink: warn+ messages for developer visibility.
    auto stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderrSink->set_level(spdlog::level::warn);

    std::vector<spdlog::sink_ptr> sinks = {fileSink, stderrSink};

    // Create and register all named loggers sharing the same sinks.
    for (const auto& name : kLoggerNames)
    {
        auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::warn);
        spdlog::register_logger(logger);
    }

    // Set the default logger to "core".
    spdlog::set_default_logger(spdlog::get("core"));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");

#ifndef _MSC_VER
    // Expose a raw file descriptor for async-signal-safe crash handler writes.
    // The POSIX signal handler in PosixSignalHandlers.cpp calls write(fd, ...)
    // which is the only async-signal-safe way to write to the log file.
    // [VS0-QUAL-SIGNAL-HANDLERS]
    int oldFd = g_errorReportFd;
    g_errorReportFd = -1; // Disable crash-handler writes during fd transition
    if (oldFd >= 0)
    {
        close(oldFd);
    }
    g_errorReportFd = open(logPath.string().c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
#endif
}

void Shutdown()
{
#ifndef _MSC_VER
    int fd = g_errorReportFd;
    g_errorReportFd = -1;
    if (fd >= 0)
    {
        close(fd);
    }
#endif
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> Get(const std::string& name)
{
    auto logger = spdlog::get(name);
    if (logger)
    {
        return logger;
    }
    // Fall back to the default logger if the name is not registered.
    // Log a one-time warning so typos are discoverable in the output.
    auto def = spdlog::default_logger();
    SPDLOG_LOGGER_WARN(def, "Logger '{}' not registered — using default", name);
    return def;
}

bool SetLevel(const std::string& loggerName, spdlog::level::level_enum level)
{
    auto logger = spdlog::get(loggerName);
    if (!logger)
    {
        return false;
    }
    logger->set_level(level);
    return true;
}

std::vector<std::pair<std::string, std::string>> ListLoggers()
{
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& name : kLoggerNames)
    {
        auto logger = spdlog::get(name);
        if (logger)
        {
            auto sv = spdlog::level::to_string_view(logger->level());
            result.emplace_back(name, std::string(sv.data(), sv.size()));
        }
    }
    return result;
}

} // namespace mu::log
