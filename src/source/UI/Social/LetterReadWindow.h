#pragma once

#include "UI/Social/SocialWindowBase.h"
#include "UI/Social/PhotoViewer.h"

class CUILetterReadWindow : public CUIBaseWindow
{
public:
    CUILetterReadWindow();
    ~CUILetterReadWindow() override;

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void SetLetter(LETTERLIST_TEXT* pLetterHead, const wchar_t* pLetterText);
    // The three button actions the view hands back.
    void Reply();
    void AskDelete();
    // -1 for the previous letter, +1 for the next; native had two copies of these twenty lines.
    void StepLetter(int direction);
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    void PullSemanticViewToFront() override;


    // Drawn in the post-RmlUi seam, not RenderOver(): the panel would cover it otherwise.

protected:
    BOOL HandleMessage() override;

public:
    CUIPhotoViewer m_Photo;

private:
    LETTERLIST_TEXT m_LetterHead;
    std::unique_ptr<UI::Social::LetterReadView> m_View;
};
