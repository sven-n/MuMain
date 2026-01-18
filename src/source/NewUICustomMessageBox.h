#ifndef _NEWUICUSOMMESSAGEBOX_H_
#define _NEWUICUSOMMESSAGEBOX_H_

#pragma once

#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"
#include "UIControls.h"

namespace SEASON3B
{
    enum
    {
        INPUTBOX_TYPE_NUMBER,
        INPUTBOX_TYPE_TEXT,
    };

    enum KEYPAD_TYPE
    {
        KEYPAD_TYPE_MOVE = 1,
        KEYPAD_TYPE_UNLOCK = 2,
        KEYPAD_TYPE_LOCK_FIRST = 3,
        KEYPAD_TYPE_LOCK_SECOND = 4,
        KEYPAD_TYPE_LOCK_FINAL = 5,
    };

    enum
    {
        INPUTBOX_WIDTH = 50,
        INPUTBOX_HEIGHT = 12,

        INPUTBOX_TEXTLIMIT = 8,
    };

    class CNewUITextInputMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            INPUTBOX_TOP_BLANK = 10,
        };

        typedef struct _MSGBOX_TEXTDATA
        {
            std::wstring strMsg;
            DWORD dwColor;
            BYTE byFontType;

            _MSGBOX_TEXTDATA()
            {
                strMsg = L"";
                dwColor = 0xffffffff;
                byFontType = MSGBOX_FONT_NORMAL;
            }
        } MSGBOX_TEXTDATA;

        typedef std::vector<MSGBOX_TEXTDATA*> type_vector_msgdata;
        typedef std::wstring type_string;

    public:
        CNewUITextInputMsgBox();
        virtual ~CNewUITextInputMsgBox();

        bool Create(DWORD dwMsgBoxType, DWORD dwInputType, int iInputBoxWidth = 100, int iInputBoxHeight = 14, int iLimitText = 256, bool bIsPassword = false);
        void Release();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        bool Update();
        bool Render();

        DWORD GetMsgBoxType();

        void GetInputBoxText(wchar_t* strText);
        void SetInputBoxOption(int iOption);
        void SetInputBoxPosition(int x, int y);
        void SetInputBoxSize(int width, int height);

    private:
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);
        void SetButtonInfo();
        void AddButtonBlank(int iAddLine);
        void RenderTexts();
        void RenderButtons();

        DWORD m_dwMsgBoxType;
        DWORD m_dwInputType;

        CUITextInputBox* m_pInputBox;
        type_vector_msgdata m_MsgTextList;

        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;

    public:
        void SetPassword(WORD password) { m_password = password; }
        WORD GetPassword() { return m_password; }

    private:
        WORD m_password;
    };

    class CNewUIKeyPadButton : public CNewUIMessageBoxButton
    {
    public:
        void Render();
    };

    class CNewUIDeleteKeyPadButton : public CNewUIMessageBoxButton
    {
    public:
        void Render();
    };

    class CNewUIKeyPadMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MSGBOX_MIDDLE_FRAME_NUM = 9,
            KEYPAD_INPUT_NUM = 2,
            KEYPAD_WIDTH = 32,
            KEYPAD_HEIGHT = 32,
        };

        typedef struct _MSGBOX_TEXTDATA
        {
            std::wstring strMsg;
            DWORD dwColor;
            BYTE byFontType;

            _MSGBOX_TEXTDATA()
            {
                strMsg = L"";
                dwColor = 0xffffffff;
                byFontType = MSGBOX_FONT_NORMAL;
            }
        } MSGBOX_TEXTDATA;

        typedef std::vector<MSGBOX_TEXTDATA*> type_vector_msgdata;
        typedef std::wstring type_string;

    public:
        CNewUIKeyPadMsgBox();
        virtual ~CNewUIKeyPadMsgBox();

        bool Create(DWORD dwType, int iInputLImit = 4);
        void Release();

        bool Update();
        bool Render();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        int GetInputLimit();
        int GetInputSize();
        void ClearInput();
        const wchar_t* GetInputText();
        void SetCheckInputText(const wchar_t* strInput);
        bool IsCheckInput();
        bool IsAllSameNumber();

        void SetStoragePassword(WORD wPassword);
        WORD GetStoragePassword();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DeleteBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT KeyPadBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetButtonInfo();

        void KeyPadInput(int iInput);
        void DeleteKeyPadInput();

        void RenderFrame();
        void RenderKeyPadInput();
        void RenderTexts();
        void RenderButtons();

        DWORD m_dwType;

        int m_iInputLimit;
        int m_iKeyPadMapping[MAX_KEYPADINPUT];
        wchar_t m_strKeyPadInput[MAX_PASSWORD_SIZE + 1];
        wchar_t m_strCheckKeyPadInput[MAX_KEYPADINPUT + 1];

        WORD m_wStoragePassword;
        type_vector_msgdata m_MsgTextList;
        CNewUIKeyPadButton m_BtnKeyPad[MAX_KEYPADINPUT];
        CNewUIDeleteKeyPadButton m_BtnDeleteKeyPad;
        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CUseFruitCheckMsgBox : public CNewUIMessageBoxBase, public INewUI3DRenderObj
    {
        enum
        {
            MSGBOX_TEXT_MAXWIDTH_3DITEM = 120,
            MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60,
            MSGBOX_3DITEM_WIDTH = 40,
            MSGBOX_3DITEM_HEIGHT = 40,
        };
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

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AddBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT MinusBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnAdd;
        CNewUIMessageBoxButton m_BtnMinus;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CGemIntegrationMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MIDDLE_COUNT = 5,
            BTN_TOP_BLANK = 60,
            BTN_GAP = 40,
        };
    public:
        CGemIntegrationMsgBox();
        virtual ~CGemIntegrationMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT UnityBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnUnity;
        CNewUIMessageBoxButton m_BtnDisjoint;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CGemIntegrationUnityMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            STATE_BASIC,
        };
        enum
        {
            MIDDLE_COUNT = 10,
            BTN_TOP_BLANK = 60,
            BTN_GAP = 40,
        };
    public:
        CGemIntegrationUnityMsgBox();
        virtual ~CGemIntegrationUnityMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();
        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT TenBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT TwentyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ThirtyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SelectMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
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
        std::vector<CNewUIMessageBoxButton>	m_cJewelButton;
        std::vector<CNewUIMessageBoxButton>	m_cMixButton;
        int									m_nMiddleCount;

        CNewUIMessageBoxButton m_BtnBlessing;
        CNewUIMessageBoxButton m_BtnSoul;
        CNewUIMessageBoxButton m_BtnTen;
        CNewUIMessageBoxButton m_BtnTwenty;
        CNewUIMessageBoxButton m_BtnThirty;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CGemIntegrationDisjointMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CGemIntegrationDisjointMsgBox();
        virtual ~CGemIntegrationDisjointMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnBlessing;
        CNewUIMessageBoxButton m_BtnSoul;
        CNewUIMessageBoxButton m_BtnDisjoint;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CSystemMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CSystemMenuMsgBox();
        virtual ~CSystemMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();
        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GameOverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChooseServerBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChooseCharacterBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OptionBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();
        void RenderFrame();
        void RenderButtons();

        // button
        CNewUIMessageBoxButton m_BtnGameOver;
        CNewUIMessageBoxButton m_BtnChooseServer;
        CNewUIMessageBoxButton m_BtnChooseCharacter;
        CNewUIMessageBoxButton m_BtnOption;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CBloodCastleResultMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MIDDLE_COUNT = 6,
        };
    public:
        CBloodCastleResultMsgBox();
        virtual ~CBloodCastleResultMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CNewUIMessageBoxButton m_BtnOk;
    };

    class CDevilSquareRankMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MIDDLE_COUNT1 = 11,
            MIDDLE_COUNT2 = 3,
        };
    public:
        CDevilSquareRankMsgBox();
        virtual ~CDevilSquareRankMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CNewUIMessageBoxButton m_BtnOk;
    };

    class CChaosCastleResultMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MIDDLE_COUNT = 6,
        };
    public:
        CChaosCastleResultMsgBox();
        virtual ~CChaosCastleResultMsgBox();

        bool Create(float fPriority = 3.f);

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void RenderFrame();

        CNewUIMessageBoxButton m_BtnOk;
    };

    class CChaosMixMenuMsgBox : public CNewUIMessageBoxBase
    {
        enum
        {
            MIDDLE_COUNT = 13,
        };
    public:
        CChaosMixMenuMsgBox();
        virtual ~CChaosMixMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GeneralMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ChaosMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Mix380BtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnGeneralMix;
        CNewUIMessageBoxButton m_BtnChaosMix;
        CNewUIMessageBoxButton m_BtnMix380;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CTrainerMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CTrainerMenuMsgBox();
        ~CTrainerMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ReviveBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnRecover;
        CNewUIMessageBoxButton m_BtnRevive;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CTrainerRecoverMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CTrainerRecoverMsgBox();
        ~CTrainerRecoverMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverDarkSpiritrBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RecoverDarkHorseBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnRecoverDarkSpirit;
        CNewUIMessageBoxButton m_BtnRecoverDarkHorse;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CElpisMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CElpisMsgBox();
        ~CElpisMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AboutRefinaryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AboutJewelOfHarmonyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RefineBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        void SetMessageType(int iMessageType) { m_iMessageType = iMessageType; }

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnAboutRefinary;
        CNewUIMessageBoxButton m_BtnAboutJewelOfHarmony;
        CNewUIMessageBoxButton m_BtnRefine;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
        int m_iMessageType;
    };

    class CDialogMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CDialogMsgBox();
        ~CDialogMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT EndBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        bool Update();
        bool Render();

    private:
        void SetAddCallbackFunc();
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

        void SetButtonInfo();
        void AddButtonBlank(int iAddLine);

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // text
        type_vector_msgdata m_MsgDataList;
        // button
        CNewUIMessageBoxButton m_BtnEnd;
    };

    class CProgressMsgBox : public CNewUIMessageBoxBase
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

        static CALLBACK_RESULT ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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

    class CCursedTempleProgressMsgBox : public CNewUIMessageBoxBase
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

        static CALLBACK_RESULT ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CompleteProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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

    class CDuelMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CDuelMsgBox();
        ~CDuelMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButton();

        // button
        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CDuelResultMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CDuelResultMsgBox();
        ~CDuelResultMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        void SetIDs(wchar_t* pszWinnerID, wchar_t* pszLoserID);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButton();

        // button
        CNewUIMessageBoxButton m_BtnOk;
        wchar_t m_szWinnerID[24];
        wchar_t m_szLoserID[24];
    };

    class CGuild_ToPerson_Position : public CNewUIMessageBoxBase
    {
        enum
        {
            STATE_BASIC,
        };
        enum
        {
            MIDDLE_COUNT = 10,
            BTN_TOP_BLANK = 60,
            BTN_GAP = 40,
        };
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

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnBlessing;
        CNewUIMessageBoxButton m_BtnSoul;
        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CCherryBlossomMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CCherryBlossomMsgBox();
        ~CCherryBlossomMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT WhiteCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RedCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT GodCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnWhiteCB;
        CNewUIMessageBoxButton m_BtnRedCB;
        CNewUIMessageBoxButton m_BtnGoldCB;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CLuckyTradeMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CLuckyTradeMenuMsgBox();
        ~CLuckyTradeMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT LuckyItemTradeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT LuckyItemRefineryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnTrade;
        CNewUIMessageBoxButton m_BtnRefinery;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CSeedMasterMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CSeedMasterMenuMsgBox();
        ~CSeedMasterMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExtractSeedBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT SeedSphereBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnExtractSeed;
        CNewUIMessageBoxButton m_BtnSeedSphere;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CSeedInvestigatorMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CSeedInvestigatorMenuMsgBox();
        ~CSeedInvestigatorMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT AttachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DetachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnAttachSocket;
        CNewUIMessageBoxButton m_BtnDetachSocket;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CResetCharacterPointMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CResetCharacterPointMsgBox();
        ~CResetCharacterPointMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ResetCharacterPointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetButtonInfo();
        bool isCharacterEquipmentItem();
        void SetAddCallbackFunc();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

    private:
        CNewUIMessageBoxButton m_ResetCharacterPointBtn;
        CNewUIMessageBoxButton m_BtnExit;
        int m_iMiddleCount;
    };

    class CDelgardoMainMenuMsgBox : public CNewUIMessageBoxBase
    {
    public:
        CDelgardoMainMenuMsgBox();
        ~CDelgardoMainMenuMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT RegBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExchangeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        // buttons
        CNewUIMessageBoxButton m_BtnReg;
        CNewUIMessageBoxButton m_BtnExchange;
        CNewUIMessageBoxButton m_BtnExit;

        int m_iMiddleCount;
    };

    class CTradeZenMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner);
    };

    class CZenReceiptMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CZenPaymentMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemValueMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopNameMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
        enum
        {
            INPUT_WIDTH = 130,
            INPUT_HEIGHT = 12,
            INPUT_TEXTLIMIT = 28,
        };
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCastleWithdrawMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageUnlockMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPasswordKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockCheckKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockFinalKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageUnlockKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
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

    class CDialogMsgBoxLayout : public TMsgBoxLayout<CDialogMsgBox>
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

    class CGuildBreakPasswordMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuild_ToPerson_PositionLayout : public TMsgBoxLayout<CGuild_ToPerson_Position>
    {
    public:
        bool SetLayout();
    };

    class CDuelMsgBoxLayout : public TMsgBoxLayout<CDuelMsgBox>
    {
    public:
        bool SetLayout();
    };

    class CDuelResultMsgBoxLayout : public TMsgBoxLayout<CDuelResultMsgBox>
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