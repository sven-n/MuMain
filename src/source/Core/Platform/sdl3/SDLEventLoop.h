#pragma once

#ifdef MU_ENABLE_SDL3

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

#endif // MU_ENABLE_SDL3
