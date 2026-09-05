
#include "stdafx.h"

#include "UI/Widgets/Window/Button.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "I18N/All.h"
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

    // CSprite (unlike RenderImage()'s ConvertPositionX/Y, which read g_fScreenRate_x/y and
    // g_fScreenOffset_x/y fresh on every call) expects LOGICAL/reference-resolution coordinates at
    // SetPosition()/SetSize() and applies scale *and* the live screen offset itself, once, inside
    // Render() -- see Sprite.cpp. An earlier version of this port pre-scaled position/size via
    // UI::Scaling::PositionX/Y() (which already folds the offset in) while keeping the sprite at
    // identity scale; CSprite's own Render() then added the offset a SECOND time, which only
    // canceled out at whatever window size made the active offset zero (800x600, apparently) --
    // see the H[6] report. Feed it raw m_Pos/m_Size instead, and give it the real transform scale
    // at Create() time.
    const UI::Scaling::Transform& transform = UI::Scaling::GetActiveTransform();

    // Rebuild the sprite's texture/frame table when the registered configuration or the frame's
    // own logical size changed (Create() reallocates the frame-UV table, so this is the expensive
    // path) -- and also when the window's been resized (CSprite::Create() bakes
    // m_fScrHeight = WindowHeight/fScaleY in at that moment for its Y-flip math) or the active
    // transform's scale changed (also baked in at Create() time, with no live setter of its own).
    // Missing either the first time this port shipped meant the button's screen position went
    // stale after a resize or simply wrong at any non-800x600 resolution -- see the H[6] report.
    if (m_spriteImgIndex != imgIndex || m_spriteFrameCount != frameCount ||
        m_spriteFrameSize.x != m_Size.x || m_spriteFrameSize.y != m_Size.y ||
        m_spriteWindowHeight != WindowHeight ||
        m_spriteScaleX != transform.scaleX || m_spriteScaleY != transform.scaleY)
    {
        std::vector<SFrameCoord> frames(frameCount);
        for (int i = 0; i < frameCount; ++i)
        {
            // Vertically-stacked frames only -- the only orientation CButton/CRadioButton's live
            // (KJH_ADD_INGAMESHOP_UI_SYSTEM) rendering actually uses; see the H[6] report.
            frames[i].nX = 0;
            frames[i].nY = i * m_Size.y;
        }

        m_sprite.Create(m_Size.x, m_Size.y, imgIndex, frameCount, frames.data(),
                         0, 0, false, SPR_SIZING_DATUMS_LT, transform.scaleX, transform.scaleY);
        // Create() locks the sprite's legal frame range to [0,0] internally (that range exists
        // for SetAction()'s animation use, which this doesn't use) -- without widening it here,
        // SetNowFrame() below silently refuses to move off frame 0 for the lifetime of this
        // sprite. See the H[6] report.
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

bool CBaseButton::RadioProcess()
{
    bool isMousein = IsMouseIn();

#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
    if (isMousein)
    {
        if (m_EventState == BUTTON_STATE_UP)
        {
            m_EventState = BUTTON_STATE_OVER;
        }

        if (mu::ui::window::IsPress(VK_LBUTTON))
        {
            if (m_EventState == BUTTON_STATE_OVER)
            {
                m_EventState = BUTTON_STATE_DOWN;
                return true;
            }
        }
    }
    else
    {
        if (m_EventState == BUTTON_STATE_OVER)
        {
            m_EventState = BUTTON_STATE_UP;
        }
    }
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
    if (mu::ui::window::IsPress(VK_LBUTTON) && isMousein)
    {
        if (m_EventState == BUTTON_STATE_UP)
        {
            m_EventState = BUTTON_STATE_DOWN;
            return true;
        }
        else if (m_EventState == BUTTON_STATE_DOWN)
        {
            m_EventState = BUTTON_STATE_UP;
        }
    }
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

    return false;
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
            // Narrow, single-caller special case (MiniMap.cpp's exit button): a hardcoded
            // 36/64 x 14.5/32 UV sub-rect crop, not a plain per-state frame. CSprite has no
            // "clip to an explicit UV fraction" primitive, so this one case deliberately keeps
            // using RenderImage() rather than being forced through CSprite -- see the H[6] report.
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

mu::ui::window::CRadioButton::CRadioButton() : m_NameColor(0xffB5B5B5), m_NameBackColor(0x00000000),
m_CurImgIndex(0), m_CurImgState(0), m_ImgWidth(0), m_ImgHeight(0), m_CurImgColor(0xffffffff)
{
    Initialize();
}

mu::ui::window::CRadioButton::~CRadioButton()
{
    Destroy();
}

void mu::ui::window::CRadioButton::Initialize()
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_hTextFont = g_hFont;
    m_bClickEffect = false;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CRadioButton::Destroy()
{
    if (m_LocaleObserverRegistered)
    {
        I18N::UnregisterLocaleObserver(&mu::ui::window::CRadioButton::OnLocaleChanged, this);
        m_LocaleObserverRegistered = false;
    }

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    UnRegisterButtonState();
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CRadioButton::ChangeText(const wchar_t* const* nameSlot)
{
    m_pNameSlot = nameSlot;
    m_Name = (nameSlot != nullptr && *nameSlot != nullptr) ? *nameSlot : L"";
    EnsureLocaleObserver();
}

void mu::ui::window::CRadioButton::EnsureLocaleObserver()
{
    if (m_LocaleObserverRegistered) return;
    I18N::RegisterLocaleObserver(&mu::ui::window::CRadioButton::OnLocaleChanged, this);
    m_LocaleObserverRegistered = true;
}

void mu::ui::window::CRadioButton::OnLocaleChanged(void* ctx) noexcept
{
    auto* self = static_cast<CRadioButton*>(ctx);
    if (self->m_pNameSlot != nullptr && *self->m_pNameSlot != nullptr)
    {
        self->m_Name = *self->m_pNameSlot;
    }
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void mu::ui::window::CRadioButton::ChangeRadioButtonImgState(int imgindex, bool bMouseOnImage, bool bLockImage, bool bClickEffect)
{
    int btState = 0;

    m_bClickEffect = bClickEffect;
    m_bLockImage = bLockImage;

    RegisterButtonState(BUTTON_STATE_UP, imgindex, btState++);

    if (bMouseOnImage == true)
    {
        RegisterButtonState(BUTTON_STATE_OVER, imgindex, btState++);
    }

    RegisterButtonState(BUTTON_STATE_DOWN, imgindex, btState++);

    if (bLockImage == true)
    {
        RegisterButtonState(BUTTON_STATE_LOCK, imgindex, btState++);
    }
}
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void mu::ui::window::CRadioButton::ChangeRadioButtonImgState(int imgindex, bool isDown, bool bClickEffect)
{
    m_bClickEffect = bClickEffect;
    RegisterButtonState(BUTTON_STATE_UP, imgindex, 0);
    RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 1);

    if (isDown)
    {
        ChangeFrame(BUTTON_STATE_DOWN);
    }
}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CRadioButton::ChangeRadioButtonImgState(int imgindex, bool isDown)
{
    RegisterButtonState(BUTTON_STATE_UP, imgindex, 0);
    RegisterButtonState(BUTTON_STATE_DOWN, imgindex, 1);

    if (isDown)
    {
        ChangeFrame(BUTTON_STATE_DOWN);
    }
}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

void mu::ui::window::CRadioButton::ChangeRadioButtonInfo(int x, int y, int sx, int sy)
{
    SetPos(x, y);
    SetSize(sx, sy);
}

void mu::ui::window::CRadioButton::RegisterButtonState(BUTTON_STATE eventstate, int imgindex, int btstate)
{
    ButtonInfo btinfo;
    btinfo.s_ImgIndex = imgindex;
    btinfo.s_BTstate = btstate;

    m_RadioButtonInfo.insert(std::make_pair(eventstate, btinfo));
}

void mu::ui::window::CRadioButton::UnRegisterButtonState()
{
    m_RadioButtonInfo.clear();
}

void mu::ui::window::CRadioButton::ChangeImgColor(BUTTON_STATE eventstate, unsigned int color)
{
    if (m_RadioButtonInfo.size() != 0)
    {
        auto iter = m_RadioButtonInfo.find(static_cast<int>(eventstate));

        if (iter != m_RadioButtonInfo.end())
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

void mu::ui::window::CRadioButton::ChangeImgIndex(int imgindex, int curimgstate)
{
    m_CurImgIndex = imgindex;
    m_CurImgState = curimgstate;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    if (m_CurImgIndex != -1 && m_CurImgIndex != BITMAP_UNKNOWN)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    if (m_CurImgIndex != -1)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    {
        BITMAP_t* b = &Bitmaps[m_CurImgIndex];

        m_ImgWidth = b->Width;
        m_ImgHeight = b->Height;
    }
}

void mu::ui::window::CRadioButton::ChangeFrame(BUTTON_STATE eventstate)
{
    m_EventState = eventstate;

    if (m_RadioButtonInfo.size() != 0)
    {
        auto iter = m_RadioButtonInfo.find(static_cast<int>(GetBTState()));

        if (iter != m_RadioButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;

            ChangeImgIndex(info.s_ImgIndex, info.s_BTstate);

            m_CurImgColor = info.s_imgColor;
        }
    }

    if (m_Name.size() != 0)
    {
        if (GetBTState() == BUTTON_STATE_UP)
            ChangeTextColor(0xffB5B5B5);
        else
            ChangeTextColor(0xffFFFFFF);
    }
}

void mu::ui::window::CRadioButton::ChangeFrame()
{
    if (m_RadioButtonInfo.size() != 0)
    {
        auto iter = m_RadioButtonInfo.find(static_cast<int>(GetBTState()));

        if (iter != m_RadioButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;

            ChangeImgIndex(info.s_ImgIndex, info.s_BTstate);

            m_CurImgColor = info.s_imgColor;
        }
    }

    if (m_Name.size() != 0)
    {
        if (GetBTState() == BUTTON_STATE_UP)
            ChangeTextColor(0xffB5B5B5);
        else
            ChangeTextColor(0xffFFFFFF);
    }
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CRadioButton::ChangeButtonState(int iImgIndex, BUTTON_STATE eventstate, int iButtonState)
{
    if (m_RadioButtonInfo.size() != 0)
    {
        auto iter = m_RadioButtonInfo.find(static_cast<int>(eventstate));

        if (iter != m_RadioButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;
            info.s_ImgIndex = iImgIndex;
            info.s_BTstate = iButtonState;
        }
    }
}

void mu::ui::window::CRadioButton::ChangeButtonState(BUTTON_STATE eventstate, int iButtonState)
{
    if (m_RadioButtonInfo.size() != 0)
    {
        auto iter = m_RadioButtonInfo.find(static_cast<int>(eventstate));

        if (iter != m_RadioButtonInfo.end())
        {
            ButtonInfo& info = (*iter).second;
            info.s_BTstate = iButtonState;
        }
    }
}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

bool mu::ui::window::CRadioButton::UpdateMouseEvent(bool isGroupevent)
{
    if (IsLock())
    {
        return false;
    }

    BUTTON_STATE backevent = GetBTState();

    bool result = false;

    if (isGroupevent)
    {
        if (GetBTState() != BUTTON_STATE_DOWN)
        {
            result = RadioProcess();
            if (backevent != GetBTState())
            {
                ChangeFrame();
            }
        }
    }
    else
    {
        result = RadioProcess();
        if (backevent != GetBTState())
        {
            ChangeFrame();
        }
    }

    return result;
}

bool mu::ui::window::CRadioButton::Render()
{
    if (m_RadioButtonInfo.size() != 0)
    {
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        if (m_CurImgIndex != BITMAP_UNKNOWN)
        {
            RenderStateImage(static_cast<int>(m_CurImgIndex), static_cast<int>(m_CurImgState), 2, m_CurImgColor);
        }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        if (m_ImgWidth < m_ImgHeight)
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
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        g_pRenderText->SetFont(m_hTextFont);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        g_pRenderText->SetFont(g_hFont);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        const SIZE Fontsize = g_pRenderText->MeasureText(m_Name.c_str(), static_cast<int>(m_Name.size()));

        int x = m_Pos.x + ((m_Size.x / 2) - (Fontsize.cx / 2));
        int y = m_Pos.y + ((m_Size.y / 2) - (Fontsize.cy / 2));

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        if ((m_bClickEffect == true) && GetBTState() == BUTTON_STATE_DOWN)
        {
            RenderTextWithColors(m_Name.c_str(), x + 1, y + 1, m_Size.x, 0, m_hTextFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
        }
        else
        {
            RenderTextWithColors(m_Name.c_str(), x, y, m_Size.x, 0, m_hTextFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
        }
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        RenderTextWithColors(m_Name.c_str(), x, y, m_Size.x, 0, g_hFont, m_NameColor, m_NameBackColor, RT3_SORT_LEFT);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// CRadioGroupButton
//////////////////////////////////////////////////////////////////////

CRadioGroupButton::CRadioGroupButton()
{
    Initialize();
}

CRadioGroupButton::~CRadioGroupButton()
{
    Destroy();
}

void CRadioGroupButton::Initialize()
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_iButtonDistance = 1;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void CRadioGroupButton::Destroy()
{
    UnRegisterRadioButton();
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void CRadioGroupButton::CreateRadioGroup(int radiocount, int imgindex, bool bFirstIndexBtnDown /* = true */,
    bool bMouseOnImage /* = false */, bool bLockImage, bool bClickEffect /* = false  */)
{
    for (int i = 0; i < radiocount; ++i)
    {
        CRadioButton* button = new CRadioButton();

        button->ChangeRadioButtonImgState(imgindex, bMouseOnImage, bLockImage, bClickEffect);
        button->ChangeRadioButtonInfo(0, 0, 0, 0);
        RegisterRadioButton(button);
    }

    int iCurIndex = -1;

    if (bFirstIndexBtnDown == true)
    {
        iCurIndex = 0;
    }

    ChangeFrame(iCurIndex);
    SetCurButtonIndex(iCurIndex);
}
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void CRadioGroupButton::CreateRadioGroup(int radiocount, int imgindex, bool bClickEffect)
{
    for (int i = 0; i < radiocount; ++i)
    {
        auto* button = new CRadioButton();

        button->ChangeRadioButtonImgState(imgindex, ((i == 0) ? true : false), bClickEffect);
        button->ChangeRadioButtonInfo(0, 0, 0, 0);
        RegisterRadioButton(button);
    }

    SetCurButtonIndex(0);
}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void CRadioGroupButton::CreateRadioGroup(int radiocount, int imgindex)
{
    for (int i = 0; i < radiocount; ++i)
    {
        CRadioButton* button = new CRadioButton();
        button->ChangeRadioButtonImgState(imgindex, ((i == 0) ? true : false));
        button->ChangeRadioButtonInfo(0, 0, 0, 0);
        RegisterRadioButton(button);
    }

    SetCurButtonIndex(0);
}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void CRadioGroupButton::ChangeRadioButtonInfo(bool iswidth, int x, int y, int sx, int sy, int iDistance/* = 1*/)
{
    int i = 0;

    m_iButtonDistance = iDistance;

    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (button)
        {
            if (iswidth) button->SetPos(x + ((sx + m_iButtonDistance) * i), y);
            else button->SetPos(x, y + ((sy + m_iButtonDistance) * i));
            button->SetSize(sx, sy);
        }

        ++i;
    }
}

void CRadioGroupButton::ChangeButtonState(BUTTON_STATE eventstate, int iButtonState)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        button->ChangeButtonState(eventstate, iButtonState);

        ++i;
    }
}

void CRadioGroupButton::ChangeButtonState(int iBtnIndex, int iImgIndex, BUTTON_STATE eventstate, int iButtonState)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i == iBtnIndex)
        {
            button->ChangeButtonState(iImgIndex, eventstate, iButtonState);
            return;
        }

        ++i;
    }
}

#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void CRadioGroupButton::ChangeRadioButtonInfo(bool iswidth, int x, int y, int sx, int sy)
{
    int i = 0;

    for (RadioButtonList::iterator iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        RadioButtonList::iterator curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (button)
        {
            if (iswidth) button->SetPos(x + ((sx + 1) * i), y);
            else button->SetPos(x, y + ((sy + 1) * i));
            button->SetSize(sx, sy);
        }

        ++i;
    }
}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

void CRadioGroupButton::ChangeRadioText(std::list<std::wstring>& textlist)
{
    auto textiter = textlist.begin();

    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        auto curtextiter = textiter;
        ++textiter;
       std::wstring text = (*curtextiter);

        button->ChangeText(text);

        if (textiter == textlist.end()) break;
    }
}

void CRadioGroupButton::ChangeRadioText(std::list<const wchar_t* const*>& slotList)
{
    auto slotIter = slotList.begin();

    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); ++iter)
    {
        if (slotIter == slotList.end()) break;

        CRadioButton* button = *iter;
        if (button != nullptr) button->ChangeText(*slotIter);

        ++slotIter;
    }
}

void CRadioGroupButton::ChangeFrame(int buttonIndex)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i != buttonIndex)
        {
            button->ChangeFrame(BUTTON_STATE_UP);
        }
        else
        {
            SetCurButtonIndex(i);
            button->ChangeFrame(BUTTON_STATE_DOWN);
        }
        ++i;
    }
}

