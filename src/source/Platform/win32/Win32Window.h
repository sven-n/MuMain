#pragma once

#ifdef _WIN32

#include "../IPlatformWindow.h"

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

namespace mu
{

class Win32Window : public IPlatformWindow
{
public:
    Win32Window() = default;
    ~Win32Window() override = default;

    [[nodiscard]] bool Create(const char* title, int width, int height, uint32_t flags) override;
    void Destroy() override;
    [[nodiscard]] void* GetNativeHandle() const override;
    void SetTitle(const char* title) override;
    void SetSize(int width, int height) override;
    void SetFullscreen(bool fullscreen) override;
    void SetMouseGrab(bool grab) override;
    [[nodiscard]] bool GetDisplaySize(int& outWidth, int& outHeight) const override;
};

} // namespace mu

#endif // _WIN32
