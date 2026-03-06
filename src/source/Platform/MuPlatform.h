#pragma once

#include <cstdint>

namespace mu
{

class IPlatformWindow;

class MuPlatform
{
public:
    [[nodiscard]] static bool Initialize();
    static void Shutdown();

    [[nodiscard]] static bool CreateWindow(const char* title, int width, int height, uint32_t flags);
    [[nodiscard]] static IPlatformWindow* GetWindow();
    [[nodiscard]] static bool PollEvents();

private:
    MuPlatform() = delete;
};

} // namespace mu
