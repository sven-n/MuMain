//*****************************************************************************
// File: NewUIBattleSoccerScore.h
//
// Desc: interface for the CNewUIBattleSoccerScore class.
//		 전투축구 점수 UI 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_)
#define AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIBattleSoccerScore : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_BSS_BACK = BITMAP_INTERFACE_NEW_BATTLE_SOCCER_SCORE_BEGIN,
        };

    private:
        enum
        {
            BSS_WIDTH = 131,
            BSS_HEIGHT = 70,
        };

        CNewUIManager* m_pNewUIMng;			// UI 매니저.
        POINT					m_Pos;					// 창의 위치.

    public:
        CNewUIBattleSoccerScore();
        virtual ~CNewUIBattleSoccerScore();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 1.8f

    private:
        void LoadImages();
        void UnloadImages();

        void RenderBackImage();
        void RenderContents();

        int FindGuildMark(wchar_t* pszGuildName);
    };
}

#endif // !defined(AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_)
