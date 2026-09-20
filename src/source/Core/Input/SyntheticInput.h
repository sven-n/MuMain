// Injected key presses and mouse clicks for scripted control of the client.
//
// An injection is applied below the game's own input readers — `IsKeyDown`
// consults the held key, a click writes the same mouse globals the event
// loop fills from real SDL events — so every handler reacts exactly as it
// does to a human. Nothing here touches the window system: no pointer
// movement, no focus change, no synthetic OS events.
//
// Sequencing follows rendered frames: `BeginFrame()` runs once per rendered
// frame (before the key-state scan) and advances one injection through
// press -> hold -> release. At most one injection is in flight at a time.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace Core::Input::Synthetic
{
enum class MouseButton : std::uint8_t
{
    Left,
    Right,
};

// Win32 virtual-key code for a key name (`esc`, `home`, `f1`, `a`, `7`, …),
// case-insensitive; empty when the name is unknown.
[[nodiscard]] std::optional<int> VirtualKeyFromName(std::string_view name);

// The key names `VirtualKeyFromName` accepts, for messages and usage.
[[nodiscard]] std::string_view KeyNames();

// Mouse button for `left` / `right`; empty for anything else.
[[nodiscard]] std::optional<MouseButton> MouseButtonFromName(std::string_view name);

// Schedules a one-frame press of the key. False when another injection is
// still in flight.
[[nodiscard]] bool PressKey(int virtualKey);

// Schedules a press-and-release of the button at a window pixel. False when
// another injection is still in flight.
[[nodiscard]] bool Click(float windowX, float windowY, MouseButton button);

// True while no injection is in flight; the command that scheduled one
// answers once this turns true again.
[[nodiscard]] bool IsIdle();

// Identifies the injection most recently accepted: every `PressKey` or
// `Click` that returns true gets a value of its own. A command reads it when
// its injection is scheduled and compares later, so it can tell its own
// injection from the next caller's.
[[nodiscard]] std::uint64_t CurrentGeneration();

// Whether the injected key is currently held; `IsKeyDown` ORs this in.
[[nodiscard]] bool IsKeyHeld(int virtualKey);

// Advances the in-flight injection by one rendered frame. Called once per
// rendered frame before the key-state scan.
void BeginFrame();

// Forgets the in-flight injection, releasing a button it had already pressed.
// Called when the command that scheduled one gives up on it — a timeout, an
// interrupt, or a caller that disconnected — and by the tests. Compare
// `CurrentGeneration()` first: whoever calls this drops whatever is in
// flight, which may belong to another command.
void Reset();
} // namespace Core::Input::Synthetic
