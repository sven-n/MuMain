// NewUICommonMessageBox.h: interface for the NewUICommonMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
#define AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_

#pragma once

#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Core/NewUI3DRenderMng.h"

namespace mu::ui::window
{
    enum
    {
        MSGBOX_COMMON_TYPE_OK,
        MSGBOX_COMMON_TYPE_OKCANCEL,
    };

    enum
    {
        MSGBOX_FONT_NORMAL,
        MSGBOX_FONT_BOLD,
    };

    static constexpr float SCREEN_WIDTH = (float)REFERENCE_WIDTH;
    static constexpr float SCREEN_HEIGHT = (float)REFERENCE_HEIGHT;

    static constexpr float MSGBOX_WIDTH = 230.0f;
    static constexpr float MSGBOX_TOP_HEIGHT = 67.0f;
    static constexpr float MSGBOX_BOTTOM_HEIGHT = 50.0f;
    static constexpr float MSGBOX_MIDDLE_HEIGHT = 15.0f;

    static constexpr float MSGBOX_BACK_BLANK_WIDTH = 8.0f;
    static constexpr float MSGBOX_BACK_BLANK_HEIGHT = 10.0f;

    static constexpr float MSGBOX_TEXT_TOP_BLANK = 35.0f;
    static constexpr float MSGBOX_TEXT_MAXWIDTH = 180.0f;

    static constexpr float MSGBOX_LINE_WIDTH = 223.0f;
    static constexpr float MSGBOX_LINE_HEIGHT = 21.0f;

    static constexpr float MSGBOX_SEPARATE_LINE_WIDTH = 205.0f;
    static constexpr float MSGBOX_SEPARATE_LINE_HEIGHT = 2.0f;

    static constexpr float MSGBOX_BTN_WIDTH = 54.0f;
    static constexpr float MSGBOX_BTN_HEIGHT = 30.0f;
    static constexpr float MSGBOX_BTN_BOTTOM_BLANK = 20.0f;

    static constexpr float MSGBOX_BTN_EMPTY_SMALL_WIDTH = 64.0f;
    static constexpr float MSGBOX_BTN_EMPTY_WIDTH = 108.0f;
    static constexpr float MSGBOX_BTN_EMPTY_BIG_WIDTH = 180.0f;
    static constexpr float MSGBOX_BTN_EMPTY_HEIGHT = 29.0f;

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

    class CMessageBoxButton
    {
    public:
        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_BTN_HOVER,
            EVENT_BTN_DOWN,
        };

        enum BTN_SIZE_TYPE
        {
            MSGBOX_BTN_CUSTOM = 0,
            MSGBOX_BTN_SIZE_OK,
            MSGBOX_BTN_SIZE_EMPTY,
            MSGBOX_BTN_SIZE_EMPTY_SMALL,
            MSGBOX_BTN_SIZE_EMPTY_BIG,
        };

        CMessageBoxButton();
        ~CMessageBoxButton();

        bool IsMouseIn();
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_CUSTOM, bool bClickEffect = false);
        void MoveTextPos(int iX, int iY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_SIZE_OK);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetText(const wchar_t* strText);
        void AddBlank(int iAddLine);

        void SetEnable(bool bEnable) { m_bEnable = bEnable; }

        void SetPos(float x, float y) { m_x = x; m_y = y; }
        float GetPosX() { return m_x; }
        float GetPosY() { return m_y; }
        float GetWidth() { return m_width; }
        float GetHeight() { return m_height; }

        void ClearEventState() { m_EventState = EVENT_NONE; }
        EVENT_STATE GetEventState() { return m_EventState; }

        virtual void Update();
        virtual void Render();

    private:
        bool m_bEnable;

        DWORD m_dwTexType;
        DWORD m_dwSizeType;

       std::wstring m_strText;
        float m_x, m_y, m_width, m_height;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        float m_fButtonWidth;
        float m_fButtonHeight;
        int		m_iMoveTextPosX;
        int		m_iMoveTextPosY;
        bool	m_bClickEffect;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        EVENT_STATE m_EventState;
    };

    class CCommonMessageBox : public CMessageBoxBase
    {
    public:
        CCommonMessageBox();
        ~CCommonMessageBox();

        DWORD GetType();

        bool Create(DWORD dwType, float fPriority = 3.f);
        bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);

        void SetPos(int x, int y);

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        bool Update();
        bool Render();

        void LockOkButton();
#ifdef PBG_ADD_NAMETOPMSGBOX
    protected:
#else //PBG_ADD_NAMETOPMSGBOX
    private:
