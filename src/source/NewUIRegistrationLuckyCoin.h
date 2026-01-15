// NewUIRegistrationLuckyCoin.h: interface for the CNewUIRegistrationLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICommonMessageBox.h"
#include "ZzzInventory.h"

namespace SEASON3B
{
    class CNewUIRegistrationLuckyCoin : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_CLOSE_REGIST = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
        };

    private:
        enum
        {
            LUCKYCOIN_REG_WIDTH = 190,
            LUCKYCOIN_REG_HEIGHT = 429,
        };

    public:
        CNewUIRegistrationLuckyCoin();
        virtual ~CNewUIRegistrationLuckyCoin();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);

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
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;
        ITEM* m_CoinItem;
        bool m_ItemAngle;
        float m_width, m_height;
        int m_RegistCount;
        CNewUIButton m_CloseButton;
        CNewUIButton m_RegistButton;
    };
}
