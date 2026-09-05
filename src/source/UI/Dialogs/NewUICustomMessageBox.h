#ifndef _NEWUICUSOMMESSAGEBOX_H_
#define _NEWUICUSOMMESSAGEBOX_H_

#pragma once

#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "UI/Widgets/UIControls.h"

namespace mu::ui::window
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

    static constexpr float INPUTBOX_WIDTH = 50.0f;
    static constexpr float INPUTBOX_HEIGHT = 12.0f;
    static constexpr int INPUTBOX_TEXTLIMIT = 8;

    class CTextInputMsgBox : public CMessageBoxBase
    {
        static constexpr float INPUTBOX_TOP_BLANK = 10.0f;

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
        CTextInputMsgBox();
        virtual ~CTextInputMsgBox();

        bool Create(DWORD dwMsgBoxType, DWORD dwInputType, int iInputBoxWidth = 100, int iInputBoxHeight = 14, int iLimitText = 256, bool bIsPassword = false);
        void Release();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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

        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;

    public:
        void SetPassword(WORD password) { m_password = password; }
        WORD GetPassword() { return m_password; }

    private:
        WORD m_password;
    };

    class CKeyPadButton : public CMessageBoxButton
    {
    public:
        void Render();
    };

    class CDeleteKeyPadButton : public CMessageBoxButton
    {
    public:
        void Render();
    };

    class CKeyPadMsgBox : public CMessageBoxBase
    {
        static constexpr int MSGBOX_MIDDLE_FRAME_NUM = 9;
        static constexpr int KEYPAD_INPUT_NUM = 2;
        static constexpr float KEYPAD_WIDTH = 32.0f;
        static constexpr float KEYPAD_HEIGHT = 32.0f;

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
        CKeyPadMsgBox();
        virtual ~CKeyPadMsgBox();

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

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT DeleteBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT KeyPadBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CKeyPadButton m_BtnKeyPad[MAX_KEYPADINPUT];
        CDeleteKeyPadButton m_BtnDeleteKeyPad;
        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;
    };

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

    class CDialogMsgBox : public CMessageBoxBase
    {
    public:
        CDialogMsgBox();
        ~CDialogMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT EndBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CMessageBoxButton m_BtnEnd;
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

    class CDuelMsgBox : public CMessageBoxBase
    {
    public:
        CDuelMsgBox();
        ~CDuelMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButton();

        // button
        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;
    };

    class CDuelResultMsgBox : public CMessageBoxBase
    {
    public:
        CDuelResultMsgBox();
        ~CDuelResultMsgBox();

        bool Create(float fPriority = 3.f);
        void Release();

        bool Update();
        bool Render();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        void SetIDs(wchar_t* pszWinnerID, wchar_t* pszLoserID);

    private:
        void SetAddCallbackFunc();
        void SetButtonInfo();

        void RenderFrame();
        void RenderTexts();
        void RenderButton();

        // button
        CMessageBoxButton m_BtnOk;
        wchar_t m_szWinnerID[24];
        wchar_t m_szLoserID[24];
    };

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

    class CTradeZenMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner);
    };

    class CZenReceiptMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CZenPaymentMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemValueMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopNameMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
        static constexpr float INPUT_WIDTH = 130.0f;
        static constexpr float INPUT_HEIGHT = 12.0f;
        static constexpr int INPUT_TEXTLIMIT = 28;
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCastleWithdrawMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageUnlockMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPasswordKeyPadMsgBoxLayout : public TMsgBoxLayout<CKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockKeyPadMsgBoxLayout : public TMsgBoxLayout<CKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockCheckKeyPadMsgBoxLayout : public TMsgBoxLayout<CKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageLockFinalKeyPadMsgBoxLayout : public TMsgBoxLayout<CKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CStorageUnlockKeyPadMsgBoxLayout : public TMsgBoxLayout<CKeyPadMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
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

    class CGuildBreakPasswordMsgBoxLayout : public TMsgBoxLayout<CTextInputMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT ReturnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

    private:
        static CALLBACK_RESULT ProcessOk(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
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