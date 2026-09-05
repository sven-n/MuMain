// NewUICursedTempleEnter.h: interface for the CCursedTempleEnter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CCursedTempleEnter : public CObject
    {
    public:

        static constexpr float CURSEDTEMPLE_ENTER_WINDOW_WIDTH = 230.0f;
        static constexpr float CURSEDTEMPLE_ENTER_WINDOW_HEIGHT = 252.0f;

        enum
        {
            CURSEDTEMPLEENTER_OPEN = 0,
            CURSEDTEMPLEENTER_EXIT,
            CURSEDTEMPLEENTER_MAXBUTTONCOUNT,
        };

    public:
        CCursedTempleEnter();
        virtual ~CCursedTempleEnter();

        bool Create(CManager* pNewUIMng, int x, int y);

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
        CManager* m_pNewUIMng;
        POINT					m_Pos;
        CButton			m_Button[CURSEDTEMPLEENTER_MAXBUTTONCOUNT];
        int						m_EnterTime;
        int						m_EnterCount;
    };

    inline
        float CCursedTempleEnter::GetLayerDepth()
    {
        return 10.3;
    }

    inline
        void CCursedTempleEnter::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CCursedTempleEnter::GetPos() const
    {
        return m_Pos;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
