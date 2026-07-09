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

    static void SetFullscreen(bool fullscreen);
    static void SetMouseGrab(bool grab);
    [[nodiscard]] static bool GetDisplaySize(int& outWidth, int& outHeight);

private:
    MuPlatform() = delete;
};

} // namespace mu
