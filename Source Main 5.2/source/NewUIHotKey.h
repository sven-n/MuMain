#ifndef _NEWUIHOTKEY_H_
#define _NEWUIHOTKEY_H_

#pragma once

#include "NewUIBase.h"

namespace SEASON3B
{
    class CNewUIManager;

    class CNewUIHotKey : public CNewUIObj
    {
        CNewUIManager* m_pNewUIMng;
        bool			m_bStateGameOver;

    public:
        CNewUIHotKey();
        virtual ~CNewUIHotKey();

        bool Create(CNewUIManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 1.0f
        float GetKeyEventOrder();	//. 1.0f

        bool CanUpdateKeyEvent();
        bool CanUpdateKeyEventRelatedMyInventory();

        void SetStateGameOver(bool bGameOver);	// 게임오버중인 상태
        bool IsStateGameOver();

        bool AutoGetItem();

    private:
        void ResetMouseRButton();
    };
}

#endif	// _NEWUIHOTKEY_H_