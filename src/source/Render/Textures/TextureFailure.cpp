#include "TextureFailure.h"

#include <cstdlib>

namespace Render::Textures
{
bool ContinueAfterTextureError(bool forceDestroy, bool choseYes)
{
    return !forceDestroy && choseYes;
}

void DispatchTextureError(const wchar_t* message, bool forceDestroy, const TextureErrorActions& actions)
{
    const bool choseYes = actions.showDialog(message, forceDestroy);
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
