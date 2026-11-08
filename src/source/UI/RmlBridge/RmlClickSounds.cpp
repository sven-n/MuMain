#include "stdafx.h"

#include "UI/RmlBridge/RmlClickSounds.h"

#include "Audio/SoundEventPlay.h"
#include "Data/GameData/SoundData/SoundEvents.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/PropertyDefinition.h>
#include <RmlUi/Core/StyleSheetSpecification.h>

namespace UI::RmlBridge
{
namespace
{
constexpr std::string_view NoClickSound = "none";
} // namespace

void RegisterClickSoundProperty()
{
    if (Rml::StyleSheetSpecification::GetProperty(ClickSoundProperty) != nullptr)
    {
        return;
    }
    Rml::StyleSheetSpecification::RegisterProperty(ClickSoundProperty, "", false, false).AddParser("string");
}

std::string ClickKindOf(Rml::Element* element)
{
    for (Rml::Element* at = element; at != nullptr; at = at->GetParentNode())
    {
        if (at->IsClassSet("disabled"))
        {
            return {};
        }
        if (const Rml::Property* property = at->GetLocalProperty(ClickSoundProperty))
        {
            const Rml::String kind = property->Get<Rml::String>();
            return kind == NoClickSound ? std::string{} : kind;
        }
    }
    return {};
}

void PlayClickKind(std::string_view kind)
{
    if (kind.empty())
    {
        return;
    }
    Data::Sounds::SoundEventContext context;
    context.kind = kind;
    ::Audio::PlayInterfaceEventSound(Data::Sounds::InterfaceClickEvent, context);
}

void PlayClickSoundOf(Rml::Element* element)
{
    if (element != nullptr)
    {
        PlayClickKind(ClickKindOf(element));
    }
}

void ClickSoundListener::ProcessEvent(Rml::Event& event)
{
    const std::string kind = ClickKindOf(event.GetTargetElement());
    if (!kind.empty() && play)
    {
        play(kind);
    }
}

void ListenForClickSounds(Rml::Context& context)
{
    // The context's elements keep a pointer to it until Rml::Shutdown.
    static ClickSoundListener listener;
    RegisterClickSoundProperty();
    context.AddEventListener("click", &listener, true);
}
} // namespace UI::RmlBridge
