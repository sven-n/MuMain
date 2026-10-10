#pragma once

#include "UI/RmlBridge/RmlThemedView.h"
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
    // The pointer is over the window as drawn, so it is the window's, not the world's.
    bool PointerOver() const;

    // Mirror room-owned participants into the presentation model.
    void AddPal(const wchar_t* name, BYTE number);
    void RemovePal(const wchar_t* name);

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
    void OnBuilt();
    void OnUnload();
    void SyncGeometry();
    void SyncWorkspace();
    void ActionRequested(const Action& action);

    void ToggleInvite();
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
    UI::RmlBridge::ThemedView<Model> m_View{"", [this](auto& c, auto& m) { RegisterModel(c, m); },
        {{"Data/Interface/RmlUi/chat_room.rml"}},
        {.modelPlaceholder = "chat_room",
         .afterBuild = [this] { OnBuilt(); },
         .beforeUnload = [this] { OnUnload(); }}};
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    float m_WorkspaceHeight = 0;
    std::wstring m_Title;
    std::wstring m_SelectedInvite;
    bool m_Placed = false;
    // Placed AND the context has applied that position: before this, the panel is still centred.
    bool m_Settled = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    bool m_ScrollToEnd = false;
    bool m_FocusField = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
} // namespace UI::Social
