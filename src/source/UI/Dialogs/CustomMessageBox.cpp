#include "stdafx.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Audio/DSPlaySound.h"
#include "UI/Widgets/UIControls.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "Engine/Object/ZzzOpenData.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "UI/Core/WindowSystem.h"
#include "Core/Text/TextLineWrap.h"

extern int DeleteIndex;
extern int AppointStatus;
extern bool LogOut;

char AppointType;

#define SUBGUILDMASTER	64
#define BATTLEMASTER	32

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
template <typename AppendLine>
int AppendWrappedMessageLines(const std::wstring& text, BYTE fontType, int maxWidth, AppendLine appendLine)
{
    g_pRenderText->SetFont(fontType == MSGBOX_FONT_BOLD ? g_hFontBold : g_hFont);
    auto lines = WrapTextToWidth(text, maxWidth, [](const wchar_t* line, size_t length)
    {
        return g_pRenderText->MeasureText(line, static_cast<int>(length)).cx;
    });
    if (lines.empty())
    {
        lines.emplace_back();
    }

    for (const auto& line : lines)
    {
        appendLine(line);
    }
    return static_cast<int>(lines.size());
}
} // namespace

mu::ui::window::CUseFruitCheckMsgBox::CUseFruitCheckMsgBox()
{
    ZeroMemory(&m_Item, sizeof(m_Item));
}

mu::ui::window::CUseFruitCheckMsgBox::~CUseFruitCheckMsgBox()
{
    Release();
}

bool mu::ui::window::CUseFruitCheckMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    if (g_pNewUI3DRenderMng)
    {
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
    }

    ITEM* pItem = g_pMyInventory->GetStandbyItem();
    if (pItem == NULL)
    {
        return false;
    }

    Set3DItem(pItem);

    wchar_t strName[50] = { 0, };
    if (pItem->Type == ITEM_FRUITS)
    {
        switch (pItem->Level)
        {
        case 0:
            mu_swprintf(strName, L"%ls", I18N::Game::ENG);
            break;
        case 1:
            mu_swprintf(strName, L"%ls", I18N::Game::STA);
            break;
        case 2:
            mu_swprintf(strName, L"%ls", I18N::Game::AGI);
            break;
        case 3:
            mu_swprintf(strName, L"%ls", I18N::Game::STR);
            break;
        case 4:
            mu_swprintf(strName, L"%ls", I18N::Game::Command);
            break;
        }
    }

    wchar_t strText[128] = { 0, };
    mu_swprintf(strText, L"( %ls%ls )", strName, I18N::Game::Fruit);
    AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    AddMsg(I18N::Game::Choose, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);

    return true;
}

void mu::ui::window::CUseFruitCheckMsgBox::Release()
{
    CMessageBoxBase::Release();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

void mu::ui::window::CUseFruitCheckMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CUseFruitCheckMsgBox::Set3DItem(ITEM* pItem)
{
    if (pItem)
    {
        memcpy(&m_Item, pItem, sizeof(ITEM));
    }
}

bool mu::ui::window::CUseFruitCheckMsgBox::Update()
{
    m_BtnAdd.Update();
    m_BtnMinus.Update();
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CUseFruitCheckMsgBox::Render()
{
    EnableAlphaTest();

    RenderFrame();
    RenderTexts();
    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CUseFruitCheckMsgBox::Render3D()
{
    float x, y, width, height;

    x = GetPos().x + 30;
    y = GetPos().y + 30;
    width = MSGBOX_3DITEM_WIDTH;
    height = MSGBOX_3DITEM_HEIGHT;

    RenderItem3D(x, y, width, height,
        m_Item.Type,
        m_Item.Level,
        m_Item.ExcellentFlags,
        m_Item.AncientDiscriminator,
        true);		// PickUp
}

bool mu::ui::window::CUseFruitCheckMsgBox::IsVisible() const
{
    return true;
}

CALLBACK_RESULT mu::ui::window::CUseFruitCheckMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CUseFruitCheckMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAdd.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_ADD);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnMinus.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_MINUS);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseFruitCheckMsgBox::AddBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    BYTE byIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(byIndex, 0, true);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseFruitCheckMsgBox::MinusBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    BYTE byIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(byIndex, 0, false);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseFruitCheckMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CUseFruitCheckMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CUseFruitCheckMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CUseFruitCheckMsgBox::AddBtnDown, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_ADD);
    AddCallbackFunc(mu::ui::window::CUseFruitCheckMsgBox::MinusBtnDown, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_MINUS);
    AddCallbackFunc(mu::ui::window::CUseFruitCheckMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CUseFruitCheckMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float triwidth = (GetSize().cx / 3.f);
    float btnhalf = (MSGBOX_BTN_EMPTY_SMALL_WIDTH) / 2.f;

    x = GetPos().x + (triwidth / 2) - btnhalf;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    m_BtnAdd.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnAdd.SetText(I18N::Game::Create);

    x = GetPos().x + triwidth + (triwidth / 2) - btnhalf;
    m_BtnMinus.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnMinus.SetText(I18N::Game::Decrease);

    x = GetPos().x + (triwidth * 2) + (triwidth / 2) - btnhalf;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Cancel);
}

void mu::ui::window::CUseFruitCheckMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 1; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CUseFruitCheckMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x + MSGBOX_TEXT_LEFT_BLANK_3DITEM; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + 60 + ((GetSize().cx - 60) / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CUseFruitCheckMsgBox::RenderButtons()
{
    m_BtnAdd.Render();
    m_BtnMinus.Render();
    m_BtnCancel.Render();
}

//////////////////////////////////////////////////////////////////////////

mu::ui::window::CGemIntegrationMsgBox::CGemIntegrationMsgBox()
{
};

mu::ui::window::CGemIntegrationMsgBox::~CGemIntegrationMsgBox()
{
    Release();
};

bool mu::ui::window::CGemIntegrationMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(I18N::Game::JewelCombination, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);

    AddMsg(I18N::Game::YouCanCombineOrDissolve);
    AddMsg(I18N::Game::VariousJewels);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CGemIntegrationMsgBox::Release()
{
    CMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool mu::ui::window::CGemIntegrationMsgBox::Update()
{
    m_BtnUnity.Update();
    m_BtnDisjoint.Update();
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CGemIntegrationMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CGemIntegrationMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CGemIntegrationMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGemIntegrationMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CGemIntegrationMsgBox::UnityBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY);
    AddCallbackFunc(mu::ui::window::CGemIntegrationMsgBox::DisjointBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT);
    AddCallbackFunc(mu::ui::window::CGemIntegrationMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnUnity.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDisjoint.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationMsgBox::UnityBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetMode(COMGEM::ATTACH);

    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationUnityMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationMsgBox::DisjointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetMode(COMGEM::DETACH);

    if (!COMGEM::FindWantedList())
    {
        g_pSystemLogBox->AddText(I18N::Game::CanTBeDismantled, mu::ui::window::TYPE_ERROR_MESSAGE);
        return CALLBACK_BREAK;
    }

    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationDisjointMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CGemIntegrationMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalf = (MSGBOX_BTN_EMPTY_WIDTH) / 2.f;

    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y = GetPos().y + BTN_TOP_BLANK;
    width = MSGBOX_BTN_EMPTY_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    m_BtnUnity.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);

    m_BtnUnity.SetText(I18N::Game::JewelCombination);

    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y += BTN_GAP;
    m_BtnDisjoint.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);

    m_BtnDisjoint.SetText(I18N::Game::DismantleJewel);

    btnhalf = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y += BTN_GAP;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);

    m_BtnCancel.SetText(I18N::Game::Close388);
}

void mu::ui::window::CGemIntegrationMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CGemIntegrationMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CGemIntegrationMsgBox::RenderButtons()
{
    m_BtnUnity.Render();
    m_BtnDisjoint.Render();
    m_BtnCancel.Render();
}

