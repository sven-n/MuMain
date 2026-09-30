#pragma once

#include "UI/Events/EventItemEntryRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <string>
#include <vector>

namespace Rml
{
class Element;
class ElementDocument;
} // namespace Rml

namespace mu::ui::window
{
// The RmlUi side of the right-docked NPC entry windows that show the entry item as a live 3D
// preview (CDoppelGangerWindow, CEmpireGuardianNPC). The frame and the separator line are a
// background-context document, painted before the native 3D pass so the item draws over them as
// it did over the original's frame; the texts and the buttons are a main-context document. Each
// window owns one, with its own documents and data models; the window keeps its data, its 3D
// preview and its requests. The lucky coin windows (CRegistrationLuckyCoin, CExchangeLuckyCoin)
// share it for their other button kinds.
class EventItemEntryView
{
public:
    struct Text
    {
        std::wstring text;
        float left = 0.f;
        float top = 0.f;
        float width = 0.f;
        bool bold = false;
        DWORD color = 0;
        bool leftAligned = false; // RT3_SORT_LEFT from the box's left edge instead of centred
    };

    struct Button
    {
        std::wstring label;
        float left = 0.f;
        float top = 0.f;
        bool locked = false;
        float width = 53.f; // newui_btn_empty_very_small
        float height = 23.f;
        bool bold = false;
        std::string style; // the window's own button kind, for its theme
    };

    EventItemEntryView(const char* modelName, const char* documentPath, const char* bgModelName,
                       const char* bgDocumentPath);

    void Build();
    void ReloadTheme();

    void SetTexts(std::vector<Text> texts);
    void SetButtons(const std::vector<Button>& buttons);

    // Per frame, inside the window's CManager transform scope.
    void Sync(bool visible, const POINT& pos);

    // A click RmlUi reported since the last call: an unlocked button's index, else -1.
    int TakePressedButton();

    // An element of the main document (a window's own text field), or nullptr before Build().
    Rml::Element* GetElementById(const char* id) const;

    // The window's own text field's contents, when its document has one. Two-way: the <input>'s
    // data-value writes it back, so this is the value, not the element's attribute.
    const Rml::String& InputValue() const;
    void SetInputValue(const Rml::String& value);

    // #panel's own live RCSS size, for the owner's native hit test. Leaves both alone when the
    // document isn't up or laid out yet, so seed them with the window's own fallback constants
    // (UI::RmlBridge::RefreshLogicalPanelSize()'s convention, which this forwards to).
    void RefreshPanelSize(float& width, float& height) const;

private:
    void SyncTexts();
    void SyncButtons();

    const char* m_ModelName;
    const char* m_DocumentPath;
    const char* m_BgModelName;
    const char* m_BgDocumentPath;
    RmlModelBinder<EventItemEntryRmlModel> m_RmlBinder;
    RmlModelBinder<EventItemEntryBgRmlModel> m_BgRmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    Rml::ElementDocument* m_pRmlBgDoc = nullptr;
    std::vector<Text> m_Texts;
    std::vector<Button> m_Buttons;
    int m_PressedButton = -1;
};
} // namespace mu::ui::window
