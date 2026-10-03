#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Party/PhotoViewerControl.h"
#include "UI/Party/LetterWriteModel.h"
#include <RmlUi/Core/EventListener.h>
#include <array>
#include <string>
#include <vector>

class CUILetterWriteWindow;

namespace UI::Party
{
// A letter being written. Two documents for the same reason the read window has two: this window
// shows the player's own character as live 3D, which renders between the two RmlUi passes, so the
// chrome goes in a background-context document and the foreground paints nothing of its own.
class LetterWriteView : public Rml::EventListener
{
public:
    explicit LetterWriteView(CUILetterWriteWindow& owner);
    ~LetterWriteView() override;

    void Build();
    bool Sync(bool shown);
    void PullToFront();
    void ProcessActions();

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
    void ReloadTheme();
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
    RmlModelBinder<Model> m_Binder;
    PhotoViewerControl m_PhotoControl;
    Rml::ElementDocument* m_Document = nullptr;
    Rml::String m_ModelName;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    std::wstring m_Title;
    // Which field to put the caret in next frame, and which held it last, as m_iLastTabIndex did.
    int m_PendingFocus = -1;
    int m_LastFocus = 0;
    bool m_Placed = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
} // namespace UI::Party