mu::ui::window::CGemIntegrationUnityMsgBox::CGemIntegrationUnityMsgBox()
{
}

mu::ui::window::CGemIntegrationUnityMsgBox::~CGemIntegrationUnityMsgBox()
{
    Release();
}

bool mu::ui::window::CGemIntegrationUnityMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(I18N::Game::JewelCombination, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    SetText();
    SetButtonInfo();

    return true;
}

void mu::ui::window::CGemIntegrationUnityMsgBox::SetText(void)
{
    m_MsgDataList.clear();
    if (COMGEM::m_cGemType == COMGEM::NOGEM)
    {
        AddMsg(I18N::Game::JewelCombination, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        AddMsg(I18N::Game::SelectAJewelToCombine);
    }
    else
    {
        AddMsg(I18N::Game::JewelCombination, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        AddMsg(I18N::Game::ChooseANumberButtonToCombine);
    }
}

void mu::ui::window::CGemIntegrationUnityMsgBox::Release()
{
    CMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool mu::ui::window::CGemIntegrationUnityMsgBox::Update()
{
    int i;
    switch (COMGEM::m_cGemType)
    {
    case COMGEM::eNOGEM:
        for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            m_cJewelButton[i].Update();
        break;
    default:
        for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            m_cMixButton[i].Update();
        break;
    }
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CGemIntegrationUnityMsgBox::Render()
{
    EnableAlphaTest();

    RenderFrame();

    RenderTexts();

    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CGemIntegrationUnityMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CGemIntegrationUnityMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::TenBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TEN);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::TwentyBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TWENTY);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::ThirtyBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_THIRTY);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    AddCallbackFunc(mu::ui::window::CGemIntegrationUnityMsgBox::SelectMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_SELECTMIX);
}

void mu::ui::window::CGemIntegrationUnityMsgBox::ResetWndSize(int _nType)
{
    int	height;

    if (_nType)
    {
        m_nMiddleCount = 7;
    }
    else
    {
        m_nMiddleCount = MIDDLE_COUNT;
    }

    height = MSGBOX_TOP_HEIGHT + (m_nMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;
    SetSize(GetSize().cx, height);
    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + GetSize().cy - 50);
}

void mu::ui::window::CGemIntegrationUnityMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;

    int	  nNum = 0;
    int	  nBtnIndex[COMGEM::eGEMTYPE_END] = { 1806, 1807, 3312, 3313, 3314, 2081, 3315, 3316, 3317, 3318 };
    wchar_t szTemp[256] = { 0, };
    CMessageBoxButton	cButton;
    x = GetPos().x;
    y = GetPos().y + 50;
    for (int i = 0; i < (int)COMGEM::eGEMTYPE_END; i++)
    {
        cButton.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x + 20.0f + (i % 2) * (20 + width), y + (height + 5.0f) * int(i / 2), width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
        cButton.SetText(I18N::Game::Lookup(nBtnIndex[i]));
        m_cJewelButton.push_back(cButton);
    }

    for (int k = 0; k < (int)COMGEM::eCOMTYPE_END; k++)
    {
        cButton.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x + 50.0f, y + (height + 10.0f) * k, MSGBOX_BTN_EMPTY_WIDTH + 20, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
        // 1808 "%d개 조합(%d젠 소요)"
        mu_swprintf(szTemp, I18N::Game::CombineDDZenIsRequired, 10 * (k + 1), 500000 * (k + 1));
        cButton.SetText(szTemp);
        m_cMixButton.push_back(cButton);
    }

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y += 15.0f + (height + 10.0f) * (int)COMGEM::eCOMTYPE_END;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Close388);

    ResetWndSize(0);
}

void mu::ui::window::CGemIntegrationUnityMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;

    for (int i = 0; i < m_nMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CGemIntegrationUnityMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CGemIntegrationUnityMsgBox::RenderButtons()
{
    int i;
    switch (COMGEM::m_cGemType)
    {
    case COMGEM::eNOGEM:
        for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            m_cJewelButton[i].Render();
        break;
    default:
        for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            m_cMixButton[i].Render();
        break;
    }
    m_BtnCancel.Render();
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
    if (pMsgBox)
    {
        int i;
        switch (COMGEM::m_cGemType)
        {
        case COMGEM::eNOGEM:
            for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            {
                if (!pMsgBox->m_cJewelButton[i].IsMouseIn())	continue;
                COMGEM::SetGem(i * 2);
                pMsgBox->ResetWndSize(1);
                //	SetText();
                break;
            }
            break;
        default:
            for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            {
                if (!pMsgBox->m_cMixButton[i].IsMouseIn())	continue;
                COMGEM::m_cComType = COMGEM::GetJewelRequireCount(i);
                pMsgBox->ResetWndSize(0);
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_SELECTMIX);
                return CALLBACK_BREAK;
            }
            break;
        }

        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::CELE);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::SOUL);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::SelectMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    if (!COMGEM::CheckInv())
    {
        return CALLBACK_BREAK;
    }

    {
        wchar_t strText[256] = { 0, };
        mu::ui::window::GenericDialogConfig cfg;
        cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
        mu_swprintf(strText, I18N::Game::Lookup(COMGEM::GetJewelIndex(COMGEM::m_cGemType, 0)), I18N::Game::JewelOfSoul, COMGEM::m_cCount);
        cfg.lines.push_back({ strText, true });
        mu_swprintf(strText, I18N::Game::CombinationCostDZen, COMGEM::m_iValue);
        cfg.lines.push_back({ strText, true });
        cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
        cfg.onSecondary = []
        {
            COMGEM::GetBack();
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationUnityMsgBoxLayout));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::TenBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::FIRST);

    if (COMGEM::CheckInv())
    {
        wchar_t strText[256] = { 0, };
        if (COMGEM::m_cGemType == COMGEM::CELE)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfBless, COMGEM::m_cCount);
        }
        else if (COMGEM::m_cGemType == COMGEM::SOUL)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfSoul, COMGEM::m_cCount);
        }
        mu::ui::window::GenericDialogConfig cfg;
        cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
        cfg.lines.push_back({ strText, true });
        mu_swprintf(strText, I18N::Game::CombinationCostDZen, COMGEM::m_iValue);
        cfg.lines.push_back({ strText, true });
        cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
        cfg.onSecondary = []
        {
            COMGEM::GetBack();
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationUnityMsgBoxLayout));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnTen.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::TwentyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::SECOND);

    if (COMGEM::CheckInv())
    {
        wchar_t strText[256] = { 0, };
        if (COMGEM::m_cGemType == COMGEM::CELE)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfBless, COMGEM::m_cCount);
        }
        else if (COMGEM::m_cGemType == COMGEM::SOUL)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfSoul, COMGEM::m_cCount);
        }
        mu::ui::window::GenericDialogConfig cfg;
        cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
        cfg.lines.push_back({ strText, true });
        mu_swprintf(strText, I18N::Game::CombinationCostDZen, COMGEM::m_iValue);
        cfg.lines.push_back({ strText, true });
        cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
        cfg.onSecondary = []
        {
            COMGEM::GetBack();
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationUnityMsgBoxLayout));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnTwenty.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::ThirtyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::THIRD);

    if (COMGEM::CheckInv())
    {
        wchar_t strText[256] = { 0, };
        if (COMGEM::m_cGemType == COMGEM::CELE)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfBless, COMGEM::m_cCount);
        }
        else if (COMGEM::m_cGemType == COMGEM::SOUL)
        {
            mu_swprintf(strText, I18N::Game::AreYouSureToCombineSXD, I18N::Game::JewelOfSoul, COMGEM::m_cCount);
        }
        mu::ui::window::GenericDialogConfig cfg;
        cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
        cfg.lines.push_back({ strText, true });
        mu_swprintf(strText, I18N::Game::CombinationCostDZen, COMGEM::m_iValue);
        cfg.lines.push_back({ strText, true });
        cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
        cfg.onSecondary = []
        {
            COMGEM::GetBack();
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationUnityMsgBoxLayout));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnThirty.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CGemIntegrationDisjointMsgBox::CGemIntegrationDisjointMsgBox()
{
    m_iMiddleFrameCount = 0;
}

