#pragma once


#include "../IPlatformEventLoop.h"

namespace mu
{

class SDLEventLoop : public IPlatformEventLoop
{
public:
    SDLEventLoop() = default;
    ~SDLEventLoop() override = default;

    [[nodiscard]] bool PollEvents() override;
};

} // namespace mu
