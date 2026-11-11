#pragma once

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>

class CUIPhotoViewer;

namespace Rml { class ElementDocument; }

namespace UI::Social
{
// Everything about a native CUIPhotoViewer standing in an RmlUi slot that the document owns: the
// image its render target fills, drag to turn, right-click to reset, and the "?" with its help --
// driven from the document instead of from the viewer's own native mouse handling.
//
// That handling cannot see a press made over a document. Context::ProcessMouseButtonDown returns
// !IsMouseInteracting(), which is false whenever anything at all is hovered, and Winmain's event
// pump only reaches HandleMouseButton() when RmlUi lets the event propagate -- so MouseLButtonPush
// is never set for a click on the panel, and every press-driven control in CUIPhotoViewer was
// dead behind the letter's own document. The wheel never passes through RmlUi at all, which is why
// zoom still works natively and only the press-driven controls moved here.
//
// The character is an image in the document now (UI::RmlBridge::RenderTarget), so the "?" and the
// help it opens are document elements drawn above it: the shared RmlUi tooltip, not native text.
class PhotoViewerControl : public Rml::EventListener
{
public:
    ~PhotoViewerControl() override;

    void Attach(Rml::ElementDocument* document, CUIPhotoViewer& viewer);
    void Detach();

    // Once a frame while the window is shown and settled: sizes the viewer's render target to
    // #photo_image, points the image at it, and mirrors the viewer's state into the document.
    void Sync();
    // While the window is hidden: stops drawing and gives up the help tooltip.
    void Suspend();

    void ProcessEvent(Rml::Event& event) override;

private:
    void SyncHelp();

    Rml::ElementDocument* m_Document = nullptr;
    CUIPhotoViewer* m_Viewer = nullptr;
    bool m_Turning = false;
    float m_LastX = 0;
};
} // namespace UI::Social
