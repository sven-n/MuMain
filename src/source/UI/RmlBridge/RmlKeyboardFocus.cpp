#include "stdafx.h"
#include "UI/RmlBridge/RmlKeyboardFocus.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

namespace UI::RmlBridge
{
    bool IsTypingIn(Rml::ElementDocument* doc)
    {
        if (doc == nullptr || !RmlUiRuntime::Instance().IsCreated())
            return false;

        Rml::Element* focused = RmlUiRuntime::Instance().GetContext()->GetFocusElement();
        if (focused == nullptr || focused->GetOwnerDocument() != doc)
            return false;

        // Same test RmlUiRuntime::IsTextInputActive() makes: only a text-entry widget takes typing.
        const Rml::String& tag = focused->GetTagName();
        return tag == "input" || tag == "textarea";
    }

    void ClaimKeyboardWhileTyping(mu::ui::window::CObject& window, Rml::ElementDocument* doc)
    {
        const HWND wanted = IsTypingIn(doc) ? reinterpret_cast<HWND>(&RmlUiRuntime::Instance()) : g_hWnd;
        if (window.GetRelatedWnd() != wanted)
            window.SetRelatedWnd(wanted);
    }
}
