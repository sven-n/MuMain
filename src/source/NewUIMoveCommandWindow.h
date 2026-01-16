// NewUIMoveCommandWindow.h: interface for the CNewUIMoveCommandWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "MoveCommandData.h"
#include "NewUIChatLogWindow.h"

namespace SEASON3B
{
    class CNewUIMoveCommandWindow : public CNewUIObj
    {
        enum IMAGE_LIST
        {
            IMAGE_MOVECOMMAND_SCROLL_TOP = CNewUIChatLogWindow::IMAGE_SCROLL_TOP,			// newui_scrollbar_up.tga (7,3)
            IMAGE_MOVECOMMAND_SCROLL_MIDDLE = CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,			// newui_scrollbar_m.tga (7,15)
            IMAGE_MOVECOMMAND_SCROLL_BOTTOM = CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,			// newui_scrollbar_down.tga (7,3)
            IMAGE_MOVECOMMAND_SCROLLBAR_ON = CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,			// newui_scroll_On.tga (15,30)
            IMAGE_MOVECOMMAND_SCROLLBAR_OFF = CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,			// newui_scroll_Off.tga (15,30)
            //IMAGE_MOVECOMMAND_DRAG_BTN		= CNewUIChatLogWindow::IMAGE_DRAG_BTN
        };

        enum
        {
            MOVECOMMAND_SCROLLBTN_WIDTH = 15,
            MOVECOMMAND_SCROLLBTN_HEIGHT = 30,
            MOVECOMMAND_SCROLLBAR_TOP_WIDTH = 7,
            MOVECOMMAND_SCROLLBAR_TOP_HEIGHT = 3,
            MOVECOMMAND_SCROLLBAR_MIDDLE_WIDTH = 7,
            MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT = 15,
            MOVECOMMAND_MAX_RENDER_TEXTLINE = 31,
        };

        enum MOVECOMMAND_MOUSE_EVENT
        {
            MOVECOMMAND_MOUSEBTN_NORMAL = 0,
            MOVECOMMAND_MOUSEBTN_OVER,
            MOVECOMMAND_MOUSEBTN_CLICKED,
        };
    private:

        //$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;
        int							m_iRealFontHeight;
        std::list<CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
        POINT						m_StartUISubjectName;
        POINT						m_StartMapNamePos;
        POINT						m_MapNameUISize;
        POINT						m_StrifePos;
        POINT						m_MapNamePos;
        POINT						m_ReqLevelPos;
        POINT						m_ReqZenPos;
        int							m_iSelectedMapName;
        POINT						m_ScrollBarPos;
        POINT						m_ScrollBtnStartPos;
        POINT						m_ScrollBtnPos;
        int							m_iScrollBarHeightPixel;
        int							m_iRenderStartTextIndex;
        int							m_iRenderEndTextIndex;
        int							m_iSelectedTextIndex;
        int							m_iScrollBtnInterval;
        int							m_iScrollBarMiddleNum;
        int							m_iScrollBarMiddleRemainderPixel;
        int							m_iNumPage;
        int							m_iCurPage;
        int							m_iTotalMoveScrBtnPixel;
        int							m_iRemainMoveScrBtnPixel;
        int							m_icurMoveScrBtnPixelperStep;
        int							m_iMaxMoveScrBtnPixelperStep;
        int							m_iMinMoveScrBtnPixelperStep;
        int							m_iTotalMoveScrBtnperStep;
        int							m_iRemainMoveScrBtnperStep;
        int							m_iTotalNumMaxMoveScrBtnperStep;
        int							m_iTotalNumMinMoveScrBtnperStep;
        int							m_iAcumMoveMouseScrollPixel;
        int							m_iMousePosY;
        int							m_iScrollBtnMouseEvent;
        bool						m_bScrollBtnActive;
        DWORD						m_dwMoveCommandKey;

    public:
        CNewUIMoveCommandWindow();
        virtual ~CNewUIMoveCommandWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        virtual void OpenningProcess();
        void ClosingProcess();
        float GetLayerDepth();

        bool IsLuckySealBuff();
        bool IsMapMove(const std::wstring& src);

        void SetMoveCommandKey(DWORD dwKey);
        DWORD GetMoveCommandKey();

        BOOL IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const;
        int GetMapIndexFromMovereq(const wchar_t* pszMapName);

    private:
        void SetStrifeMap();
        void SettingCanMoveMap();
        void RenderFrame();
        //void MoveScrollBtn(int iMoveValue);
        void UpdateScrolling();
        void ScrollUp(int iMoveValue);
        void ScrollDown(int iMoveValue);
        // bSign : true(DownScroll,+), false(UpScroll,-)
        void RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign = true);
        void LoadImages();
        void UnloadImages();
    };
};
