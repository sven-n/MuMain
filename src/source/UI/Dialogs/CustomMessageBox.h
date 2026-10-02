#ifndef _NEWUICUSOMMESSAGEBOX_H_
#define _NEWUICUSOMMESSAGEBOX_H_

#pragma once

#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/MessageBoxView.h"
#include "GameLogic/Items/JewelUnmixSelection.h"

namespace mu::ui::window
{
// Lahap jewel dismantling, with RmlUi row selection and scrolling.
class CGemIntegrationDisjointMsgBox : public CMessageBoxBase
{
public:
    CGemIntegrationDisjointMsgBox();
    virtual ~CGemIntegrationDisjointMsgBox();

    bool Create(float fPriority = 3.f);
    void Release();

    bool Update();
    bool Render();

    static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT DisjointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
    void SetAddCallbackFunc();
    void SetButtonInfo();
    void ChangeMiddleFrameSmall();
    void ChangeMiddleFrameBig();

    void SyncView();
    void SyncGemList();

    int m_iMiddleFrameCount;
    MessageBoxView m_View;
    std::vector<GameLogic::Items::JewelUnmixSelection::Entry> m_DisplayedItems;
    MessageBoxView::List m_ListRows;

    // texts
    type_vector_msgdata m_MsgDataList;
    // button
    CMessageBoxButton m_BtnBlessing;
    CMessageBoxButton m_BtnSoul;
    CMessageBoxButton m_BtnDisjoint;
    CMessageBoxButton m_BtnCancel;
};

    // CSystemMenuMsgBox/CSystemMenuMsgBoxLayout are now ShowSystemMenuDialog() (WindowCommon.h),
    // on CGenericMenuDialog (UI/Dialogs/GenericMenuDialog.h).

    class CBloodCastleResultMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 6.0f;
    public:
        CBloodCastleResultMsgBox();
        virtual ~CBloodCastleResultMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
        MessageBoxView m_View; // draws the box (SyncMatchResultView()); native only without it
    };

    class CDevilSquareRankMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT1 = 11.0f;
        static constexpr float MIDDLE_COUNT2 = 3.0f;
    public:
        CDevilSquareRankMsgBox();
        virtual ~CDevilSquareRankMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
        MessageBoxView m_View; // draws the box (SyncMatchResultView()); native only without it
    };

    class CChaosCastleResultMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 6.0f;
    public:
        CChaosCastleResultMsgBox();
        virtual ~CChaosCastleResultMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
        MessageBoxView m_View; // draws the box (SyncMatchResultView()); native only without it
    };

    // CChaosMixMenuMsgBox/CTrainerMenuMsgBox/CTrainerRecoverMsgBox are now
    // ShowChaosMixMenuDialog()/ShowTrainerMenuDialog()/ShowTrainerRecoverDialog() (WindowCommon.h),
    // all on CGenericMenuDialog (UI/Dialogs/GenericMenuDialog.h).

    // A notice with a progress bar that closes itself when the bar is full (the castle siege seal,
    // crown switch and crown defence notices). MessageBoxView draws it.
    class CProgressMsgBox : public CMessageBoxBase
    {
    public:
        CProgressMsgBox();
        ~CProgressMsgBox();

        bool Create(DWORD dwElapseTime = 3000, float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
        void SetElapseTime(DWORD dwElapseTime);

        bool Update();
        bool Render();

        static CALLBACK_RESULT ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

        void SyncView();

    private:
        type_vector_msgdata m_MsgDataList;

        DWORD m_dwStartTime;
        DWORD m_dwEndTime;
        DWORD m_dwElapseTime;
        MessageBoxView m_View;
    };

    class CCursedTempleProgressMsgBox : public CMessageBoxBase
    {
    public:
        CCursedTempleProgressMsgBox();
        ~CCursedTempleProgressMsgBox();

        bool Create(DWORD dwElapseTime = 3000, float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        bool Update();
        bool Render();

        void SetNpcIndex(DWORD dwIndex);
        DWORD GetNpcIndex();

        static CALLBACK_RESULT ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CompleteProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

        void RenderFrame();
        void RenderTexts();
        void RenderProgress();

        bool CheckHeroAction();
        void SyncView();

    private:
        type_vector_msgdata m_MsgDataList;

        DWORD m_dwStartTime;
        DWORD m_dwEndTime;
        DWORD m_dwElapseTime;

        DWORD m_dwNpcIndex;
        MessageBoxView m_View; // draws the box (as CProgressMsgBox's); native only without it
    };

    // CDuelMsgBox/CDuelResultMsgBox are now CGenericConfirmDialog's portrait2D field.

    // The guild master's appointment box: assistant or battle master for the selected member, OK
    // and Close. MessageBoxView draws it; the box keeps its callbacks and the request.
    class CGuild_ToPerson_Position : public CMessageBoxBase
    {
        enum
        {
            STATE_BASIC,
        };
        static constexpr float MIDDLE_COUNT = 10.0f;
        static constexpr float BTN_TOP_BLANK = 60.0f;
        static constexpr float BTN_GAP = 40.0f;
        enum GUILD_STATUS
        {
            G_NONE = (BYTE)-1,
            G_PERSON = 0,
            G_MASTER = 128,
            G_SUB_MASTER = 64,
            G_BATTLE_MASTER = 32
        };

    public:
        CGuild_ToPerson_Position();
        ~CGuild_ToPerson_Position();

        bool Create(float fPriority = 3.f);
        void Release();
        bool Update();
        bool Render();

        static CALLBACK_RESULT BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();

        MessageBoxView m_View;
    };

    // CCherryBlossomMsgBox/CLuckyTradeMenuMsgBox/CSeedMasterMenuMsgBox/CSeedInvestigatorMenuMsgBox/
    // CResetCharacterPointMsgBox/CDelgardoMainMenuMsgBox are now ShowCherryBlossomMenuDialog()/
    // ShowLuckyTradeMenuDialog()/ShowSeedMasterMenuDialog()/ShowSeedInvestigatorMenuDialog()/
    // ShowResetCharacterPointDialog()/ShowDelgardoMainMenuDialog() (WindowCommon.h), all on
    // CGenericMenuDialog (UI/Dialogs/GenericMenuDialog.h). ShowCherryBlossomMenuDialog() has no
    // live callers -- grep-confirmed zero CreateMessageBox call sites for it.

    class CGemIntegrationDisjointMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationDisjointMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CBloodCastleResultMsgBoxLayout : public TMsgBoxLayout<CBloodCastleResultMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CDevilSquareRankMsgBoxLayout : public TMsgBoxLayout<CDevilSquareRankMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CChaosCastleResultMsgBoxLayout : public TMsgBoxLayout<CChaosCastleResultMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCrownSwitchPopLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCrownSwitchPushLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCrownSwitchOtherPushLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSealRegisterStartLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSealRegisterSuccessLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSealRegisterFailLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSealRegisterOtherLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSealRegisterOtherCampLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCrownDefenseRemoveLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCrownDefenseCreateLayout : public TMsgBoxLayout<CProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCursedTempleHolicItemGetLayout : public TMsgBoxLayout<CCursedTempleProgressMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CCursedTempleHolicItemSaveLayout : public TMsgBoxLayout<CCursedTempleProgressMsgBox>
    {
    public:
        bool SetLayout();
    };


    class CGuild_ToPerson_PositionLayout : public TMsgBoxLayout<CGuild_ToPerson_Position>
    {
    public:
        bool SetLayout();
    };

}

#endif // _NEWUICUSOMMESSAGEBOX_H_