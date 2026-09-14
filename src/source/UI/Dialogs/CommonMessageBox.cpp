#include "stdafx.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Guild/GuildMakeWindow.h"
#include "Guild/GuildInfoWindow.h"
#include "UI/Inventory/MyInventory.h"
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
#include "UI/Inventory/LuckyItemWnd.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInterface.h"
#include "I18N/All.h"
#include "Core/Text/TextLineWrap.h"

using namespace SEASON3B;
using namespace mu::ui::window;

extern wchar_t DeleteID[];
extern int DeleteIndex, AppointStatus;
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

mu::ui::window::CMessageBoxButton::CMessageBoxButton()
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

mu::ui::window::CMessageBoxButton::~CMessageBoxButton()
{
}

bool mu::ui::window::CMessageBoxButton::IsMouseIn()
{
    if (m_bEnable == false)
        return false;

    return mu::ui::window::WindowGeometry(m_x, m_y, m_width, m_height).Contains(MouseX, MouseY);
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CMessageBoxButton::MoveTextPos(int iX, int iY)
{
    m_iMoveTextPosX = iX;
    m_iMoveTextPosY = iY;
}

void mu::ui::window::CMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType, bool bClickEffect)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType)
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

void mu::ui::window::CMessageBoxButton::SetText(const wchar_t* strText)
{
    if (wcslen(strText) > 0)
    {
        m_strText = strText;
    }
}

void mu::ui::window::CMessageBoxButton::AddBlank(int iAddBlank)
{
    m_y += iAddBlank;
}

void mu::ui::window::CMessageBoxButton::Update()
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

void mu::ui::window::CMessageBoxButton::Render()
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

mu::ui::window::CCommonMessageBox::CCommonMessageBox()
{
}

mu::ui::window::CCommonMessageBox::~CCommonMessageBox()
{
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

DWORD mu::ui::window::CCommonMessageBox::GetType()
{
    return m_dwType;
}

bool mu::ui::window::CCommonMessageBox::Create(DWORD dwType, float fPriority)
{
    m_dwType = dwType;

    SetAddCallbackFunc();

    const int x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    const int y = 100;
    const int width = MSGBOX_WIDTH;
    const int height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    if (!CMessageBoxBase::Create(x, y, width, height, fPriority))
        return false;

    SetButtonInfo();

    return true;
}

bool mu::ui::window::CCommonMessageBox::Create(DWORD dwType, const type_string& strMsg, DWORD dwColor, BYTE byFontType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    AddMsg(strMsg, dwColor, byFontType);

    return true;
}

void mu::ui::window::CCommonMessageBox::SetPos(int x, int y)
{
    CMessageBoxBase::SetPos(x, y);

    float fx, fy;
    fx = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    fy = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnOk.SetPos(fx, fy);
    fx = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    fy = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnCancel.SetPos(fx, fy);
}

void mu::ui::window::CCommonMessageBox::SetAddCallbackFunc()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    }
}

void mu::ui::window::CCommonMessageBox::SetButtonInfo()
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
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    }
}

void mu::ui::window::CCommonMessageBox::AddButtonBlank(int iAddLine)
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

void mu::ui::window::CCommonMessageBox::AddButtonBlank(int iAddLine, int _iImgSize)
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

void mu::ui::window::CCommonMessageBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
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

int mu::ui::window::CCommonMessageBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType, int _TextSize)
{
    return AppendWrappedText(m_MsgDataList, strMsg, dwColor, byFontType, _TextSize);
}

bool mu::ui::window::CCommonMessageBox::Update()
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

bool mu::ui::window::CCommonMessageBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CCommonMessageBox::RenderFrame()
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

void mu::ui::window::CCommonMessageBox::RenderTexts()
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

void mu::ui::window::CCommonMessageBox::RenderButtons()
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

void mu::ui::window::CCommonMessageBox::LockOkButton()
{
    m_BtnOk.SetEnable(false);
}

CALLBACK_RESULT mu::ui::window::CCommonMessageBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCommonMessageBox*>(pOwner);
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

