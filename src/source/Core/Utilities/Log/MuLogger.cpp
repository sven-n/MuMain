#include "stdafx.h"

#include "MuLogger.h"

#include <array>
#include <cstdio>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <spdlog/details/log_msg_buffer.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace mu::log
{
namespace
{
constexpr std::array<const char*, 11> kLoggerNames = {"core",  "network",  "render", "data",     "gameplay", "ui",
                                                      "audio", "platform", "dotnet", "gameshop", "scenes"};

// What is logged before Init names the log folder: kept for Init to write into the log, and echoed to stderr in
// case the process ends first. The first lines show what went wrong first, so later ones are only counted.
class EarlySink final : public spdlog::sinks::base_sink<std::mutex>
{
public:
    static constexpr std::size_t Capacity = 1024;

    // Sends every later line to target, after the kept ones when replay is set; returns how many were dropped.
    // Under the sink's lock, so a line another thread logs meanwhile still comes after the kept ones.
    std::size_t ForwardTo(spdlog::sink_ptr target, bool replay)
    {
        std::lock_guard lock(mutex_);
        if (replay)
        {
            try
            {
                for (const auto& line : m_lines)
                    target->log(line);
                target->flush();
            }
            catch (const spdlog::spdlog_ex&)
            {
                // A failed write loses the early lines, not the process.
            }
        }
        m_lines.clear();
        m_target = std::move(target);
        return m_dropped;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        if (m_target)
        {
            m_target->log(msg);
            return;
        }
        if (m_lines.size() < Capacity)
            m_lines.emplace_back(msg);
        else
            ++m_dropped;

        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        // Text-mode stderr writes the platform's line end for '\n' itself.
        std::string_view text(formatted.data(), formatted.size());
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
            text.remove_suffix(1);
        std::fwrite(text.data(), 1, text.size(), stderr);
        std::fputc('\n', stderr);
    }

    void flush_() override
    {
        if (m_target)
            m_target->flush();
        else
            std::fflush(stderr);
    }

private:
    std::vector<spdlog::details::log_msg_buffer> m_lines;
    std::size_t m_dropped = 0;
    spdlog::sink_ptr m_target;
};

struct LoggerState
{
    std::mutex mutex;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers;
    std::shared_ptr<EarlySink> early = std::make_shared<EarlySink>();
    std::shared_ptr<spdlog::sinks::dist_sink_mt> sink =
        std::make_shared<spdlog::sinks::dist_sink_mt>(std::vector<spdlog::sink_ptr>{early});
    bool explicitlyInitialized = false;
};

// Never destroyed: other statics log from their destructors, whatever order they were built in.
LoggerState& State()
{
    static auto* const state = new LoggerState();
    return *state;
}

std::shared_ptr<spdlog::logger> CreateLogger(LoggerState& state, const std::string& name)
{
    auto logger = std::make_shared<spdlog::logger>(name, state.sink);
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::err);
    spdlog::register_logger(logger);
    state.loggers.emplace(name, logger);
    return logger;
}

} // namespace

void Init(const std::filesystem::path& logDirectory)
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    if (state.explicitlyInitialized)
        return;

    const std::filesystem::path directory = logDirectory.empty() ? std::filesystem::current_path() : logDirectory;
    std::error_code error;
    std::filesystem::create_directories(directory, error);

    const std::filesystem::path file = directory / "MuError.log";
    std::string fileError;
    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file.string(), 5 * 1024 * 1024, 3);
    }
    catch (const spdlog::spdlog_ex& exception)
    {
        fileError = exception.what();
        sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
    }
    // Without a file the early lines are on stderr already.
    const std::size_t dropped = state.early->ForwardTo(sink, fileError.empty());
    state.sink->set_sinks({sink});

    for (const char* name : kLoggerNames)
    {
        if (!state.loggers.contains(name))
            CreateLogger(state, name);
    }
    const auto& core = state.loggers.at("core");
    if (dropped > 0)
        core->warn("{} log lines from before the log opened were dropped", dropped);
    if (!fileError.empty())
        core->warn("No log file: {} could not be opened ({}); logging to stderr", file.string(), fileError);
    state.early = std::make_shared<EarlySink>();

    state.explicitlyInitialized = true;
}

void Shutdown()
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    for (auto& entry : state.loggers)
        entry.second->flush();
    state.loggers.clear();
    state.sink->set_sinks({state.early});
    spdlog::shutdown();
    state.explicitlyInitialized = false;
}

std::shared_ptr<spdlog::logger> Get(const std::string& name)
{
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    const auto found = state.loggers.find(name);
    return found != state.loggers.end() ? found->second : CreateLogger(state, name);
}
} // namespace mu::log
