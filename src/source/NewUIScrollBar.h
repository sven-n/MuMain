//=============================================================================
//	NewUIScrollBar.h
//=============================================================================
#pragma once
#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIChatLogWindow.h"

namespace SEASON3B
{
    class CNewUIScrollBar : public CNewUIObj
    {
        enum KEnumTypeIMAGE_LIST
        {
            IMAGE_SCROLL_TOP = CNewUIChatLogWindow::IMAGE_SCROLL_TOP,			// newui_scrollbar_up.tga (7,3)
            IMAGE_SCROLL_MIDDLE = CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,			// newui_scrollbar_m.tga (7,15)
            IMAGE_SCROLL_BOTTOM = CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,			// newui_scrollbar_down.tga (7,3)
            IMAGE_SCROLLBAR_ON = CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,			// newui_scroll_On.tga (15,30)
            IMAGE_SCROLLBAR_OFF = CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,			// newui_scroll_Off.tga (15,30)
        };

        enum KEnumTypeSCROLLBTN
        {
            SCROLLBTN_WIDTH = 15,
            SCROLLBTN_HEIGHT = 30,
            SCROLLBAR_TOP_WIDTH = 7,
            SCROLLBAR_TOP_HEIGHT = 3,
            SCROLLBAR_MIDDLE_WIDTH = 7,
            SCROLLBAR_MIDDLE_HEIGHT = 15
        };

        enum KEnumTypeSCROLLBAR_MOUSEBTN
        {
            SCROLLBAR_MOUSEBTN_INVALID = -1,
            SCROLLBAR_MOUSEBTN_FIRST,
            SCROLLBAR_MOUSEBTN_NORMAL = SCROLLBAR_MOUSEBTN_FIRST,
            SCROLLBAR_MOUSEBTN_OVER,
            SCROLLBAR_MOUSEBTN_CLICKED,
            SCROLLBAR_MOUSEBTN_TOTAL,
            SCROLLBAR_MOUSEBTN_LAST = SCROLLBAR_MOUSEBTN_TOTAL - 1,
        };

    protected:
        int							m_iHeight;
        POINT						m_ptPos;

        POINT						m_ptScrollBtnStartPos;
        POINT						m_ptScrollBtnPos;

        int							m_iScrollBarPickGap;

        int							m_iScrollBarMovePixel;
        int							m_iScrollBarHeightPixel;
        int							m_iScrollBarMiddleNum;
        int							m_iScrollBarMiddleRemainderPixel;

        int							m_iScrollBtnMouseEvent;
        bool						m_bScrollBtnActive;

        float						m_fPercentOfSize;

        int							m_iBeginPos;
        int							m_iCurPos;
        int							m_iMaxPos;

    public:
        CNewUIScrollBar();
        virtual ~CNewUIScrollBar();

        bool Create(int iX, int iY, int iHeight);
        void Release();

        float GetLayerDepth();

        void LoadImages();
        void UnloadImages();

        void SetPos(int x, int y);

        bool UpdateBtnEvent();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();

        bool Update();
        bool Render();

        void UpdateScrolling();
        void ScrollUp(int iMoveValue);
        void ScrollDown(int iMoveValue);

        float GetPercent() { return m_fPercentOfSize; }
        void SetPercent(float fPercent);

        int GetMaxPos() { return m_iMaxPos; }
        void SetMaxPos(int iMaxPos);

        int GetCurPos() { return m_iCurPos; }
        void SetCurPos(int iMoveValue);
    };
}