mu::ui::window::CGemIntegrationDisjointMsgBox::~CGemIntegrationDisjointMsgBox()
{
    Release();
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    m_iMiddleFrameCount = 1;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(I18N::Game::DismantleJewel, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    SetButtonInfo();
    ChangeMiddleFrameBig();
    AddMsg(L" ", RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    AddMsg(I18N::Game::SelectAJewelToDissolve, CLRDW_YELLOW, MSGBOX_FONT_BOLD);

    return true;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::Release()
{
    CMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Update()
{
    m_BtnCancel.Update();

    if (true)
    {
        COMGEM::MoveUnMixList();

        UNMIX_TEXT* pUT = COMGEM::m_UnmixTarList.GetSelectedText();
        if (pUT)
        {
            m_BtnDisjoint.SetEnable(true);
            m_BtnDisjoint.Update();
        }
    }

    return true;
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    RenderGemList();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::ChangeMiddleFrameSmall()
{
    int height = 0;

    m_iMiddleFrameCount = 1;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    SetSize(GetSize().cx, height);

    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + 80);

    m_BtnDisjoint.SetEnable(false);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::ChangeMiddleFrameBig()
{
    int height = 0;

    m_iMiddleFrameCount = 10;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    SetSize(GetSize().cx, height);

    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + GetSize().cy - 50);

    m_BtnDisjoint.SetPos(m_BtnDisjoint.GetPosX(), GetPos().y + GetSize().cy - 85);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_BLESSING);
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_SOUL);
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::DisjointBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT);
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnBlessing.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_BLESSING);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSoul.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_SOUL);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDisjoint.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    COMGEM::SetGem(COMGEM::CELE);

    COMGEM::ResetWantedList();
    COMGEM::FindWantedList();

    if (COMGEM::m_UnmixTarList.IsEmpty() == true)
    {
        g_pSystemLogBox->AddText(I18N::Game::CanTBeDismantled, mu::ui::window::TYPE_ERROR_MESSAGE);
        COMGEM::GetBack();
        pMsgBox->ChangeMiddleFrameSmall();
    }
    else
    {
        pMsgBox->ChangeMiddleFrameBig();
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    COMGEM::SetGem(COMGEM::SOUL);

    COMGEM::ResetWantedList();
    COMGEM::FindWantedList();

    if (COMGEM::m_UnmixTarList.IsEmpty() == true)
    {
        g_pSystemLogBox->AddText(I18N::Game::CanTBeDismantled, mu::ui::window::TYPE_ERROR_MESSAGE);
        COMGEM::GetBack();
        pMsgBox->ChangeMiddleFrameSmall();
    }
    else
    {
        pMsgBox->ChangeMiddleFrameBig();
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::DisjointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    UNMIX_TEXT* pUT = COMGEM::m_UnmixTarList.GetSelectedText();
    if (pUT)
    {
        const ITEM* pItem = FindInventoryItemBySlot(pUT->m_iInvenIdx);
        if (pItem == nullptr)
        {
            return CALLBACK_BREAK;
        }

        COMGEM::SelectFromList(pUT->m_iInvenIdx, pUT->m_cLevel);

        int	iGemLevel = COMGEM::GetUnMixGemLevel() + 1;
        int	  nIdx = COMGEM::Check_Jewel(pItem->Type);
        COMGEM::SetGem(nIdx);

        wchar_t strText[256] = { 0, };
        mu_swprintf(strText, I18N::Game::AreYouSureToDisbandSD, I18N::Game::Lookup(COMGEM::GetJewelIndex(nIdx, COMGEM::eGEM_NAME)), iGemLevel);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
        cfg.lines.push_back({ strText, true });
        mu_swprintf(strText, I18N::Game::DissolvingCostDZen, COMGEM::m_iValue);
        cfg.lines.push_back({ strText, true });
        cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
        cfg.onSecondary = []
        {
            COMGEM::GetBack();
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationDisjointMsgBoxLayout));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 40;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Close388);

    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    m_BtnDisjoint.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnDisjoint.SetText(I18N::Game::Disband);
    m_BtnDisjoint.SetEnable(false);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleFrameCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::RenderGemList()
{
    
    

    int x, y;
    y = GetPos().y + 80;

    m_BtnDisjoint.Render();

    x = GetPos().x + (GetSize().cx / 2) - (COMGEM::m_UnmixTarList.GetWidth() / 2);

    COMGEM::m_UnmixTarList.SetPosition(x, y + 40 + COMGEM::m_UnmixTarList.GetHeight() / 2.0f);
    COMGEM::RenderUnMixList();
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::RenderButtons()
{
    m_BtnCancel.Render();
}

//////////////////////////////////////////////////////////////////////////

mu::ui::window::CSystemMenuMsgBox::CSystemMenuMsgBox()
{
}

mu::ui::window::CSystemMenuMsgBox::~CSystemMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CSystemMenuMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (5 * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CSystemMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CSystemMenuMsgBox::Update()
{
    m_BtnGameOver.Update();
    m_BtnChooseServer.Update();
    m_BtnChooseCharacter.Update();
    m_BtnOption.Update();
    m_BtnCancel.Update();
    return true;
}

bool mu::ui::window::CSystemMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CSystemMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 5; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CSystemMenuMsgBox::RenderButtons()
{
    m_BtnGameOver.Render();
    m_BtnChooseServer.Render();
    m_BtnChooseCharacter.Render();
    m_BtnOption.Render();
    m_BtnCancel.Render();
}

void mu::ui::window::CSystemMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::GameOverBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_GAMEOVER);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::ChooseServerBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSESERVER);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::ChooseCharacterBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSECHARACTER);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::OptionBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_OPTION);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    AddCallbackFunc(mu::ui::window::CSystemMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void mu::ui::window::CSystemMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 23;
    m_BtnGameOver.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGameOver.SetText(I18N::Game::ExitGame);

    y += 30.f;
    m_BtnChooseServer.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChooseServer.SetText(I18N::Game::SelectServer);

    y += 30.f;
    m_BtnChooseCharacter.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChooseCharacter.SetText(I18N::Game::SwitchCharacter);

    y += 30.f;
    m_BtnOption.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnOption.SetText(I18N::Game::Option385);

    y += 30.f;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnCancel.SetText(I18N::Game::Cancel);
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSystemMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnGameOver.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_GAMEOVER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChooseServer.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSESERVER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChooseCharacter.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSECHARACTER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnOption.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_OPTION);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::GameOverBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_ErrorReport.Write(L"> Menu - Exit game. ");
    g_ErrorReport.WriteCurrentTime();

    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_ERROR_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(LogOutType::CloseGame);
        PostMessage(g_hWnd, WM_CLOSE, 0, 0);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 0");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::ChooseServerBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    View_End_Result = false;
    Suc_Or_Fail = -1;
    M34CryWolf1st::CryWolfMVPInit();

    g_ErrorReport.Write(L"> Menu - Join another server. ");
    g_ErrorReport.WriteCurrentTime();

    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_ERROR_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        g_pNewUIMng->ResetActiveUIObj();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToServerSelection);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 2");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::ChooseCharacterBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    View_End_Result = false;
    Suc_Or_Fail = -1;
    M34CryWolf1st::CryWolfMVPInit();

    g_ErrorReport.Write(L"> Menu - Join with another character. ");
    g_ErrorReport.WriteCurrentTime();

    //  게임내에서 설정한 데이터 저장.
    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        g_pNewUIMng->ResetActiveUIObj();
        LogOut = true;SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToCharacterSelection);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 1");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::OptionBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_OPTION);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSystemMenuMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CBloodCastleResultMsgBox::CBloodCastleResultMsgBox()
{
}

