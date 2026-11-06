#pragma once

#include <RmlUi/Core/EventListener.h>

#include <functional>
#include <string>
#include <string_view>

namespace Rml
{
class Context;
class Element;
} // namespace Rml

// Click sounds by style (sound editor design, Q-UI1): a theme's style sheet
// gives a button a click kind,
//
//   .btn-ok { click-sound: confirm; }
//
// and Data/Audio/SoundEvents.json's "interfaceClick" event picks the kind's
// sound (the Sound Editor's Events tab). "none" silences an element inside
// one that has a kind. A key that stands for a button plays the button's
// kind (PlayClickSoundOf).
namespace UI::RmlBridge
{
inline constexpr const char* ClickSoundProperty = "click-sound";

// Makes the property known to the style sheets: after Rml::Initialise and
// before a style sheet that names it loads. Once per Initialise is enough; a
// second call does nothing.
void RegisterClickSoundProperty();

// The kind of a click on `element`: the click-sound of the element or of its
// nearest parent that names one; "" for none, for "none", and when the
// element or a parent up to that one is disabled (class "disabled").
std::string ClickKindOf(Rml::Element* element);

// Plays the kind's sound (interfaceClick) as an interface sound; nothing
// for "".
void PlayClickKind(std::string_view kind);

// Plays the click sound of the element, for a key that stands for it
// (Enter for OK); nothing for nullptr.
void PlayClickSoundOf(Rml::Element* element);

// Hears every click of a context before the buttons' own handlers (the
// capture phase), so a handler that stops the click still sounds.
class ClickSoundListener final : public Rml::EventListener
{
public:
    void ProcessEvent(Rml::Event& event) override;

    // Plays a kind; PlayClickKind unless a test records them.
    std::function<void(std::string_view)> play = PlayClickKind;
};

// Registers the property and listens to the context's clicks, with one
// listener that lives as long as the game.
void ListenForClickSounds(Rml::Context& context);
} // namespace UI::RmlBridge
