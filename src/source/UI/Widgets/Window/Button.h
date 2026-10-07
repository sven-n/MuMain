
#if !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)
#define AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_

#pragma once

#include "UI/Widgets/Window/Tooltip.h"
#include "Render/Sprites/Sprite.h"

namespace mu::ui::window
{
    enum BUTTON_STATE
    {
        BUTTON_STATE_UP = 0,
        BUTTON_STATE_DOWN,
        BUTTON_STATE_OVER,
    };

    struct ButtonInfo
    {
        int s_ImgIndex;
        int s_BTstate;
        unsigned int s_imgColor;
        ButtonInfo() : s_ImgIndex(0), s_BTstate(0), s_imgColor(0xffffffff) { }
    };

    typedef std::map<int, ButtonInfo>  ButtonStateMap;

    class CBaseButton
    {
    public:
        CBaseButton();
        virtual ~CBaseButton();

    public:
        void SetPos(const POINT& pos);
        void SetSize(const POINT& size);
        void SetPos(int x, int y);
        void SetSize(int sx, int sy);

    public:
        const POINT& GetPos();
        const POINT& GetSize();
        const BUTTON_STATE GetBTState();

    public:
        void Lock();
        void UnLock();
        bool IsLock();

    public:
        bool Process();

    protected:
        // Shared CSprite-driven rendering for the state-indexed, vertically-stacked-frame image
        // CButton draws. CSprite bakes scale/WindowHeight in at Create()
        // time, so the sprite must be rebuilt whenever those go stale.
        void RenderStateImage(int imgIndex, int frame, int frameCount, unsigned int color);

    private:
        // Hit-tests via WindowGeometry; built fresh from m_Pos/m_Size each call rather than cached.
        bool IsMouseIn() const;

        CSprite m_sprite;
        int     m_spriteImgIndex = -1;
        int     m_spriteFrameCount = -1;
        POINT   m_spriteFrameSize{ 0, 0 };
        unsigned int m_spriteWindowHeight = 0;
        float   m_spriteScaleX = 0.0f;
        float   m_spriteScaleY = 0.0f;

    protected:
        POINT					m_Pos;
        POINT					m_Size;
        BUTTON_STATE			m_EventState;
        bool					m_Lock;
    };

    inline
        void CBaseButton::SetPos(const POINT& pos)
    {
        m_Pos = pos;
    }

    inline
        void CBaseButton::SetSize(const POINT& size)
    {
        m_Size = size;
    }

    inline
        const POINT& CBaseButton::GetPos()
    {
        return m_Pos;
    }

    inline
        const POINT& CBaseButton::GetSize()
    {
        return m_Size;
    }

    inline
        const BUTTON_STATE CBaseButton::GetBTState()
    {
        return m_EventState;
    }

#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE			// #ifndef
    inline
        void CBaseButton::Lock()
    {
        m_Lock = true;
    }

    inline
        void CBaseButton::UnLock()
    {
        m_Lock = false;
    }
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

    inline
        bool CBaseButton::IsLock()
    {
        return m_Lock;
    }

    class CButton : public CBaseButton
    {
    public:
        CButton();
        virtual ~CButton();
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonImgState(bool imgregister, int imgindex, bool overflg = false, bool isimgwidth = false, bool bClickEffect = false);

#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonImgState(bool imgregister, int imgindex,
            bool overflg = false, bool isimgwidth = false);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonInfo(int x, int y, int sx, int sy);

    private:
        void Initialize();
        void Destroy();

    public:
        void RegisterButtonState(BUTTON_STATE eventstate, int imgindex, int btstate);
        void UnRegisterButtonState();

    public:
        void ChangeImgColor(BUTTON_STATE eventstate, unsigned int color);
        void ChangeText(std::wstring btname);
        // Slot overload: stores a pointer to an I18N variable so the label refreshes on locale change.
        void ChangeText(const wchar_t* const* nameSlot);
        void SetFont(HFONT hFont);

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonState(BUTTON_STATE eventstate, int iButtonState);
        void MoveTextPos(int iX, int iY);
        void MoveTextTipPos(int iX, int iY);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        void ChangeTextBackColor(const DWORD bcolor);
        void ChangeTextColor(const DWORD color);

        // Forwards to the owned CTooltip; kept as CButton's own method names for existing call sites.
        void ChangeToolTipText(std::wstring tooltiptext, bool istoppos = false);
        // Slot overload — see ChangeText(const wchar_t* const*).
        void ChangeToolTipText(const wchar_t* const* tooltipSlot, bool istoppos = false);
        void ChangeToolTipTextColor(const DWORD color);
        void SetToolTipFont(HFONT hFont);

        void ChangeImgWidth(bool isimgwidth);
        void ChangeImgIndex(int imgindex, int curimgstate = 0);
        void ChangeAlpha(unsigned char fAlpha, bool isfontalph = true);
        void ChangeAlpha(float fAlpha, bool isfontalph = true);
    public:
        bool UpdateMouseEvent();

    public:
        bool Render(bool RendOption = false);

    private:
        void ChangeFrame();

    private:
        ButtonStateMap           m_ButtonInfo;

    private:
       std::wstring		m_Name;
       // When set (by the slot overload of ChangeText), the cached string refreshes on locale change.
       const wchar_t* const* m_pNameSlot = nullptr;
       // Whether I18N::RegisterLocaleObserver has been called, so EnsureLocaleObserver is idempotent.
       bool                  m_LocaleObserverRegistered = false;

       // Owned tooltip; ChangeToolTipText()/etc. just forward into it.
       CTooltip              m_tooltip;

        HFONT					m_hTextFont;
        DWORD					m_NameColor;
        DWORD					m_NameBackColor;

        int						m_CurImgIndex;
        int						m_CurImgState;

        WORD					m_ImgWidth;
        WORD					m_ImgHeight;

        unsigned int			m_CurImgColor;
        bool                    m_IsImgWidth;

        unsigned char			m_fAlpha;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        bool					m_bClickEffect;
        int						m_iMoveTextPosX;
        int						m_iMoveTextPosY;
        int						m_iMoveTextTipPosX;
        int						m_iMoveTextTipPosY;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    private:
        void EnsureLocaleObserver();
        static void OnLocaleChanged(void* ctx) noexcept;
    };

    inline void CButton::ChangeImgWidth(bool isimgwidth)
    {
        m_IsImgWidth = isimgwidth;
    }

    inline void CButton::ChangeText(std::wstring btname)
    {
        // Drop any prior I18N slot binding so the locale observer won't clobber this literal string.
        m_pNameSlot = nullptr;
        m_Name = btname;
    }

    inline
        void CButton::SetFont(HFONT hFont)
    {
        m_hTextFont = hFont;
    }

    inline
        void CButton::ChangeTextBackColor(const DWORD bcolor)
    {
        m_NameBackColor = bcolor;
    }

    inline
        void CButton::ChangeTextColor(const DWORD color)
    {
        m_NameColor = color;
    }

    inline
        void CButton::ChangeToolTipText(std::wstring tooltiptext, bool istoppos)
    {
        m_tooltip.SetText(std::move(tooltiptext));
        m_tooltip.SetAnchorAbove(istoppos);
    }

    inline
        void CButton::SetToolTipFont(HFONT hFont)
    {
        m_tooltip.SetFont(hFont);
    }

    inline
        void CButton::ChangeToolTipTextColor(const DWORD color)
    {
        m_tooltip.SetTextColor(color);
    }
};

#endif // !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)