mu::ui::window::CBloodCastleResultMsgBox::~CBloodCastleResultMsgBox()
{
}

bool mu::ui::window::CBloodCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CBloodCastleResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CBloodCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    return true;
}

bool mu::ui::window::CBloodCastleResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool mu::ui::window::CBloodCastleResultMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CBloodCastleResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

CALLBACK_RESULT mu::ui::window::CBloodCastleResultMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CBloodCastleResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CBloodCastleResultMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CDevilSquareRankMsgBox::CDevilSquareRankMsgBox()
{
}

mu::ui::window::CDevilSquareRankMsgBox::~CDevilSquareRankMsgBox()
{
}

bool mu::ui::window::CDevilSquareRankMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CDevilSquareRankMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CDevilSquareRankMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT1 * MSGBOX_MIDDLE_HEIGHT)
        + (MIDDLE_COUNT2 * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_LINE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);

    return true;
}

bool mu::ui::window::CDevilSquareRankMsgBox::Update()
{
    m_BtnOk.Update();
    matchEvent::SetPosition(GetPos().x, GetPos().y);

    return true;
}

bool mu::ui::window::CDevilSquareRankMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();

    return true;
}

CALLBACK_RESULT mu::ui::window::CDevilSquareRankMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDevilSquareRankMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CDevilSquareRankMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CDevilSquareRankMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT1; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
    y += height;

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;

    for (int i = 0; i < MIDDLE_COUNT2; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 75; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 93; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 255; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 273; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);
}

mu::ui::window::CChaosCastleResultMsgBox::CChaosCastleResultMsgBox()
{
}

mu::ui::window::CChaosCastleResultMsgBox::~CChaosCastleResultMsgBox()
{
}

bool mu::ui::window::CChaosCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CChaosCastleResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CChaosCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    return true;
}

bool mu::ui::window::CChaosCastleResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool mu::ui::window::CChaosCastleResultMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CChaosCastleResultMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CChaosCastleResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CChaosCastleResultMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

void mu::ui::window::CChaosCastleResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////

mu::ui::window::CChaosMixMenuMsgBox::CChaosMixMenuMsgBox()
{
}

mu::ui::window::CChaosMixMenuMsgBox::~CChaosMixMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CChaosMixMenuMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CChaosMixMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CChaosMixMenuMsgBox::Update()
{
    m_BtnGeneralMix.Update();
    m_BtnChaosMix.Update();
    m_BtnMix380.Update();
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CChaosMixMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CChaosMixMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CChaosMixMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnGeneralMix.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_GENERALMIX);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChaosMix.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_CHAOSMIX);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnMix380.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_MIX380);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CChaosMixMenuMsgBox::GeneralMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(0);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CChaosMixMenuMsgBox::ChaosMixBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(1);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CChaosMixMenuMsgBox::Mix380BtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(2);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CChaosMixMenuMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.ClearCheckRecipeResult();
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CChaosMixMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CChaosMixMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CChaosMixMenuMsgBox::GeneralMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_GENERALMIX);
    AddCallbackFunc(mu::ui::window::CChaosMixMenuMsgBox::ChaosMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_CHAOSMIX);
    AddCallbackFunc(mu::ui::window::CChaosMixMenuMsgBox::Mix380BtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_MIX380);
    AddCallbackFunc(mu::ui::window::CChaosMixMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CChaosMixMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnGeneralMix.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGeneralMix.SetText(I18N::Game::RegularCombination);

    y = GetPos().y + 155;
    m_BtnChaosMix.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChaosMix.SetText(I18N::Game::ChaosWeaponCombination);

    y = GetPos().y + 225;
    m_BtnMix380.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnMix380.SetText(I18N::Game::ItemOptionCombination);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Close388);
}

void mu::ui::window::CChaosMixMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CChaosMixMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 128, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::SelectMethodOfCombination);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);

    mu_swprintf(szText, I18N::Game::Wings7TypesFruitDevilSInvitation);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::Dinorant1015ItemsCloakOfInvisibility, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::FenrirSHornScrollOfBloodCondorSFeather, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 3 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 100;
    mu_swprintf(szText, I18N::Game::ChaosDragonAxeChaosLightningStaff, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::ChaosNatureBow, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 85;
    mu_swprintf(szText, I18N::Game::Add380ItemOption, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CChaosMixMenuMsgBox::RenderButtons()
{
    m_BtnGeneralMix.Render();
    m_BtnChaosMix.Render();
    m_BtnMix380.Render();
    m_BtnCancel.Render();
}

mu::ui::window::CProgressMsgBox::CProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

mu::ui::window::CProgressMsgBox::~CProgressMsgBox()
{
    Release();
}

bool mu::ui::window::CProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetAddCallbackFunc();

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void mu::ui::window::CProgressMsgBox::Release()
{
}

void mu::ui::window::CProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
}

void mu::ui::window::CProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

void mu::ui::window::CProgressMsgBox::SetElapseTime(DWORD dwElapseTime)
{
    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;
}

int mu::ui::window::CProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedMessageLines(strMsg, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH),
        [&](const std::wstring& line)
        {
            auto* message = new MSGBOX_TEXTDATA;
            message->strMsg = line;
            message->dwColor = dwColor;
            message->byFontType = byFontType;
            m_MsgDataList.push_back(message);
        });
}

bool mu::ui::window::CProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    return true;
}

bool mu::ui::window::CProgressMsgBox::Render()
{
    EnableAlphaTest();

    RenderFrame();
    RenderTexts();
    RenderProgress();

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CProgressMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CProgressMsgBox::RenderTexts()
{
    

    float x, y;

    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CProgressMsgBox::RenderProgress()
{
    DWORD dwTime = timeGetTime();
    float fProgress = (float)(dwTime - m_dwStartTime) / m_dwElapseTime;

    float x, y;
    x = GetPos().x + MSGBOX_WIDTH / 2 - 160.f / 2;
    y = GetPos().y + GetSize().cy - 50.f;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BG, x, y, 160.f, 18.f);
    x += 5.f;
    y += 5.f;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BAR, x, y, 150.f * fProgress, 8.f);
}

CALLBACK_RESULT mu::ui::window::CProgressMsgBox::ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

mu::ui::window::CCursedTempleProgressMsgBox::CCursedTempleProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

mu::ui::window::CCursedTempleProgressMsgBox::~CCursedTempleProgressMsgBox()
{
    Release();
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetAddCallbackFunc();

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void mu::ui::window::CCursedTempleProgressMsgBox::Release()
{
}

void mu::ui::window::CCursedTempleProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CCursedTempleProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    AddCallbackFunc(mu::ui::window::CCursedTempleProgressMsgBox::CompleteProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
}

void mu::ui::window::CCursedTempleProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

int mu::ui::window::CCursedTempleProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedMessageLines(strMsg, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH),
        [&](const std::wstring& line)
        {
            auto* message = new MSGBOX_TEXTDATA;
            message->strMsg = line;
            message->dwColor = dwColor;
            message->byFontType = byFontType;
            m_MsgDataList.push_back(message);
        });
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
    }

    if (CheckHeroAction() == false || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM) == false)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    return true;
}

void mu::ui::window::CCursedTempleProgressMsgBox::SetNpcIndex(DWORD dwIndex)
{
    m_dwNpcIndex = dwIndex;
}

DWORD mu::ui::window::CCursedTempleProgressMsgBox::GetNpcIndex()
{
    return m_dwNpcIndex;
}

