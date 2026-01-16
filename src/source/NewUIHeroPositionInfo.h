// NewUIHeroPositionInfo.h: interface for the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
#define AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUIHeroPositionInfo : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_HERO_POSITION_INFO_BASE_WINDOW = BITMAP_HERO_POSITION_INFO_BEGIN,		// newui_position02.tga			(70, 25)
        };
    private:
        enum HERO_POSITION_INFO_BASE_WINDOW_SIZE
        {
            HERO_POSITION_INFO_BASEA_WINDOW_WIDTH = 22,
            HERO_POSITION_INFO_BASE_WINDOW_HEIGHT = 25,
            HERO_POSITION_INFO_BASEB_WINDOW_WIDTH = 80,
            HERO_POSITION_INFO_BASEC_WINDOW_WIDTH = 22,
        };

    private:
        int              WidenX;
        CNewUIManager* m_pNewUIMng;
        POINT            m_Pos;
        POINT            m_CurHeroPosition;
        CNewUIButton     m_BtnConfig;
        CNewUIButton     m_BtnStart;
        CNewUIButton     m_BtnStop;
    public:
        CNewUIHeroPositionInfo();
        virtual ~CNewUIHeroPositionInfo();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 4.3f

        void OpenningProcess();
        void ClosingProcess();

        void SetCurHeroPosition(int x, int y);
        void MoveTextTipPos(CNewUIButton* m_Btn, int iX, int iY);
        void SetButtonInfo(CNewUIButton* m_Btn, int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt,std::wstring btname,std::wstring tooltiptext, bool istoppos);
    private:
        void LoadImages();
        void UnloadImages();
    };
}

#endif // !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
