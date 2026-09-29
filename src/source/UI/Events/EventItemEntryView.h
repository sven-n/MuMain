#pragma once

#include "UI/Events/EventItemEntryRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <string>
#include <vector>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The RmlUi side of the right-docked NPC entry windows that show the entry item as a live 3D
// preview (CDoppelGangerWindow, CEmpireGuardianNPC). The frame and the separator line are a
// background-context document, painted before the native 3D pass so the item draws over them as
// it did over the original's frame; the texts and the buttons are a main-context document. Each
// window owns one, with its own documents and data models; the window keeps its data, its 3D
// preview and its requests.
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
    };

    struct Button
    {
        std::wstring label;
        float left = 0.f;
        float top = 0.f;
        bool locked = false;
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

private:
    void SyncTexts();

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
