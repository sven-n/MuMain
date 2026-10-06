#pragma once

#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Social/PhotoViewerControl.h"
#include "UI/Social/LetterReadModel.h"
#include <RmlUi/Core/EventListener.h>
#include <array>
#include <string>
#include <vector>

class CUILetterReadWindow;

namespace UI::Social
{
// A letter being read: one document, the sender's character live 3D in a render target it shows
// (CUIPhotoViewer). Instanced per window through ThemedView's modelPlaceholder, as rooms are.
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

    CUILetterReadWindow& m_Owner;
    UI::RmlBridge::ThemedView<Model> m_View{"", [this](auto& c, auto& m) { RegisterModel(c, m); },
        {{"Data/Interface/RmlUi/letter_read.rml"}},
        {.modelPlaceholder = "letter_read",
         .afterBuild = [this] { OnBuilt(); },
         .beforeUnload = [this] { OnUnload(); }}};
    PhotoViewerControl m_PhotoControl;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    std::wstring m_Title;
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
