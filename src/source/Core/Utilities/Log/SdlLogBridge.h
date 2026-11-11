#pragma once

namespace Core::Log::Sdl
{
class ScopedLogOutput
{
public:
    ScopedLogOutput();
    ~ScopedLogOutput();

    ScopedLogOutput(const ScopedLogOutput&) = delete;
    ScopedLogOutput& operator=(const ScopedLogOutput&) = delete;

private:
    bool m_installed = false;
};
} // namespace Core::Log::Sdl
