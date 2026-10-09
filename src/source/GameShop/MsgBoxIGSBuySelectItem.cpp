
#include "stdafx.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSBuySelectItem.h"

static_assert(GameShop::kBuyOptionTextLength == MAX_TEXT_LENGTH);
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "MsgBoxIGSBuyConfirm.h"
#include "MsgBoxIGSSendGift.h"

CMsgBoxIGSBuySelectItem::CMsgBoxIGSBuySelectItem()
{
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_wItemCode = -1;

    m_iDescriptionLine = 0;

    m_szPackageName[0] = '\0';
    m_szPrice[0] = '\0';

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }
}

CMsgBoxIGSBuySelectItem::~CMsgBoxIGSBuySelectItem()
{
    Release();
}

bool CMsgBoxIGSBuySelectItem::Create(float fPriority)
{
    SetAddCallbackFunc();

    CMessageBoxBase::Create((IGS_WINDOW_WIDTH / 2) - (IGS_FRAME_WIDTH / 2), (IGS_WINDOW_HEIGHT / 2) - (IGS_FRAME_HEIGHT / 2), IGS_FRAME_WIDTH, IGS_FRAME_HEIGHT, fPriority);

    m_RmlView.Ensure();
    return true;
}

void CMsgBoxIGSBuySelectItem::Initialize(CShopPackage* pPackage)
{
    int iProductSeq, iPriceSeq;

    m_wItemCode = _wtoi(pPackage->InGamePackageID);

    m_iPackageSeq = pPackage->PackageProductSeq;
    m_iDisplaySeq = pPackage->ProductDisplaySeq;

    m_bGiftEnabled = (pPackage->GiftFlag == 184);

    wcsncpy(m_szPackageName, pPackage->PackageProductName, MAX_TEXT_LENGTH);

    ZeroMemory(m_szDescription, sizeof(wchar_t) * UIMAX_TEXT_LINE * MAX_TEXT_LENGTH);

    g_pRenderText->SetFont(g_hFont);
    m_iDescriptionLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, pPackage->Description, IGS_TEXT_DISCRIPTION_WIDTH, false, '#');

    pPackage->SetProductSeqFirst();
    if (pPackage->GetProductSeqNext(iProductSeq) == false)
    {
    }

    pPackage->SetPriceSeqFirst();
    while (pPackage->GetPriceSeqNext(iPriceSeq))
    {
        AddData(pPackage->PackageProductSeq, pPackage->ProductDisplaySeq, iPriceSeq, iProductSeq, pPackage->PricUnitName, pPackage->CashType);
    }
}

void CMsgBoxIGSBuySelectItem::Release()
{
    m_ItemTarget.Disable();
    m_RmlView.Release();
    CMessageBoxBase::Release();
    m_BuyOptions.Clear();
}

bool CMsgBoxIGSBuySelectItem::Update()
{
    // Buy, Gift, Cancel: the events the native buttons sent.
    static constexpr DWORD kButtonEvents[] = {MSGBOX_EVENT_USER_COMMON_OK, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT,
                                              MSGBOX_EVENT_USER_COMMON_CANCEL};
    const int pressed = std::exchange(m_PressedButton, -1);
    if (pressed >= 0 && pressed < static_cast<int>(std::size(kButtonEvents)) && (pressed != 1 || m_bGiftEnabled))
    {
        g_MessageBox->SendEvent(this, kButtonEvents[pressed]);
        return true;
    }

    // The picked option's price and item, once per change of pick.
    if (m_BuyOptions.TakeSelectionChanged())
    {
        if (const GameShop::BuyOption* pItem = m_BuyOptions.Selected())
        {
            wcscpy(m_szPrice, pItem->m_szItemPrice);
            m_wItemCode = pItem->m_wItemCode;
        }
    }

    SyncRmlModel();
    return true;
}

void CMsgBoxIGSBuySelectItem::BindRmlModel(Rml::DataModelConstructor& c, BuySelectRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    auto row = c.RegisterStruct<OptionRow>();
    row.RegisterMember("name", &OptionRow::name);
    row.RegisterMember("selected", &OptionRow::selected);
    c.RegisterArray<std::vector<OptionRow>>();
    c.Bind("options", &model.options);
    c.Bind("title", &model.title);
    c.Bind("name", &model.name);
    c.Bind("price", &model.price);
    GameShop::BindDialogCommon(c, model, m_PressedButton);
    c.Bind("description_lines", &model.descriptionLines);
    c.BindEventCallback("igs_select_option",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() == 1)
                                m_BuyOptions.SelectRow(args[0].Get<int>(-1));
                        });
}

