#ifndef _NEWUIHOTKEY_H_
#define _NEWUIHOTKEY_H_

#pragma once

#include "UI/Core/WindowObject.h"

namespace mu::ui::window
{
    class CManager;

    class CHotKey : public CObject
    {
        CManager* m_pNewUIMng;
        bool			m_bStateGameOver;

    public:
        CHotKey();
        virtual ~CHotKey();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 1.0f
        float GetKeyEventOrder();	//. 1.0f

        bool CanUpdateKeyEvent();
        bool CanUpdateKeyEventRelatedMyInventory();

        // The P, G and D keys' windows, shared with the HUD buttons that open the same windows.
        // CanToggleWindowByButton() is the keys' rules for a click: no window while the hero is
        // dead, watching a duel, or has a window open that works with the inventory (trade,
        // storage, shops, mixing) or that takes the keys (NPC windows).
        bool CanToggleWindowByButton();
        void TogglePartyWindow();
        void ToggleGuildWindow();
        void ToggleCommandWindow();

        void SetStateGameOver(bool bGameOver);	// 게임오버중인 상태
        bool IsStateGameOver();

        bool AutoGetItem();

    private:
        void ResetMouseRButton();
        bool IsKeyTakingWindowVisible();
    };
}

#endif	// _NEWUIHOTKEY_H_