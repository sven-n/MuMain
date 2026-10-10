
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
#include "UI/RmlBridge/RmlSyncField.h"
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
    SetAddCallbackFunc();

    CMessageBoxBase::Create(IGS_FRAME_WIDTH, IGS_FRAME_HEIGHT, fPriority);

    InitInputBox();

    return true;
}

void CMsgBoxIGSSendGift::InitInputBox()
{
    m_RmlView.Ensure();
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
}

bool CMsgBoxIGSSendGift::Update()
{
    // The fields are two-way bound, so the model is what the player typed.
    const auto& model = m_RmlView.GetModel();
    wcsncpy(m_szID, StringUtils::NarrowToWide(model.recipient).c_str(), MAX_USERNAME_SIZE);
    m_szID[MAX_USERNAME_SIZE] = L'\0';
    wcsncpy(m_szMessage, StringUtils::NarrowToWide(model.message).c_str(), MAX_GIFT_MESSAGE_SIZE - 1);
    m_szMessage[MAX_GIFT_MESSAGE_SIZE - 1] = L'\0';
    // OK, Cancel: the events the native buttons sent.
    const int pressed = std::exchange(m_PressedButton, -1);
    if (pressed == 0 || pressed == 1)
    {
        g_MessageBox->SendEvent(this, pressed == 0 ? MSGBOX_EVENT_USER_COMMON_OK : MSGBOX_EVENT_USER_COMMON_CANCEL);
        return true;
    }

    SyncRmlModel();

    if (mu::ui::window::IsPress(VK_TAB) == true)
    {
        ChangeInputBoxFocus();
    }

    return true;
}

bool CMsgBoxIGSSendGift::Render()
{
    return true;
}

void CMsgBoxIGSSendGift::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSSendGift::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSSendGift::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
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
    c.Bind("text_px", &model.textPx);
    c.Bind("recipient", &model.recipient);
    c.Bind("message", &model.message);
    c.Bind("title", &model.title);
    c.Bind("recipient_label", &model.recipientLabel);
    c.Bind("message_label", &model.messageLabel);
    GameShop::BindDialogCommon(c, model, m_PressedButton);
    c.Bind("item_lines", &model.itemLines);
    c.Bind("notice_lines", &model.noticeLines);
}

void CMsgBoxIGSSendGift::SyncRmlModel()
{
    if (!m_RmlView.Document())
        return;
    auto& binder = m_RmlView.Binder();
    UI::RmlBridge::SyncNativeTextSize(binder);

    const auto narrow = [](const wchar_t* text) { return StringUtils::WideToNarrow(text); };
    SyncField(binder, &SendGiftRmlModel::title, "title", narrow(I18N::Game::SendGiftItems));
    SyncField(binder, &SendGiftRmlModel::recipientLabel, "recipient_label", narrow(I18N::Game::RecipientSCharacterName));
    SyncField(binder, &SendGiftRmlModel::messageLabel, "message_label", narrow(I18N::Game::MessageToTheRecipient));
    SyncField(binder, &SendGiftRmlModel::itemLines, "item_lines",
              std::vector<Rml::String>{narrow(m_szName), narrow(m_szPrice), narrow(m_szPeriod)});
    std::vector<Rml::String> notice;
    for (int i = 0; i < m_iNumNoticeLine && i < NUM_LINE_CMB; i++)
        notice.push_back(narrow(m_szNotice[i]));
    SyncField(binder, &SendGiftRmlModel::noticeLines, "notice_lines", std::move(notice));
    SyncField(binder, &SendGiftRmlModel::buttons, "buttons",
              std::vector<GameShop::DialogButton>{{narrow(I18N::Game::OK), true}, {narrow(I18N::Game::Cancel), true}});

    std::vector<Rml::String> debugLines;
#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    mu_swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    debugLines.push_back(narrow(szText));
    mu_swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    debugLines.push_back(narrow(szText));
    mu_swprintf(szText, L"Price Seq : %d", m_iPriceSeq);
    debugLines.push_back(narrow(szText));
    mu_swprintf(szText, L"ItemCode : %d", m_wItemCode);
    debugLines.push_back(narrow(szText));
    mu_swprintf(szText, L"CashType : %d", m_iCashType);
    debugLines.push_back(narrow(szText));
#endif // FOR_WORK
    SyncField(binder, &SendGiftRmlModel::debugLines, "debug_lines", std::move(debugLines));

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), true);
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
