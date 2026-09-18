#pragma once

union SDL_Event;
struct SDL_Window;

namespace Core::Input
{
    // Abstract seam between the platform's raw SDL input and whatever UI framework claims it
    // first. Exists so the call sites that decide "does this event/mouse-position belong to the
    // UI right now" (Winmain.cpp's event pump, gameplay's mouse-gating checks in Selection.cpp /
    // ZzzInterface.cpp) depend on this interface, not on a concrete UI framework type. RmlUi
    // (RmlUiRuntime) is the only implementation today, but nothing outside RmlUiRuntime itself
    // needs to know that -- SDL stays independent of RmlUi, and the UI framework could
    // theoretically be replaced by swapping the registered consumer alone.
    class IUiInputConsumer
    {
    public:
        virtual ~IUiInputConsumer() = default;

        // @return false if the UI claimed this event (an element under the cursor / holding
        // focus consumed it) -- the caller should skip its own legacy/gameplay handling for this
        // event. true means the UI did not consume it and normal handling should proceed. Matches
        // Rml::Context::Process*'s own "still propagating" convention so implementations can
        // forward it directly.
        virtual bool ProcessSdlEvent(SDL_Event& event, SDL_Window* window) = 0;

        // True while the mouse is hovering/pressed over any element the registered consumer owns.
        virtual bool IsMouseOverUI() const = 0;
    };

    // Registers the single active UI input consumer. Set once at startup (RmlUiRuntime::Create())
    // and cleared at shutdown (RmlUiRuntime::Destroy()); nullptr is a valid "no UI yet" state.
    void SetUiInputConsumer(IUiInputConsumer* consumer);

    // Routes one SDL event to the registered UI consumer, if any. Returns true (event still
    // propagating) when no consumer is registered, matching IUiInputConsumer::ProcessSdlEvent's
    // own "not consumed" convention -- callers don't need to null-check.
    bool RouteToUi(SDL_Event& event, SDL_Window* window);

    // True if the registered UI consumer reports the mouse is over UI. False if none registered.
    bool IsMouseOverUI();
}
