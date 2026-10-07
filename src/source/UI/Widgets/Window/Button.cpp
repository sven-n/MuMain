
#include "stdafx.h"

#include "UI/Widgets/Window/Button.h"
#include "UI/Social/SocialWindowBase.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"
#include <vector>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
    void PointSet(POINT& p, int x, int y)
    {
        p.x = x; p.y = y;
    }
};

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// CBaseButton
//////////////////////////////////////////////////////////////////////

CBaseButton::CBaseButton() : m_Lock(false), m_EventState(BUTTON_STATE_UP)
{
    PointSet(m_Pos, 0, 0);
    PointSet(m_Size, 0, 0);
}

CBaseButton::~CBaseButton()
{
}

void CBaseButton::SetPos(int x, int y)
{
    PointSet(m_Pos, x, y);
}

void CBaseButton::SetSize(int sx, int sy)
{
    PointSet(m_Size, sx, sy);
}

bool CBaseButton::IsMouseIn() const
{
    return mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y).Contains(MouseX, MouseY);
}

void CBaseButton::RenderStateImage(int imgIndex, int frame, int frameCount, unsigned int color)
{
    if (imgIndex < 0 || frameCount <= 0)
    {
        return;
    }

    // CSprite expects raw logical m_Pos/m_Size and applies scale + screen offset itself inside
    // Render() -- pre-scaling the position here (as an earlier version did) double-applies the
    // offset, which only happens to cancel out at 800x600.
    const UI::Scaling::Transform& transform = UI::Scaling::GetActiveTransform();

    // Rebuild the sprite's frame table when its config changes, or on a resize/scale change --
    // CSprite bakes WindowHeight and transform scale in at Create() time with no live setter.
    if (m_spriteImgIndex != imgIndex || m_spriteFrameCount != frameCount ||
        m_spriteFrameSize.x != m_Size.x || m_spriteFrameSize.y != m_Size.y ||
        m_spriteWindowHeight != WindowHeight ||
        m_spriteScaleX != transform.scaleX || m_spriteScaleY != transform.scaleY)
    {
        std::vector<SFrameCoord> frames(frameCount);
        for (int i = 0; i < frameCount; ++i)
        {
            // Vertically-stacked frames only -- the only orientation this rendering path uses.
            frames[i].nX = 0;
            frames[i].nY = i * m_Size.y;
        }

        m_sprite.Create(m_Size.x, m_Size.y, imgIndex, frameCount, frames.data(),
                         0, 0, false, SPR_SIZING_DATUMS_LT, transform.scaleX, transform.scaleY);
        // Create() locks the sprite's legal frame range to [0,0]; widen it or SetNowFrame() below can't move off frame 0.
        m_sprite.SetAction(0, frameCount - 1);

        m_spriteImgIndex = imgIndex;
        m_spriteFrameCount = frameCount;
        m_spriteFrameSize = m_Size;
        m_spriteWindowHeight = WindowHeight;
        m_spriteScaleX = transform.scaleX;
        m_spriteScaleY = transform.scaleY;
    }

    m_sprite.SetNowFrame(frame);
    m_sprite.SetPosition(m_Pos.x, m_Pos.y);
    m_sprite.SetSize(m_Size.x, m_Size.y);

    m_sprite.SetColor(GetRed(color), GetGreen(color), GetBlue(color));
    m_sprite.SetAlpha(GetAlpha(color));
    m_sprite.Show(true);
    m_sprite.Render();
}

