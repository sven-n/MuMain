#pragma once

#include "Core/Utilities/Log/MuLogger.h"

#include <spdlog/sinks/ostream_sink.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// What a log (such as "data" or "gameplay") gets while it lives.
class LogCapture
{
public:
    explicit LogCapture(const std::string& logName)
        : m_logger(mu::log::Get(logName)), m_sink(std::make_shared<spdlog::sinks::ostream_sink_st>(m_text))
    {
        m_logger->sinks().push_back(m_sink);
    }

    ~LogCapture()
    {
        std::vector<spdlog::sink_ptr>& sinks = m_logger->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), m_sink), sinks.end());
    }

    LogCapture(const LogCapture&) = delete;
    LogCapture& operator=(const LogCapture&) = delete;

    // The logged line that holds `part`; empty when none does.
    std::string LineWith(std::string_view part) const
    {
        std::istringstream lines(m_text.str());
        std::string line;
        while (std::getline(lines, line))
        {
            if (line.find(part) != std::string::npos)
            {
                return line;
            }
        }
        return {};
    }

private:
    std::ostringstream m_text;
    std::shared_ptr<spdlog::logger> m_logger;
    spdlog::sink_ptr m_sink;
};
