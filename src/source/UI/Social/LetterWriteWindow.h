#pragma once

#include "UI/Social/SocialWindowBase.h"
#include "UI/Social/PhotoViewer.h"

class CUILetterWriteWindow : public CUIBaseWindow
{
public:
    CUILetterWriteWindow();
    ~CUILetterWriteWindow() override;

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void SetMailtoText(const wchar_t* pszText);
    void SetMainTitleText(const wchar_t* pszText);
    void SetMailContextText(const wchar_t* pszText);
    void SetSendState(BOOL bFlag);
    // The two button actions the view hands back.
    void Send();
    void RequestClose();

    BOOL CloseCheck() override;
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    bool SemanticFieldHasFocus() const override;
    void PullSemanticViewToFront() override;


    // Drawn in the post-RmlUi seam, not RenderOver(): the panel would cover it otherwise.
    void RenderAboveRmlUi() override;

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

public:
    CUIPhotoViewer m_Photo;

private:
    BOOL m_bIsSend;
    std::unique_ptr<UI::Social::LetterWriteView> m_View;
};
