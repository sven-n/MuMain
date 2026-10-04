#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Social/ChatRoomModel.h"
#include <RmlUi/Core/EventListener.h>
#include <string>
#include <vector>

class CUIChatWindow;

namespace UI::Social
{
// One room, one document, one data model. The window-identity spike settled this: the vendored
// data-for keys DOM state by array index, so rooms sharing a repeater would hand a focused field
// and its scroll offset to a different room the moment an earlier one closed. The model name
// carries the window id for the same reason.
//
// Chrome, dragging, resizing and maximize come from the shared window_shell template, as the
// friend shell's do. Geometry is mirrored into the manager's native reference coordinates.
class ChatRoomView : public Rml::EventListener
{
public:
    explicit ChatRoomView(CUIChatWindow& owner);
    ~ChatRoomView() override;

    void Build();
    bool Sync(bool shown);
    void PullToFront();
    void ProcessActions();

    // Participants. AddPal returns the new count, as CUIChatWindow::AddChatPal did.
    int AddPal(const wchar_t* name, BYTE number);
    void RemovePal(const wchar_t* name);
    int PalCount() const;
    // The room's other member when it is a pair; nullptr with nobody or a crowd (piResult 2).
    const wchar_t* ChatFriend(int* result);
    void MakeTitleText(wchar_t* out, size_t capacity) const;

    // byIndex is the server's participant index; 255 is a system line with no speaker.
    void AddLine(BYTE byIndex, const wchar_t* text, int type);
    void SetLocked(bool locked);
    // Enter arrives through the native key path, as every other RmlUi field in this client takes
    // it (CChatInputBox, CGenericConfirmDialog); the owner gates it on this room's own field.
    bool FieldHasFocus() const;
    void SubmitDraft();
    // Opening the invitation column widens the window and closing it narrows it again, as
    // CUIChatWindow's own SetSize(GetWidth() +/- 80) did.
    float InviteColumnWidth() const;
    bool InviteShown() const;
    void RefreshInviteList();
    const wchar_t* SelectedInvite();
    void FocusField();

    void RestoreLayout(float x, float y, float width, float height, bool resize = false);
    void RestoreMaximized(bool maximized, float top, float height);
    void Maximize();
    void ProcessEvent(Rml::Event& event) override;

private:
    using Model = ChatRoomModel;
    struct Action
    {
        Rml::String name;
        Rml::Variant value;
    };
    void RegisterModel(Rml::DataModelConstructor& constructor, Model& model);
    void Unload();
    void ReloadTheme();
    void SyncGeometry();
    void SyncWorkspace();
    void ActionRequested(const Action& action);

    void ToggleInvite();
    void InviteSelected();
    void SyncPalVisibility();
    void PublishTitle();
    void SyncDraggedPosition();
    void PublishPosition();
    void ClampToWorkspace();
    void ApplyLayout();
    void PlaceAtRest();
    void ScrollLogToEnd();
    Rml::Element* Panel() const;
    Rml::Element* Field() const;

    CUIChatWindow& m_Owner;
    RmlModelBinder<Model> m_Binder;
    Rml::ElementDocument* m_Document = nullptr;
    Rml::String m_ModelName;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    std::wstring m_Title;
    std::wstring m_SelectedInvite;
    std::wstring m_LastSent;
    std::wstring m_NameLookup;
    bool m_Placed = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    bool m_ScrollToEnd = false;
    bool m_FocusField = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
} // namespace UI::Social
