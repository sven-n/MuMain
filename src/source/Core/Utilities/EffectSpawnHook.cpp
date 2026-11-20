#include "stdafx.h"

#ifdef _EDITOR

#include "EffectSpawnHook.h"

namespace Core::EffectSpawnHook
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
} // namespace Core::EffectSpawnHook

#endif // _EDITOR
