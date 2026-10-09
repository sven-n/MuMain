#include "stdafx.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include <RmlUi/Core/ElementDocument.h>

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSBuyPackageItem.h"

#include "Core/Utilities/UsefulDef.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "InGameShopSystem.h"
#include "MsgBoxIGSBuyConfirm.h"
#include "MsgBoxIGSSendGift.h"

CMsgBoxIGSBuyPackageItem::CMsgBoxIGSBuyPackageItem()
{
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_wItemCode = -1;
    m_iCashType = 0;
    m_szPackageName[0] = '\0';
    m_szPrice[0] = '\0';
    m_szPeriod[0] = '\0';

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }
}

CMsgBoxIGSBuyPackageItem::~CMsgBoxIGSBuyPackageItem()
{
    Release();
}

bool CMsgBoxIGSBuyPackageItem::Create(float fPriority)
{
    SetAddCallbackFunc();

    CMessageBoxBase::Create((IGS_WINDOW_WIDTH / 2) - (IGS_FRAME_WIDTH / 2), (IGS_WINDOW_HEIGHT / 2) - (IGS_FRAME_HEIGHT / 2), IGS_FRAME_WIDTH, IGS_FRAME_HEIGHT, fPriority);

    m_RmlView.Ensure();
    return true;
}

void CMsgBoxIGSBuyPackageItem::Initialize(CShopPackage* pPackage)
{
    int iProductSeq;
    int iValue = 0;
    wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };

    m_iPackageSeq = pPackage->PackageProductSeq;
    m_iDisplaySeq = pPackage->ProductDisplaySeq;
    m_iCashType = pPackage->CashType;

    m_bGiftEnabled = (pPackage->GiftFlag == 184);

    wcsncpy(m_szPackageName, pPackage->PackageProductName, MAX_TEXT_LENGTH);
    ConvertGold(pPackage->Price, szText);
    mu_swprintf(m_szPrice, L"%ls %ls", szText, pPackage->PricUnitName);

    // Period
    pPackage->SetProductSeqFirst();
    pPackage->GetProductSeqNext(iProductSeq);

    g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);

    if (iValue > 0)
    {
        mu_swprintf(m_szPeriod, L"%d %ls", iValue, szText);
    }
    else
    {
        mu_swprintf(m_szPeriod, L"-");
    }

    m_wItemCode = _wtoi(pPackage->InGamePackageID);

    ZeroMemory(m_szDescription, sizeof(wchar_t) * UIMAX_TEXT_LINE * MAX_TEXT_LENGTH);

    g_pRenderText->SetFont(g_hFont);
    int nLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, pPackage->Description, IGS_LISTBOX_WIDTH, false, '#');

    for (int i = 0; i < nLine; ++i)
    {
        if (m_szDescription[i][0] != L'\0')
            m_DescriptionLines.emplace_back(m_szDescription[i]);
    }
}

void CMsgBoxIGSBuyPackageItem::Release()
{
    m_ItemTarget.Disable();
    m_RmlView.Release();
    CMessageBoxBase::Release();
    m_DescriptionLines.clear();
}

bool CMsgBoxIGSBuyPackageItem::Update()
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

    SyncRmlModel();
    return true;
}

bool CMsgBoxIGSBuyPackageItem::Render()
{
    return true;
}

// Into #igs_item, in window pixels: at the theme's #igs_item_box.
void CMsgBoxIGSBuyPackageItem::RenderItem()
{
    if (m_wItemCode == 65535)
        return;

    Rml::Element* itemBox = m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("igs_item_box") : nullptr;
    Rml::Vector2f offset;
    Rml::Vector2f size;
    if (itemBox != nullptr && UI::RmlBridge::DrawnBox(*itemBox, Rml::BoxArea::Border, offset, size))
        RenderItem3D(offset.x, offset.y, size.x, size.y, m_wItemCode, 0, 0, 0, true);
}

void CMsgBoxIGSBuyPackageItem::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::BuyBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::PresentBtnDown, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::BuyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
    ShowIGSBuyConfirmDialog(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::PresentBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);

    CMsgBoxIGSSendGift* pMsgBox = NULL;
    CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftLayout), &pMsgBox);

    pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void CMsgBoxIGSBuyPackageItem::BindRmlModel(Rml::DataModelConstructor& c, BuyPackageRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("title", &model.title);
    c.Bind("name", &model.name);
    c.Bind("price", &model.price);
    auto line = c.RegisterStruct<DescriptionLine>();
    line.RegisterMember("text", &DescriptionLine::text);
    c.RegisterArray<std::vector<DescriptionLine>>();
    c.Bind("description_lines", &model.descriptionLines);
    GameShop::BindDialogCommon(c, model, m_PressedButton);
}

void CMsgBoxIGSBuyPackageItem::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("igs_item") : nullptr, true);
    if (!m_RmlView.Document())
        return;
    auto& binder = m_RmlView.Binder();
    UI::RmlBridge::PlaceOnStage(m_Placement, m_RmlView.Document(), "panel", GetPos());
    UI::RmlBridge::SyncNativeTextSize(binder);

    SyncField(binder, &BuyPackageRmlModel::title, "title", StringUtils::WideToNarrow(I18N::Game::Shop));
    SyncField(binder, &BuyPackageRmlModel::name, "name", StringUtils::WideToNarrow(m_szPackageName));
    SyncField(binder, &BuyPackageRmlModel::price, "price", StringUtils::WideToNarrow(m_szPrice));

    std::vector<DescriptionLine> lines;
    lines.reserve(m_DescriptionLines.size());
    for (const std::wstring& text : m_DescriptionLines)
        lines.push_back({StringUtils::WideToNarrow(text.c_str())});
    SyncField(binder, &BuyPackageRmlModel::descriptionLines, "description_lines", std::move(lines));

    SyncField(binder, &BuyPackageRmlModel::buttons, "buttons",
              std::vector<GameShop::DialogButton>{{StringUtils::WideToNarrow(I18N::Game::Buy1124), true},
                                                  {StringUtils::WideToNarrow(I18N::Game::Gift), m_bGiftEnabled},
                                                  {StringUtils::WideToNarrow(I18N::Game::Cancel), true}});

    std::vector<Rml::String> debugLines;
#ifdef FOR_WORK
    wchar_t szText[256] = { '\0', };
    if (m_wItemCode == 65535)
        mu_swprintf(szText, L"Package item information is not available.");
    else
        mu_swprintf(szText, L"ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode / MAX_ITEM_INDEX, m_wItemCode % MAX_ITEM_INDEX);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    mu_swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    mu_swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
    debugLines.push_back("Price Seq : 0");
    mu_swprintf(szText, L"CashType : %d", m_iCashType);
    debugLines.push_back(StringUtils::WideToNarrow(szText));
#endif // FOR_WORK
    SyncField(binder, &BuyPackageRmlModel::debugLines, "debug_lines", std::move(debugLines));

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), true);
}

bool CMsgBoxBuyPackageItemLayout::SetLayout()
{
    CMsgBoxIGSBuyPackageItem* pMsgBox = GetMsgBox();
    if (pMsgBox == nullptr)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
