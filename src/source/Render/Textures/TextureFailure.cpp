#include "TextureFailure.h"

#include <cstdlib>

namespace Render::Textures
{
namespace
{
TextureErrorCollector s_collector = nullptr;
}

void SetTextureErrorCollector(TextureErrorCollector collector)
{
    s_collector = collector;
}

TextureErrorCollector GetTextureErrorCollector()
{
    return s_collector;
}

bool ContinueAfterTextureError(bool forceDestroy, bool choseYes)
{
    return !forceDestroy && choseYes;
}

void DispatchTextureError(const std::string& title, const std::string& message, bool forceDestroy,
                          const TextureErrorActions& actions)
{
    const bool choseYes = actions.showDialog(title, message, forceDestroy);
    if (ContinueAfterTextureError(forceDestroy, choseYes))
    {
        return;
    }
    actions.closeConnection();
    actions.destroySound();
    actions.destroyWindow();
    ExitAfterTextureError();
}

[[noreturn]] void ExitAfterTextureError()
{
    std::exit(EXIT_FAILURE);
}
} // namespace Render::Textures
