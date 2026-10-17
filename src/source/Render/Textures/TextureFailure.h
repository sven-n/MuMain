#pragma once

#include <string>

namespace Render::Textures
{
// The optional dialog allows continuing only when the player chooses to.
bool ContinueAfterTextureError(bool forceDestroy, bool choseYes);
struct TextureErrorActions
{
    bool (*showDialog)(const std::string& title, const std::string& message, bool forceDestroy);
    void (*closeConnection)();
    void (*destroySound)();
    void (*destroyWindow)();
};
void DispatchTextureError(const std::string& title, const std::string& message, bool forceDestroy,
                          const TextureErrorActions& actions);
[[noreturn]] void ExitAfterTextureError();
} // namespace Render::Textures
