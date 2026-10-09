#include "stdafx.h"
#include "UI/RmlBridge/RmlPointer.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

bool UI::RmlBridge::IsPointerOver(Rml::ElementDocument* document)
{
    if (document == nullptr || !document->IsVisible())
        return false;
    Rml::Context* context = document->GetContext();
    const Rml::Element* hover = context != nullptr ? context->GetHoverElement() : nullptr;
    return hover != nullptr && hover != document && hover->GetOwnerDocument() == document;
}

bool UI::RmlBridge::IsPointerOver(Rml::Element* element)
{
    Rml::ElementDocument* document = element != nullptr ? element->GetOwnerDocument() : nullptr;
    if (document == nullptr || !document->IsVisible())
        return false;
    Rml::Context* context = document->GetContext();
    for (Rml::Element* hover = context != nullptr ? context->GetHoverElement() : nullptr; hover != nullptr;
         hover = hover->GetParentNode())
    {
        if (hover == element)
            return true;
    }
    return false;
}

bool UI::RmlBridge::IsPointWithin(Rml::Element* element, Rml::Vector2f point)
{
    Rml::ElementDocument* document = element != nullptr ? element->GetOwnerDocument() : nullptr;
    if (document == nullptr || !document->IsVisible() || !element->IsVisible(true))
        return false;
    return element->Project(point) && element->IsPointWithinElement(point);
}

bool UI::RmlBridge::IsPointerWithin(Rml::Element* element)
{
    return IsPointWithin(element, {g_fWindowMouseX, g_fWindowMouseY});
}
