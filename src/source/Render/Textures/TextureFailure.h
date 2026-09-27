#pragma once

namespace Render::Textures
{
// The optional dialog allows continuing only on an explicit yes response.
bool ContinueAfterTextureError(bool forceDestroy, bool choseYes);
struct TextureErrorActions
{
    bool (*showDialog)(const wchar_t* message, bool forceDestroy);
    void (*closeConnection)();
    void (*destroySound)();
    void (*destroyWindow)();
};
void DispatchTextureError(const wchar_t* message, bool forceDestroy, const TextureErrorActions& actions);
[[noreturn]] void ExitAfterTextureError();
} // namespace Render::Textures
