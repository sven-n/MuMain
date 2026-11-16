#include "TextureFailure.h"

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
    if (!ContinueAfterTextureError(forceDestroy, choseYes))
    {
        actions.requestQuit();
    }
}
} // namespace Render::Textures
