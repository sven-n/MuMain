
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Events/EventItemEntryView.h"
#include "Engine/Object/ZzzInventory.h"

namespace mu::ui::window
{
// Delgado's lucky coin registration window. lucky_coin_registration.rml draws it, the live 3D
// coin C++ draws into it included; C++ keeps the count, the Register lock and every request it
// sends.
class CRegistrationLuckyCoin : public CObject
{
private:
    static constexpr float LUCKYCOIN_REG_WIDTH = 190.0f;
    static constexpr float LUCKYCOIN_REG_HEIGHT = 429.0f;

public:
    CRegistrationLuckyCoin();
    virtual ~CRegistrationLuckyCoin();

    bool Create(CManager* pNewUIMng, int x, int y);

    void SetPos(int x, int y);
    const POINT& GetPos()
    {
        return m_Pos;
    }

    bool Render();
    bool Update();
    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
    bool UpdateKeyEvent();

    float GetLayerDepth()
    {
        return 4.2f;
    }

    const int& GetRegistCount()
    {
        return m_RegistCount;
    }

    void SetRegistCount(int nRegistCount)
    {
        m_RegistCount = nRegistCount;
    }

    bool GetItemRotation()
    {
        return m_ItemAngle;
    }
    void SetItemRotation(bool _bInput)
    {
        m_ItemAngle = _bInput;
    }

    void LockLuckyCoinRegBtn();
    void UnLockLuckyCoinRegBtn();

    void OpeningProcess();
    void ClosingProcess();

    void Release();

private:
    void SyncView();
    void RenderLuckyCoin();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    ITEM* m_CoinItem;
    bool m_ItemAngle;
    int m_RegistCount;
    bool m_RegisterLocked = false;
    EventItemEntryView m_View{"lucky_coin_registration", "Data/Interface/RmlUi/lucky_coin_registration.rml"};
};
}
