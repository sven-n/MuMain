#pragma once

#include "UI/Social/SocialWindowBase.h"

class CUIFriendWindow : public CUIBaseWindow
{
public:
    CUIFriendWindow();
    ~CUIFriendWindow() override;
    void Init(const wchar_t* title, DWORD parent = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void Reset();
    void Close();
    void RefreshPalList();
    void RefreshLetterList();
    void AddWindow(DWORD id, const wchar_t* title);
    void RemoveWindow(DWORD id);
    void ResetWindow();
    DWORD GetCurrentSelectedWindow();
    LETTERLIST_TEXT* GetCurrentSelectedLetter();
    void PrevNextCursorMove(int line);
    void SetTabIndex(int tab);
    int GetTabIndex();
    bool HasSemanticView() const override { return true; }
    bool SyncSemanticView(bool shown) override;
    void PullSemanticViewToFront() override;
    void RestoreSemanticLayout(int x, int y, int width, int height);
    void RestoreSemanticMaximized();

protected:
    BOOL HandleMessage() override;

private:
    float SemanticScaleRatio() const;

    std::unique_ptr<UI::Social::FriendShell> m_Shell;
};
