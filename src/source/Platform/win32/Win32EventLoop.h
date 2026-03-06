#pragma once

#ifdef _WIN32

#include "../IPlatformEventLoop.h"

namespace mu
{

class Win32EventLoop : public IPlatformEventLoop
{
public:
    Win32EventLoop() = default;
    ~Win32EventLoop() override = default;

    [[nodiscard]] bool PollEvents() override;
};

} // namespace mu

#endif // _WIN32
