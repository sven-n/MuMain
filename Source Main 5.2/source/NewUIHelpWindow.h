// NewUIHelpWindow.h: interface for the CNewUIHelpWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
#define AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_

#pragma once

#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIHelpWindow : public CNewUIObj
    {
    public:
        CNewUIHelpWindow();
        virtual ~CNewUIHelpWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 7.1f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();

        void AutoUpdateIndex();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT			m_Pos;

        int m_iIndex;
    };
}

#endif // !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
