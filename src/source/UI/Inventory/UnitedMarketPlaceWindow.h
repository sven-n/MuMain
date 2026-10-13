#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "UI/Inventory/UnitedMarketPlaceRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// Julia's market warp service. united_market_place.rml draws it; C++ keeps the texts, the Warp
// button's lock, the corner close, Escape and the warp request.
class CUnitedMarketPlaceWindow : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_UNITEDMARKETPLACEWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK, // Reference
        IMAGE_UNITEDMARKETPLACEWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
        IMAGE_UNITEDMARKETPLACEWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
        IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
        IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
        IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
        IMAGE_UNITEDMARKETPLACEWINDOW_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
        IMAGE_UNITEDMARKETPLACEWINDOW_BTN_CLOSE = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
    };

private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;


public:
    CUnitedMarketPlaceWindow();
    virtual ~CUnitedMarketPlaceWindow();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool IsVisible() const;

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f

    void SetRemainTime(int iTime);
    void LockEnterButton(BOOL bLock);


private:
    void BuildRmlUi();
    void SyncRmlModel();

    void BindRmlModel(Rml::DataModelConstructor& c, UnitedMarketPlaceRmlModel& model);
    UI::RmlBridge::ThemedView<UnitedMarketPlaceRmlModel> m_RmlView{"united_market_place",
        [this](Rml::DataModelConstructor& c, UnitedMarketPlaceRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/united_market_place.rml"}}};
    bool m_PendingWarp = false;
    bool m_PendingExit = false;

    int m_iRemainTime;
    BOOL m_bIsEnterButtonLocked;
};
}
