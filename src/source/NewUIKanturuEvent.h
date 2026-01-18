// NewUIKanturu2ndEnterNpc.h: interface for the CNewUIKanturu2ndEnterNpc class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
#define AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUIKanturu2ndEnterNpc : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_KANTURU2ND_TOP = CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP,
            IMAGE_KANTURU2ND_MIDDLE = CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE,
            IMAGE_KANTURU2ND_BOTTOM = CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM,
            IMAGE_KANTURU2ND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_KANTURU2ND_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
        };
        enum
        {
            KANTURU2ND_ENTER_WINDOW_WIDTH = 230,
            KANTURU2ND_ENTER_WINDOW_HEIGHT = 267,
        };

        enum MSGBOX_TYPE
        {
            POPUP_NONE = 0,
            POPUP_USER_OVER,
            POPUP_NOT_MUNSTONE,
            POPUP_FAILED,
            POPUP_FAILED2,
            POPUP_UNIRIA = 5,
            POPUP_CHANGERING,
            POPUP_NOT_HELPER,
        };

    public:
        CNewUIKanturu2ndEnterNpc();
        virtual ~CNewUIKanturu2ndEnterNpc();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 10.1f

        void SetNpcObject(OBJECT* pObj);
        bool IsNpcAnimation();
        void SetNpcAnimation(bool bValue);
        bool IsEnterRequest();
        void SetEnterRequest(bool bValue);
        void CreateMessageBox(BYTE btResult);

        void ReceiveKanturu3rdInfo(BYTE btState, BYTE btDetailState, BYTE btEnter, BYTE btUserCount, int iRemainTime);
        void ReceiveKanturu3rdEnter(BYTE btResult);
        void SendRequestKanturu3rdInfo();
        void SendRequestKanturu3rdEnter();

    private:
        void Initialize();

        void LoadImages();
        void UnloadImages();

        void SetButtonInfo();

        bool BtnProcess();

        void RenderFrame();
        void RenderButtons();
        void RenderTexts();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        BYTE m_byState;

        bool m_bNpcAnimation;
        OBJECT* m_pNpcObject;

        bool m_bEnterRequest;

        DWORD m_dwRefreshTime;
        DWORD m_dwRefreshButtonGapTime;

        wchar_t m_strSubject[MAX_GLOBAL_TEXT_STRING];
        wchar_t m_strStateText[KANTURU2ND_STATETEXT_MAX][MAX_GLOBAL_TEXT_STRING];
        int	m_iStateTextNum;

        CNewUIButton m_BtnEnter;
        CNewUIButton m_BtnRefresh;
        CNewUIButton m_BtnClose;
    };

    class CNewUIKanturuInfoWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_KANTURUINFO_WINDOW = BITMAP_KANTURU_INFO_BEGIN,
        };
        enum
        {
            KANTURUINFO_WINDOW_WIDTH = 99,
            KANTURUINFO_WINDOW_HEIGHT = 78,
        };
    public:
        CNewUIKanturuInfoWindow();
        virtual ~CNewUIKanturuInfoWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 1.92f
        float GetKeyEventOrder();	//. 9.1f

        void SetTime(int iTimeLimit);

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        void RenderInfo();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        int m_iMinute;
        int m_iSecond;
        DWORD m_dwSyncTime;
    };
}

#endif // !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
