#pragma once

#include "UI/Events/EventItemEntryRmlModel.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <memory>
#include <string>
#include <vector>

namespace Rml
{
class Element;
class ElementDocument;
} // namespace Rml

namespace mu::ui::window
{
class CObject;

// The RmlUi side of the right-docked NPC entry windows that show the entry item as a live 3D
// preview (CDoppelGangerWindow, CEmpireGuardianNPC): the frame, the preview, the texts and the
// buttons, in one document. Each window owns one, with its own document and data model; the window
// keeps its data, draws its preview and sends its requests. The Golden Archer's and the lucky coin
// windows share it for their other button kinds.
class EventItemEntryView
{
public:
    // Where a line sits, what colour it is and how it is aligned are the document's
    // <document>_rows.rcss, one :nth-child rule per line. `width` is the box the native renderer
    // shrinks the text to, and `bold` the font it measures in.
    struct Text
    {
        std::wstring text;
        float width = 0.f;
        bool bold = false;
    };

    // Where a button sits and which sprite it wears are the same file's; `width` and `height` are
    // the label's own centring box, which CButton::Render() measured against.
    struct Button
    {
        std::wstring label;
        bool locked = false;
        float width = 53.f; // newui_btn_empty_very_small
        float height = 23.f;
        bool bold = false;
    };

    EventItemEntryView(const char* modelName, const char* documentPath);

    // The live preview: `draw` runs once a frame into the document's #entry_item, its
    // RenderItem3D() rectangles in `owner`'s layout space, as the shared item camera drew them.
    void SetItemDrawer(std::function<void()> draw, const CObject* owner);

    void Build();
    // Unloads the document; from the window's own Release().
    void Release() { m_View.Release(); }
    // Runs after every build of the main document, including a theme switch's.
    void SetAfterBuild(std::function<void()> afterBuild) { m_View.SetAfterBuild(std::move(afterBuild)); }

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
    void BindModel(Rml::DataModelConstructor& c, EventItemEntryRmlModel& model);
    void SyncTexts();
    void SyncButtons();

    UI::RmlBridge::ThemedView<EventItemEntryRmlModel> m_View;
    std::vector<Text> m_Texts;
    std::vector<Button> m_Buttons;
    int m_PressedButton = -1;
    std::unique_ptr<UI::Items::ItemCameraTarget> m_ItemTarget;
};
} // namespace mu::ui::window
