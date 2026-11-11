#include "stdafx.h"

#ifdef _EDITOR

#include "AssetLoadWorld.h"

namespace Core::AssetLoadWorld
{
namespace
{
Source g_source = nullptr;
}

void SetSource(Source source)
{
    g_source = source;
}

int Get()
{
    return g_source != nullptr ? g_source() : LoadingScreen;
}
} // namespace Core::AssetLoadWorld

#endif // _EDITOR
