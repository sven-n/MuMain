// Scripted input is synchronously routed through the game window's UI-first
// event path once per rendered frame. UI-consumed input does not enter the
// legacy key/button readers; no SDL queue or OS pointer is changed.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

union SDL_Event;
struct SDL_Window;

namespace Core::Input::Synthetic
{
enum class MouseButton : std::uint8_t
{
    Left,
    Right,
};

#ifdef MU_ENABLE_CONTROL_SOCKET
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

// Committed UTF-8 text, optionally followed by a separately framed Return.
[[nodiscard]] bool TypeText(std::string_view text, bool enter);
[[nodiscard]] bool ValidText(std::string_view text);

// Main-loop delivery target. Events are delivered synchronously on the rendered
// frame; setting nullptr retracts pending work before the window is destroyed.
using EventDelivery = bool (*)(SDL_Event&, bool& propagates);
void SetEventDelivery(EventDelivery delivery, SDL_Window* window);
[[nodiscard]] bool DeliveryFailed();
void CancelDelivery();
void CancelForPhysicalButton(unsigned char button);

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

// Forgets the in-flight injection, retracting a button it had already pressed.
// Called when the command that scheduled one gives up on it — a timeout, an
// interrupt, or a caller that disconnected — and by the tests. Compare
// `CurrentGeneration()` first: whoever calls this drops whatever is in
// flight, which may belong to another command.
void Reset();
#else
// Without the control socket there is nothing to inject: the two calls the
// rest of the client makes (the frame advance and the held-key test) compile
// to nothing, exactly as the control taps do, so no call site needs a
// conditional and the injector itself is not built into a player client.
[[nodiscard]] inline bool IsKeyHeld(int)
{
    return false;
}

inline void BeginFrame() {}

inline void Reset() {}

inline void CancelForPhysicalButton(unsigned char) {}
inline void CancelDelivery() {}

using EventDelivery = bool (*)(SDL_Event&, bool&);
inline void SetEventDelivery(EventDelivery, SDL_Window*) {}
#endif
} // namespace Core::Input::Synthetic
