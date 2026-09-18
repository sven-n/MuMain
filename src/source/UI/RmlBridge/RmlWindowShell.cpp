#include "stdafx.h"
#include "RmlWindowShell.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

namespace UI::RmlBridge
{
    Rml::Element* PromoteToWindowShellFooter(Rml::ElementDocument* document, const char* elementId)
    {
        if (!document)
            return nullptr;

        Rml::Element* footer = document->GetElementById("window_shell_footer");
        Rml::Element* element = document->GetElementById(elementId);
        if (!footer || !element)
            return nullptr;

        Rml::Element* parent = element->GetParentNode();
        if (!parent || parent == footer)
            return element;

        if (Rml::ElementPtr owned = parent->RemoveChild(element))
            return footer->AppendChild(std::move(owned));

        return nullptr;
    }
}
