#include "GlobalBitmapPath.h"

#include "Core/Platform/PathResolve.h"
#include "Core/Platform/WinCompat.h"

namespace Render::Sprites
{
std::string NarrowBitmapPath(const std::wstring& wide)
{
    const std::string utf8 = mu_wchar_to_utf8(wide.c_str());
#ifdef _WIN32
    return utf8;
#else
    return MuResolvePath(utf8.c_str());
#endif
}
} // namespace Render::Sprites
