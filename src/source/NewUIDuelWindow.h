// NewUIDuelWindow.h: interface for the CNewUIDuelWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_)
#define AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_

#pragma once

#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIDuelWindow : public CNewUIObj
    {
    private:
        enum IMAGE_LIST
        {
            IMAGE_DUEL_BACK = BITMAP_INTERFACE_NEW_BATTLE_SOCCER_SCORE_BEGIN,
        };
        enum
        {
            DUEL_WND_WIDTH = 131,
            DUEL_WND_HEIGHT = 70,
        };
    public:
        CNewUIDuelWindow();
        virtual ~CNewUIDuelWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 1.1f

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        void RenderContents();

        CNewUIManager* m_pNewUIMng;		// UI 매니저.
        POINT m_Pos;					// 창의 위치.
    };
}

#endif // !defined(AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_)