CALLBACK_RESULT mu::ui::window::CCursedTempleProgressMsgBox::ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarEnabled(false);
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CCursedTempleProgressMsgBox::CompleteProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCursedTempleProgressMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    SocketClient->ToGameServer()->SendTalkToNpcRequest(pMsgBox->GetNpcIndex());

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarCloseTimer();
    return CALLBACK_CONTINUE;
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderProgress();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CCursedTempleProgressMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CCursedTempleProgressMsgBox::RenderTexts()
{
    

    float x, y;

    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CCursedTempleProgressMsgBox::RenderProgress()
{
    DWORD dwTime = timeGetTime();
    float fProgress = (float)(dwTime - m_dwStartTime) / m_dwElapseTime;

    float x, y;
    x = GetPos().x + MSGBOX_WIDTH / 2 - 160.f / 2;
    y = GetPos().y + GetSize().cy - 50.f;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BG, x, y, 160.f, 18.f);
    x += 5.f;
    y += 5.f;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BAR, x, y, 150.f * fProgress, 8.f);
}

bool mu::ui::window::CCursedTempleProgressMsgBox::CheckHeroAction()
{
    if (g_isCharacterBuff((&Hero->Object), eDeBuff_Harden)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Stun)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_CursedTempleRestraint)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Sleep)
        )
    {
        return false;
    }

    int action = Hero->Object.CurrentAction;

    if (!(action >= PLAYER_SET && action <= PLAYER_STOP_RIDE_WEAPON)
        && !(action == PLAYER_SHOCK)
        && !(action == PLAYER_FENRIR_STAND)
        && !(action == PLAYER_FENRIR_STAND_TWO_SWORD)
        && !(action == PLAYER_FENRIR_STAND_ONE_RIGHT)
        && !(action == PLAYER_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_DARKLORD_STAND)
        && !(action == PLAYER_STOP_RIDE_HORSE)
        && !(action == PLAYER_ATTACK_STRIKE)
        && !(action == PLAYER_STOP_TWO_HAND_SWORD_TWO)
        && !(action >= PLAYER_RAGE_FENRIR_STAND && action <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_RAGE_UNI_STOP_ONE_RIGHT))
    {
        return false;
    }

    return true;
}

mu::ui::window::CDuelMsgBox::CDuelMsgBox()
{
}

mu::ui::window::CDuelMsgBox::~CDuelMsgBox()
{
}

bool mu::ui::window::CDuelMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 70;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT * 7 + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CDuelMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CDuelMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CDuelMsgBox::OkBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
    AddCallbackFunc(mu::ui::window::CDuelMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_CANCEL);
    AddCallbackFunc(mu::ui::window::CDuelMsgBox::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void mu::ui::window::CDuelMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;

    x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CDuelMsgBox::Release()
{
}

bool mu::ui::window::CDuelMsgBox::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CDuelMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButton();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CDuelMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x + (GetSize().cx / 2) - 74;
    y = GetPos().y + 15;
    width = 148;
    height = 138;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_DUEL_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 7; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CDuelMsgBox::RenderTexts()
{
    wchar_t strDuelID[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(strDuelID, L"[%ls]", g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 115, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 135, I18N::Game::YouAreChallengedToADuel, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 151, I18N::Game::WouldYouLikeToAcceptTheChallenge, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CDuelMsgBox::RenderButton()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

CALLBACK_RESULT mu::ui::window::CDuelMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDuelMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
            return CALLBACK_BREAK;
        }
        else if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CDuelMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, TRUE);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CDuelMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, FALSE);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_CONTINUE;
}

mu::ui::window::CDuelResultMsgBox::CDuelResultMsgBox()
{
    m_szWinnerID[0] = '\0';
    m_szLoserID[0] = '\0';
}

mu::ui::window::CDuelResultMsgBox::~CDuelResultMsgBox()
{
}

bool mu::ui::window::CDuelResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 70;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT * 7 + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CDuelResultMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CDuelResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CDuelResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
    AddCallbackFunc(mu::ui::window::CDuelResultMsgBox::OkBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void mu::ui::window::CDuelResultMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;

    x = GetPos().x + (GetSize().cx / 2) - MSGBOX_BTN_WIDTH / 2;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CDuelResultMsgBox::Release()
{
}

bool mu::ui::window::CDuelResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool mu::ui::window::CDuelResultMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButton();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CDuelResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x + (GetSize().cx / 2) - 74;
    y = GetPos().y + 15;
    width = 148;
    height = 138;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_DUEL_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 7; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CDuelResultMsgBox::RenderTexts()
{
    wchar_t strDuelID[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 100, I18N::Game::DuelFinished, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(strDuelID, I18N::Game::SHasJustWon, m_szWinnerID);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 120, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    mu_swprintf(strDuelID, I18N::Game::TheDuelWithS, m_szLoserID);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 135, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 151, I18N::Game::Lookup(2697), MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CDuelResultMsgBox::RenderButton()
{
    m_BtnOk.Render();
}

CALLBACK_RESULT mu::ui::window::CDuelResultMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDuelResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CDuelResultMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    // 	SendRequestDuelOk(1, g_iDuelPlayerIndex, g_szDuelPlayerID);

    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_CONTINUE;
}

void CDuelResultMsgBox::SetIDs(wchar_t* pszWinnerID, wchar_t* pszLoserID)
{
    wcsncpy(m_szWinnerID, pszWinnerID, MAX_USERNAME_SIZE);
    m_szWinnerID[MAX_USERNAME_SIZE] = '\0';
    wcsncpy(m_szLoserID, pszLoserID, MAX_USERNAME_SIZE);
    m_szLoserID[MAX_USERNAME_SIZE] = '\0';
}

CCherryBlossomMsgBox::CCherryBlossomMsgBox()
{
    m_iMiddleCount = 8;
}

CCherryBlossomMsgBox::~CCherryBlossomMsgBox()
{
    Release();
}

bool CCherryBlossomMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void CCherryBlossomMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool CCherryBlossomMsgBox::Update()
{
    m_BtnWhiteCB.Update();
    m_BtnRedCB.Update();
    m_BtnGoldCB.Update();
    m_BtnExit.Update();

    return true;
}

bool CCherryBlossomMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT CCherryBlossomMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCherryBlossomMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnWhiteCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_WHITE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRedCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_RED);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnGoldCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_GOLD);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::WhiteCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::RedCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::GodCBBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    //	SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_CONTINUE;
}

void CCherryBlossomMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CCherryBlossomMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CCherryBlossomMsgBox::WhiteCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_WHITE);
    AddCallbackFunc(mu::ui::window::CCherryBlossomMsgBox::RedCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_RED);
    AddCallbackFunc(mu::ui::window::CCherryBlossomMsgBox::GodCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_GOLD);
    AddCallbackFunc(mu::ui::window::CCherryBlossomMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void CCherryBlossomMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 50;
    m_BtnWhiteCB.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnWhiteCB.SetText(I18N::Game::Lookup(2542));

    y = GetPos().y + 100;
    m_BtnRedCB.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRedCB.SetText(I18N::Game::Lookup(2543));

    y = GetPos().y + 150;
    m_BtnGoldCB.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGoldCB.SetText(I18N::Game::GoldenCherryBlossomsBranches);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    // 1002 "닫기"
    m_BtnExit.SetText(I18N::Game::Close388);
}

void CCherryBlossomMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void CCherryBlossomMsgBox::RenderTexts()
{
    wchar_t title[256];

    MONSTER_SCRIPT* m = &MonsterScript[450];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(title, L"%ls", m->Name);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 10, title, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    wchar_t titleinfo[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(titleinfo, L"%ls", I18N::Game::GoldenCherryBlossomsBranches);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 70, titleinfo, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void CCherryBlossomMsgBox::RenderButtons()
{
    m_BtnWhiteCB.Render();
    m_BtnRedCB.Render();
    m_BtnGoldCB.Render();
    m_BtnExit.Render();
}

bool mu::ui::window::CUseFruitCheckMsgBoxLayout::SetLayout()
{
    CUseFruitCheckMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CGemIntegrationMsgBoxLayout::SetLayout()
{
    CGemIntegrationMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CGemIntegrationUnityMsgBoxLayout::SetLayout()
{
    CGemIntegrationUnityMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CGemIntegrationDisjointMsgBoxLayout::SetLayout()
{
    CGemIntegrationDisjointMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CBloodCastleResultMsgBoxLayout::SetLayout()
{
    CBloodCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CDevilSquareRankMsgBoxLayout::SetLayout()
{
    CDevilSquareRankMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CChaosCastleResultMsgBoxLayout::SetLayout()
{
    CChaosCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CChaosMixMenuMsgBoxLayout::SetLayout()
{
    CChaosMixMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CCrownSwitchPopLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::CrownSwitchHasBeenReleased);

    return true;
}

bool mu::ui::window::CCrownSwitchPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::CrownSwitchHasBeenActivated);

    return true;
}

bool mu::ui::window::CCrownSwitchOtherPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterStartLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterSuccessLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::OfficialSealRegistrationIsSuccessful);

    return true;
}

bool mu::ui::window::CSealRegisterFailLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterOtherLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::AnotherCharacterIsRegisteringTheOfficialSeal);

    return true;
}

bool mu::ui::window::CSealRegisterOtherCampLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::OtherSiegeTeamIsRunningTheCrownSwitch);

    return true;
}

bool mu::ui::window::CCrownDefenseRemoveLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::ShieldOfTheCrownHasBeenRemoved);

    return true;
}

bool mu::ui::window::CCrownDefenseCreateLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::ShieldOfTheCrownHasBeenActivated);

    return true;
}

bool mu::ui::window::CCursedTempleHolicItemGetLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouAreCurrentGainingTheSacredItem);

    return true;
}

bool mu::ui::window::CCursedTempleHolicItemSaveLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouAreCurrentlyStoringTheSacredItem);

    return true;
}

