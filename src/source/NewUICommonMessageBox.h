// NewUICommonMessageBox.h: interface for the NewUICommonMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
#define AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_

#pragma once

#include "NewUIMessageBox.h"
#include "NewUI3DRenderMng.h"

namespace SEASON3B
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

    enum
    {
        SCREEN_WIDTH = 640,
        SCREEN_HEIGHT = 480,

        MSGBOX_WIDTH = 230,
        MSGBOX_TOP_HEIGHT = 67,
        MSGBOX_BOTTOM_HEIGHT = 50,
        MSGBOX_MIDDLE_HEIGHT = 15,

        MSGBOX_BACK_BLANK_WIDTH = 8,
        MSGBOX_BACK_BLANK_HEIGHT = 10,

        MSGBOX_TEXT_TOP_BLANK = 35,
        MSGBOX_TEXT_MAXWIDTH = 180,

        MSGBOX_LINE_WIDTH = 223,
        MSGBOX_LINE_HEIGHT = 21,

        MSGBOX_SEPARATE_LINE_WIDTH = 205,
        MSGBOX_SEPARATE_LINE_HEIGHT = 2,

        MSGBOX_BTN_WIDTH = 54,
        MSGBOX_BTN_HEIGHT = 30,
        MSGBOX_BTN_BOTTOM_BLANK = 20,

        MSGBOX_BTN_EMPTY_SMALL_WIDTH = 64,
        MSGBOX_BTN_EMPTY_WIDTH = 108,
        MSGBOX_BTN_EMPTY_BIG_WIDTH = 180,
        MSGBOX_BTN_EMPTY_HEIGHT = 29,
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

    class CNewUIMessageBoxButton
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

        CNewUIMessageBoxButton();
        ~CNewUIMessageBoxButton();

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

    class CNewUICommonMessageBox : public CNewUIMessageBoxBase
    {
    public:
        CNewUICommonMessageBox();
        ~CNewUICommonMessageBox();

        DWORD GetType();

        bool Create(DWORD dwType, float fPriority = 3.f);
        bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);

        void SetPos(int x, int y);

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    //////////////////////////////////////////////////////////////////////////

    class CNewUI3DItemCommonMsgBox : public CNewUIMessageBoxBase, public INewUI3DRenderObj
    {
        enum
        {
            MSGBOX_TEXT_MAXWIDTH_3DITEM = 120,
            MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60,
            MSGBOX_3DITEM_WIDTH = 40,
            MSGBOX_3DITEM_HEIGHT = 40,
        };
    public:
        CNewUI3DItemCommonMsgBox();
        ~CNewUI3DItemCommonMsgBox();

        DWORD GetType();

        bool Create(DWORD dwType, float fPriority = 3.f);
        bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);
        void Release();

        void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

        void Set3DItem(ITEM* pItem);
        void SetItemValue(int iValue);
        int GetItemValue();

        static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

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
        CNewUIMessageBoxButton m_BtnOk;
        CNewUIMessageBoxButton m_BtnCancel;
    };

    class CFenrirRepairMsgBox : public CNewUICommonMessageBox
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

    class CServerLostMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildRequestMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildFireMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMapEnterWerwolfMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMapEnterGateKeeperMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPartyMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CTradeMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CTradeAlertMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildWarMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CBattleSoccerMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CServerImmigrationErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalshopCreateMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CFenrirRepairMsgBoxLayout : public TMsgBoxLayout<CFenrirRepairMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CInfinityArrowCancelMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CBuffSwellOfMPCancelMsgBoxLayOut : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationUnityCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationUnityResultMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationDisjointCheckMsgBoxLayout :public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGemIntegrationDisjointResultMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CChaosCastleTimeCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CHarvestEventLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CWhiteAngelEventLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCanNotUseWordMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
    };

    class CMixCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseReviveCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUsePortalCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CReturnPortalCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDuelCreateErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDuelWatchErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CDoppelGangerMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildRelationShipMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCastleMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSiegeLevelMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CSiegeGiveUpMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CGatemanMoneyMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
    class CGatemanFailMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CQuestGiveUpMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

#ifdef ASG_ADD_TIME_LIMIT_QUEST
    class CQuestCountLimitMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
#endif	// ASG_ADD_TIME_LIMIT_QUEST

    class CHighValueItemCheckMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseFruitMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUsePartChargeFruitMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class COsbourneMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemValueCheckMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CPersonalShopItemBuyMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildOutPerson : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildBreakMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildPerson_Get_Out : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGuildPerson_Cancel_Position_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnionGuild_Break_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnionGuild_Out_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CMaster_Level_Interface : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Get_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Dont_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Dont_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Wat_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Destroy_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Ing_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CCry_Wolf_Result_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseSantaInvitationMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSantaTownLeaveMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CSantaTownSantaMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUseRegistLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CRegistOverLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CExchangeLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CExchangeLuckyCoinInvenErrMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CGambleBuyMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CEmpireGuardianMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CUnitedMarketPlaceMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };

    class CLuckyItemMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
    {
    public:
        bool SetLayout();
        static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
        static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    };
}

#endif // !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