// �߰� : Pruarin(07.09.03)
void CRadioGroupButton::LockButtonindex(int buttonIndex)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i == buttonIndex)
        {
            button->Lock();
            break;
        }

        ++i;
    }
}

#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
void CRadioGroupButton::UnLockButtonIndex(int buttonIndex)
{
    int i = 0;
    for (RadioButtonList::iterator iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        RadioButtonList::iterator curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i == buttonIndex)
        {
            button->UnLock();
            break;
        }

        ++i;
    }
}
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

void CRadioGroupButton::RegisterRadioButton(CRadioButton* button)
{
    m_RadioList.push_back(button);
}

void CRadioGroupButton::UnRegisterRadioButton()
{
    for (auto iter = m_RadioList.rbegin(); iter != m_RadioList.rend(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        SAFE_DELETE(button);
    }

    m_RadioList.clear();
}

int CRadioGroupButton::UpdateMouseEvent()
{
    int i = 0;

    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (button->UpdateMouseEvent(true))
        {
            ChangeFrame(i);
            return GetCurButtonIndex();
        }

        ++i;
    }

    return static_cast<int>(RADIOGROUPEVENT_NONE);
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void CRadioGroupButton::SetFont(HFONT hFont)
{
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        button->SetFont(hFont);
    }
}

