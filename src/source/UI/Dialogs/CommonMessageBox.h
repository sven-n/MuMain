
#if !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
#define AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_

#pragma once

#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/WindowManager.h"

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

    // Where a message box's button stands and what it says, in the box's stage units; the box's
    // view draws it and reports its clicks.
    class CMessageBoxButton
    {
    public:
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

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_CUSTOM, bool bClickEffect = false);
        void MoveTextPos(int iX, int iY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_SIZE_OK);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetText(const wchar_t* strText);
        void AddBlank(int iAddLine);

        void SetEnable(bool bEnable) { m_bEnable = bEnable; }
        bool IsEnabled() const
        {
            return m_bEnable;
        }

        void SetPos(float x, float y) { m_x = x; m_y = y; }
        float GetPosX() { return m_x; }
        float GetPosY() { return m_y; }
        float GetWidth() { return m_width; }
        float GetHeight() { return m_height; }

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
    };

    // CHighValueItemCheckMsgBoxLayout, CUseFruitMsgBoxLayout, CUsePartChargeFruitMsgBoxLayout,
    // CPersonalShopItemBuyMsgBoxLayout, CGambleBuyMsgBoxLayout, CPersonalShopItemValueCheckMsgBoxLayout
    // are now CGenericConfirmDialog-based.

}

#endif // !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
