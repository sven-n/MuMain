#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Social/PhotoViewerControl.h"
#include "UI/Social/LetterReadModel.h"
#include <RmlUi/Core/EventListener.h>
#include <array>
#include <string>
#include <vector>

class CUILetterReadWindow;

namespace UI::Social
{
// A letter being read: two documents, because this window shows the sender's character as live 3D.
//
//   background context  ->  chrome and the panel's back   (before the native pass)
//   native pass         ->  CUIPhotoViewer                (unchanged)
//   main context        ->  the header, the body and the buttons, and nothing else
//
// The foreground keeps window_shell's boxes -- the title rail still drags, #content still hosts the
// buttons -- but drops its paint, so the portrait is never covered. Both documents are instanced
// per window through LoadThemedDocument()'s placeholder overload, as rooms are.
class LetterReadView : public Rml::EventListener
{
public:
    explicit LetterReadView(CUILetterReadWindow& owner);
    ~LetterReadView() override;

    void Build();
    bool Sync(bool shown);
    void PullToFront();
    void ProcessActions();

    void SetLetter(const wchar_t* sender, const wchar_t* date, const wchar_t* time, const wchar_t* body);
    void RestoreLayout(float x, float y, float width, float height, bool resize = false);
    void Maximize();
    void ProcessEvent(Rml::Event& event) override;

private:
    using Model = LetterReadModel;
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

    CUILetterReadWindow& m_Owner;
    RmlModelBinder<Model> m_Binder;
    PhotoViewerControl m_PhotoControl;
    Rml::ElementDocument* m_Document = nullptr;
    Rml::String m_ModelName;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    std::wstring m_Title;
    bool m_Placed = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
} // namespace UI::Social
