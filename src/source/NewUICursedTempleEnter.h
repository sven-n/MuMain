// NewUICursedTempleEnter.h: interface for the CNewUICursedTempleEnter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUICursedTempleEnter : public CNewUIObj
    {
    public:
        enum UI_SIZE
        {
            CURSEDTEMPLE_ENTER_WINDOW_WIDTH = 230,
            CURSEDTEMPLE_ENTER_WINDOW_HEIGHT = 252,
        };

        enum
        {
            CURSEDTEMPLEENTER_OPEN = 0,
            CURSEDTEMPLEENTER_EXIT,
            CURSEDTEMPLEENTER_MAXBUTTONCOUNT,
        };

    public:
        CNewUICursedTempleEnter();
        virtual ~CNewUICursedTempleEnter();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);

    private:
        void SetButtonInfo();

    public:
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();

    public:
        bool CheckEnterLevel(int& enterlevel);
        bool CheckEnterItem(ITEM* p, int enterlevel);
        bool CheckInventory(BYTE& itempos, int enterlevel);

    public:
        bool Render();

    private:
        void RenderFrame();
        void RenderText();
        void RenderButtons();

    public:
        void SetPos(int x, int y);

    public:
        const POINT& GetPos() const;
        float GetLayerDepth();	//. 5.0f

    public:
        void SetCursedTempleEnterInfo(const BYTE* cursedtempleinfo);
        void ReceiveCursedTempleEnterInfo(const BYTE* cursedtempleinfo);

    private:
        void Initialize();
        void Destroy();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        CNewUIButton			m_Button[CURSEDTEMPLEENTER_MAXBUTTONCOUNT];
        int						m_EnterTime;
        int						m_EnterCount;
    };

    inline
        float CNewUICursedTempleEnter::GetLayerDepth()
    {
        return 10.3;
    }

    inline
        void CNewUICursedTempleEnter::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CNewUICursedTempleEnter::GetPos() const
    {
        return m_Pos;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
