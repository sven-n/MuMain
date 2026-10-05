#pragma once

#include "UI/Events/EventEntryRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <string>
#include <vector>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The RmlUi side of the right-docked event entry windows that share the original's layout
// (CEnterBloodCastle, CEnterDevilSquare): the docked frame, a bold title, description lines, a
// column of 180 x 29 level buttons of which one is enabled, the exit button. Each window owns one,
// with its own document and data model; the window keeps its data, its hit tests and requests.
class EventEntryView
{
public:
    struct Button
    {
        std::wstring label;
        bool enabled = false;
    };

    EventEntryView(const char* modelName, const char* documentPath);

    void Build();
    void ReloadTheme();

    // The static content, set when the window opens. Where the lines and buttons land is the
    // theme's: each window's own .rcss gives its rows (event_entry.rcss).
    void SetContent(const wchar_t* title, const std::vector<std::wstring>& lines,
                    const std::vector<Button>& buttons);

    // Per frame, inside the window's CManager transform scope.
    void Sync(bool visible, const POINT& pos);

    // The theme's #panel size in the window's layout units; false (outputs untouched) before the
    // document has laid out.
    bool PanelSize(float& width, float& height) const;
    Rml::ElementDocument* Document() const { return m_pRmlDoc; }

    // A click RmlUi reported since the last call: an enabled level button's index (else -1), the
    // exit button.
    int TakePressedButton();
    bool TakeExitPressed();

private:
    void SyncTextSizes();

    const char* m_ModelName;
    const char* m_DocumentPath;
    RmlModelBinder<EventEntryRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    std::wstring m_Title;
    std::vector<std::wstring> m_LineTexts;
    int m_PressedButton = -1;
    bool m_ExitPressed = false;
};
} // namespace mu::ui::window
