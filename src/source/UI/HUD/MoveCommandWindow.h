#pragma once

#include <algorithm>

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "Network/MoveCommandData.h"
#include "UI/HUD/MoveCommandRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Scaling/UITransform.h"

namespace Rml
{
    class ElementDocument;
}

namespace UI::MoveCommand
{
    // Reference-space geometry, all of it derived from the dock height rather than fixed: the
    // window starts at windowY and grows down to a whole number of rows short of the dock's own
    // bottom edge, which is where the bottom HUD frame begins.
    struct Layout
    {
        int windowWidth;
        int windowHeight;
        int visibleRows;
        int listTop;
        int closeTop;
    };

    inline Layout CalculateLayout(int windowY, int rowHeight)
    {
        constexpr int kWindowWidth = 230;
        constexpr int kFixedChromeHeight = 60;
        constexpr int kListOffsetY = 38;
        constexpr int kCloseBottomGap = 6;

        const int safeRowHeight = std::max(rowHeight, 1);
        const int availableHeight = UI::Scaling::DockLogicalBottom - windowY;
        const int visibleRows = std::max(1, (availableHeight - kFixedChromeHeight) / safeRowHeight);
        const int windowHeight = kFixedChromeHeight + visibleRows * safeRowHeight;
        const int listTop = windowY + kListOffsetY;
        const int closeTop = windowY + windowHeight - safeRowHeight - kCloseBottomGap;
        // The close bar's own left inset (2) and width (230 - 5) are fixed, so they live in RCSS
        // with the rest of the static geometry rather than being computed and pushed from here.
        return { kWindowWidth, windowHeight, visibleRows, listTop, closeTop };
    }
}

namespace mu::ui::window
{
    class CMoveCommandWindow : public CObject
    {
    private:

        //$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
        CManager* m_pNewUIMng;
        POINT						m_Pos;
        int							m_iRealFontHeight;
        std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
        POINT						m_MapNameUISize;
        UI::MoveCommand::Layout		m_layout{};
        DWORD						m_dwMoveCommandKey;

        RmlModelBinder<MoveCommandRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // One-shot: rewind the list to the top on the first frame after the window opens, once
        // the document is visible and RmlUi has laid it out. Re-asserting a scroll position every
        // frame would fight the player's own drag and wheel.
        bool						m_bRewindPending = false;

    public:
        CMoveCommandWindow();
        virtual ~CMoveCommandWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void ReloadRmlTheme();

        virtual void OpenningProcess();
        void ClosingProcess();
        float GetLayerDepth();

        bool IsLuckySealBuff();
        bool IsMapMove(const std::wstring& src);

        void SetMoveCommandKey(DWORD dwKey);
        DWORD GetMoveCommandKey();

        BOOL IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const;
        int GetMapIndexFromMovereq(const wchar_t* pszMapName);
        // Whether the character currently meets a warp list entry's own
        // requirements - level, zen, the wings for Icarus, the Uniria rule
        // for Atlans. The list's requirements are recomputed first, exactly
        // as the click path does before reading the flag.
        bool CanMoveToMap(const wchar_t* pszMapName);

    private:
        // The warp list entry a name belongs to, main or sub name, case
        // insensitively - the one lookup behind both public queries.
        SEASON3B::CMoveCommandData::MOVEINFODATA* FindMoveInfo(const wchar_t* pszMapName);
        void SetStrifeMap();
        void SettingCanMoveMap();
        void RefreshDataAndLayout();
        // The font-derived row height and everything CalculateLayout() derives from it, without
        // re-copying the warp list itself.
        void RefreshLayoutMetrics();

        void BuildRmlUi();
        void SyncRmlModel();
        void RebuildRowModel();
        // Warps to the list entry at `row` (a position in m_listMoveInfoData, as pushed into
        // MoveCommandRowEntry::index), if the character still meets its requirements.
        void RmlClickWarp(int row);
    };
};