void CRadioGroupButton::SetFont(HFONT hFont, int iButtonIndex)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i == iButtonIndex)
        {
            button->SetFont(hFont);
            break;
        }

        ++i;
    }
}

POINT CRadioGroupButton::GetPos(int iButtonIndex)
{
    int i = 0;
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        if (i == iButtonIndex)
        {
            return button->GetPos();
        }
        ++i;
    }

    POINT Pos;
    Pos.x = 0;
    Pos.y = 0;

    return Pos;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

bool CRadioGroupButton::Render()
{
    for (auto iter = m_RadioList.begin(); iter != m_RadioList.end(); )
    {
        auto curiter = iter;
        ++iter;
        CRadioButton* button = (*curiter);

        button->Render();
    }

    return true;
}

mu::ui::window::CCheckBox::CCheckBox()
{
    s_ImgIndex = -1;
    m_Pos.x = 0; m_Pos.y = 0;
    m_Size.x = 15; m_Size.y = 15;
    m_Name.clear();
    m_hTextFont = g_hFont;
    m_NameColor = 0xFFFFFFFF;
    m_NameBackColor = 0x00000000;
    m_ImgWidth = 0.0;
    m_ImgHeight = 15.f;
    State = 0;
}

mu::ui::window::CCheckBox::~CCheckBox()
{
    if (m_LocaleObserverRegistered)
    {
        I18N::UnregisterLocaleObserver(&CCheckBox::OnLocaleChanged, this);
        m_LocaleObserverRegistered = false;
    }
}

