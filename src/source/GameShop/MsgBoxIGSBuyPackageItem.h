#if !defined(AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_)
#define AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "GameShop/IgsDialogModel.h"
#include "GameShop/ShopListManager/ShopPackage.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Scaling/UITransform.h"

using namespace SEASON3B;
using namespace mu::ui::window;

#include <RmlUi/Core/Types.h>

#include <string>
#include <vector>

// A package of one price, from the cash shop's Buy: igs_buy_package.rml draws it, the package's
// live 3D item included; its buttons send this box's own events.
class CMsgBoxIGSBuyPackageItem : public CMessageBoxBase
{
    enum
    {
        IGS_WINDOW_WIDTH = 640,
        IGS_WINDOW_HEIGHT = 429,
        IGS_FRAME_WIDTH = 198,
        IGS_FRAME_HEIGHT = 291,
        IGS_LISTBOX_WIDTH = 158,
    };

public:
    CMsgBoxIGSBuyPackageItem();
    virtual ~CMsgBoxIGSBuyPackageItem();

    bool Create(float fPriority = 3.f);
    void Release();
    bool Update();
    bool Render();
    Rml::Element* GetPanel() const override
    {
        return m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("panel") : nullptr;
    }

    void Initialize(CShopPackage* pPackage);

    static CALLBACK_RESULT BuyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT PresentBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void SetAddCallbackFunc();
    void RenderItem();

    // The description, one wrapped line per entry, in arrival order. RmlUi owns the scrolling.
    std::vector<std::wstring> m_DescriptionLines;

    struct DescriptionLine
    {
        Rml::String text;
        bool operator==(const DescriptionLine&) const = default;
    };
    struct BuyPackageRmlModel
    {
        float textPx = 0.f;
        Rml::String title, name, price;
        std::vector<DescriptionLine> descriptionLines;
        std::vector<GameShop::DialogButton> buttons;
        std::vector<Rml::String> debugLines;
    };
    void BindRmlModel(Rml::DataModelConstructor& c, BuyPackageRmlModel& model);
    UI::RmlBridge::ThemedView<BuyPackageRmlModel> m_RmlView{"igs_buy_package",
        [this](Rml::DataModelConstructor& c, BuyPackageRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/igs_buy_package.rml"}}};
    int m_PressedButton = -1;
    // Where the dialog stands on the stage (UI::RmlBridge::PlaceOnStage()).
    UI::RmlBridge::SlotPlacement m_Placement;
    UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItem(); }};

    void SyncRmlModel();

    int m_iPackageSeq;
    int m_iDisplaySeq;
    WORD m_wItemCode;
    int m_iCashType;
    bool m_bGiftEnabled = false;

    wchar_t m_szPackageName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];
    wchar_t m_szDescription[UIMAX_TEXT_LINE][MAX_TEXT_LENGTH];
};

class CMsgBoxBuyPackageItemLayout : public TMsgBoxLayout<CMsgBoxIGSBuyPackageItem>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSBUYPACKAGEITEM_H__42A6E746_9439_4E71_9C86_7CDF5F96AFE3__INCLUDED_)
