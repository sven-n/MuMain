// NewUICommonMessageBox.cpp: implementation of the NewUICommonMessageBox class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "UI/Dialogs/NewUICustomMessageBox.h"
#include "Guild/NewUIGuildMakeWindow.h"
#include "Guild/NewUIGuildInfoWindow.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/UIControls.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Guild/UIGuildInfo.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Quests/CSQuest.h"
#include "World/MapInfra/PortalMgr.h"
#include "GameLogic/Social/GambleSystem.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Inventory/NewUILuckyItemWnd.h"
#include "UI/Core/NewUISystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInterface.h"
#include "I18N/All.h"
#include "Core/Text/TextLineWrap.h"

using namespace SEASON3B;
using namespace mu::ui::window;

extern int DoBreakUpGuildAction_New(POPUP_RESULT Result);


extern int DeleteGuildIndex;
extern wchar_t DeleteID[];
extern int DeleteIndex, AppointStatus;
extern int s_nTargetFireMemberIndex;
extern int Button_Down;
extern int BackUp_Key;
extern BYTE m_AltarState[];

extern BYTE Rank;
extern int Exp;
extern BYTE Ranking[5];
extern CLASS_TYPE HeroClass[5];
extern int HeroScore[5];
extern wchar_t HeroName[5][MAX_USERNAME_SIZE + 1];
extern char	View_Suc_Or_Fail;

extern int BuyCost;

namespace
{
void SelectMessageFont(BYTE fontType)
{
    g_pRenderText->SetFont(fontType == MSGBOX_FONT_BOLD ? g_hFontBold : g_hFont);
}

int AppendWrappedText(type_vector_msgdata& messages, const type_string& text, DWORD color, BYTE fontType,
                      int maxWidth)
{
    SelectMessageFont(fontType);
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
        auto* message = new MSGBOX_TEXTDATA;
        message->strMsg = line;
        message->dwColor = color;
        message->byFontType = fontType;
        messages.push_back(message);
    }
    return static_cast<int>(lines.size());
}
} // namespace

mu::ui::window::CNewUIMessageBoxButton::CNewUIMessageBoxButton()
{
    m_bEnable = true;

    m_dwTexType = 0;
    m_x = m_y = m_width = m_height = 0.f;

    m_EventState = EVENT_NONE;
    m_dwSizeType = MSGBOX_BTN_SIZE_OK;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_iMoveTextPosX = 0;
    m_iMoveTextPosY = 0;
    m_bClickEffect = false;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

mu::ui::window::CNewUIMessageBoxButton::~CNewUIMessageBoxButton()
{
}

bool mu::ui::window::CNewUIMessageBoxButton::IsMouseIn()
{
    if (m_bEnable == false)
        return false;

    return mu::ui::window::CheckMouseIn(m_x, m_y, m_width, m_height);
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CNewUIMessageBoxButton::MoveTextPos(int iX, int iY)
{
    m_iMoveTextPosX = iX;
    m_iMoveTextPosY = iY;
}

void mu::ui::window::CNewUIMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType, bool bClickEffect)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CNewUIMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
{
    m_dwTexType = dwTexType;
    m_dwSizeType = dwSizeType;
    m_x = x; m_y = y;
    m_width = width; m_height = height;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_bClickEffect = bClickEffect;

    if (m_dwSizeType == MSGBOX_BTN_SIZE_OK)
    {
        m_fButtonWidth = MSGBOX_BTN_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_SMALL)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_BIG)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_BIG_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else
    {
        m_fButtonWidth = width; m_fButtonHeight = height;
    }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CNewUIMessageBoxButton::SetText(const wchar_t* strText)
{
    if (wcslen(strText) > 0)
    {
        m_strText = strText;
    }
}

void mu::ui::window::CNewUIMessageBoxButton::AddBlank(int iAddBlank)
{
    m_y += iAddBlank;
}

void mu::ui::window::CNewUIMessageBoxButton::Update()
{
    if (m_bEnable == false)
        return;

    if (m_EventState == EVENT_NONE && MouseLButtonPush == false && IsMouseIn() == true)
    {
        m_EventState = EVENT_BTN_HOVER;
        return;
    }
    if (m_EventState == EVENT_BTN_HOVER && MouseLButtonPush == false && IsMouseIn() == false)
    {
        m_EventState = EVENT_NONE;
        return;
    }
    if (m_EventState == EVENT_BTN_HOVER)
    {
        if (MouseLButtonPush == true)
        {
            if (IsMouseIn() == true)
            {
                m_EventState = EVENT_BTN_DOWN;
                return;
            }
        }
    }
    if (m_EventState == EVENT_BTN_DOWN && MouseLButtonPush == false)
    {
        m_EventState = EVENT_NONE;
        return;
    }
}

void mu::ui::window::CNewUIMessageBoxButton::Render()
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    const DWORD buttonColor = m_bEnable
        ? RGBA(255, 255, 255, 255)
        : RGBA(204, 204, 204, 230);
    BITMAP_t* pImage = &Bitmaps[m_dwTexType];
    RenderColorBitmap(m_dwTexType, m_x, m_y, m_width, m_height,
        (0.5f / (float)pImage->Width), ((static_cast<float>(m_EventState) * m_fButtonHeight + 0.5f) / (float)pImage->Height),
        (m_fButtonWidth - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width),
        (m_fButtonHeight - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height), buttonColor);
#else //KJH_ADD_INGAMESHOP_UI_SYSTEM
    float fv = 0.f;
    float fBtnOrigWidth = 0.f;
    float fBtnOrigHeight = 0.f;

    if (m_dwSizeType == MSGBOX_BTN_SIZE_OK)
    {
        fBtnOrigWidth = MSGBOX_BTN_WIDTH;
        fBtnOrigHeight = MSGBOX_BTN_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY)
    {
        fBtnOrigWidth = MSGBOX_BTN_EMPTY_WIDTH;
        fBtnOrigHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_SMALL)
    {
        fBtnOrigWidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
        fBtnOrigHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_BIG)
    {
        fBtnOrigWidth = MSGBOX_BTN_EMPTY_BIG_WIDTH;
        fBtnOrigHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    if (m_EventState == EVENT_BTN_HOVER)
    {
        fv = m_height * 1.f / 128.f;
    }
    else if (m_EventState == EVENT_BTN_DOWN)
    {
        fv = m_height * 2.f / 128.f;
    }
    else
    {
        fv = 0.f;
    }

    const DWORD buttonColor = m_bEnable
        ? RGBA(255, 255, 255, 255)
        : RGBA(153, 153, 153, 153);

    if (m_dwSizeType == MSGBOX_BTN_SIZE_OK || m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_SMALL)
    {
        RenderColorBitmap(m_dwTexType, m_x, m_y, m_width, m_height, 0.f, fv,
            fBtnOrigWidth / 64.f, fBtnOrigHeight / 128.f, buttonColor);
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY)
    {
        RenderColorBitmap(m_dwTexType, m_x, m_y, m_width, m_height, 0.f, fv,
            fBtnOrigWidth / 128.f, fBtnOrigHeight / 128.f, buttonColor);
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_BIG)
    {
        RenderColorBitmap(m_dwTexType, m_x, m_y, m_width, m_height, 0.f, fv,
            fBtnOrigWidth / 256.f, fBtnOrigHeight / 128.f, buttonColor);
    }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    if (m_strText.size() > 0)
    {
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        g_pRenderText->SetFont(g_hFont);

        if (m_bEnable == false)
        {
            g_pRenderText->SetTextColor(128, 128, 128, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }

        g_pRenderText->SetBgColor(0);

        const SIZE Fontsize = g_pRenderText->MeasureText(
            m_strText.c_str(), static_cast<int>(m_strText.size()));

        int x = m_x + ((m_width / 2) - (Fontsize.cx / 2));
        int y = m_y + ((m_height / 2) - (Fontsize.cy / 2));

        if ((m_bClickEffect == true) && (m_EventState == EVENT_BTN_DOWN))
        {
            g_pRenderText->RenderText(x + m_iMoveTextPosX + 1, y + m_iMoveTextPosY + 1, m_strText.c_str());
        }
        else
        {
            g_pRenderText->RenderText(x + m_iMoveTextPosX, y + m_iMoveTextPosY, m_strText.c_str());
        }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        int x;
        size_t TextExtentWidth;

        g_pRenderText->SetFont(g_hFont);
        const SIZE TextSize = g_pRenderText->MeasureText(
            m_strText.c_str(), static_cast<int>(m_strText.size()));
        TextExtentWidth = static_cast<size_t>(TextSize.cx);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0);

        x = m_x + (m_width / 2) - (TextExtentWidth / 2);

        g_pRenderText->RenderText(x, m_y + 10, m_strText.c_str());
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    }
}

mu::ui::window::CNewUICommonMessageBox::CNewUICommonMessageBox()
{
}

mu::ui::window::CNewUICommonMessageBox::~CNewUICommonMessageBox()
{
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

DWORD mu::ui::window::CNewUICommonMessageBox::GetType()
{
    return m_dwType;
}

bool mu::ui::window::CNewUICommonMessageBox::Create(DWORD dwType, float fPriority)
{
    m_dwType = dwType;

    SetAddCallbackFunc();

    const int x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    const int y = 100;
    const int width = MSGBOX_WIDTH;
    const int height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    if (!CNewUIMessageBoxBase::Create(x, y, width, height, fPriority))
        return false;

    SetButtonInfo();

    return true;
}

bool mu::ui::window::CNewUICommonMessageBox::Create(DWORD dwType, const type_string& strMsg, DWORD dwColor, BYTE byFontType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    AddMsg(strMsg, dwColor, byFontType);

    return true;
}

void mu::ui::window::CNewUICommonMessageBox::SetPos(int x, int y)
{
    CNewUIMessageBoxBase::SetPos(x, y);

    float fx, fy;
    fx = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    fy = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnOk.SetPos(fx, fy);
    fx = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    fy = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnCancel.SetPos(fx, fy);
}

void mu::ui::window::CNewUICommonMessageBox::SetAddCallbackFunc()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    }
}

