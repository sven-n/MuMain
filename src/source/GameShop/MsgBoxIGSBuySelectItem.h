#if !defined(AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_)
#define AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "UI/Dialogs/CommonMessageBox.h"
#include "GameShop/IgsDialogModel.h"
#include "GameShop/ShopListManager/ShopPackage.h"


using namespace SEASON3B;
using namespace mu::ui::window;

#include "GameShop/BuyOptionSelection.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Types.h>

#include <vector>

// A package with several prices, from the cash shop's Buy: igs_buy_select.rml draws it, the picked
// option's live 3D item included; its buttons send this box's own events.
class CMsgBoxIGSBuySelectItem : public CMessageBoxBase
{
public:
    CMsgBoxIGSBuySelectItem();
    virtual ~CMsgBoxIGSBuySelectItem();

private:
    enum
    {
        IGS_FRAME_WIDTH = 215,
        IGS_FRAME_HEIGHT = 346,
        IGS_TEXT_DISCRIPTION_WIDTH = 185,
    };

public:
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
    void AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, wchar_t* pszPriceUnit,
                 int iCashType);

    GameShop::BuyOptionSelection m_BuyOptions;

    struct OptionRow
    {
        Rml::String name;
        bool selected = false;
        bool operator==(const OptionRow&) const = default;
    };
    struct BuySelectRmlModel
    {
        float textPx = 0.f;
        Rml::String title, name, price;
        std::vector<Rml::String> descriptionLines;
        std::vector<OptionRow> options;
        std::vector<GameShop::DialogButton> buttons;
        std::vector<Rml::String> debugLines;
    };
    void BindRmlModel(Rml::DataModelConstructor& c, BuySelectRmlModel& model);
    UI::RmlBridge::ThemedView<BuySelectRmlModel> m_RmlView{"igs_buy_select",
        [this](Rml::DataModelConstructor& c, BuySelectRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/igs_buy_select.rml"}}};
    int m_PressedButton = -1;
    UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItem(); }};

    void SyncRmlModel();

    int m_iPackageSeq;
    int m_iDisplaySeq;
    WORD m_wItemCode;
    int m_iDescriptionLine;
    bool m_bGiftEnabled = false;

    wchar_t m_szPackageName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szDescription[UIMAX_TEXT_LINE][MAX_TEXT_LENGTH];
};

class CMsgBoxIGSBuySelectItemLayout : public TMsgBoxLayout<CMsgBoxIGSBuySelectItem>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_MSGBOXIGSBUYSELECTITEM_H__96137D00_144C_4E10_B335_383E5DAB5D50__INCLUDED_)
