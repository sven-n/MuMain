#pragma once

#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Social/PhotoViewerControl.h"
#include "UI/Social/LetterWriteModel.h"
#include <RmlUi/Core/EventListener.h>
#include <array>
#include <string>
#include <vector>

class CUILetterWriteWindow;

namespace UI::Social
{
// A letter being written: one document, the player's own character live 3D in a render target it
// shows (CUIPhotoViewer), as the read window.
class LetterWriteView : public Rml::EventListener
{
public:
    explicit LetterWriteView(CUILetterWriteWindow& owner);
    ~LetterWriteView() override;

    void Build();
    bool Sync(bool shown);
    void PullToFront();
    void ProcessActions();
    // The pointer is over the window as drawn, so it is the window's, not the world's.
    bool PointerOver() const;
    bool PointerOverPhoto() const;

    void SetMailto(const wchar_t* text);
    void SetSubject(const wchar_t* text);
    void SetBody(const wchar_t* text);
    std::wstring Mailto() const;
    std::wstring Subject() const;
    std::wstring Body() const;
    void FocusField(int index);
    void RestoreFocus();
    void SetSending(bool sending);
    bool AnyFieldHasFocus() const;

    void RestoreLayout(float x, float y, float width, float height, bool resize = false);
    void Maximize();
    void ProcessEvent(Rml::Event& event) override;

private:
    using Model = LetterWriteModel;
    struct Action
    {
        Rml::String name;
    };
    void RegisterModel(Rml::DataModelConstructor& constructor, Model& model);
    void Unload();
    void OnBuilt();
    void OnUnload();
    void SyncGeometry();
    void SyncWorkspace();
    void SyncPhoto();
    void ActionRequested(const Action& action);
    void SyncDraggedPosition();
    void PublishPosition();
    void ClampToWorkspace();
    void ApplyLayout();
    void PlaceAtRest();
    Rml::Element* Panel() const;

    CUILetterWriteWindow& m_Owner;
    UI::RmlBridge::ThemedView<Model> m_View{"", [this](auto& c, auto& m) { RegisterModel(c, m); },
        {{"Data/Interface/RmlUi/letter_write.rml"}},
        {.modelPlaceholder = "letter_write",
         .afterBuild = [this] { OnBuilt(); },
         .beforeUnload = [this] { OnUnload(); }}};
    PhotoViewerControl m_PhotoControl;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    float m_WorkspaceHeight = 0;
    std::wstring m_Title;
    // Which field to put the caret in next frame, and which held it last, as m_iLastTabIndex did.
    int m_PendingFocus = -1;
    int m_LastFocus = 0;
    bool m_Placed = false;
    // Placed AND the context has applied that position: before this, the panel is still centred.
    bool m_Settled = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
} // namespace UI::Social
