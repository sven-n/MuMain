#pragma once

#include "UI/Events/EventEntryRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

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
// column of level buttons of which one is enabled, the exit button. Each window owns one,
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
    // Unloads the document; from the window's own Release().
    void Release() { m_View.Release(); }

    // Complete translated content, set when the window opens. The theme owns its layout.
    void SetContent(const wchar_t* title, const std::vector<std::wstring>& lines,
                    const std::vector<Button>& buttons);

    // Per frame, inside the window's CManager transform scope.
    void Sync(bool visible);

    // The pointer is over what the document draws (UI::RmlBridge::IsPointerOver()).
    bool IsPointerOver() const;
    Rml::ElementDocument* Document() const { return m_View.Document(); }

    // A click RmlUi reported since the last call: an enabled level button's index (else -1), the
    // exit button.
    int TakePressedButton();
    bool TakeExitPressed();

private:
    void BindModel(Rml::DataModelConstructor& c, EventEntryRmlModel& model);
    void SyncTextSizes();

    UI::RmlBridge::ThemedView<EventEntryRmlModel> m_View;
    int m_PressedButton = -1;
    bool m_ExitPressed = false;
};
} // namespace mu::ui::window