bool CBaseButton::Process()
{
    bool isMousein = IsMouseIn();

    if (mu::ui::window::IsNone(VK_LBUTTON) && isMousein)
    {
        m_EventState = BUTTON_STATE_OVER;
    }
    else if ((mu::ui::window::IsRepeat(VK_LBUTTON) || mu::ui::window::IsPress(VK_LBUTTON)) && isMousein)
    {
        m_EventState = BUTTON_STATE_DOWN;
    }
    else if (mu::ui::window::IsRelease(VK_LBUTTON) && isMousein)
    {
        m_EventState = BUTTON_STATE_UP;
        return true;
    }
    else
    {
        m_EventState = BUTTON_STATE_UP;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// CButton
//////////////////////////////////////////////////////////////////////
mu::ui::window::CButton::CButton() : CBaseButton(), m_CurImgIndex(0),
m_CurImgState(0), m_ImgWidth(0), m_ImgHeight(0),
m_NameColor(0xFFFFFFFF), m_NameBackColor(0x00000000),
m_CurImgColor(0xFFFFFFFF),
#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE			// #ifndef
m_IsImgWidth(false),
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
m_fAlpha(1.0f)
{
    Initialize();
}

mu::ui::window::CButton::~CButton()
{
    Destroy();
}

void mu::ui::window::CButton::Initialize()
{
    m_hTextFont = g_hFont;
    m_tooltip.SetFont(g_hFont);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_iMoveTextPosX = 0;
    m_iMoveTextPosY = 0;
    m_bClickEffect = false;
    m_iMoveTextTipPosX = 0;
    m_iMoveTextTipPosY = 0;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CButton::Destroy()
{
    if (m_LocaleObserverRegistered)
    {
        I18N::UnregisterLocaleObserver(&mu::ui::window::CButton::OnLocaleChanged, this);
        m_LocaleObserverRegistered = false;
    }

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    UnRegisterButtonState();
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CButton::ChangeText(const wchar_t* const* nameSlot)
{
    m_pNameSlot = nameSlot;
    m_Name = (nameSlot != nullptr && *nameSlot != nullptr) ? *nameSlot : L"";
    EnsureLocaleObserver();
}

void mu::ui::window::CButton::ChangeToolTipText(const wchar_t* const* tooltipSlot, bool istoppos)
{
    m_tooltip.SetText(tooltipSlot);
    m_tooltip.SetAnchorAbove(istoppos);
}

void mu::ui::window::CButton::EnsureLocaleObserver()
{
    if (m_LocaleObserverRegistered) return;
    I18N::RegisterLocaleObserver(&mu::ui::window::CButton::OnLocaleChanged, this);
    m_LocaleObserverRegistered = true;
}

void mu::ui::window::CButton::OnLocaleChanged(void* ctx) noexcept
{
    auto* self = static_cast<CButton*>(ctx);
    if (self->m_pNameSlot != nullptr && *self->m_pNameSlot != nullptr)
    {
        self->m_Name = *self->m_pNameSlot;
    }
    // Tooltip-slot refresh is m_tooltip's own concern -- it registers its own locale observer.
}

#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void mu::ui::window::CButton::ChangeButtonImgState(bool imgregister, int imgindex, bool overflg /* = false */,
    bool bLockImage /* = false */, bool bClickEffect /* = false  */)
{
    m_bClickEffect = bClickEffect;

    if (imgregister)
    {
        RegisterButtonState(BUTTON_STATE_UP, imgindex, 0);

        if (overflg)
        {
            RegisterButtonState(BUTTON_STATE_OVER, imgindex, 1);
            RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 2);
        }
        else
        {
            RegisterButtonState(BUTTON_STATE_OVER, imgindex, 0);
            RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 1);
        }

        if (bLockImage)
        {
            RegisterButtonState(BUTTON_STATE_LOCK, imgindex, 3);
        }
        else
        {
            RegisterButtonState(BUTTON_STATE_LOCK, imgindex, 0);
        }

        ChangeImgIndex(imgindex, 0);
    }
}
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CButton::ChangeButtonImgState(bool imgregister, int imgindex, bool overflg, bool isimgwidth, bool bClickEffect)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CButton::ChangeButtonImgState(bool imgregister, int imgindex, bool overflg, bool isimgwidth)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_bClickEffect = bClickEffect;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    if (imgregister)
    {
        if (overflg)
        {
            RegisterButtonState(BUTTON_STATE_UP, imgindex, 0);
            RegisterButtonState(BUTTON_STATE_OVER, imgindex, 1);
            RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 2);
        }
        else
        {
            RegisterButtonState(BUTTON_STATE_UP, imgindex, 0);
            RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 1);
        }

        ChangeImgIndex(imgindex, 0);
        ChangeImgWidth(isimgwidth);
    }
}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

void mu::ui::window::CButton::ChangeButtonInfo(int x, int y, int sx, int sy)
{
    SetPos(x, y);
    SetSize(sx, sy);
}

void mu::ui::window::CButton::RegisterButtonState(BUTTON_STATE eventstate, int imgindex, int btstate)
{
    ButtonInfo btinfo;
    btinfo.s_ImgIndex = imgindex;
    btinfo.s_BTstate = btstate;

    m_ButtonInfo.insert(std::make_pair(eventstate, btinfo));
}

void mu::ui::window::CButton::UnRegisterButtonState()
{
    m_ButtonInfo.clear();
}

