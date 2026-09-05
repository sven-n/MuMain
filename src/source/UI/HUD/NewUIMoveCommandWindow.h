// NewUIMoveCommandWindow.h: interface for the CNewUIMoveCommandWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <algorithm>
#include <cstddef>

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "Network/MoveCommandData.h"
#include "UI/HUD/NewUIChatLogWindow.h"
#include "UI/Scaling/UITransform.h"

namespace UI::MoveCommand
{
    struct Layout
    {
        int windowWidth;
        int windowHeight;
        int visibleRows;
        int listTop;
        int closeTop;
        int closeLeft;
        int closeWidth;
        int scrollTrackTop;
        int scrollTrackHeight;
        int thumbTravel;
    };

    struct DragState
    {
        int scrollOffset;
        bool dragging;
        bool releaseConsumed;
    };

    inline Layout CalculateLayout(int windowY, int rowHeight)
    {
        constexpr int kWindowWidth = 230;
        constexpr int kFixedChromeHeight = 60;
        constexpr int kListOffsetY = 38;
        constexpr int kCloseBottomGap = 6;
        constexpr int kCloseLeft = 2;
        constexpr int kCloseRightGap = 5;
        constexpr int kScrollBarCapHeight = 3;
        constexpr int kScrollThumbHeight = 30;

        const int safeRowHeight = std::max(rowHeight, 1);
        const int availableHeight = UI::Scaling::DockLogicalBottom - windowY;
        const int visibleRows = std::max(1, (availableHeight - kFixedChromeHeight) / safeRowHeight);
        const int windowHeight = kFixedChromeHeight + visibleRows * safeRowHeight;
        const int listTop = windowY + kListOffsetY;
        const int closeTop = windowY + windowHeight - safeRowHeight - kCloseBottomGap;
        const int closeWidth = kWindowWidth - kCloseRightGap;
        const int scrollTrackTop = listTop - kScrollBarCapHeight;
        const int scrollTrackHeight = visibleRows * safeRowHeight;
        const int thumbTravel = std::max(0, scrollTrackHeight - kScrollThumbHeight);
        return { kWindowWidth, windowHeight, visibleRows, listTop, closeTop, kCloseLeft, closeWidth, scrollTrackTop, scrollTrackHeight, thumbTravel };
    }

    inline int MaximumScrollOffset(std::size_t itemCount, int visibleRows)
    {
        if (visibleRows <= 0 || itemCount <= static_cast<std::size_t>(visibleRows))
            return 0;
        return static_cast<int>(itemCount - static_cast<std::size_t>(visibleRows));
    }

    inline int ClampScrollOffset(int offset, std::size_t itemCount, int visibleRows)
    {
        return std::clamp(offset, 0, MaximumScrollOffset(itemCount, visibleRows));
    }

    inline int ThumbYForScrollOffset(int offset, const Layout& layout, std::size_t itemCount)
    {
        const int maximumOffset = MaximumScrollOffset(itemCount, layout.visibleRows);
        if (maximumOffset == 0 || layout.thumbTravel == 0)
            return layout.scrollTrackTop;

        const int clampedOffset = ClampScrollOffset(offset, itemCount, layout.visibleRows);
        return layout.scrollTrackTop + (clampedOffset * layout.thumbTravel + maximumOffset / 2) / maximumOffset;
    }

    inline int ScrollOffsetForThumbY(int thumbY, const Layout& layout, std::size_t itemCount)
    {
        const int maximumOffset = MaximumScrollOffset(itemCount, layout.visibleRows);
        if (maximumOffset == 0 || layout.thumbTravel == 0)
            return 0;

        const int travel = std::clamp(thumbY - layout.scrollTrackTop, 0, layout.thumbTravel);
        return (travel * maximumOffset + layout.thumbTravel / 2) / layout.thumbTravel;
    }

    inline DragState UpdateDragState(bool dragging, bool released, int mouseY, int grabOffsetY, int scrollOffset,
                                     const Layout& layout, std::size_t itemCount)
    {
        if (!dragging)
            return { scrollOffset, false, false };

        return { ScrollOffsetForThumbY(mouseY - grabOffsetY, layout, itemCount), !released, released };
    }
}

namespace mu::ui::window
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
        std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
        POINT						m_StartUISubjectName;
        POINT						m_StartMapNamePos;
        POINT						m_MapNameUISize;
        POINT						m_StrifePos;
        POINT						m_MapNamePos;
        POINT						m_ReqLevelPos;
        POINT						m_ReqZenPos;
        int							m_iSelectedMapName;
        int							m_iSelectedTextIndex;
        int							m_iScrollBtnMouseEvent;
        UI::MoveCommand::Layout		m_layout{};
        int							m_scrollOffset{0};
        int							m_scrollDragGrabOffsetY{0};
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
        void RefreshDataAndLayout();
        void SetScrollOffset(int offset);
        int VisibleEndIndex() const;
        void RenderFrame();
        void LoadImages();
        void UnloadImages();
    };
};
