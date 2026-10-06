#pragma once

#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Social/FriendShellModel.h"
#include <RmlUi/Core/EventListener.h>
#include <array>
#include <vector>

class CUIFriendWindow;

namespace UI::Social
{
class FriendShell : public Rml::EventListener
{
public:
    explicit FriendShell(CUIFriendWindow& owner);
    ~FriendShell() override;
    void Build();
    bool Sync(bool shown);
    void PullToFront();
    void ProcessActions();
    void RefreshFriends();
    void RefreshLetters();
    void AddWindow(DWORD id, const wchar_t* title);
    void RemoveWindow(DWORD id);
    void ResetWindows();
    DWORD SelectedWindow() const;
    DWORD SelectedLetter() const;
    void SelectLetterLine(int line);
    void SetTab(int tab);
    int GetTab() const;
    void RequestPaneFocus();
    void RestoreLayout(float x, float y, float width, float height, bool resize = false);
    void RestoreMaximized(bool maximized, float top, float height);
    void Maximize();
    void ProcessEvent(Rml::Event& event) override;

private:
    using Model = FriendShellModel;
    struct Action { Rml::String name; Rml::Variant value; };
    void RegisterModel(Rml::DataModelConstructor& constructor, Model& model);
    void BindLabels(Rml::DataModelConstructor& constructor);
    void Unload();
    void OnBuilt();
    void OnUnload();
    void SyncGeometry();
    void SyncWorkspace();
    bool SelectRow(const Action& action);
    void ActionRequested(const Action& action);
    void ActivateFriend();
    void DeleteFriend();
    void OpenLetter();
    void WriteLetter(bool reply, bool toFriend);
    void DeleteLetters();
    void ToggleLetter(int id);
    void ToggleAllLetters();
    void ActivateWindow();
    void ToggleChat();
    void MoveSelection(int key);
    void ScrollToSelection();
    void FocusActivePane();
    void SyncDraggedPosition();
    void PublishPosition();
    void ClampToWorkspace();
    void ApplyLayout();
    void PlaceAtRest();
    Rml::Element* Panel() const;
    Rml::Element* ActivePane() const;

    CUIFriendWindow& m_Owner;
    UI::RmlBridge::ThemedView<Model> m_View{"friend_shell", [this](auto& c, auto& m) { RegisterModel(c, m); },
        {{"Data/Interface/RmlUi/friend_shell.rml"}},
        {.afterBuild = [this] { OnBuilt(); },
         .afterReload = [this] { m_RestoreScroll = true; },
         .beforeUnload = [this] { OnUnload(); }}};
    std::array<Rml::String, 22> m_Labels;
    std::vector<Action> m_Actions;
    Rml::Vector2i m_Viewport{};
    float m_DpRatio = 0;
    std::array<float, 3> m_Scroll{};
    std::wstring m_Title;
    bool m_Placed = false;
    // Placed AND the context has applied that position: before this, the panel is still centred.
    bool m_Settled = false;
    bool m_FocusPane = false;
    bool m_Maximized = false;
    bool m_CustomSize = false;
    bool m_CustomPosition = false;
    bool m_RestoreScroll = false;
    float m_Left = 0, m_Top = 0, m_Width = 0, m_Height = 0;
    std::array<float, 4> m_RestoreRect{};
};
}
