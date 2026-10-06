#include "stdafx.h"
#include "Core/Utilities/FrameProfiler.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"
#include "UI/Placement/WindowPlacement.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>

namespace UI::RmlBridge
{
void RegisterWorkspaceDocument(std::string_view name, std::function<Rml::ElementDocument*()> document,
                               const char* rootId, WorkspaceDocumentOptions options)
{
    const auto root = [document, id = std::string(rootId)]() -> Rml::Element*
    {
        auto* doc = document();
        return doc != nullptr ? doc->GetElementById(id) : nullptr;
    };
    UI::Placement::PlacementParticipant participant;
    participant.visible = [root, keep = options.placedWhileHidden]()
    {
        const auto* element = root();
        return element != nullptr && (keep || element->IsVisible(true));
    };
    participant.measure = [root, measure = options.measure]()
    {
        if (measure)
            return measure();
        auto* element = root();
        if (element == nullptr)
            return UI::Placement::PlacementParticipant::Size{};
        {
            FRAME_PROFILE(UILayout);
            element->GetOwnerDocument()->UpdateDocument();
        }
        const float dp = element->GetContext()->GetDensityIndependentPixelRatio();
        const auto size = element->GetBox().GetSize(Rml::BoxArea::Border);
        return UI::Placement::PlacementParticipant::Size{size.x / dp, size.y / dp};
    };
    participant.place = [root, placed = options.placed](const UI::Placement::PlacementParticipant::Box* box)
    {
        if (placed)
            placed(box);
        auto* element = root();
        if (element == nullptr)
            return;
        element->SetClass("workspace-placed", box != nullptr);
        if (box == nullptr)
        {
            element->RemoveProperty(Rml::PropertyId::Left);
            element->RemoveProperty(Rml::PropertyId::Top);
            element->RemoveProperty(Rml::PropertyId::Transform);
            return;
        }
        element->SetProperty(Rml::PropertyId::Left, Rml::Property(box->left, Rml::Unit::PX));
        element->SetProperty(Rml::PropertyId::Top, Rml::Property(box->top, Rml::Unit::PX));
        const float ratio = box->scale / element->GetContext()->GetDensityIndependentPixelRatio();
        element->SetProperty("transform", "scale(" + std::to_string(ratio) + ")");
        {
            FRAME_PROFILE(UILayout);
            element->GetOwnerDocument()->UpdateDocument();
        }
    };
    UI::Placement::RegisterParticipant(name, std::move(participant));
}
}