void mu::ui::window::CCheckBox::CheckBoxImgState(int imgindex)
{
    s_ImgIndex = imgindex;
}

void mu::ui::window::CCheckBox::RegisterBoxState(bool eventstate)
{
    State = eventstate;
}

void mu::ui::window::CCheckBox::ChangeText(std::wstring btname)
{
    m_pNameSlot = nullptr;
    m_Name = btname;
}

void mu::ui::window::CCheckBox::ChangeText(const wchar_t* const* nameSlot)
{
    m_pNameSlot = nameSlot;
    m_Name = (nameSlot != nullptr && *nameSlot != nullptr) ? *nameSlot : L"";
    EnsureLocaleObserver();
}

void mu::ui::window::CCheckBox::EnsureLocaleObserver()
{
    if (m_LocaleObserverRegistered) return;
    I18N::RegisterLocaleObserver(&CCheckBox::OnLocaleChanged, this);
    m_LocaleObserverRegistered = true;
}

void mu::ui::window::CCheckBox::OnLocaleChanged(void* ctx) noexcept
{
    auto* self = static_cast<CCheckBox*>(ctx);
    if (self->m_pNameSlot != nullptr && *self->m_pNameSlot != nullptr)
    {
        self->m_Name = *self->m_pNameSlot;
    }
}