bool mu::ui::window::CTrainerMenuMsgBoxLayout::SetLayout()
{
    CTrainerMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CTrainerRecoverMsgBoxLayout::SetLayout()
{
    CTrainerRecoverMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CElpisMsgBoxLayout::SetLayout()
{
    CElpisMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CSystemMenuMsgBoxLayout::SetLayout()
{
    CSystemMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CDuelMsgBoxLayout::SetLayout()
{
    CDuelMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CDuelResultMsgBoxLayout::SetLayout()
{
    CDuelResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool CCherryBlossomMsgBoxLayout::SetLayout()
{
    CCherryBlossomMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CSeedMasterMenuMsgBoxLayout::SetLayout()
{
    CSeedMasterMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CSeedInvestigatorMenuMsgBoxLayout::SetLayout()
{
    CSeedInvestigatorMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CResetCharacterPointMsgBoxLayout::SetLayout()
{
    CResetCharacterPointMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CDelgardoMainMenuMsgBoxLayout::SetLayout()
{
    CDelgardoMainMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CLuckyTradeMenuMsgBoxLayout::SetLayout()
{
    CLuckyTradeMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

mu::ui::window::CLuckyTradeMenuMsgBox::CLuckyTradeMenuMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CLuckyTradeMenuMsgBox::~CLuckyTradeMenuMsgBox()
{
    Release();
}

void mu::ui::window::CLuckyTradeMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CLuckyTradeMenuMsgBox::Update()
{
    m_BtnTrade.Update();
    m_BtnRefinery.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CLuckyTradeMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

bool mu::ui::window::CLuckyTradeMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CLuckyTradeMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    CLuckyTradeMenuMsgBox* pMsgBox = dynamic_cast<CLuckyTradeMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnTrade.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_TRADE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRefinery.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_REFINERY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CLuckyTradeMenuMsgBox::LuckyItemTradeBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pLuckyItemWnd->SetAct(eLuckyItemType_Trade);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYITEMWND);
    //g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CLuckyTradeMenuMsgBox::LuckyItemRefineryBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pLuckyItemWnd->SetAct(eLuckyItemType_Refinery);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYITEMWND);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CLuckyTradeMenuMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CLuckyTradeMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CLuckyTradeMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CLuckyTradeMenuMsgBox::LuckyItemTradeBtnDown, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_TRADE);
    AddCallbackFunc(mu::ui::window::CLuckyTradeMenuMsgBox::LuckyItemRefineryBtnDown, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_REFINERY);
    AddCallbackFunc(mu::ui::window::CLuckyTradeMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CLuckyTradeMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnTrade.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnTrade.SetText(L"럭키아이템 교환");	// "GlobalText"

    y = GetPos().y + 120;
    m_BtnRefinery.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRefinery.SetText(L"럭키아이템 제련");	// "GlobalText"

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CLuckyTradeMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CLuckyTradeMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, L"럭키아이템 교환NPC");	// "LuckyItem Trade NPC"
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, L"럭키아이템으로 교환하거나 제련할 수 있습니?");
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CLuckyTradeMenuMsgBox::RenderButtons()
{
    m_BtnTrade.Render();
    m_BtnRefinery.Render();
    m_BtnExit.Render();
}

//////////////////////////////////////////////////////////////////////////

mu::ui::window::CTrainerMenuMsgBox::CTrainerMenuMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CTrainerMenuMsgBox::~CTrainerMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CTrainerMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CTrainerMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CTrainerMenuMsgBox::Update()
{
    m_BtnRecover.Update();
    m_BtnRevive.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CTrainerMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CTrainerMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CTrainerMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnRecover.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRevive.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_REVIVE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CTrainerMenuMsgBox::RecoverBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CTrainerRecoverMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTrainerMenuMsgBox::ReviveBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_TRAINER);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTrainerMenuMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CTrainerMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CTrainerMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CTrainerMenuMsgBox::RecoverBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER);
    AddCallbackFunc(mu::ui::window::CTrainerMenuMsgBox::ReviveBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_REVIVE);
    AddCallbackFunc(mu::ui::window::CTrainerMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CTrainerMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnRecover.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecover.SetText(I18N::Game::RestoreLifeDurability);

    y = GetPos().y + 120;
    m_BtnRevive.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRevive.SetText(I18N::Game::ResurrectSpirit);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CTrainerMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CTrainerMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::Trainer);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::Hi);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::SWhatIsYourCommand, Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CTrainerMenuMsgBox::RenderButtons()
{
    m_BtnRecover.Render();
    m_BtnRevive.Render();
    m_BtnExit.Render();
}

mu::ui::window::CTrainerRecoverMsgBox::CTrainerRecoverMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CTrainerRecoverMsgBox::~CTrainerRecoverMsgBox()
{
    Release();
}

bool mu::ui::window::CTrainerRecoverMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CTrainerRecoverMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CTrainerRecoverMsgBox::Update()
{
    m_BtnRecoverDarkSpirit.Update();
    m_BtnRecoverDarkHorse.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CTrainerRecoverMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CTrainerRecoverMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CTrainerRecoverMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnRecoverDarkSpirit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKSPRIT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRecoverDarkHorse.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKHORSE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CTrainerRecoverMsgBox::RecoverDarkSpiritrBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    npcBreeder::RecoverPet(REVIVAL_DARKSPIRIT);
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTrainerRecoverMsgBox::RecoverDarkHorseBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    npcBreeder::RecoverPet(REVIVAL_DARKHORSE);
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTrainerRecoverMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CTrainerRecoverMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CTrainerRecoverMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CTrainerRecoverMsgBox::RecoverDarkSpiritrBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKSPRIT);
    AddCallbackFunc(mu::ui::window::CTrainerRecoverMsgBox::RecoverDarkHorseBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKHORSE);
    AddCallbackFunc(mu::ui::window::CTrainerRecoverMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CTrainerRecoverMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;

    y = GetPos().y + 65;
    m_BtnRecoverDarkHorse.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecoverDarkHorse.SetText(I18N::Game::DarkHorse);

    y = GetPos().y + 115;
    m_BtnRecoverDarkSpirit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecoverDarkSpirit.SetText(I18N::Game::DarkRaven);

    btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CTrainerRecoverMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CTrainerRecoverMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::Trainer);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::SelectThePetToRecoverLife);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(206, 192, 146, 255);
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKHORSE, szText);
    g_pRenderText->RenderText(fPos_x, fPos_y + 75, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKSPIRIT, szText);
    g_pRenderText->RenderText(fPos_x, fPos_y + 125, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CTrainerRecoverMsgBox::RenderButtons()
{
    m_BtnRecoverDarkSpirit.Render();
    m_BtnRecoverDarkHorse.Render();
    m_BtnExit.Render();
}

mu::ui::window::CElpisMsgBox::CElpisMsgBox()
{
    m_iMiddleCount = 12;
    m_iMessageType = 0;
}

mu::ui::window::CElpisMsgBox::~CElpisMsgBox()
{
    Release();
}

bool mu::ui::window::CElpisMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CElpisMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CElpisMsgBox::Update()
{
    m_BtnAboutRefinary.Update();
    m_BtnAboutJewelOfHarmony.Update();
    m_BtnRefine.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CElpisMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CElpisMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAboutRefinary.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnAboutJewelOfHarmony.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRefine.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_REFINE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CElpisMsgBox::AboutRefinaryBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        pMsgBox->SetMessageType(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
    }

    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CElpisMsgBox::AboutJewelOfHarmonyBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        pMsgBox->SetMessageType(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
    }

    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CElpisMsgBox::RefineBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ELPIS);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CElpisMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CElpisMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CElpisMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CElpisMsgBox::AboutRefinaryBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
    AddCallbackFunc(mu::ui::window::CElpisMsgBox::AboutJewelOfHarmonyBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
    AddCallbackFunc(mu::ui::window::CElpisMsgBox::RefineBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_REFINE);
    AddCallbackFunc(mu::ui::window::CElpisMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CElpisMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 145;
    m_BtnAboutRefinary.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAboutRefinary.SetText(I18N::Game::AboutRefinery);

    y = GetPos().y + 175;
    m_BtnAboutJewelOfHarmony.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAboutJewelOfHarmony.SetText(I18N::Game::JewelOfHarmony);

    y = GetPos().y + 205;
    m_BtnRefine.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRefine.SetText(I18N::Game::RefineGemstone);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CElpisMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x; y = GetPos().y + 120, width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
}

void mu::ui::window::CElpisMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::Elpis);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetTextColor(220, 183, 131, 255);	// 황금색

    switch (m_iMessageType)
    {
    case 0:
        mu_swprintf(szText, I18N::Game::WhatWouldYouLikeToKnow);
        g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
        break;
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY:
    {
        wchar_t Textlist[7][100];
        int lineSize = 0;
        lineSize = CutText3(I18N::Game::GemstoneOfJewelOfHarmonyHas, Textlist[0], MSGBOX_WIDTH - 60.0f, 7, 100);
        for (int i = 0; i < lineSize; ++i)
        {
            g_pRenderText->RenderText(fPos_x + 20, fPos_y + (i + 1) * 18, Textlist[i]);
        }
    }
    break;
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY:
    {
        wchar_t Textlist[7][100];
        int lineSize = 0;
        lineSize = CutText3(I18N::Game::NewPowerCanBeGrantedTo, Textlist[0], MSGBOX_WIDTH - 60.0f, 7, 100);
        for (int i = 0; i < lineSize; ++i)
        {
            g_pRenderText->RenderText(fPos_x + 20, fPos_y + (i + 1) * 18, Textlist[i]);
        }
    }
    break;
    }
}

void mu::ui::window::CElpisMsgBox::RenderButtons()
{
    m_BtnAboutRefinary.Render();
    m_BtnAboutJewelOfHarmony.Render();
    m_BtnRefine.Render();
    m_BtnExit.Render();
}

mu::ui::window::CSeedMasterMenuMsgBox::CSeedMasterMenuMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CSeedMasterMenuMsgBox::~CSeedMasterMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CSeedMasterMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CSeedMasterMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CSeedMasterMenuMsgBox::Update()
{
    m_BtnExtractSeed.Update();
    m_BtnSeedSphere.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CSeedMasterMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CSeedMasterMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSeedMasterMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnExtractSeed.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_EXTRACT_SEED);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSeedSphere.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_SEED_SPHERE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CSeedMasterMenuMsgBox::ExtractSeedBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_EXTRACT_SEED);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSeedMasterMenuMsgBox::SeedSphereBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_SEED_SPHERE);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSeedMasterMenuMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CSeedMasterMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CSeedMasterMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CSeedMasterMenuMsgBox::ExtractSeedBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_EXTRACT_SEED);
    AddCallbackFunc(mu::ui::window::CSeedMasterMenuMsgBox::SeedSphereBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_SEED_SPHERE);
    AddCallbackFunc(mu::ui::window::CSeedMasterMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CSeedMasterMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnExtractSeed.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnExtractSeed.SetText(I18N::Game::SeedExtraction);

    y = GetPos().y + 120;
    m_BtnSeedSphere.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnSeedSphere.SetText(I18N::Game::SeedSphereAssembly);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CSeedMasterMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CSeedMasterMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::SeedMaster);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::ExtractTheSeedOrTheSeedSphere);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::YouMayAssemblyThemTogether);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CSeedMasterMenuMsgBox::RenderButtons()
{
    m_BtnExtractSeed.Render();
    m_BtnSeedSphere.Render();
    m_BtnExit.Render();
}

mu::ui::window::CSeedInvestigatorMenuMsgBox::CSeedInvestigatorMenuMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CSeedInvestigatorMenuMsgBox::~CSeedInvestigatorMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CSeedInvestigatorMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CSeedInvestigatorMenuMsgBox::Update()
{
    m_BtnAttachSocket.Update();
    m_BtnDetachSocket.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CSeedInvestigatorMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CSeedInvestigatorMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSeedInvestigatorMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAttachSocket.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_ATTACH_SOCKET);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDetachSocket.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_DETACH_SOCKET);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CSeedInvestigatorMenuMsgBox::AttachSocketBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ATTACH_SOCKET);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSeedInvestigatorMenuMsgBox::DetachSocketBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_DETACH_SOCKET);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSeedInvestigatorMenuMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CSeedInvestigatorMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CSeedInvestigatorMenuMsgBox::AttachSocketBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_ATTACH_SOCKET);
    AddCallbackFunc(mu::ui::window::CSeedInvestigatorMenuMsgBox::DetachSocketBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_DETACH_SOCKET);
    AddCallbackFunc(mu::ui::window::CSeedInvestigatorMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnAttachSocket.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAttachSocket.SetText(I18N::Game::SeedSphereApplication);

    y = GetPos().y + 120;
    m_BtnDetachSocket.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnDetachSocket.SetText(I18N::Game::SeedSphereDestruction);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::SeedResearcher);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::EitherApplyTheSeedSphere);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::OrDestroyTheSeedSphereAccordingly);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CSeedInvestigatorMenuMsgBox::RenderButtons()
{
    m_BtnAttachSocket.Render();
    m_BtnDetachSocket.Render();
    m_BtnExit.Render();
}

mu::ui::window::CResetCharacterPointMsgBox::CResetCharacterPointMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CResetCharacterPointMsgBox::~CResetCharacterPointMsgBox()
{
    Release();
}

bool mu::ui::window::CResetCharacterPointMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x = 0, y = 0, width = 0, height = 0;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();
    return true;
}

