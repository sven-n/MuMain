//////////////////////////////////////////////////////////////////////
// NewUIGuildInfoWindow.h: interface for the CNewUIGuildInfoWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMainFrameWindow.h"
#include "NewUIChatLogWindow.h"
#include "NewUIMyInventory.h"

namespace SEASON3B
{
    class CNewUIMiniMap : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MINIMAP_INTERFACE = BITMAP_MINI_MAP_BEGIN,
        };

        enum MASTER_DATA
        {
            SKILL_ICON_DATA_WDITH = 4,
            SKILL_ICON_DATA_HEIGHT = 8,
            SKILL_ICON_WIDTH = 20,
            SKILL_ICON_HEIGHT = 28,
            SKILL_ICON_STARTX1 = 75,
            SKILL_ICON_STARTY1 = 75,
        };

        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_SCROLL_BTN_DOWN,
        };

    private:
       std::wstring		m_TooltipText;
        HFONT					m_hToolTipFont;
        DWORD					m_TooltipTextColor;

        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        POINT					m_Width;
        POINT					m_MiniWidth;
        POINT					m_Lenth[6];
        int						m_MiniPos;
        CNewUIButton			m_BtnExit;
        MINI_MAP				m_Mini_Map_Data[MAX_MINI_MAP_DATA];
        float					m_Btn_Loc[MAX_MINI_MAP_DATA][4];

    public:
        bool					m_bSuccess;
        CNewUIMiniMap();
        virtual ~CNewUIMiniMap();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void SetBtnPos(int Num, float x, float y, float nx, float ny);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 8.1f

        void OpenningProcess();
        void ClosingProcess();
        void OpenMasterLevel(const wchar_t* filename);
        CNewUIButton m_BtnToolTip;
        void LoadImages(const wchar_t* Filename);
        void UnloadImages();

    private:
        void Render_Text();
        void Render_Icon();
        void Render_Scroll();
        bool Check_Mouse(int mx, int my);
        bool Check_Btn(int mx, int my);
    };
}
