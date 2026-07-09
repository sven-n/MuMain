#pragma once

#include <memory>
#include <string>

namespace mu::log
{

class Logger
{
public:
    template <typename... Args>
    void trace(const char*, Args&&...)
    {
    }

    template <typename... Args>
    void debug(const char*, Args&&...)
    {
    }

    template <typename... Args>
    void info(const char*, Args&&...)
    {
    }

    template <typename... Args>
    void warn(const char*, Args&&...)
    {
    }

    template <typename... Args>
    void error(const char*, Args&&...)
    {
    }
};

inline std::shared_ptr<Logger> Get(const std::string&)
{
    static auto logger = std::make_shared<Logger>();
    return logger;
}

inline volatile int g_errorReportFd = -1;

} // namespace mu::log
