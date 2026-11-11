#include "stdafx.h"

#ifdef _EDITOR

#include "WorldClearing.h"

namespace Core::WorldClearing
{
namespace
{
Listener g_listener = nullptr;
}

void SetListener(Listener listener)
{
    g_listener = listener;
}

void Notify()
{
    if (g_listener != nullptr)
        g_listener();
}
} // namespace Core::WorldClearing

#endif // _EDITOR