#endif //PBG_ADD_NAMETOPMSGBOX
        void SetAddCallbackFunc();
#ifdef PBG_ADD_NAMETOPMSGBOX
        int SeparateText(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, int _TextSize = MSGBOX_TEXT_MAXWIDTH);
#else //PBG_ADD_NAMETOPMSGBOX
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);
#endif //PBG_ADD_NAMETOPMSGBOX
        void SetButtonInfo();
        void AddButtonBlank(int iAddLine);
#ifdef PBG_ADD_NAMETOPMSGBOX
        void AddButtonBlank(int iAddLine, int _iImgSize);
#endif //PBG_ADD_NAMETOPMSGBOX

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        DWORD m_dwType;
        type_vector_msgdata m_MsgDataList;

        // button
        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;
    };

    //////////////////////////////////////////////////////////////////////////

    class C3DItemCommonMsgBox : public CMessageBoxBase, public I3DRenderObj
    {
        static constexpr float MSGBOX_TEXT_MAXWIDTH_3DITEM = 120.0f;
        static constexpr float MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60.0f;
        static constexpr float MSGBOX_3DITEM_WIDTH = 40.0f;
        static constexpr float MSGBOX_3DITEM_HEIGHT = 40.0f;
    public:
        C3DItemCommonMsgBox();
        ~C3DItemCommonMsgBox();

        DWORD GetType();

        bool Create(DWORD dwType, float fPriority = 3.f);
        bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        void Set3DItem(ITEM* pItem);
        void SetItemValue(int iValue);
        int GetItemValue();

        static CALLBACK_RESULT LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

    private:
        void SetAddCallbackFunc();
        int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

        void SetButtonInfo();
        void AddButtonBlank(int iAddLine);

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        DWORD m_dwType;
        ITEM m_Item;
        int m_iItemValue;

        type_vector_msgdata m_MsgDataList;

        // button
        CMessageBoxButton m_BtnOk;
        CMessageBoxButton m_BtnCancel;
    };

    class CFenrirRepairMsgBox : public CCommonMessageBox
    {
    public:
        void SetSourceIndex(int iIndex);
        void SetTargetIndex(int iIndex);
        int GetSourceIndex();
        int GetTargetIndex();

    private:
        int m_iSourceIndex;
        int m_iTargetIndex;
    };

    class CServerLostMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildRequestMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildFireMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMapEnterWerwolfMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMapEnterGateKeeperMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPartyMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CTradeMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CTradeAlertMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildWarMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CBattleSoccerMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CServerImmigrationErrorMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalshopCreateMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CFenrirRepairMsgBoxLayout : public TMsgBoxLayout<CFenrirRepairMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CInfinityArrowCancelMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CBuffSwellOfMPCancelMsgBoxLayOut : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationUnityCheckMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationUnityResultMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationDisjointCheckMsgBoxLayout :public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationDisjointResultMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CChaosCastleTimeCheckMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CHarvestEventLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CWhiteAngelEventLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCanNotUseWordMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
    };

    class CMixCheckMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseReviveCharmMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUsePortalCharmMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CReturnPortalCharmMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDuelCreateErrorMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDuelWatchErrorMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDoppelGangerMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildRelationShipMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCastleMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSiegeLevelMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CSiegeGiveUpMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CGatemanMoneyMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CGatemanFailMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CQuestGiveUpMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

#ifdef ASG_ADD_TIME_LIMIT_QUEST
    class CQuestCountLimitMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
#endif	// ASG_ADD_TIME_LIMIT_QUEST

    class CHighValueItemCheckMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseFruitMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUsePartChargeFruitMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class COsbourneMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemValueCheckMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemBuyMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildOutPerson : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildBreakMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildPerson_Get_Out : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildPerson_Cancel_Position_MsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnionGuild_Break_MsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnionGuild_Out_MsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMaster_Level_Interface : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Get_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Set_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Set_Temple1 : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Dont_Set_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Dont_Set_Temple1 : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Wat_Set_Temple1 : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Destroy_Set_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Ing_Set_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Result_Set_Temple : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseSantaInvitationMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSantaTownLeaveMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSantaTownSantaMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseRegistLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CRegistOverLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CExchangeLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CExchangeLuckyCoinInvenErrMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGambleBuyMsgBoxLayout : public TMsgBoxLayout<C3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CEmpireGuardianMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnitedMarketPlaceMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CLuckyItemMsgBoxLayout : public TMsgBoxLayout<CCommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
}

#endif // !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
