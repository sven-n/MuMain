
#include "stdafx.h"
#include "I18N/All.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Render/Text/CUIRenderText.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSSendGift.h"
#include "Audio/DSPlaySound.h"

#include "MsgBoxIGSSendGiftConfirm.h"
#include "UI/Core/WindowCommon.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core.h>

CMsgBoxIGSSendGift::CMsgBoxIGSSendGift()
{
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_iPriceSeq = 0;
    m_wItemCode = -1;
    m_iCashType = 0;

    m_szID[0] = '\0';
    m_szMessage[0] = '\0';

    m_szName[0] = '\0';
    m_szPrice[0] = '\0';
    m_szPeriod[0] = '\0';

    for (int i = 0; i < NUM_LINE_CMB; i++)
    {
        m_szNotice[i][0] = '\0';
    }

    m_iNumNoticeLine = 0;
}

CMsgBoxIGSSendGift::~CMsgBoxIGSSendGift()
{
    Release();
}

bool CMsgBoxIGSSendGift::Create(float fPriority)
{
    LoadImages();
    SetAddCallbackFunc();

    CMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2),
        (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2),
        IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);

    SetButtonInfo();
    InitInputBox();

    SetMsgBackOpacity();

    return true;
}

void CMsgBoxIGSSendGift::InitInputBox()
{
    BuildRmlUi();
    if (m_RmlView.Document())
    {
        if (auto* field = m_RmlView.Document()->GetElementById("igs_gift_id"))
            field->Focus();
    }
}

void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType, const wchar_t* pszName, const wchar_t* pszPrice, const wchar_t* pszPeriod)
{
    m_iPackageSeq = iPackageSeq;
    m_iDisplaySeq = iDisplaySeq;
    m_iPriceSeq = iPriceSeq;
    m_wItemCode = wItemCode;
    m_iCashType = iCashType;

    mu_swprintf(m_szName, I18N::Game::ItemS, pszName);
    mu_swprintf(m_szPrice, I18N::Game::PriceS, pszPrice);
    mu_swprintf(m_szPeriod, I18N::Game::DurationS, pszPeriod);

    m_iNumNoticeLine = ::DivideStringByPixel(&m_szNotice[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, I18N::Game::GiftedItemsCannotBeReturnedDeliverTheGiftS, IGS_TEXT_NOTICE_WIDTH);
}

void CMsgBoxIGSSendGift::Release()
{
    m_RmlView.Release();
    CMessageBoxBase::Release();
    UnloadImages();
}

bool CMsgBoxIGSSendGift::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    // The fields are two-way bound, so the model is what the player typed.
    const auto& model = m_RmlView.GetModel();
    wcsncpy(m_szID, StringUtils::NarrowToWide(model.recipient).c_str(), MAX_USERNAME_SIZE);
    m_szID[MAX_USERNAME_SIZE] = L'\0';
    wcsncpy(m_szMessage, StringUtils::NarrowToWide(model.message).c_str(), MAX_GIFT_MESSAGE_SIZE - 1);
    m_szMessage[MAX_GIFT_MESSAGE_SIZE - 1] = L'\0';
    SyncRmlModel();

    if (mu::ui::window::IsPress(VK_TAB) == true)
    {
        ChangeInputBoxFocus();
    }

    return true;
}

