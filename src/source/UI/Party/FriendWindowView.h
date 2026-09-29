#pragma once

// RmlUi presentation of the friends family (CUIWindowMgr's windows, UIWindows.h): one document
// (friend_window.rml) and one data model per open window. The native windows and controls keep
// their data, hit tests, drags, resizing, buttons and messages; each frame the document is rebuilt
// from what their Render() drew -- the same geometry, as named parts (FriendWindowRmlModel.h) --
// and the documents are stacked in the manager's draw order.

#include "UI/Party/FriendWindowRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Scaling/UITransform.h"

#include <array>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Rml
{
class Element;
class ElementDocument;
class EventListener;
} // namespace Rml

class CUIBaseWindow;
class CUIButton;
class CUITextInputBox;

// A text field of a window (CUIBaseWindow::GetRmlTextField()), shown as an RmlUi <input> (slots
// 0 and 1) or <textarea> (slot 2) over the window: its box in native reference px relative to the
// window's top-left corner, and its text colour (native RGBA()).
struct FriendWindowFieldLayout
{
    static constexpr int SlotCount = 3;
    static constexpr int MultilineSlot = 2;

    bool shown = false;
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    unsigned long color = 0;

    bool operator==(const FriendWindowFieldLayout&) const = default;
};

// Collects the parts of one window, in native (FloatingWorkspace reference) coordinates, relative
// to the window's top-left corner, into the parts of the previous frame: a part that did not
// change is not touched, and its text and colour are not formatted again.
class FriendWindowRmlBuilder
{
public:
    FriendWindowRmlBuilder(int originX, int originY, std::vector<FriendWindowPart>& parts,
                           std::vector<FriendWindowPart>& underlayParts);

    void Fill(const char* role, double x, double y, double width, double height);
    void Sprite(const char* role, double x, double y, double width, double height);
    // RenderText(x, y, text) in g_hFont (g_hFontBold when bold); a box width > 0 clips (align 0)
    // or centres (align 1) the text in it.
    void Text(const wchar_t* text, double x, double y, DWORD color, bool bold = false, double boxWidth = 0.0,
              int align = 0);
    // CUIButton::Render().
    void Button(CUIButton& button);
    // RenderCheckBox().
    void CheckBox(double x, double y, bool checked);
    // The old-style list scroll bar the friends family's list boxes draw (their RenderInterface()).
    template <typename List> void ListScrollBar(List& list);
    // CUITextInputBox::Render(): its background as a part, the field itself as the slot's input.
    void Field(int slot, CUITextInputBox& box);

    // A flat part under the window's native 3D content (CUIPhotoViewer): drawn by the window's
    // underlay document in the background context, before the native pass.
    void UnderlayFill(const char* role, double x, double y, double width, double height);

    // Drop the parts past the last one collected; true if any part changed this frame.
    bool Finish()
    {
        return m_Main.Finish();
    }
    bool FinishUnderlay()
    {
        return m_Underlay.Finish();
    }
    const std::array<FriendWindowFieldLayout, FriendWindowFieldLayout::SlotCount>& GetFields() const
    {
        return m_Fields;
    }

private:
    FriendWindowPart& NextPart(int kind, const char* role, double x, double y, double width, double height);

    double m_OriginX;
    double m_OriginY;
    UI::Scaling::Transform m_Transform;
    struct PartList
    {
        std::vector<FriendWindowPart>* parts;
        size_t count = 0;
        bool changed = false;

        bool Finish();
    };
    PartList m_Main;
    PartList m_Underlay;
    PartList* m_Active = &m_Main;
    std::array<FriendWindowFieldLayout, FriendWindowFieldLayout::SlotCount> m_Fields{};
};

// One window's document.
class FriendWindowView
{
public:
    explicit FriendWindowView(DWORD windowUIID);
    ~FriendWindowView();
    FriendWindowView(const FriendWindowView&) = delete;
    FriendWindowView& operator=(const FriendWindowView&) = delete;

    // Rebuilds the document from the window (nullptr or !shown: hidden). Returns true when the
    // document became visible this frame.
    bool Sync(CUIBaseWindow* window, bool shown);
    void PullToFront();
    void ReloadTheme();

    // A key pressed in the slot's field (its keydown listener): Enter and Tab go to the native
    // field, as they did when it had the keyboard. Returns true if the key was used.
    bool OnFieldKey(int slot, int keyIdentifier);
    void OnFieldEdited(int slot)
    {
        m_Fields[slot].edited = true;
    }

private:
    // RmlUi presentation of one text field: its element, where it was last placed, the value both
    // sides last agreed on, and whether the player edited it since.
    struct Field
    {
        Rml::Element* element = nullptr;
        std::unique_ptr<Rml::EventListener> listener;
        FriendWindowFieldLayout layout; // where it was last placed, and under which window corner / scale
        float rootX = 0.f;
        float rootY = 0.f;
        float scale = 0.f;
        int maxLength = -1;
        std::wstring syncedValue;
        bool edited = false;
    };

    void Build();
    void BuildUnderlay();
    void Unload();
    void SyncFields(CUIBaseWindow& window, const FriendWindowRmlBuilder& builder, bool topWindow);
    void PlaceField(Field& field, const FriendWindowFieldLayout& layout, const UI::Scaling::Transform& transform);
    void PushFieldValue(Field& field, CUITextInputBox& box);

    DWORD m_WindowUIID;
    std::string m_ModelName;
    Rml::ElementDocument* m_pDoc = nullptr;
    RmlModelBinder<FriendWindowRmlModel> m_Binder;
    // Parts under the window's native 3D content (FriendWindowRmlBuilder::UnderlayFill()).
    Rml::ElementDocument* m_pUnderDoc = nullptr;
    RmlModelBinder<FriendWindowRmlModel> m_UnderBinder;
    std::array<Field, FriendWindowFieldLayout::SlotCount> m_Fields;
};

// The documents of every window of the manager with an RmlUi view.
class FriendWindowViews
{
public:
    FriendWindowViews();
    ~FriendWindowViews();

    // windows: the manager's windows in draw order (back to front); nullptr entries are skipped.
    void Sync(const std::list<CUIBaseWindow*>& windows, bool familyShown);

private:
    std::map<DWORD, std::unique_ptr<FriendWindowView>> m_Views;
    std::list<DWORD> m_Order; // the draw order the documents were last stacked in
};