void mu::ui::window::CNewUICommonMessageBox::SetButtonInfo()
{
    float x, y, width, height;

    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    }
}

void mu::ui::window::CNewUICommonMessageBox::AddButtonBlank(int iAddLine)
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        m_BtnCancel.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    }
}

void mu::ui::window::CNewUICommonMessageBox::AddButtonBlank(int iAddLine, int _iImgSize)
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.AddBlank(iAddLine * _iImgSize);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.AddBlank(iAddLine * _iImgSize);
        m_BtnCancel.AddBlank(iAddLine * _iImgSize);
        break;
    }
}

void mu::ui::window::CNewUICommonMessageBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
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
        AddButtonBlank(iLine);
    }
}

int mu::ui::window::CNewUICommonMessageBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType, int _TextSize)
{
    return AppendWrappedText(m_MsgDataList, strMsg, dwColor, byFontType, _TextSize);
}

bool mu::ui::window::CNewUICommonMessageBox::Update()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Update();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Update();
        m_BtnCancel.Update();
        break;
    }

    return true;
}

bool mu::ui::window::CNewUICommonMessageBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CNewUICommonMessageBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CNewUICommonMessageBox::RenderTexts()
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

void mu::ui::window::CNewUICommonMessageBox::RenderButtons()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Render();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Render();
        m_BtnCancel.Render();
        break;
    }
}

void mu::ui::window::CNewUICommonMessageBox::LockOkButton()
{
    m_BtnOk.SetEnable(false);
}

CALLBACK_RESULT mu::ui::window::CNewUICommonMessageBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUICommonMessageBox*>(pOwner);
    if (pMsgBox)
    {
        switch (pMsgBox->GetType())
        {
        case MSGBOX_COMMON_TYPE_OK:
            if (pMsgBox->m_BtnOk.IsMouseIn() == true)
            {
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
                return CALLBACK_BREAK;
            }
            break;
        case MSGBOX_COMMON_TYPE_OKCANCEL:
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
            break;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CNewUICommonMessageBox::Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CNewUI3DItemCommonMsgBox::CNewUI3DItemCommonMsgBox()
{
    ZeroMemory(&m_Item, sizeof(m_Item));
    m_iItemValue = 0;
}

mu::ui::window::CNewUI3DItemCommonMsgBox::~CNewUI3DItemCommonMsgBox()
{
    Release();
}

DWORD mu::ui::window::CNewUI3DItemCommonMsgBox::GetType()
{
    return m_dwType;
}

bool mu::ui::window::CNewUI3DItemCommonMsgBox::Create(DWORD dwType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    if (g_pNewUI3DRenderMng)
    {
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
    }

    return true;
}

bool mu::ui::window::CNewUI3DItemCommonMsgBox::Create(DWORD dwType, const type_string& strMsg, DWORD dwColor, BYTE byFontType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;
    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    AddMsg(strMsg, dwColor, byFontType);

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Add3DRenderObj(this);

    return true;
}
void mu::ui::window::CNewUI3DItemCommonMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::Set3DItem(ITEM* pItem)
{
    if (pItem)
    {
        memcpy(&m_Item, pItem, sizeof(ITEM));
    }
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::SetItemValue(int iValue)
{
    m_iItemValue = iValue;
}

int mu::ui::window::CNewUI3DItemCommonMsgBox::GetItemValue()
{
    return m_iItemValue;
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 3)
    {
        float height = GetSize().cy;

        if (iOrigSize < 3)
        {
            iLine = iLine + iOrigSize - 3;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
        AddButtonBlank(iLine);
    }
}

CALLBACK_RESULT mu::ui::window::CNewUI3DItemCommonMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUI3DItemCommonMsgBox*>(pOwner);
    if (pMsgBox)
    {
        switch (pMsgBox->GetType())
        {
        case MSGBOX_COMMON_TYPE_OK:
            if (pMsgBox->m_BtnOk.IsMouseIn() == true)
            {
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
                return CALLBACK_BREAK;
            }
            break;
        case MSGBOX_COMMON_TYPE_OKCANCEL:
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
            break;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CNewUI3DItemCommonMsgBox::Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CNewUI3DItemCommonMsgBox::Update()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Update();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Update();
        m_BtnCancel.Update();
        break;
    }

    return true;
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::SetAddCallbackFunc()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CNewUI3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    }
}

int mu::ui::window::CNewUI3DItemCommonMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedText(
        m_MsgDataList, strMsg, dwColor, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH_3DITEM));
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    }
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::AddButtonBlank(int iAddLine)
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        m_BtnCancel.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    }
}