void mu::ui::window::CButton::ChangeImgIndex(int imgindex, int curimgstate)
{
    m_CurImgIndex = imgindex;
    m_CurImgState = curimgstate;

    if (m_CurImgIndex != -1)
    {
        BITMAP_t* b = &Bitmaps[m_CurImgIndex];

        m_ImgWidth = b->Width;
        m_ImgHeight = b->Height;
    }
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CButton::ChangeButtonState(BUTTON_STATE eventstate, int iButtonState)
{
    if (m_ButtonInfo.size() != 0)
    {
        auto iter = m_ButtonInfo.find(static_cast<int>(eventstate));

        if (iter != m_ButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;
            info.s_BTstate = iButtonState;
        }
    }
}

void mu::ui::window::CButton::MoveTextPos(int iX, int iY)
{
    m_iMoveTextPosX = iX;
    m_iMoveTextPosY = iY;
}

void mu::ui::window::CButton::MoveTextTipPos(int iX, int iY)
{
    m_iMoveTextTipPosX = iX;
    m_iMoveTextTipPosY = iY;
}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

void mu::ui::window::CButton::ChangeAlpha(unsigned char fAlpha, bool isfontalph)
{
    m_CurImgColor &= ~(0xff << 24);
    m_CurImgColor |= (fAlpha << 24);

    if (isfontalph)
    {
        m_NameColor &= ~(0xff << 24);
        m_NameColor |= (fAlpha << 24);
    }
}

void mu::ui::window::CButton::ChangeAlpha(float fAlpha, bool isfontalph)
{
    m_CurImgColor &= ~(0xff << 24);
    m_CurImgColor |= (static_cast<unsigned char>((float)(0xff) * fAlpha) << 24);

    if (isfontalph)
    {
        m_NameColor &= ~(0xff << 24);
        m_NameColor |= (static_cast<unsigned char>((float)(0xff) * fAlpha) << 24);
    }
}

void mu::ui::window::CButton::ChangeImgColor(BUTTON_STATE eventstate, unsigned int color)
{
    if (m_ButtonInfo.size() != 0)
    {
        auto iter = m_ButtonInfo.find(static_cast<int>(eventstate));

        if (iter != m_ButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;
            info.s_imgColor = color;
            if (GetBTState() == eventstate)
            {
                m_CurImgColor = color;
            }
        }
    }
}

void mu::ui::window::CButton::ChangeFrame()
{
    if (m_ButtonInfo.size() != 0)
    {
        auto iter = m_ButtonInfo.find(static_cast<int>(GetBTState()));

        if (iter != m_ButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;

            ChangeImgIndex(info.s_ImgIndex, info.s_BTstate);

            m_CurImgColor = info.s_imgColor;
        }
    }
}

bool mu::ui::window::CButton::UpdateMouseEvent()
{
    if (IsLock())
    {
        return false;
    }

    BUTTON_STATE backevent = GetBTState();

    bool result = Process();

    if (backevent != GetBTState())
    {
        ChangeFrame();
    }

    return result;
}

#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void mu::ui::window::CButton::Lock()
{
    CBaseButton::Lock();
    ChangeFrame();
}

void mu::ui::window::CButton::UnLock()
{
    CBaseButton::UnLock();
    ChangeFrame();
}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

bool mu::ui::window::CButton::Render(bool RendOption)
{
    if (!m_ButtonInfo.empty())
    {
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        if (RendOption == true)
        {
            // Single-caller special case (MiniMap's exit button): a hardcoded UV sub-rect crop that CSprite can't express, so it stays on RenderImage().
            RenderImage(m_CurImgIndex, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, 0.f, m_CurImgState * m_Size.y, 36.f / 64.f, (29.f / 32.f) / 2.f);
        }
        else
        {
            RenderStateImage(m_CurImgIndex, m_CurImgState, 3, m_CurImgColor);
        }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        if (m_IsImgWidth)
        {
            RenderImage(m_CurImgIndex, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, m_CurImgState * m_Size.x, 0.0f, m_CurImgColor);
        }
        else
        {
            RenderImage(m_CurImgIndex, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, 0.0f, m_CurImgState * m_Size.y, m_CurImgColor);
        }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    }

    if (m_Name.size() != 0)
    {
        g_pRenderText->SetFont(m_hTextFont);
        const SIZE Fontsize = g_pRenderText->MeasureText(m_Name.c_str(), static_cast<int>(m_Name.size()));

        int x = m_Pos.x + ((m_Size.x / 2) - (Fontsize.cx / 2));
        int y = m_Pos.y + ((m_Size.y / 2) - (Fontsize.cy / 2));

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        if ((m_bClickEffect == true) && (GetBTState() == BUTTON_STATE_DOWN))
        {
            RenderTextWithColors(m_Name.c_str(), x + m_iMoveTextPosX + 1, y + m_iMoveTextPosY + 1, m_Size.x, 0, m_hTextFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
        }
        else
        {
            RenderTextWithColors(m_Name.c_str(), x + m_iMoveTextPosX, y + m_iMoveTextPosY, m_Size.x, 0, m_hTextFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
        }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        RenderTextWithColors(m_Name.c_str(), x, y, m_Size.x, 0, m_hTextFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    }

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_tooltip.Render(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y, m_iMoveTextTipPosX, m_iMoveTextTipPosY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_tooltip.Render(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    return true;
}
