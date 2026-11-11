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

// For test binaries, which cannot show the dialog: while a collector is set,
// a texture error goes to it instead, and the loading goes on. The game never
// sets one.
using TextureErrorCollector = void (*)(const std::string& message);
void SetTextureErrorCollector(TextureErrorCollector collector);
TextureErrorCollector GetTextureErrorCollector();
} // namespace Render::Textures