void CMsgBoxIGSBuySelectItem::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("igs_item") : nullptr, true);
    if (!m_RmlView.Document())
        return;
    auto& binder = m_RmlView.Binder();
    UI::RmlBridge::PlaceOnStage(m_Placement, m_RmlView.Document(), "panel", GetPos());
    UI::RmlBridge::SyncNativeTextSize(binder);

    SyncField(binder, &BuySelectRmlModel::title, "title", StringUtils::WideToNarrow(I18N::Game::Shop));
    SyncField(binder, &BuySelectRmlModel::name, "name", StringUtils::WideToNarrow(m_szPackageName));
    SyncField(binder, &BuySelectRmlModel::price, "price", StringUtils::WideToNarrow(m_szPrice));
    std::vector<Rml::String> description;
    for (int i = 0; i < m_iDescriptionLine && i < UIMAX_TEXT_LINE; i++)
        description.push_back(StringUtils::WideToNarrow(m_szDescription[i]));
    SyncField(binder, &BuySelectRmlModel::descriptionLines, "description_lines", std::move(description));

    std::vector<OptionRow> rows;
    rows.reserve(m_BuyOptions.Options().size());
    const int selected = m_BuyOptions.SelectedRow();
    int index = 0;
    for (const GameShop::BuyOption& option : m_BuyOptions.Options())
    {
        rows.push_back({StringUtils::WideToNarrow(option.m_szItemName), index == selected});
        ++index;
    }
    SyncField(binder, &BuySelectRmlModel::options, "options", std::move(rows));

    SyncField(binder, &BuySelectRmlModel::buttons, "buttons",
              std::vector<GameShop::DialogButton>{{StringUtils::WideToNarrow(I18N::Game::Buy1124), true},
                                                  {StringUtils::WideToNarrow(I18N::Game::Gift), m_bGiftEnabled},
                                                  {StringUtils::WideToNarrow(I18N::Game::Cancel), true}});

    std::vector<Rml::String> debugLines;
#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    if (m_wItemCode == 65535)
        mu_swprintf(szText, L"Bad item index.");
    else
        mu_swprintf(szText, L"ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode / MAX_ITEM_INDEX, m_wItemCode % MAX_ITEM_INDEX);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    mu_swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    mu_swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    const GameShop::BuyOption* pDebugItem = m_BuyOptions.Selected();
    mu_swprintf(szText, L"Price Seq : %d", pDebugItem ? pDebugItem->m_iPriceSeq : -1);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    mu_swprintf(szText, L"Cash Type : %d", pDebugItem ? pDebugItem->m_iCashType : -1);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
#endif // FOR_WORK
    SyncField(binder, &BuySelectRmlModel::debugLines, "debug_lines", std::move(debugLines));

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), true);
}

bool CMsgBoxIGSBuySelectItem::Render()
{
    return true;
}

// Into #igs_item, in window pixels: at the theme's #igs_item_box.
void CMsgBoxIGSBuySelectItem::RenderItem()
{
    if (m_wItemCode == 65535)
        return;

    Rml::Element* itemBox = m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("igs_item_box") : nullptr;
    Rml::Vector2f offset;
    Rml::Vector2f size;
    if (itemBox != nullptr && UI::RmlBridge::DrawnBox(*itemBox, Rml::BoxArea::Border, offset, size))
        RenderItem3D(offset.x, offset.y, size.x, size.y, m_wItemCode, 0, 0, 0, true);
}

void CMsgBoxIGSBuySelectItem::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSBuySelectItem::BuyBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSBuySelectItem::PresentBtnDown, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
    AddCallbackFunc(CMsgBoxIGSBuySelectItem::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSBuySelectItem::BuyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuySelectItem*>(pOwner);
    const GameShop::BuyOption* pItem = pOwnMsgBox->m_BuyOptions.Selected();
    if (pItem == nullptr)
        return CALLBACK_BREAK; // no option offered or none picked: nothing to buy

    ShowIGSBuyConfirmDialog(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pOwnMsgBox->m_wItemCode, pItem->m_iCashType, pItem->m_szItemName, pItem->m_szItemPrice, pItem->m_szItemPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuySelectItem::PresentBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuySelectItem*>(pOwner);
    const GameShop::BuyOption* pItem = pOwnMsgBox->m_BuyOptions.Selected();
    if (pItem == nullptr)
        return CALLBACK_BREAK; // nothing picked: no gift to send

    CMsgBoxIGSSendGift* pMsgBox = NULL;
    CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftLayout), &pMsgBox);
    pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pItem->m_wItemCode, pItem->m_iCashType, pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuySelectItem::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void CMsgBoxIGSBuySelectItem::AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, wchar_t* pszPriceUnit, int iCashType)
{
    int iValue;
    wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };

    GameShop::BuyOption Item;
    Item = GameShop::BuyOption{};
    Item.m_iPackageSeq = iPackageSeq;
    Item.m_iDisplaySeq = iDisplaySeq;
    Item.m_iPriceSeq = iPriceSeq;
    Item.m_iCashType = iCashType;

    g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
    Item.m_wItemCode = iValue;

    g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, szText);
    wcscpy(Item.m_szItemName, szText);

    g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_PRICE, iValue, szText);
    mu_swprintf(Item.m_szItemPrice, L"%ls %ls", szText, pszPriceUnit);

    g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
    if (iValue > 0)
    {
        mu_swprintf(Item.m_szItemPeriod, L"%d %ls", iValue, szText);
    }
    else
    {
        mu_swprintf(Item.m_szItemPeriod, L"-");
    }

    g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
    if (iValue > 0)
    {
        mu_swprintf(Item.m_szAttribute, I18N::Game::QuantityDDurationS, iValue, Item.m_szItemPeriod);
    }
    else
    {
        mu_swprintf(Item.m_szAttribute, I18N::Game::DurationS, Item.m_szItemPeriod);
    }

    m_BuyOptions.Add(Item);
    m_BuyOptions.SelectLast();
}

bool CMsgBoxIGSBuySelectItemLayout::SetLayout()
{
    CMsgBoxIGSBuySelectItem* pMsgBox = GetMsgBox();
    if (pMsgBox == nullptr)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