bool mu::ui::window::CNewUI3DItemCommonMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::Render3D()
{
    float x, y, width, height;

    x = GetPos().x + 30;
    y = GetPos().y + 30;
    width = MSGBOX_3DITEM_WIDTH;
    height = MSGBOX_3DITEM_HEIGHT;

    RenderItem3D(x, y, width, height, m_Item.Type, m_Item.Level, m_Item.ExcellentFlags, m_Item.AncientDiscriminator, true);		// PickUp
}

bool mu::ui::window::CNewUI3DItemCommonMsgBox::IsVisible() const
{
    return true;
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
    y += height;
    if (m_MsgDataList.size() > 3)
    {
        int iCount = m_MsgDataList.size() - 3;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::CNewUI3DItemCommonMsgBox::RenderTexts()
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

void mu::ui::window::CNewUI3DItemCommonMsgBox::RenderButtons()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Render();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Render();
        m_BtnCancel.Render();
        break;
    }
}

bool mu::ui::window::CServerLostMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK, 10.f))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouAreDisconnectedFromTheServer);
    pMsgBox->AddCallbackFunc(CServerLostMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->RemoveCallbackFunc(MSGBOX_EVENT_PRESSKEY_ESC);

    return true;
}

CALLBACK_RESULT mu::ui::window::CServerLostMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SendMessage(g_hWnd, WM_DESTROY, 0, 0);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_POP_ALL_EVENTS;
}

bool mu::ui::window::CGuildRequestMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGuildRequestMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildRequestMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGuildRequestMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);

    pMsgBox->AddCallbackFunc(CGuildRequestMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildRequestMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildJoinResponse(true, GuildPlayerKey);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildRequestMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildJoinResponse(false, GuildPlayerKey);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildFireMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGuildFireMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CNewUICommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CGuildFireMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CNewUICommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildFireMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    DeleteGuildIndex = s_nTargetFireMemberIndex;
    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_BREAK;
}

