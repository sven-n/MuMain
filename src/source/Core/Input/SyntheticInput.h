// Injected key presses and mouse clicks for scripted control of the client.
//
// An injection is pushed onto SDL's own event queue, so it travels the exact
// path a physical key or click takes: the event loop routes it to the UI
// consumer first (RmlUi, which is driven by SDL events alone and would never
// see an injection written straight into the legacy input globals) and to the
// legacy readers when the UI leaves it unclaimed. `IsKeyDown` additionally
// consults the held key, because SDL's own keyboard and mouse state reports
// physical devices and is not moved by a pushed event.
//
// Sequencing follows rendered frames: `BeginFrame()` runs once per rendered
// frame (before the key-state scan) and advances one injection through
// press -> hold -> release, so a press and its release are always separated by
// a rendered frame -- the legacy readers consume press/release edges per frame
// and would miss a pair delivered in one pump. At most one injection is in
// flight at a time.
//
// No window means no event: before the game window exists (and in unit tests,
// where SDL is not initialised) an injection still walks its frames, but pushes
// nothing.
#pragma once

#include <cstddef>
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

// Schedules a press-and-release of the button at a window pixel, preceded by a
// pointer move there. False when another injection is still in flight.
[[nodiscard]] bool Click(float windowX, float windowY, MouseButton button);

// Schedules `text` (UTF-8) as typed characters for whatever field has keyboard
// focus, optionally followed by a Return press for fields that submit on it.
// A key press carries no character of its own, so this is the only way to fill
// a text field -- `PressKey` moves the caret, this writes into it. False when
// another injection is still in flight, or when the text is empty or longer
// than `MaxTypedTextBytes`.
[[nodiscard]] bool TypeText(std::string_view text, bool pressEnter);

// Longest `TypeText` accepts, in bytes. Any real field in the game (account,
// character, guild name, chat line) is far shorter.
inline constexpr std::size_t MaxTypedTextBytes = 256;

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

// Forgets the in-flight injection, releasing a key or button it had already
// pressed, so the UI is not left holding one.
// Called when the command that scheduled one gives up on it — a timeout, an
// interrupt, or a caller that disconnected — and by the tests. Compare
// `CurrentGeneration()` first: whoever calls this drops whatever is in
// flight, which may belong to another command.
void Reset();
} // namespace Core::Input::Synthetic
