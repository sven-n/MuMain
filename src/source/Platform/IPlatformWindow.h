#pragma once

#include <cstdint>

namespace mu
{

constexpr uint32_t MU_WINDOW_FULLSCREEN = 0x1;

class IPlatformWindow
{
public:
    virtual ~IPlatformWindow() = default;

    [[nodiscard]] virtual bool Create(const char* title, int width, int height, uint32_t flags) = 0;
    virtual void Destroy() = 0;
    [[nodiscard]] virtual void* GetNativeHandle() const = 0;
    virtual void SetTitle(const char* title) = 0;
    virtual void SetSize(int width, int height) = 0;
};

} // namespace mu