bool mu::ui::window::CMapEnterWerwolfMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->SetPos((SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2), 50);
    pMsgBox->AddMsg(I18N::Game::WerewolfGuardsman, RGBA(254, 176, 72, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::DoYouEvenKnowAboutMe, RGBA(170, 218, 146, 255));
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::IfYouHavePassedThroughThe);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::YouMustBeLocatedCloselyTogether);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::InOrderToReceiveHelpFrom);

    BYTE byQuestState = g_csQuest.getQuestState2(QUEST_3RD_CHANGE_UP_2);
    if (QUEST_ING != byQuestState && QUEST_END != byQuestState)
    {
        pMsgBox->LockOkButton();
    }

    pMsgBox->AddCallbackFunc(CMapEnterWerwolfMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    pMsgBox->AddCallbackFunc(CMapEnterWerwolfMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CMapEnterWerwolfMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    DWORD dwGold = CharacterMachine->Gold;

    if (dwGold >= 3000000)
    {
        SocketClient->ToGameServer()->SendEnterOnWerewolfRequest();
    }
    else
    {
        g_pSystemLogBox->AddText(I18N::Game::YouAreShortOfZen, mu::ui::window::TYPE_ERROR_MESSAGE);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool CMapEnterGateKeeperMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->SetPos((SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2), 50);
    pMsgBox->AddMsg(I18N::Game::Gatekeeper, 0xFF49B0FF, MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::HmmWhoAreYouIMConfusedAreYouEvenApprovedOfBalgass, 0xFF61F191);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::LugadrS12ApostlesAreHelping);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::ApostleDevinSThirdMissionRequest);

    BYTE byQuestState = g_csQuest.getQuestState2(QUEST_3RD_CHANGE_UP_3);
    if (QUEST_ING != byQuestState)
    {
        pMsgBox->LockOkButton();
    }

    pMsgBox->AddCallbackFunc(CMapEnterGateKeeperMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CMapEnterGateKeeperMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT CMapEnterGateKeeperMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendEnterOnGatekeeperRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CPartyMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(CharactersClient[FindCharacterIndex(PartyKey)].ID);
    pMsgBox->AddMsg(I18N::Game::SomeoneRequestsYouToJoinTheirAParty);
    pMsgBox->AddCallbackFunc(CPartyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPartyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CPartyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPartyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CPartyMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendPartyInviteResponse(true, PartyKey);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CPartyMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendPartyInviteResponse(false, PartyKey);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CTradeMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t szYourID[MAX_USERNAME_SIZE + 1];
    g_pTrade->GetYourID(szYourID);
    pMsgBox->AddMsg(szYourID);
    pMsgBox->AddMsg(I18N::Game::WouldLikeToTradeWithYou);
    pMsgBox->AddCallbackFunc(CTradeMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CTradeMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CTradeMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CTradeMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CTradeMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendTradeRequestResponse(true);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTradeMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendTradeRequestResponse(false);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CTradeAlertMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    DWORD adwColor[4] = {
        static_cast<DWORD>(RGBA(255, 178, 0, 255)),
        static_cast<DWORD>(RGBA(255, 178, 0, 255)),
        static_cast<DWORD>(RGBA(255, 178, 0, 255)),
        static_cast<DWORD>(RGBA(255, 32, 32, 255))
    };

    for (int i = 0; i < 4; ++i)
        pMsgBox->AddMsg(I18N::Game::Lookup(371 + i), adwColor[i], MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CTradeAlertMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CTradeAlertMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CTradeAlertMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CTradeAlertMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CTradeAlertMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pTrade->AlertTrade();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CTradeAlertMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildWarMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, 15.f))
        return false;

    wchar_t strText[128];
    mu_swprintf(strText, I18N::Game::SGuildChallengesYou, GuildWarName);
    pMsgBox->AddMsg(strText);
    pMsgBox->AddMsg(I18N::Game::ToAGuildWar);

    pMsgBox->AddCallbackFunc(CGuildWarMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildWarMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CGuildWarMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CGuildWarMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildWarMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildWarResponse(true);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildWarMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildWarResponse(false);
    InitGuildWar();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CBattleSoccerMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, 15.f))
        return false;

    wchar_t strText[128];
    mu_swprintf(strText, I18N::Game::SGuildChallengesYou, GuildWarName);
    pMsgBox->AddMsg(strText);
    pMsgBox->AddMsg(I18N::Game::YouHaveBeenChallengedToBattleSoccer);

    pMsgBox->AddCallbackFunc(CBattleSoccerMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CBattleSoccerMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CBattleSoccerMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CBattleSoccerMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CBattleSoccerMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildWarResponse(true);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CBattleSoccerMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildWarResponse(false);
    InitGuildWar();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CServerImmigrationErrorMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::ThePasswordYouHaveEnteredIsIncorrect);

    pMsgBox->AddCallbackFunc(CServerImmigrationErrorMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CServerImmigrationErrorMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CPersonalshopCreateMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::DoYouWantToOpenAStore);

    pMsgBox->AddCallbackFunc(CPersonalshopCreateMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPersonalshopCreateMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CPersonalshopCreateMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPersonalshopCreateMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CPersonalshopCreateMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    wchar_t shopTitle[MAX_SHOPTITLE]{};
    g_pMyShopInventory->GetTitle(shopTitle);
    wcscpy(g_szPersonalShopTitle, shopTitle);
    SocketClient->ToGameServer()->SendPlayerShopOpen(MU_C16(shopTitle));

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CPersonalshopCreateMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CFenrirRepairMsgBoxLayout::SetLayout()
{
    CFenrirRepairMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToRepairFenrirSHorn, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CFenrirRepairMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CFenrirRepairMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CFenrirRepairMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CFenrirRepairMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CFenrirRepairMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CFenrirRepairMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    SendRequestUse(pMsgBox->GetSourceIndex(), pMsgBox->GetTargetIndex());
    CNewUIInventoryCtrl::BackupPickedItem();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CFenrirRepairMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    CNewUIInventoryCtrl::BackupPickedItem();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CFenrirRepairMsgBox::SetSourceIndex(int iIndex)
{
    m_iSourceIndex = iIndex;
}

void mu::ui::window::CFenrirRepairMsgBox::SetTargetIndex(int iIndex)
{
    m_iTargetIndex = iIndex;
}

int mu::ui::window::CFenrirRepairMsgBox::GetSourceIndex()
{
    return m_iSourceIndex;
}

int mu::ui::window::CFenrirRepairMsgBox::GetTargetIndex()
{
    return m_iTargetIndex;
}

bool mu::ui::window::CInfinityArrowCancelMsgBoxLayout::SetLayout()
{
    extern int g_iCancelSkillTarget;
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t strText[MAX_GLOBAL_TEXT_STRING];
    mu_swprintf(strText, L"%ls%ls", SkillAttribute[AT_SKILL_INFINITY_ARROW].Name, I18N::Game::WouldYouLikeToCancel);
    g_iCancelSkillTarget = AT_SKILL_INFINITY_ARROW; // todo: is considering master skill required here?

    pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CInfinityArrowCancelMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CInfinityArrowCancelMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CInfinityArrowCancelMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CInfinityArrowCancelMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CInfinityArrowCancelMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    extern int g_iCancelSkillTarget;
    SocketClient->ToGameServer()->SendMagicEffectCancelRequest(g_iCancelSkillTarget, HeroKey);
    g_iCancelSkillTarget = 0;

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CInfinityArrowCancelMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    extern int g_iCancelSkillTarget;
    g_iCancelSkillTarget = 0;

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CBuffSwellOfMPCancelMsgBoxLayOut::SetLayout()
{
    extern int g_iCancelSkillTarget;
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t strText[MAX_GLOBAL_TEXT_STRING];
    mu_swprintf(strText, L"%ls%ls", SkillAttribute[AT_SKILL_EXPANSION_OF_WIZARDRY].Name, I18N::Game::WouldYouLikeToCancel);
    g_iCancelSkillTarget = AT_SKILL_EXPANSION_OF_WIZARDRY;

    pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CBuffSwellOfMPCancelMsgBoxLayOut::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CBuffSwellOfMPCancelMsgBoxLayOut::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CBuffSwellOfMPCancelMsgBoxLayOut::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CBuffSwellOfMPCancelMsgBoxLayOut::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CBuffSwellOfMPCancelMsgBoxLayOut::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    extern int g_iCancelSkillTarget;
    SocketClient->ToGameServer()->SendMagicEffectCancelRequest(g_iCancelSkillTarget, HeroKey);
    g_iCancelSkillTarget = 0;

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CBuffSwellOfMPCancelMsgBoxLayOut::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    extern int g_iCancelSkillTarget;
    g_iCancelSkillTarget = 0;

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGemIntegrationUnityCheckMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGemIntegrationUnityCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGemIntegrationUnityCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CGemIntegrationUnityCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CGemIntegrationUnityCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::ProcessCSAction();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    COMGEM::Exit();

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::GetBack();

    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationUnityMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGemIntegrationUnityResultMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    wchar_t strText[256] = { 0, };
    mu_swprintf(strText, L"%ls%ls %ls", I18N::Game::JewelCombination, I18N::Game::To1816, I18N::Game::CongratulationsYouHaveSuccessfully);
    pMsgBox->AddMsg(strText, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CGemIntegrationUnityResultMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationUnityResultMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGemIntegrationDisjointCheckMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGemIntegrationDisjointCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGemIntegrationDisjointCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CGemIntegrationDisjointCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CGemIntegrationDisjointCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::ProcessCSAction();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    COMGEM::Exit();

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::GetBack();

    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationDisjointMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGemIntegrationDisjointResultMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    wchar_t strText[256] = { 0, };
    mu_swprintf(strText, L"%ls%ls %ls", I18N::Game::DismantleJewel, I18N::Game::To1816, I18N::Game::CongratulationsYouHaveSuccessfully);
    pMsgBox->AddMsg(strText, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CGemIntegrationDisjointResultMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointResultMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CChaosCastleTimeCheckMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CChaosCastleTimeCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CChaosCastleTimeCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CChaosCastleTimeCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);

    pMsgBox->AddCallbackFunc(CChaosCastleTimeCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CChaosCastleTimeCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    ITEM* pItem = g_pMyInventory->GetStandbyItem();
    if (pItem)
    {
        int iSrcIndex = g_pMyInventory->GetStandbyItemIndex();
        SocketClient->ToGameServer()->SendChaosCastleEnterRequest(pItem->Level, iSrcIndex);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CChaosCastleTimeCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CHarvestEventLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToReceiveTheItem, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddCallbackFunc(CHarvestEventLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CHarvestEventLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CHarvestEventLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CHarvestEventLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CHarvestEventLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendLeoHelperItemRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CHarvestEventLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CWhiteAngelEventLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToReceiveTheItem, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CWhiteAngelEventLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CWhiteAngelEventLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CWhiteAngelEventLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CWhiteAngelEventLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CWhiteAngelEventLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendWhiteAngelItemRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CWhiteAngelEventLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool  mu::ui::window::CLuckyItemMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    // 아이템 제목
    int				nTextIndex[10] = { 0, };
    eLUCKYITEMTYPE	eAct = g_pLuckyItemWnd->GetAct();

    switch (eAct)
    {
    case eLuckyItemType_Trade:
        nTextIndex[0] = 3288;
        nTextIndex[1] = 3297;
        nTextIndex[2] = 3298;
        nTextIndex[3] = 3299;
        break;
    case eLuckyItemType_Refinery:
        nTextIndex[0] = 3289;
        nTextIndex[1] = 539;
        break;
    default:
        return false;
        break;
    }

    pMsgBox->AddMsg(I18N::Game::Lookup(nTextIndex[0]), RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    for (int i = 1; i < 10; i++)
    {
        if (nTextIndex[i] <= 0)	break;
        pMsgBox->AddMsg(I18N::Game::Lookup(nTextIndex[i]), RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    }

    pMsgBox->AddCallbackFunc(CLuckyItemMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CLuckyItemMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CLuckyItemMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CLuckyItemMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CLuckyItemMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendChaosMachineMixRequest(static_cast<ChaosMachineMixType>(g_pLuckyItemWnd->SetActAction()), 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CLuckyItemMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool  mu::ui::window::CMixCheckMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t strText[256];
    if (g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1] == 0)
    {
        mu_swprintf(strText, L"%ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]));
    }
    else if (g_MixRecipeMgr.GetCurRecipe()->m_iMixName[2] == 0)
    {
        mu_swprintf(strText, L"%ls %ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]),
            I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1]));
    }
    else
    {
        mu_swprintf(strText, L"%ls %ls %ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]),
            I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1]),
            I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[2]));
    }

    pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::DoYouWantToCombineYourItems, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddCallbackFunc(CMixCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CMixCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CMixCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CMixCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CMixCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pMixInventory->SetMixState(mu::ui::window::CNewUIMixInventory::MIX_REQUESTED);
    SocketClient->ToGameServer()->SendChaosMachineMixRequest(
        static_cast<ChaosMachineMixType>(g_MixRecipeMgr.GetCurMixID()),
        g_MixRecipeMgr.GetMixSubType());

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CMixCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUseReviveCharmMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToSaveTheLocation);
    pMsgBox->AddCallbackFunc(CUseReviveCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUseReviveCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUseReviveCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CUseReviveCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUseReviveCharmMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    BYTE srcIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(srcIndex, 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseReviveCharmMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUsePortalCharmMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToSaveTheLocation);
    pMsgBox->AddCallbackFunc(CUsePortalCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUsePortalCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUsePortalCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CUsePortalCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUsePortalCharmMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_PortalMgr.SavePortalPosition();
    BYTE srcIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(srcIndex, 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUsePortalCharmMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CReturnPortalCharmMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToSaveTheLocation);
    pMsgBox->AddCallbackFunc(CReturnPortalCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CReturnPortalCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CReturnPortalCharmMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CReturnPortalCharmMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CReturnPortalCharmMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    BYTE srcIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(srcIndex, 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CReturnPortalCharmMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CDuelCreateErrorMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::ColosseumIsOccupied, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddMsg(I18N::Game::TryItAgainLater, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CDuelCreateErrorMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CDuelCreateErrorMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CDuelWatchErrorMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::NotAvailable, RGBA(255, 255, 128, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::TooManyPeopleInTheColossum, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CDuelWatchErrorMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CDuelWatchErrorMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CDoppelGangerMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddCallbackFunc(CDoppelGangerMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CDoppelGangerMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildRelationShipMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGuildRelationShipMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildRelationShipMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGuildRelationShipMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);

    pMsgBox->AddCallbackFunc(CGuildRelationShipMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildRelationShipMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    const mu::ui::window::ServerMessageInfo info = g_pGuildInfoWindow->GetServerMessage();

    SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
        info.s_byRelationShipType,
        info.s_byRelationShipRequestType,
        0x01,
        MAKEWORD(info.s_byTargetUserIndexH, info.s_byTargetUserIndexL));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildRelationShipMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    const mu::ui::window::ServerMessageInfo info = g_pGuildInfoWindow->GetServerMessage();

    SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
        info.s_byRelationShipType,
        info.s_byRelationShipRequestType,
        0x00,
        MAKEWORD(info.s_byTargetUserIndexH, info.s_byTargetUserIndexL));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCastleMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CCastleMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CCastleMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CCastleMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CCastleMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCastleMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    switch (g_pCastleWindow->GetCurrMsgBoxRequest())
    {
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_BUY_GATE:
        g_SenatusInfo.DoGateRepairAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_REPAIR_GATE:
        g_SenatusInfo.DoGateRepairAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_UPGRADE_GATE_HP:
        g_SenatusInfo.DoGateUpgradeHPAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE:
        g_SenatusInfo.DoGateUpgradeDefenseAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_BUY_STATUE:
        g_SenatusInfo.DoStatueRepairAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_REPAIR_STATUE:
        g_SenatusInfo.DoStatueRepairAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_HP:
        g_SenatusInfo.DoStatueUpgradeHPAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE:
        g_SenatusInfo.DoStatueUpgradeDefenseAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER:
        g_SenatusInfo.DoStatueUpgradeRecoverAction();
        break;
    case mu::ui::window::CNewUICastleWindow::CASTLE_MSGREQ_APPLY_TAX:
        g_SenatusInfo.DoApplyTaxAction();
        break;
    default:
        break;
    };

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CCastleMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CSiegeLevelMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouHaveNoAbility, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddMsg(I18N::Game::ToAttackTheCastle, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CSiegeLevelMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CSiegeLevelMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CSiegeGiveUpMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::AreYouReallyWantToQuitTheSiegeWargare, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CSiegeGiveUpMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CSiegeGiveUpMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CSiegeGiveUpMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CSiegeGiveUpMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CSiegeGiveUpMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCastleSiegeUnregisterRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CSiegeGiveUpMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGatemanMoneyMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::EnteringIsNotAllowed, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddMsg(I18N::Game::InsufficientZenForEntering, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CGatemanMoneyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CGatemanMoneyMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}
bool mu::ui::window::CGatemanFailMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();

    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::UnfortunatelyYouHaveFailed, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    pMsgBox->AddCallbackFunc(CGatemanFailMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CGatemanFailMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CQuestGiveUpMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::IfYouGiveUpYouWill);

    pMsgBox->AddCallbackFunc(CQuestGiveUpMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CQuestGiveUpMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CQuestGiveUpMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CQuestGiveUpMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CQuestGiveUpMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    const DWORD dwSelectedQuest = g_pMyQuestInfoWindow->GetSelQuestIndex();
    const auto questNumber = static_cast<uint16_t>(LOWORD(dwSelectedQuest));
    const auto questGroup = static_cast<uint16_t>(HIWORD(dwSelectedQuest));
    SocketClient->ToGameServer()->SendQuestCancelRequest(questNumber, questGroup);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CQuestGiveUpMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

#ifdef ASG_ADD_TIME_LIMIT_QUEST
bool mu::ui::window::CQuestCountLimitMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouCannotAcceptAnyMoreQuest);
    pMsgBox->AddMsg(I18N::Game::YouCanProceedMaximum10Quests);
    pMsgBox->AddMsg(I18N::Game::AtTheSameTime);
    pMsgBox->AddMsg(I18N::Game::YouNeedToClearAtLeast1QuestTo);
    pMsgBox->AddMsg(I18N::Game::AcceptThisOne);

    pMsgBox->AddCallbackFunc(CQuestCountLimitMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return true;
}

CALLBACK_RESULT mu::ui::window::CQuestCountLimitMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}
#endif	// ASG_ADD_TIME_LIMIT_QUEST

bool mu::ui::window::CCanNotUseWordMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::RestrictedWordsAre, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);
    pMsgBox->AddMsg(I18N::Game::Included, RGBA(255, 255, 255, 255), MSGBOX_FONT_NORMAL);

    return true;
}

bool mu::ui::window::CHighValueItemCheckMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    ITEM* pItem = NULL;
    if (pPickedItem)
    {
        pItem = pPickedItem->GetItem();
    }
    if (pItem)
    {
        pMsgBox->Set3DItem(pItem);
    }

    pMsgBox->AddMsg(I18N::Game::AnExpensiveItem, RGBA(255, 0, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::CheckTheItemPlease, RGBA(255, 178, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::AreYouSureYouWantToSellIt, RGBA(255, 178, 0, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CHighValueItemCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CHighValueItemCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CHighValueItemCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CHighValueItemCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CHighValueItemCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    int iSourceIndex = -1;

    if (pPickedItem)
    {
        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            iSourceIndex = pPickedItem->GetSourceLinealPos();
        }
        else
        {
            iSourceIndex = pPickedItem->GetSourceLinealPos();
        }
    }

    if (iSourceIndex >= MAX_EQUIPMENT_INDEX && iSourceIndex < MAX_MY_INVENTORY_EX_INDEX)
    {
        SocketClient->ToGameServer()->SendSellItemToNpcRequest(iSourceIndex);
        g_pNPCShop->SetSellingItem(true);
        // Note: picked item will be cleaned up by ReceiveSell when server responds
    }
    else
    {
        // If no valid item, restore it
        CNewUIInventoryCtrl::BackupPickedItem();
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CHighValueItemCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    CNewUIInventoryCtrl::BackupPickedItem();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUseFruitMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    ITEM* pItem = g_pMyInventory->GetStandbyItem();
    if (pItem == NULL)
    {
        return false;
    }

    pMsgBox->Set3DItem(pItem);

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

    pMsgBox->AddMsg(strName, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::DoYouWantToUseTheFruit, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CUseFruitMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUseFruitMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUseFruitMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CUseFruitMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUseFruitMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CUseFruitCheckMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseFruitMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUsePartChargeFruitMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    ITEM* pItem = g_pMyInventory->GetStandbyItem();
    if (pItem == NULL)
    {
        return false;
    }

    pMsgBox->Set3DItem(pItem);

    wchar_t strName[50] = { 0, };

    if (pItem->Type == ITEM_HELPER + 54)
    {
        mu_swprintf(strName, L"%ls", I18N::Game::STR);
    }
    else if (pItem->Type == ITEM_HELPER + 55)
    {
        mu_swprintf(strName, L"%ls", I18N::Game::AGI);
    }
    else if (pItem->Type == ITEM_HELPER + 56)
    {
        mu_swprintf(strName, L"%ls", I18N::Game::STA);
    }
    else if (pItem->Type == ITEM_HELPER + 57)
    {
        mu_swprintf(strName, L"%ls", I18N::Game::ENG);
    }
    else if (pItem->Type == ITEM_HELPER + 58)
    {
        mu_swprintf(strName, L"%ls", I18N::Game::Command);
    }

    pMsgBox->AddMsg(strName, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::ThisIsMoreThanTheValueOfYourResettablePoints, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToReset, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CUsePartChargeFruitMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUsePartChargeFruitMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUsePartChargeFruitMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CUsePartChargeFruitMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUsePartChargeFruitMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    BYTE srcIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(srcIndex, 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUsePartChargeFruitMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

bool mu::ui::window::CPersonalShopItemValueCheckMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    ITEM* pItemObj = NULL;
    if (g_pMyShopInventory->GetTargetIndex() == -1)
    {
        pItemObj = g_pMyShopInventory->FindItem(g_pMyShopInventory->GetSourceIndex());
    }
    else
    {
        if (pPickedItem)
        {
            pItemObj = pPickedItem->GetItem();
        }
    }

    if (pItemObj)
    {
        pMsgBox->Set3DItem(pItemObj);
    }

    pMsgBox->AddCallbackFunc(CPersonalShopItemValueCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPersonalShopItemValueCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    pMsgBox->AddCallbackFunc(CPersonalShopItemValueCheckMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPersonalShopItemValueCheckMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CPersonalShopItemValueCheckMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUI3DItemCommonMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    if (g_pMyShopInventory->IsEnablePersonalShop() == true)
    {
        SocketClient->ToGameServer()->SendPlayerShopClose();
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    int iSourceIndex = -1, iTargetIndex = -1;

    if (pPickedItem)
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        iSourceIndex = pPickedItem->GetSourceLinealPos();
        iTargetIndex = g_pMyShopInventory->GetTargetIndex();

        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            int iItemPrice = pMsgBox->GetItemValue();
            SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iItemPrice);
            SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
        }
        else if (pPickedItem->GetOwnerInventory() == NULL)
        {
            int iItemPrice = pMsgBox->GetItemValue();
            BYTE byIndex = iSourceIndex;
            SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iItemPrice);

            SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
        }
        else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
        {
            int iItemPrice = pMsgBox->GetItemValue();
            BYTE byIndex = MAX_MY_INVENTORY_EX_INDEX + iSourceIndex;
            SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iItemPrice);

            SendRequestEquipmentItem(STORAGE_TYPE::MYSHOP, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
        }

        AddPersonalItemPrice(iTargetIndex, pMsgBox->GetItemValue(), g_IsPurchaseShop);
    }
    else
    {
        ITEM* pItem = g_pMyShopInventory->FindItem(g_pMyShopInventory->GetSourceIndex());
        if (pItem)
        {
            iSourceIndex = g_pMyShopInventory->GetItemInventoryIndex(pItem);
            if (iSourceIndex >= 0)
            {
                int iItemPrice = pMsgBox->GetItemValue();
                SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iItemPrice);
                AddPersonalItemPrice(iSourceIndex, iItemPrice, g_IsPurchaseShop);
            }
        }
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CPersonalShopItemValueCheckMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    mu::ui::window::CNewUIInventoryCtrl::BackupPickedItem();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CPersonalShopItemBuyMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    ITEM* pItem = g_pPurchaseShopInventory->FindItem(g_pPurchaseShopInventory->GetSourceIndex());
    if (pItem)
    {
        pMsgBox->Set3DItem(pItem);
    }

    pMsgBox->AddMsg(I18N::Game::DoYouWantToBuyAnItem);
    pMsgBox->AddCallbackFunc(CPersonalShopItemBuyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPersonalShopItemBuyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CPersonalShopItemBuyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPersonalShopItemBuyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CPersonalShopItemBuyMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    ITEM* pItem = g_pPurchaseShopInventory->FindItem(g_pPurchaseShopInventory->GetSourceIndex());
    CHARACTER* pCha = &CharactersClient[g_pPurchaseShopInventory->GetShopCharacterIndex()];

    if (pItem && pCha)
    {
        int sourceIndex = g_pPurchaseShopInventory->GetItemInventoryIndex(pItem);
        if (sourceIndex >= 0)
        {
            SocketClient->ToGameServer()->SendPlayerShopItemBuyRequest(pCha->Key, MU_C16(pCha->ID), sourceIndex);
        }
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CPersonalShopItemBuyMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::COsbourneMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::Warning2223, RGBA(255, 0, 0, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L" ");
    pMsgBox->AddMsg(I18N::Game::RefineryHasStartedRefineryIsA, RGBA(223, 191, 103, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(COsbourneMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(COsbourneMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);

    return true;
}

CALLBACK_RESULT mu::ui::window::COsbourneMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_OSBOURNE);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildOutPerson::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::AllianceMasterCanTDisbandTheGuild, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CGuildOutPerson::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildOutPerson::OkBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);

    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildOutPerson::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildBreakMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(I18N::Game::OnceYouDisbandTheGuild, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::AllTheItemsAndZenInTheGuildVaultWillDisappear, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::AlsoTheGuildRankingInformationWillDisappear, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToDisbandTheGuild, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);

    pMsgBox->AddCallbackFunc(CGuildBreakMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildBreakMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGuildBreakMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    pMsgBox->AddCallbackFunc(CGuildBreakMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildBreakMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGuildBreakPasswordMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildBreakMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildPerson_Get_Out::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t Buff[300];
    mu_swprintf(Buff, I18N::Game::CharacterS, GuildList[DeleteIndex].Name);
    pMsgBox->AddMsg(Buff, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToRelease, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CGuildPerson_Get_Out::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildPerson_Get_Out::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGuildPerson_Get_Out::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    pMsgBox->AddCallbackFunc(CGuildPerson_Get_Out::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildPerson_Get_Out::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGuildBreakPasswordMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildPerson_Get_Out::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuildPerson_Cancel_Position_MsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddCallbackFunc(CGuildPerson_Cancel_Position_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGuildPerson_Cancel_Position_MsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGuildPerson_Cancel_Position_MsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    pMsgBox->AddCallbackFunc(CGuildPerson_Cancel_Position_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGuildPerson_Cancel_Position_MsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendGuildRoleAssignRequest(G_PERSON, MU_C16(GuildList[DeleteIndex].Name), 0x03);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuildPerson_Cancel_Position_MsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Result_Set_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    wchar_t Text[300];

    mu_swprintf(Text, L"%ls    %ls    %ls    %ls", I18N::Game::Rank, I18N::Game::Character, I18N::Game::Class, I18N::Game::Score);

    int TextColor = (255 << 24) + (21 << 16) + (148 << 8) + (255);
    pMsgBox->AddMsg(Text, TextColor);

    TextColor = (255 << 24) + (255 << 16) + (255 << 8) + (255);

    for (int i = 0; i < 5; i++)
    {
        if (HeroScore[i] == -1)
            continue;

        mu_swprintf(Text, L"%d      %ls      %ls      %d", i + 1, HeroName[i], gCharacterManager.GetCharacterClassText(HeroClass[i]), HeroScore[i]);

        pMsgBox->AddMsg(Text, TextColor);
    }

    TextColor = (255 << 24) + (255 << 16) + (0 << 8) + (255);
    pMsgBox->AddMsg(L"    ", TextColor);
    pMsgBox->AddMsg(L"    ", TextColor);
    pMsgBox->AddMsg(L"    ", TextColor);
    pMsgBox->AddMsg(L"    ", TextColor);

    if (View_Suc_Or_Fail == 1)
    {
        mu_swprintf(Text, I18N::Game::MonsterStrengthDecreased10);
        pMsgBox->AddMsg(Text, TextColor);
        mu_swprintf(Text, I18N::Game::_5IncreaseInCastleAndArenaInvitationCombineRate);
        pMsgBox->AddMsg(Text, TextColor);
    }
    else
    {
        pMsgBox->AddMsg(I18N::Game::AllNPCsInCrywolfHaveBeenDeleted, TextColor);
    }

    pMsgBox->AddCallbackFunc(CCry_Wolf_Result_Set_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Result_Set_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Ing_Set_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::ContractIsOngoingThereforeDualCompactIsNotPossible);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Ing_Set_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Ing_Set_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Destroy_Set_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::FurtherContractCanTBeDoneSinceTheAltarHasBeenDestroyed);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Destroy_Set_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Destroy_Set_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Wat_Set_Temple1::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::PleaseTryAgainInAWhile);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Wat_Set_Temple1::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Wat_Set_Temple1::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Dont_Set_Temple1::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::DisqualifiedForTheContractRequirement);
    pMsgBox->AddMsg(I18N::Game::OnlyLevelAbove350IsAllowedToMakeAContract);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Dont_Set_Temple1::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Dont_Set_Temple1::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Dont_Set_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::ContractCanTBeMadeWhenYouAreOnAMount);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Dont_Set_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Dont_Set_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Set_Temple1::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::WeNeedAGuardianToProtectTheWolf);

    pMsgBox->AddCallbackFunc(CCry_Wolf_Set_Temple1::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Set_Temple1::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Set_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouHaveBeenRegisteredToBeAGuardianToProtectTheWolf);
    pMsgBox->AddMsg(I18N::Game::YourRoleAsAGuardianWillBeCancelledWhenYouWarp);
    BackUp_Key = CharactersClient[TargetNpc].Key;

    pMsgBox->AddCallbackFunc(CCry_Wolf_Get_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Set_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}
//CMaster_Level_Interface
bool mu::ui::window::CMaster_Level_Interface::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    auto Need_Point = g_pMasterLevelInterface->GetConsumePoint();
    wchar_t szText[256];
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToStrengthenTheSkill);
    mu_swprintf(szText, I18N::Game::MasterLevelPointRequirementD, Need_Point);
    pMsgBox->AddMsg(szText);

    pMsgBox->AddCallbackFunc(CMaster_Level_Interface::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CMaster_Level_Interface::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CMaster_Level_Interface::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CMaster_Level_Interface::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CMaster_Level_Interface::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CMaster_Level_Interface::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto In_Skill = g_pMasterLevelInterface->GetCurSkillID();
    SocketClient->ToGameServer()->SendAddMasterSkillPoint(In_Skill);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
    return CALLBACK_BREAK;
}

bool mu::ui::window::CCry_Wolf_Get_Temple::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t szText[256];
    int Num = CharactersClient[TargetNpc].Object.Type - MODEL_CRYWOLF_ALTAR1;
    BYTE State = (m_AltarState[Num] & 0x0f);
    mu_swprintf(szText, I18N::Game::ContractCanBeMadeForDTimes, State);
    pMsgBox->AddMsg(szText);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToProceedWithTheContract);
    BackUp_Key = CharactersClient[TargetNpc].Key;
    pMsgBox->AddCallbackFunc(CCry_Wolf_Get_Temple::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CCry_Wolf_Get_Temple::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CCry_Wolf_Get_Temple::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CCry_Wolf_Get_Temple::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Get_Temple::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    Button_Down = 1;
    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CCry_Wolf_Set_Temple1));
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CCry_Wolf_Get_Temple::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    if (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT || Hero->Helper.Type == MODEL_HORN_OF_FENRIR)
    {
        mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CCry_Wolf_Dont_Set_Temple));
    }
    else
    {
        Button_Down = 2;
        SocketClient->ToGameServer()->SendCrywolfContractRequest(BackUp_Key);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUnionGuild_Break_MsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    wchar_t szText[256];
    mu_swprintf(szText, I18N::Game::SGuildFromTheAlliance, DeleteID);
    pMsgBox->AddMsg(szText);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToRelease);
    pMsgBox->AddCallbackFunc(CUnionGuild_Break_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUnionGuild_Break_MsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUnionGuild_Break_MsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    pMsgBox->AddCallbackFunc(CUnionGuild_Break_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUnionGuild_Break_MsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUnionGuild_Break_MsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendRemoveAllianceGuildRequest(MU_C16(DeleteID));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUnionGuild_Out_MsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return false;

    pMsgBox->AddMsg(I18N::Game::AllianceMasterCanTWithdrawTheGuild, RGBA(255, 255, 255, 255), MSGBOX_FONT_BOLD);
    pMsgBox->AddCallbackFunc(CUnionGuild_Out_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUnionGuild_Out_MsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CUnionGuild_Out_MsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUseSantaInvitationMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return FALSE;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToMoveToTheSantaSVillage);
    pMsgBox->AddCallbackFunc(CUseSantaInvitationMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CUseSantaInvitationMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CUseSantaInvitationMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CUseSantaInvitationMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return TRUE;
}

CALLBACK_RESULT mu::ui::window::CUseSantaInvitationMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    ITEM* pItem = g_pMyInventory->GetStandbyItem();

    if (pItem)
    {
        int iSrcIndex = g_pMyInventory->GetStandbyItemIndex();
        SendRequestUse(iSrcIndex, 0);
    }
    else
    {
        //N/A
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CUseSantaInvitationMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool CSantaTownLeaveMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return FALSE;

    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToReturnToDevias);
    pMsgBox->AddCallbackFunc(CSantaTownLeaveMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CSantaTownLeaveMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CSantaTownLeaveMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CSantaTownLeaveMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return TRUE;
}

CALLBACK_RESULT CSantaTownLeaveMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendMoveToDeviasBySnowmanRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CSantaTownLeaveMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool CSantaTownSantaMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return FALSE;

    pMsgBox->AddCallbackFunc(CSantaTownSantaMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CSantaTownSantaMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CSantaTownSantaMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CSantaTownSantaMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return TRUE;
}

CALLBACK_RESULT CSantaTownSantaMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendSantaClausItemRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CSantaTownSantaMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUseRegistLuckyCoinMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    wchar_t szText[100] = { 0, };
    mu_swprintf(szText, I18N::Game::YouAreLackOfSItems, I18N::Game::Register);
    pMsgBox->AddMsg(szText);
    pMsgBox->AddCallbackFunc(CUseRegistLuckyCoinMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    return TRUE;
}

CALLBACK_RESULT mu::ui::window::CUseRegistLuckyCoinMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CRegistOverLuckyCoinMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    pMsgBox->AddMsg(I18N::Game::YouCanOnlyApplyOncePerYourAccount);

    pMsgBox->AddCallbackFunc(CRegistOverLuckyCoinMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return TRUE;
}
CALLBACK_RESULT mu::ui::window::CRegistOverLuckyCoinMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CExchangeLuckyCoinMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    wchar_t szText[100] = { 0, };
    mu_swprintf(szText, I18N::Game::YouAreLackOfSItems, I18N::Game::Exchange1940);
    pMsgBox->AddMsg(szText);

    pMsgBox->AddCallbackFunc(CExchangeLuckyCoinMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return TRUE;
}

CALLBACK_RESULT mu::ui::window::CExchangeLuckyCoinMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CExchangeLuckyCoinInvenErrMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    pMsgBox->AddMsg(I18N::Game::MoreThan2X4SpaceInInventoryIsNeeded);

    pMsgBox->AddCallbackFunc(CExchangeLuckyCoinInvenErrMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return TRUE;
}

CALLBACK_RESULT mu::ui::window::CExchangeLuckyCoinInvenErrMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGambleBuyMsgBoxLayout::SetLayout()
{
    CNewUI3DItemCommonMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    ITEM* pItem = g_pNPCShop->GetStandbyItem();
    if (pItem == NULL)
    {
        return false;
    }
    pMsgBox->Set3DItem(pItem);
    pMsgBox->AddMsg(I18N::Game::WouldYouLikeToPurchase);
    pMsgBox->AddCallbackFunc(CGambleBuyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CGambleBuyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CGambleBuyMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CGambleBuyMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

CALLBACK_RESULT mu::ui::window::CGambleBuyMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    GambleSystem& gambleSys = GambleSystem::Instance();
    if (gambleSys.IsGambleShop() && BuyCost != 0)
    {
        const auto& itemInfo = gambleSys.GetBuyItemInfoConst();
        SocketClient->ToGameServer()->SendBuyItemFromNpcRequest(itemInfo.ItemIndex);
        BuyCost = itemInfo.ItemCost;
        g_ConsoleDebug->Write(MCD_SEND, L"0x32 [SendRequestBuy(%d)]", itemInfo.ItemIndex);
    }
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGambleBuyMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CEmpireGuardianMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    pMsgBox->AddCallbackFunc(CEmpireGuardianMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return TRUE;
}

CALLBACK_RESULT mu::ui::window::CEmpireGuardianMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CUnitedMarketPlaceMsgBoxLayout::SetLayout()
{
    CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return FALSE;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OK))
        return FALSE;

    pMsgBox->AddCallbackFunc(CUnitedMarketPlaceMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    return TRUE;
}
CALLBACK_RESULT mu::ui::window::CUnitedMarketPlaceMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}
