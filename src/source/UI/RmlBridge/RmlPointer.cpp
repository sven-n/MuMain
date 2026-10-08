#include "stdafx.h"
#include "UI/RmlBridge/RmlPointer.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

bool UI::RmlBridge::IsPointerOver(Rml::ElementDocument* document)
{
    if (document == nullptr || !document->IsVisible())
        return false;
    Rml::Context* context = document->GetContext();
    const Rml::Element* hover = context != nullptr ? context->GetHoverElement() : nullptr;
    return hover != nullptr && hover != document && hover->GetOwnerDocument() == document;
}
