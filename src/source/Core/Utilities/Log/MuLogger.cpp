#include "stdafx.h"

#include "MuLogger.h"

#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace mu::log
{
namespace
{
constexpr std::array<const char*, 11> kLoggerNames = {
    "core", "network", "render", "data", "gameplay", "ui",
    "audio", "platform", "dotnet", "gameshop", "scenes"
};

std::mutex g_mutex;
std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> g_loggers;
std::vector<spdlog::sink_ptr> g_sinks;
bool g_initialized = false;

std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name)
{
    auto logger = std::make_shared<spdlog::logger>(name, g_sinks.begin(), g_sinks.end());
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::err);
    spdlog::register_logger(logger);
    g_loggers.emplace(name, logger);
    return logger;
}
} // namespace

void Init(const std::filesystem::path& logDirectory)
{
    std::lock_guard lock(g_mutex);
    if (g_initialized)
        return;

    const std::filesystem::path directory = logDirectory.empty() ? std::filesystem::current_path() : logDirectory;
    std::error_code error;
    std::filesystem::create_directories(directory, error);

    g_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    g_sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(directory / "MuError.log", 5 * 1024 * 1024, 3));

    for (const char* name : kLoggerNames)
        CreateLogger(name);

    g_initialized = true;
}

void Shutdown()
{
    std::lock_guard lock(g_mutex);
    for (auto& [name, logger] : g_loggers)
        logger->flush();
    g_loggers.clear();
    g_sinks.clear();
    spdlog::shutdown();
    g_initialized = false;
}

std::shared_ptr<spdlog::logger> Get(const std::string& name)
{
    std::lock_guard lock(g_mutex);
    if (!g_initialized)
    {
        const std::filesystem::path directory = std::filesystem::current_path();
        g_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        g_sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(directory / "MuError.log", 5 * 1024 * 1024, 3));
        g_initialized = true;
    }

    const auto found = g_loggers.find(name);
    return found != g_loggers.end() ? found->second : CreateLogger(name);
}
} // namespace mu::log
