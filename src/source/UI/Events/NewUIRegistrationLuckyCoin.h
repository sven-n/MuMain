// NewUIRegistrationLuckyCoin.h: interface for the CRegistrationLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "Engine/Object/ZzzInventory.h"

namespace mu::ui::window
{
    class CRegistrationLuckyCoin : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_CLOSE_REGIST = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
        };

    private:
        static constexpr float LUCKYCOIN_REG_WIDTH = 190.0f;
        static constexpr float LUCKYCOIN_REG_HEIGHT = 429.0f;

    public:
        CRegistrationLuckyCoin();
        virtual ~CRegistrationLuckyCoin();

        bool Create(CManager* pNewUIMng, int x, int y);

        void SetPos(int x, int y);
        const POINT& GetPos() { return m_Pos; }

        bool Render();
        bool Update();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool BtnProcess();
        void SetBtnInfo();

        float GetLayerDepth()
        {
            return 4.2f;
        }

        const int& GetRegistCount() { return m_RegistCount; }

        void SetRegistCount(int nRegistCount) { m_RegistCount = nRegistCount; }

        bool GetItemRotation() { return m_ItemAngle; }
        void SetItemRotation(bool _bInput) { m_ItemAngle = _bInput; }

        void LockLuckyCoinRegBtn();
        void UnLockLuckyCoinRegBtn();

        void OpeningProcess();
        void ClosingProcess();

        void Release();

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();
        void RenderLuckyCoin();

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;
        ITEM* m_CoinItem;
        bool m_ItemAngle;
        float m_width, m_height;
        int m_RegistCount;
        CButton m_CloseButton;
        CButton m_RegistButton;
    };
}
