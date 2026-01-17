// NewUIGuildInfoWindow.h: interface for the CNewUIGuildInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICRYWOLF_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUICRYWOLF_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUICryWolf : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MVP_INTERFACE = BITMAP_INTERFACE_CRYWOLF_BEGIN,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        int						m_iHour;
        int						m_iMinute;
        int						m_iSecond;
        DWORD					m_dwSyncTime;
        int						m_icntTime;
        bool					m_bTimeStart;

    public:
        CNewUICryWolf();
        virtual ~CNewUICryWolf();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        bool Render(int Posx, int Posy, int nPosx, int nPosy, float u, float v, float su, float sv, int Index, bool Scale = false, bool StartScale = false, float Alpha = 1.f);

        float GetLayerDepth();	//. 10.0f

        void OpenningProcess();
        void ClosingProcess();
        float ConvertX(float x);
        float ConvertY(float y);
        void SetTime(int iHour, int iMinute);
        void InitTime();

    private:
        void LoadImages();
        void UnloadImages();
    };
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