CALLBACK_RESULT mu::ui::window::CCommonMessageBox::Close(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::C3DItemCommonMsgBox::C3DItemCommonMsgBox()
{
    ZeroMemory(&m_Item, sizeof(m_Item));
    m_iItemValue = 0;
}

mu::ui::window::C3DItemCommonMsgBox::~C3DItemCommonMsgBox()
{
    Release();
}

DWORD mu::ui::window::C3DItemCommonMsgBox::GetType()
{
    return m_dwType;
}

bool mu::ui::window::C3DItemCommonMsgBox::Create(DWORD dwType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;

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

    return true;
}

bool mu::ui::window::C3DItemCommonMsgBox::Create(DWORD dwType, const type_string& strMsg, DWORD dwColor, BYTE byFontType, float fPriority)
{
    int x, y, width, height;

    m_dwType = dwType;
    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    AddMsg(strMsg, dwColor, byFontType);

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Add3DRenderObj(this);

    return true;
}
void mu::ui::window::C3DItemCommonMsgBox::Release()
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

void mu::ui::window::C3DItemCommonMsgBox::Set3DItem(ITEM* pItem)
{
    if (pItem)
    {
        memcpy(&m_Item, pItem, sizeof(ITEM));
    }
}

void mu::ui::window::C3DItemCommonMsgBox::SetItemValue(int iValue)
{
    m_iItemValue = iValue;
}

int mu::ui::window::C3DItemCommonMsgBox::GetItemValue()
{
    return m_iItemValue;
}

void mu::ui::window::C3DItemCommonMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
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

CALLBACK_RESULT mu::ui::window::C3DItemCommonMsgBox::LButtonUp(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<C3DItemCommonMsgBox*>(pOwner);
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

CALLBACK_RESULT mu::ui::window::C3DItemCommonMsgBox::Close(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::C3DItemCommonMsgBox::Update()
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

void mu::ui::window::C3DItemCommonMsgBox::SetAddCallbackFunc()
{
    switch (m_dwType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_OK);
        AddCallbackFunc(mu::ui::window::C3DItemCommonMsgBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
        //AddCallbackFunc(mu::ui::window::CCommonMessageBox::Close, MSGBOX_EVENT_PRESSKEY_RETURN);
        break;
    }
}

int mu::ui::window::C3DItemCommonMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedText(
        m_MsgDataList, strMsg, dwColor, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH_3DITEM));
}

void mu::ui::window::C3DItemCommonMsgBox::SetButtonInfo()
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
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    }
}

void mu::ui::window::C3DItemCommonMsgBox::AddButtonBlank(int iAddLine)
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

bool mu::ui::window::C3DItemCommonMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::C3DItemCommonMsgBox::Render3D()
{
    float x, y, width, height;

    x = GetPos().x + 30;
    y = GetPos().y + 30;
    width = MSGBOX_3DITEM_WIDTH;
    height = MSGBOX_3DITEM_HEIGHT;

    RenderItem3D(x, y, width, height, m_Item.Type, m_Item.Level, m_Item.ExcellentFlags, m_Item.AncientDiscriminator, true);		// PickUp
}

bool mu::ui::window::C3DItemCommonMsgBox::IsVisible() const
{
    return true;
}

void mu::ui::window::C3DItemCommonMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
    y += height;
    if (m_MsgDataList.size() > 3)
    {
        int iCount = m_MsgDataList.size() - 3;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void mu::ui::window::C3DItemCommonMsgBox::RenderTexts()
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

void mu::ui::window::C3DItemCommonMsgBox::RenderButtons()
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

#ifdef ASG_ADD_TIME_LIMIT_QUEST
bool mu::ui::window::CQuestCountLimitMsgBoxLayout::SetLayout()
{
    CCommonMessageBox* pMsgBox = GetMsgBox();
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

CALLBACK_RESULT mu::ui::window::CQuestCountLimitMsgBoxLayout::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}
#endif	// ASG_ADD_TIME_LIMIT_QUEST

// CPersonalShopItemValueCheckMsgBoxLayout ported to CGenericConfirmDialog (MyShopInventory.cpp's
// ShowPersonalShopItemValueDialog()) -- see docs/rmlui-ui-system/dialog-migration-plan.md.