bool CMsgBoxIGSSendGift::Render()
{
    EnableAlphaTest();

    RenderMsgBackColor(true);

    RenderFrame();
    RenderTexts();
    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void CMsgBoxIGSSendGift::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSSendGift::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSSendGift::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSSendGift::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSSendGift::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

    if (pOwnMsgBox)
    {
        if (pOwnMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }

        if (pOwnMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CMsgBoxIGSSendGift::OKButtonDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

    if (pOwnMsgBox->m_szID[0] == '\0')
    {
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::GiftRecipientSIDIsMissing);
    }
    else if (wcscmp(pOwnMsgBox->m_szID, Hero->ID) == 0)
    {
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::YouCannotSendAGiftToYourself);
    }
    else
    {
        ShowIGSSendGiftConfirmDialog(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSSendGift::CancelButtonDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

void CMsgBoxIGSSendGift::SetButtonInfo()
{
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_OK_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(I18N::Game::OK);

    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(0, -1);
    m_BtnCancel.SetText(I18N::Game::Cancel);
}

void CMsgBoxIGSSendGift::RenderFrame()
{
    RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
    RenderImage(IMAGE_IGS_DECO, GetPos().x + IMAGE_IGS_DECO_POS_X, GetPos().y + IMAGE_IGS_DECO_POS_Y, IMAGE_IGS_DECO_WIDTH, IMAGE_IGS_DECO_HEIGHT);
    RenderImage(IMAGE_IGS_INPUTTEXT, GetPos().x + IMAGE_IGS_ID_INPUT_BOX_POS_X, GetPos().y + IMAGE_IGS_ID_INPUT_BOX_POS_Y, IMAGE_IGS_ID_INPUT_BOX_WIDTH, IMAGE_IGS_ID_INPUT_BOX_HEIGHT);
}

void CMsgBoxIGSSendGift::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_POS_Y, I18N::Game::SendGiftItems, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ID_TITLE_POS_X, GetPos().y + IGS_TEXT_ID_TITLE_POS_Y, I18N::Game::RecipientSCharacterName, IGS_TEXT_ID_TITLE_WIDTH, 0, RT3_SORT_LEFT);

    g_pRenderText->SetTextColor(0, 0, 0, 255);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_MESSAGE_TITLE_POS_Y, I18N::Game::MessageToTheRecipient, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    g_pRenderText->SetTextColor(247, 186, 0, 255);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_NAME_POS_Y, m_szName, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PRICE_POS_Y, m_szPrice, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, m_szPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    for (int i = 0; i < m_iNumNoticeLine; i++)
    {
        g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_NOTICE_POS_Y + i * 10, m_szNotice[i], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
    }

#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    mu_swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 200, 0, RT3_SORT_LEFT);
    mu_swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 200, 0, RT3_SORT_LEFT);
    mu_swprintf(szText, L"Price Seq : %d", m_iPriceSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 200, 0, RT3_SORT_LEFT);
    mu_swprintf(szText, L"ItemCode : %d", m_wItemCode);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 200, 0, RT3_SORT_LEFT);
    mu_swprintf(szText, L"CashType : %d", m_iCashType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 50, szText, 200, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

void CMsgBoxIGSSendGift::RenderButtons()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

// Tab walks the two fields, as it did when they were native boxes.
void CMsgBoxIGSSendGift::ChangeInputBoxFocus()
{
    if (!m_RmlView.Document())
        return;
    const char* next = FieldHasFocus("igs_gift_id") ? "igs_gift_message" : "igs_gift_id";
    if (auto* field = m_RmlView.Document()->GetElementById(next))
        field->Focus();
}

bool CMsgBoxIGSSendGift::FieldHasFocus(const char* id) const
{
    auto* field = m_RmlView.Document() ? m_RmlView.Document()->GetElementById(id) : nullptr;
    return field != nullptr && field->IsPseudoClassSet("focus");
}

void CMsgBoxIGSSendGift::BindRmlModel(Rml::DataModelConstructor& c, SendGiftRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("text_px", &model.textPx);
    c.Bind("recipient", &model.recipient);
    c.Bind("message", &model.message);
}

void CMsgBoxIGSSendGift::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CMsgBoxIGSSendGift::SyncRmlModel()
{
    if (!m_RmlView.Document())
        return;
    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), GetPos());
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), true);
}

void CMsgBoxIGSSendGift::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_back01.tga", IMAGE_IGS_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_icon.tga", IMAGE_IGS_DECO, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_namebox.tga", IMAGE_IGS_INPUTTEXT, GL_LINEAR);
}

void CMsgBoxIGSSendGift::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_FRAME);
    DeleteBitmap(IMAGE_IGS_DECO);
    DeleteBitmap(IMAGE_IGS_INPUTTEXT);
}

bool CMsgBoxIGSSendGiftLayout::SetLayout()
{
    CMsgBoxIGSSendGift* pMsgBox = GetMsgBox();
    if (pMsgBox == nullptr)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
