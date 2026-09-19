#pragma once

namespace mu
{

class IPlatformEventLoop
{
public:
    virtual ~IPlatformEventLoop() = default;

    /// Pumps all pending events. Returns false if quit was requested.
    [[nodiscard]] virtual bool PollEvents() = 0;
};

} // namespace mu
