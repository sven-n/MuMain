#pragma once

#ifdef MU_ENABLE_SDL3

#include "../IPlatformWindow.h"

struct SDL_Window;

namespace mu
{

class SDLWindow : public IPlatformWindow
{
public:
    SDLWindow() = default;
    ~SDLWindow() override;

    [[nodiscard]] bool Create(const char* title, int width, int height, uint32_t flags) override;
    void Destroy() override;
    [[nodiscard]] void* GetNativeHandle() const override;
    void SetTitle(const char* title) override;
    void SetSize(int width, int height) override;
    void SetFullscreen(bool fullscreen) override;
    void SetMouseGrab(bool grab) override;
    [[nodiscard]] bool GetDisplaySize(int& outWidth, int& outHeight) const override;

private:
    SDL_Window* m_pWindow = nullptr;
};

} // namespace mu

#endif // MU_ENABLE_SDL3
