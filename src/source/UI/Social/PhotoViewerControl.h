#pragma once

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>

class CUIPhotoViewer;

namespace Rml { class ElementDocument; }

namespace UI::Social
{
// Drag to turn, right-click to reset, and the "?" help, for a native CUIPhotoViewer standing in an
// RmlUi slot -- driven from the document instead of from the viewer's own native mouse handling.
//
// That handling cannot see a press made over a document. Context::ProcessMouseButtonDown returns
// !IsMouseInteracting(), which is false whenever anything at all is hovered, and Winmain's event
// pump only reaches HandleMouseButton() when RmlUi lets the event propagate -- so MouseLButtonPush
// is never set for a click on the panel, and every press-driven control in CUIPhotoViewer was
// dead behind the letter's own document. The wheel never passes through RmlUi at all, which is why
// zoom still works natively and only the press-driven controls moved here.
//
// The "?" icon itself is still drawn by the viewer, in the same post-RmlUi seam as the character:
// this only owns its hit area, since an RCSS decorator would have to re-find the native sprite for
// no visual gain.
class PhotoViewerControl : public Rml::EventListener
{
public:
    ~PhotoViewerControl() override;

    void Attach(Rml::ElementDocument* document, CUIPhotoViewer& viewer);
    void Detach();

    void ProcessEvent(Rml::Event& event) override;

private:
    Rml::ElementDocument* m_Document = nullptr;
    CUIPhotoViewer* m_Viewer = nullptr;
    bool m_Turning = false;
    float m_LastX = 0;
};
} // namespace UI::Social
