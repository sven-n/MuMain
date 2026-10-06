
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// Delgado's lucky coin exchange window. lucky_coin_exchange.rml draws it; C++ keeps the button
// locks, Escape and every request it sends.
class CExchangeLuckyCoin : public CObject
{
private:
    enum ENTERBC_WINDOW_SIZE
    {
        EXCHANGE_LUCKYCOIN_WINDOW_WIDTH = 190,
        EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT = 429,
    };

public:
    enum
    {
        MAX_EXCHANGE_BTN = 3,
        EXCHANGE_BTN_VAL = 33,
        EXCHANGE_TEXT_VAL = 14,
    };

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    bool m_ExchangeLocked = false;
    EventItemEntryView m_View{"lucky_coin_exchange", "Data/Interface/RmlUi/lucky_coin_exchange.rml"};

public:
    CExchangeLuckyCoin();
    virtual ~CExchangeLuckyCoin();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool BtnProcess();

    float GetLayerDepth(); //. 4.2f

    void OpenningProcess();
    void ClosingProcess();

    void LockExchangeBtn();
    void UnLockExchangeBtn();

private:
    void SyncView();
};
}