void mu::ui::window::CCheckBox::CheckBoxInfo(int x, int y, int sx, int sy)
{
    m_Pos.x = x; m_Pos.y = y;
    m_Size.x = sx; m_Size.y = sy;
}

bool mu::ui::window::CCheckBox::GetBoxState()
{
    return State;
}

void mu::ui::window::CCheckBox::Render()
{
    EnableAlphaTest();

    // Own small parallel version of CBaseButton::RenderStateImage's pattern (H item 6) -- see
    // Button.h's m_sprite comment for why this isn't shared code. Two vertically-stacked frames:
    // checked = frame 0 (top), unchecked = frame 1 (bottom) -- matches the two RenderImage() calls
    // this replaces, which drew the same one frame twice (both offsets were identical:
    // (State) ? 0.0 : m_Size.y).
    if (s_ImgIndex >= 0)
    {
        // See CBaseButton::RenderStateImage's comment -- CSprite takes LOGICAL coordinates and
        // applies scale + the live screen offset itself inside Render(); pre-scaling here (as an
        // earlier version of this port did) double-applied the offset. Rebuilt on a WindowHeight
        // or active-scale change for the same reason as that comment explains.
        const UI::Scaling::Transform& transform = UI::Scaling::GetActiveTransform();

        if (m_spriteImgIndex != s_ImgIndex || m_spriteFrameSize.x != m_Size.x || m_spriteFrameSize.y != m_Size.y ||
            m_spriteWindowHeight != WindowHeight ||
            m_spriteScaleX != transform.scaleX || m_spriteScaleY != transform.scaleY)
        {
            SFrameCoord frames[2] = { { 0, 0 }, { 0, m_Size.y } };
            m_sprite.Create(m_Size.x, m_Size.y, s_ImgIndex, 2, frames,
                             0, 0, false, SPR_SIZING_DATUMS_LT, transform.scaleX, transform.scaleY);
            // See CBaseButton::RenderStateImage's comment -- Create() locks the legal frame range
            // to [0,0]; without this, SetNowFrame(1) below would never actually take effect.
            m_sprite.SetAction(0, 1);

            m_spriteImgIndex = s_ImgIndex;
            m_spriteFrameSize = m_Size;
            m_spriteWindowHeight = WindowHeight;
            m_spriteScaleX = transform.scaleX;
            m_spriteScaleY = transform.scaleY;
        }

        m_sprite.SetNowFrame(State ? 0 : 1);
        m_sprite.SetPosition(m_Pos.x, m_Pos.y);
        m_sprite.SetSize(m_Size.x, m_Size.y);

        m_sprite.Show(true);
        m_sprite.Render();
    }

    g_pRenderText->SetFont(m_hTextFont);
    g_pRenderText->SetTextColor(m_NameColor);
    g_pRenderText->SetBgColor(m_NameBackColor);
    g_pRenderText->RenderText(m_Pos.x + m_Size.x + 1, m_Pos.y + 4, m_Name.c_str(), 0, 0);
}

bool mu::ui::window::CCheckBox::UpdateMouseEvent()
{
    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_Size.x, m_Size.y))
    {
        if (IsRelease(VK_LBUTTON))
        {
            State = !State;
            return TRUE;
        }
    }
    return 0;
}
