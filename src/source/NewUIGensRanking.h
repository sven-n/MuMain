// NewUIGensRanking.h: interface for the CNewUIGensRanking class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "_TextureIndex.h"

#ifdef PBG_ADD_GENSRANKING

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIScrollBar.h"
#include "NewUITextBox.h"
#define MAX_TITLELENGTH		32

namespace SEASON3B
{
    class CNewUIGensRanking : public CNewUIObj
    {
        enum IMAGE_LIST
        {
            IMAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_CLOSE_REGIST = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
            IMAGE_GENS_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_NEWMARK_DUPRIAN = BITMAP_GENS_MARK_DUPRIAN,
            IMAGE_NEWMARK_BARNERT = BITMAP_GENS_MARK_BARNERT,

            IMAGE_GENSINFO_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,
            IMAGE_GENSINFO_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,
            IMAGE_GENSINFO_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,
            IMAGE_GENSINFO_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,
            IMAGE_GENSINFO_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_GENSINFO_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_GENSINFO_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_GENSINFO_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

            IMAGE_RANKBACK = BITMAP_GENS_RANKBACK,
            IMAGE_RANKBACK_TEXTBOX = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN,
        };

        enum IMAGE_SIZE
        {
            GENSRANKING_WIDTH = 190,
            GENSRANKING_HEIGHT = 429,
            TEAMNAME_LENTH = 10,
            GENSMARK_WIDTH = 50,
            GENSMARK_HEIGHT = 69,
            GENSRANKBACK_WIDTH = 170,
            GENSRANKBACK_HEIGHT = 88,
            GENSRANKTEXTBACK_WIDTH = 170,
            GENSRANKTEXTBACK_HEIGHT = 21,
        };

        enum IMAGE_INDEX
        {
            TITLENAME_NONE = 0,
            TITLENAME_START = 1,
            TITLENAME_END = 14,
        };

    public:
        enum IMAGE_AREA
        {
            MARK_UIINFO = 0,
            MARK_BOOLEAN,
            MARK_RANKINFOWIN,
        };

        enum GENS_TYPE
        {
            GENSTYPE_NONE = 0,
            GENSTYPE_DUPRIAN,
            GENSTYPE_BARNERT,
        };

    private:
        void Init();
        void Destroy();
        void LoadImages();
        void UnloadImages();

        POINT m_Pos;

        FLOAT m_fBooleanSize;

        int m_nContribution;
        wchar_t m_szRanking[TEAMNAME_LENTH];
        wchar_t m_szGensTeam[TEAMNAME_LENTH];

        GENS_TYPE m_byGensInfluence;
        POINT m_ptRenderMarkPos;

        CNewUIButton m_BtnExit;

        wchar_t m_szTitleName[TITLENAME_END][MAX_TITLELENGTH];

        int m_nNextContribution;

    public:
        CNewUIManager* m_pNewUIMng;
        CNewUIScrollBar* m_pScrollBar;
        CNewUITextBox* m_pTextBox;
        CNewUIGensRanking();
        virtual ~CNewUIGensRanking();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void SetPos(int x, int y);
        const POINT& GetPos() { return m_Pos; }

        bool Render();
        void RenderFrame();
        void RenderTexts();
        void RenderButtons();
        void RenderMark(int x, int y, GENS_TYPE _GensInfluence);
        void RenderInfoFrame(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth = 60, int iTitleHeight = 20);

        bool Update();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();

        bool BtnProcess();
        void SetBtnInfo(float _PosX, float _PosY);

        void OpenningProcess();
        void ClosingProcess();

        float GetLayerDepth() { return 4.2f; }

        void SetContribution(int _Contribution);
        int GetContribution();

        void SetNextContribution(int _NextContribution);
        int GetNextContribution();

        bool SetRanking(int _Ranking);
        wchar_t* GetRanking();

        bool SetGensInfo();
        bool SetGensTeamName(const wchar_t* _pTeamName);
        wchar_t* GetGensTeamName();

        void SetTitleName();
        wchar_t* GetTitleName(BYTE _index);

        void RanderMark(float _x, float _y, GENS_TYPE _GensInfluence, BYTE _GensRankInfo, IMAGE_AREA _ImageArea = MARK_RANKINFOWIN, float _RenderY = 0);
        int GetImageIndex(BYTE _index);
    };
}

#endif //PBG_ADD_GENSRANKING
