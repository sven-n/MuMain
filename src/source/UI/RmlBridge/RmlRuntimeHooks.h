#pragma once

// Installs the game's side of RmlUiRuntime (RmlUiRuntimeHooks): the UI scale rule, native text,
// main-scene document suspension, render-target textures and the diagnostics overlay. Call once,
// before RmlUiRuntime::Create().
namespace UI::RmlBridge
{
void InstallRuntimeHooks();
}
