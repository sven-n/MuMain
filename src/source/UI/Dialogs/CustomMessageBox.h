#ifndef _NEWUICUSOMMESSAGEBOX_H_
#define _NEWUICUSOMMESSAGEBOX_H_

#pragma once

#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Widgets/UIControls.h"

namespace mu::ui::window
{
    class CUseFruitCheckMsgBox : public CMessageBoxBase, public I3DRenderObj
    {
        static constexpr float MSGBOX_TEXT_MAXWIDTH_3DITEM = 120.0f;
        static constexpr float MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60.0f;
        static constexpr float MSGBOX_3DITEM_WIDTH = 40.0f;
        static constexpr float MSGBOX_3DITEM_HEIGHT = 40.0f;
    public:
        CUseFruitCheckMsgBox();
        virtual ~CUseFruitCheckMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        void Set3DItem(ITEM* pItem);

        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AddBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT MinusBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        ITEM m_Item;

        type_vector_msgdata m_MsgDataList;

        // button
        CMessageBoxButton m_BtnAdd;
        CMessageBoxButton m_BtnMinus;
        CMessageBoxButton m_BtnCancel;
    };

    class CGemIntegrationMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 5.0f;
        static constexpr float BTN_TOP_BLANK = 60.0f;
        static constexpr float BTN_GAP = 40.0f;
    public:
        CGemIntegrationMsgBox();
        virtual ~CGemIntegrationMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT UnityBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DisjointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // texts
        type_vector_msgdata m_MsgDataList;
        // button
        CMessageBoxButton m_BtnUnity;
        CMessageBoxButton m_BtnDisjoint;
        CMessageBoxButton m_BtnCancel;
    };

    class CGemIntegrationUnityMsgBox : public CMessageBoxBase
    {
        enum
        {
            STATE_BASIC,
        };
        static constexpr float MIDDLE_COUNT = 10.0f;
        static constexpr float BTN_TOP_BLANK = 60.0f;
        static constexpr float BTN_GAP = 40.0f;
    public:
        CGemIntegrationUnityMsgBox();
        virtual ~CGemIntegrationUnityMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();
        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT TenBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT TwentyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ThirtyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SelectMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // texts
        type_vector_msgdata m_MsgDataList;
        // button

        void SetText(void);
        void ResetWndSize(int _nType);
        std::vector<CMessageBoxButton>	m_cJewelButton;
        std::vector<CMessageBoxButton>	m_cMixButton;
        int									m_nMiddleCount;

        CMessageBoxButton m_BtnBlessing;
        CMessageBoxButton m_BtnSoul;
        CMessageBoxButton m_BtnTen;
        CMessageBoxButton m_BtnTwenty;
        CMessageBoxButton m_BtnThirty;
        CMessageBoxButton m_BtnCancel;
    };

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

        void RenderFrame();
        void RenderTexts();
        void RenderGemList();
        void RenderButtons();

        int m_iMiddleFrameCount;

        // texts
        type_vector_msgdata m_MsgDataList;
        // button
        CMessageBoxButton m_BtnBlessing;
        CMessageBoxButton m_BtnSoul;
        CMessageBoxButton m_BtnDisjoint;
        CMessageBoxButton m_BtnCancel;
    };

    class CSystemMenuMsgBox : public CMessageBoxBase
    {
    public:
        CSystemMenuMsgBox();
        virtual ~CSystemMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();
        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GameOverBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChooseServerBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChooseCharacterBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OptionBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();
        void RenderFrame();
        void RenderButtons();

        // button
        CMessageBoxButton m_BtnGameOver;
        CMessageBoxButton m_BtnChooseServer;
        CMessageBoxButton m_BtnChooseCharacter;
        CMessageBoxButton m_BtnOption;
        CMessageBoxButton m_BtnCancel;
    };

    class CBloodCastleResultMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 6.0f;
    public:
        CBloodCastleResultMsgBox();
        virtual ~CBloodCastleResultMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
    };

    class CDevilSquareRankMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT1 = 11.0f;
        static constexpr float MIDDLE_COUNT2 = 3.0f;
    public:
        CDevilSquareRankMsgBox();
        virtual ~CDevilSquareRankMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
    };

    class CChaosCastleResultMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 6.0f;
    public:
        CChaosCastleResultMsgBox();
        virtual ~CChaosCastleResultMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CMessageBoxButton m_BtnOk;
    };

    class CChaosMixMenuMsgBox : public CMessageBoxBase
    {
        static constexpr float MIDDLE_COUNT = 13.0f;
    public:
        CChaosMixMenuMsgBox();
        virtual ~CChaosMixMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GeneralMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChaosMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Mix380BtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnGeneralMix;
        CMessageBoxButton m_BtnChaosMix;
        CMessageBoxButton m_BtnMix380;
        CMessageBoxButton m_BtnCancel;
    };

    class CTrainerMenuMsgBox : public CMessageBoxBase
    {
    public:
        CTrainerMenuMsgBox();
        ~CTrainerMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ReviveBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnRecover;
        CMessageBoxButton m_BtnRevive;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CTrainerRecoverMsgBox : public CMessageBoxBase
    {
    public:
        CTrainerRecoverMsgBox();
        ~CTrainerRecoverMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverDarkSpiritrBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverDarkHorseBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnRecoverDarkSpirit;
        CMessageBoxButton m_BtnRecoverDarkHorse;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CElpisMsgBox : public CMessageBoxBase
    {
    public:
        CElpisMsgBox();
        ~CElpisMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AboutRefinaryBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AboutJewelOfHarmonyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RefineBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        void SetMessageType(int iMessageType) { m_iMessageType = iMessageType; }

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnAboutRefinary;
        CMessageBoxButton m_BtnAboutJewelOfHarmony;
        CMessageBoxButton m_BtnRefine;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
        int m_iMessageType;
    };

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

        void RenderFrame();
        void RenderTexts();
        void RenderProgress();

    private:
        type_vector_msgdata m_MsgDataList;

        DWORD m_dwStartTime;
        DWORD m_dwEndTime;
        DWORD m_dwElapseTime;
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

    private:
        type_vector_msgdata m_MsgDataList;

        DWORD m_dwStartTime;
        DWORD m_dwEndTime;
        DWORD m_dwElapseTime;

        DWORD m_dwNpcIndex;
    };

    // CDuelMsgBox/CDuelResultMsgBox ported to CGenericConfirmDialog's portrait2D field --
    // see docs/rmlui-ui-system/dialog-migration-plan.md.

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

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // texts
        type_vector_msgdata m_MsgDataList;
        // button
        CMessageBoxButton m_BtnBlessing;
        CMessageBoxButton m_BtnSoul;
        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;
    };

    class CCherryBlossomMsgBox : public CMessageBoxBase
    {
    public:
        CCherryBlossomMsgBox();
        ~CCherryBlossomMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT WhiteCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RedCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GodCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnWhiteCB;
        CMessageBoxButton m_BtnRedCB;
        CMessageBoxButton m_BtnGoldCB;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CLuckyTradeMenuMsgBox : public CMessageBoxBase
    {
    public:
        CLuckyTradeMenuMsgBox();
        ~CLuckyTradeMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT LuckyItemTradeBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT LuckyItemRefineryBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnTrade;
        CMessageBoxButton m_BtnRefinery;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CSeedMasterMenuMsgBox : public CMessageBoxBase
    {
    public:
        CSeedMasterMenuMsgBox();
        ~CSeedMasterMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExtractSeedBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SeedSphereBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnExtractSeed;
        CMessageBoxButton m_BtnSeedSphere;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CSeedInvestigatorMenuMsgBox : public CMessageBoxBase
    {
    public:
        CSeedInvestigatorMenuMsgBox();
        ~CSeedInvestigatorMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AttachSocketBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DetachSocketBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnAttachSocket;
        CMessageBoxButton m_BtnDetachSocket;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CResetCharacterPointMsgBox : public CMessageBoxBase
    {
    public:
        CResetCharacterPointMsgBox();
        ~CResetCharacterPointMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ResetCharacterPointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetButtonInfo();
        bool isCharacterEquipmentItem();
        void SetAddCallbackFunc();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

    private:
        CMessageBoxButton m_ResetCharacterPointBtn;
        CMessageBoxButton m_BtnExit;
        int m_iMiddleCount;
    };

    class CDelgardoMainMenuMsgBox : public CMessageBoxBase
    {
    public:
        CDelgardoMainMenuMsgBox();
        ~CDelgardoMainMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RegBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExchangeBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CMessageBoxButton m_BtnReg;
        CMessageBoxButton m_BtnExchange;
        CMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CUseFruitCheckMsgBoxLayout : public TMsgBoxLayout<CUseFruitCheckMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CGemIntegrationMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CGemIntegrationUnityMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationUnityMsgBox>
    {
    public:
        bool SetLayout();
    };

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

    class CChaosMixMenuMsgBoxLayout : public TMsgBoxLayout<CChaosMixMenuMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CTrainerMenuMsgBoxLayout : public TMsgBoxLayout<CTrainerMenuMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CTrainerRecoverMsgBoxLayout : public TMsgBoxLayout<CTrainerRecoverMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CElpisMsgBoxLayout : public TMsgBoxLayout<CElpisMsgBox>
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

    class CSystemMenuMsgBoxLayout : public TMsgBoxLayout<CSystemMenuMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CGuild_ToPerson_PositionLayout : public TMsgBoxLayout<CGuild_ToPerson_Position>
    {
    public:
        bool SetLayout();
    };

    class CCherryBlossomMsgBoxLayout : public TMsgBoxLayout<CCherryBlossomMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CSeedMasterMenuMsgBoxLayout : public TMsgBoxLayout<CSeedMasterMenuMsgBox>
    {
    public:
        bool SetLayout();
    };
    class CSeedInvestigatorMenuMsgBoxLayout : public TMsgBoxLayout<CSeedInvestigatorMenuMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CResetCharacterPointMsgBoxLayout : public TMsgBoxLayout<CResetCharacterPointMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CDelgardoMainMenuMsgBoxLayout : public TMsgBoxLayout<CDelgardoMainMenuMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CLuckyTradeMenuMsgBoxLayout : public TMsgBoxLayout<CLuckyTradeMenuMsgBox>
    {
    public:
        bool SetLayout();
    };
}

#endif // _NEWUICUSOMMESSAGEBOX_H_