void mu::ui::window::CResetCharacterPointMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 105;
    m_ResetCharacterPointBtn.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_ResetCharacterPointBtn.SetText(I18N::Game::StatReInitialization); // "스탯 초기화"

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CResetCharacterPointMsgBox::Release()
{
    CMessageBoxBase::Release();
}

void mu::ui::window::CResetCharacterPointMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CResetCharacterPointMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CResetCharacterPointMsgBox::ResetCharacterPointBtnDown, MSGBOX_EVENT_USER_CUSTOM_RESET_CHARACTER_POINT);
    AddCallbackFunc(mu::ui::window::CResetCharacterPointMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT mu::ui::window::CResetCharacterPointMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CResetCharacterPointMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_ResetCharacterPointBtn.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_RESET_CHARACTER_POINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

bool mu::ui::window::CResetCharacterPointMsgBox::Update()
{
    m_ResetCharacterPointBtn.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CResetCharacterPointMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CResetCharacterPointMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CResetCharacterPointMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::ReInitializationHelper);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 25;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::ClickOnTheButtonToReinitializeAllStatPoints);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CResetCharacterPointMsgBox::RenderButtons()
{
    m_ResetCharacterPointBtn.Render();
    m_BtnExit.Render();
}

bool mu::ui::window::CResetCharacterPointMsgBox::isCharacterEquipmentItem()
{
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (CharacterMachine->Equipment[i].Type != -1) {
            return true;
        }
    }
    return false;
}

CALLBACK_RESULT mu::ui::window::CResetCharacterPointMsgBox::ResetCharacterPointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);

    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (CharacterMachine->Equipment[i].Type != -1)
        {
            g_pSystemLogBox->AddText(I18N::Game::TheAppliedEquipmentsCannotBeReset, mu::ui::window::TYPE_ERROR_MESSAGE);
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            return CALLBACK_BREAK;
        }
    }

    SocketClient->ToGameServer()->SendResetCharacterPointRequest();

    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CResetCharacterPointMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

mu::ui::window::CGuild_ToPerson_Position::CGuild_ToPerson_Position()
{
    COMGEM::m_cGemType = COMGEM::CELE;
    AppointType = SUBGUILDMASTER;
}

mu::ui::window::CGuild_ToPerson_Position::~CGuild_ToPerson_Position()
{
    Release();
}

bool mu::ui::window::CGuild_ToPerson_Position::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void mu::ui::window::CGuild_ToPerson_Position::Release()
{
    CMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool mu::ui::window::CGuild_ToPerson_Position::Update()
{
    m_BtnBlessing.Update();
    m_BtnSoul.Update();
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool mu::ui::window::CGuild_ToPerson_Position::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CGuild_ToPerson_Position::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CGuild_ToPerson_Position::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CGuild_ToPerson_Position::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 50;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + 57;//(GetPos().x + (msgboxhalfwidth / 2) - btnhalfwidth) + 60;
    y = GetPos().y + 30;
    m_BtnBlessing.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnBlessing.SetText(I18N::Game::AppointAsAssistantGuildMaster);

    y += 27;
    m_BtnSoul.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnSoul.SetText(I18N::Game::AppointAsABattleMaster);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x -= 9;
    y += 70;
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnOk.SetText(I18N::Game::OK);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x += 64;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Close388);
}

void mu::ui::window::CGuild_ToPerson_Position::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x;
    y = GetPos().y + 2.f;
    width = (GetSize().cx - MSGBOX_BACK_BLANK_WIDTH);
    height = (GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT) - 75;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    int iCount = 5;
    for (int i = 0; i < iCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CGuild_ToPerson_Position::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = (GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2)) + 80;

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        const SIZE TextSize = g_pRenderText->MeasureText(
            (*vi)->strMsg.c_str(), static_cast<int>((*vi)->strMsg.size()));
        const size_t TextExtentWidth = static_cast<size_t>(TextSize.cx);
        const size_t TextExtentHeight = static_cast<size_t>(TextSize.cy);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void mu::ui::window::CGuild_ToPerson_Position::RenderButtons()
{
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();

    wchar_t strText[256];
    if (COMGEM::m_cGemType == COMGEM::CELE)
    {
        mu_swprintf(strText, I18N::Game::SAsAS, GuildList[DeleteIndex].Name, I18N::Game::AssistM);
        AppointType = SUBGUILDMASTER;
        AddMsg(strText, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        m_BtnBlessing.Render();
    }
    else
    {
        m_BtnBlessing.Render();
    }

    if (COMGEM::m_cGemType == COMGEM::SOUL)
    {
        mu_swprintf(strText, I18N::Game::SAsAS, GuildList[DeleteIndex].Name, I18N::Game::BattleM);
        AppointType = BATTLEMASTER;
        AddMsg(strText, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        m_BtnSoul.Render();
    }
    else
    {
        m_BtnSoul.Render();
    }

    m_BtnOk.Render();
    m_BtnCancel.Render();
    AddMsg(I18N::Game::DoYouWantToAppoint, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGuild_ToPerson_Position*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnBlessing.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSoul.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::CELE);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::SOUL);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();
    SocketClient->ToGameServer()->SendGuildRoleAssignRequest(
        AppointType,
        MU_C16(GuildList[DeleteIndex].Name),
        AppointType == G_PERSON ? 0x01 : 0x02);

    SocketClient->ToGameServer()->SendGuildListRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuild_ToPerson_PositionLayout::SetLayout()
{
    CGuild_ToPerson_Position* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

mu::ui::window::CDelgardoMainMenuMsgBox::CDelgardoMainMenuMsgBox()
{
    m_iMiddleCount = 7;
}

mu::ui::window::CDelgardoMainMenuMsgBox::~CDelgardoMainMenuMsgBox()
{
    Release();
}

bool mu::ui::window::CDelgardoMainMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void mu::ui::window::CDelgardoMainMenuMsgBox::Release()
{
    CMessageBoxBase::Release();
}

bool mu::ui::window::CDelgardoMainMenuMsgBox::Update()
{
    m_BtnReg.Update();
    m_BtnExchange.Update();
    m_BtnExit.Update();

    return true;
}

bool mu::ui::window::CDelgardoMainMenuMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT mu::ui::window::CDelgardoMainMenuMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDelgardoMainMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnReg.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_REGISTRATION_LUCKY_COIN);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExchange.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_EXCHANGE_LUCKY_COIN);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CDelgardoMainMenuMsgBox::RegBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CDelgardoMainMenuMsgBox::ExchangeBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    g_pNewUISystem->Show(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CDelgardoMainMenuMsgBox::ExitBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CDelgardoMainMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CDelgardoMainMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(mu::ui::window::CDelgardoMainMenuMsgBox::RegBtnDown, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_REGISTRATION_LUCKY_COIN);
    AddCallbackFunc(mu::ui::window::CDelgardoMainMenuMsgBox::ExchangeBtnDown, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_EXCHANGE_LUCKY_COIN);
    AddCallbackFunc(mu::ui::window::CDelgardoMainMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void mu::ui::window::CDelgardoMainMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnReg.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnReg.SetText(I18N::Game::LuckyCoinRegistration);

    y = GetPos().y + 120;
    m_BtnExchange.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnExchange.SetText(I18N::Game::LuckyCoinExchange);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(I18N::Game::Close388);
}

void mu::ui::window::CDelgardoMainMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CDelgardoMainMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    mu_swprintf(szText, I18N::Game::Delgado);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 26;
    g_pRenderText->SetFont(g_hFont);
    mu_swprintf(szText, I18N::Game::RegisterYourLuckyCoinsOr);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::UseTheLuckyCoinsYouAlreadyHave);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    mu_swprintf(szText, I18N::Game::AndExchangeThemForItems);
    g_pRenderText->RenderText(fPos_x, fPos_y + 3 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CDelgardoMainMenuMsgBox::RenderButtons()
{
    m_BtnReg.Render();
    m_BtnExchange.Render();
    m_BtnExit.Render();
}
