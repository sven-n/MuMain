// NewUIButton.h: interface for the CNewUIButton class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)
#define AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_

#pragma once

namespace SEASON3B
{
    enum BUTTON_STATE
    {
        BUTTON_STATE_UP = 0,
        BUTTON_STATE_DOWN,
        BUTTON_STATE_OVER,
    };

    enum RADIOGROUPEVENT
    {
        RADIOGROUPEVENT_NONE = -1,
    };

    struct ButtonInfo
    {
        int s_ImgIndex;
        int s_BTstate;
        unsigned int s_imgColor;
        ButtonInfo() : s_ImgIndex(0), s_BTstate(0), s_imgColor(0xffffffff) { }
    };

    typedef std::map<int, ButtonInfo>  ButtonStateMap;

    class CNewUIBaseButton
    {
    public:
        CNewUIBaseButton();
        virtual ~CNewUIBaseButton();

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
        bool RadioProcess();
        bool Process();

    protected:
        POINT					m_Pos;
        POINT					m_Size;
        BUTTON_STATE			m_EventState;
        bool					m_Lock;
    };

    inline
        void CNewUIBaseButton::SetPos(const POINT& pos)
    {
        m_Pos = pos;
    }

    inline
        void CNewUIBaseButton::SetSize(const POINT& size)
    {
        m_Size = size;
    }

    inline
        const POINT& CNewUIBaseButton::GetPos()
    {
        return m_Pos;
    }

    inline
        const POINT& CNewUIBaseButton::GetSize()
    {
        return m_Size;
    }

    inline
        const BUTTON_STATE CNewUIBaseButton::GetBTState()
    {
        return m_EventState;
    }

#ifndef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE			// #ifndef
    inline
        void CNewUIBaseButton::Lock()
    {
        m_Lock = true;
    }

    inline
        void CNewUIBaseButton::UnLock()
    {
        m_Lock = false;
    }
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE

    inline
        bool CNewUIBaseButton::IsLock()
    {
        return m_Lock;
    }

    class CNewUIButton : public CNewUIBaseButton
    {
    public:
        CNewUIButton();
        virtual ~CNewUIButton();
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
        void SetFont(HFONT hFont);

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonState(BUTTON_STATE eventstate, int iButtonState);
        void MoveTextPos(int iX, int iY);
        void MoveTextTipPos(int iX, int iY);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        void ChangeTextBackColor(const DWORD bcolor);
        void ChangeTextColor(const DWORD color);

        void ChangeToolTipText(std::wstring tooltiptext, bool istoppos = false);
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
       std::wstring		m_TooltipText;

        HFONT					m_hTextFont;
        HFONT					m_hToolTipFont;
        DWORD					m_NameColor;
        DWORD					m_NameBackColor;
        DWORD					m_TooltipTextColor;

        int						m_CurImgIndex;
        int						m_CurImgState;

        WORD					m_ImgWidth;
        WORD					m_ImgHeight;

        unsigned int			m_CurImgColor;
        bool					m_IsTopPos;
        bool                    m_IsImgWidth;

        unsigned char			m_fAlpha;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        bool					m_bClickEffect;
        int						m_iMoveTextPosX;
        int						m_iMoveTextPosY;
        int						m_iMoveTextTipPosX;
        int						m_iMoveTextTipPosY;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    };

    inline void CNewUIButton::ChangeImgWidth(bool isimgwidth)
    {
        m_IsImgWidth = isimgwidth;
    }

    inline void CNewUIButton::ChangeText(std::wstring btname)
    {
        m_Name = btname;
    }

    inline
        void CNewUIButton::SetFont(HFONT hFont)
    {
        m_hTextFont = hFont;
    }

    inline
        void CNewUIButton::ChangeTextBackColor(const DWORD bcolor)
    {
        m_NameBackColor = bcolor;
    }

    inline
        void CNewUIButton::ChangeTextColor(const DWORD color)
    {
        m_NameColor = color;
    }

    inline
        void CNewUIButton::ChangeToolTipText(std::wstring tooltiptext, bool istoppos)
    {
        m_TooltipText = tooltiptext;
        m_IsTopPos = istoppos;
        //m_hToolTipFont = g_hFont;
    }

    inline
        void CNewUIButton::SetToolTipFont(HFONT hFont)
    {
        m_hToolTipFont = hFont;
    }

    inline
        void CNewUIButton::ChangeToolTipTextColor(const DWORD color)
    {
        m_TooltipTextColor = color;
    }

    class CNewUIRadioButton : public CNewUIBaseButton
    {
    public:
        CNewUIRadioButton();
        virtual ~CNewUIRadioButton();

    public:
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeRadioButtonImgState(int imgindex, bool isDown = false, bool bClickEffect = false);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeRadioButtonImgState(int imgindex, bool isDown = false);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeRadioButtonInfo(int x, int y, int sx, int sy);
        void ChangeFrame(BUTTON_STATE eventstate);

    public:
        void ChangeImgColor(BUTTON_STATE eventstate, unsigned int color);
        void ChangeText(std::wstring btname);
        void ChangeTextBackColor(const DWORD bcolor);
        void ChangeTextColor(const DWORD color);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeButtonState(BUTTON_STATE eventstate, int iButtonState);
        void ChangeButtonState(int iImgIndex, BUTTON_STATE eventstate, int iButtonState);
        void SetFont(HFONT hFont);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    public:
        void RegisterButtonState(BUTTON_STATE eventstate, int imgindex, int btstate);
        void UnRegisterButtonState();

    public:
        bool UpdateMouseEvent(bool isGroupevent = false);

    public:
        bool Render();

    private:
        void ChangeImgIndex(int imgindex, int curimgstate = 0);
        void ChangeFrame();
        void Initialize();
        void Destroy();

    private:
        ButtonStateMap           m_RadioButtonInfo;
       std::wstring		 m_Name;

        DWORD					m_NameColor;
        DWORD					m_NameBackColor;
        DWORD					m_CurImgIndex;
        DWORD					m_CurImgState;
        DWORD					m_ImgWidth;
        DWORD					m_ImgHeight;
        DWORD					m_CurImgColor;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        HFONT					m_hTextFont;
        bool					m_bClickEffect;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
        bool					m_bLockImage;
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
    };

    inline
        void CNewUIRadioButton::ChangeText(std::wstring btname)
    {
        m_Name = btname;
    }

    inline
        void CNewUIRadioButton::ChangeTextBackColor(const DWORD bcolor)
    {
        m_NameBackColor = bcolor;
    }

    inline
        void CNewUIRadioButton::ChangeTextColor(const DWORD color)
    {
        m_NameColor = color;
    }

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    inline
        void CNewUIRadioButton::SetFont(HFONT hFont)
    {
        m_hTextFont = hFont;
    }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    class CNewUIRadioGroupButton
    {
    public:
        CNewUIRadioGroupButton();
        virtual ~CNewUIRadioGroupButton();

    public:
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void CreateRadioGroup(int radiocount, int imgindex, bool bClickEffect = false);
        void ChangeRadioButtonInfo(bool iswidth, int x, int y, int sx, int sy, int iDistance = 1);
        void MoveTextPos(int iX, int iY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void CreateRadioGroup(int radiocount, int imgindex);
        void ChangeRadioButtonInfo(bool iswidth, int x, int y, int sx, int sy);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void ChangeRadioText(std::list<std::wstring>& textlist);
        void ChangeFrame(int buttonIndex);
        void LockButtonindex(int buttonIndex);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetFont(HFONT hFont, int iButtonIndex);
        void SetFont(HFONT hFont);
        void ChangeButtonState(BUTTON_STATE eventstate, int iButtonState);
        void ChangeButtonState(int iBtnIndex, int iImgIndex,
            BUTTON_STATE eventstate, int iButtonState);
        POINT GetPos(int iButtonIndex);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        void SetFontIndex(int buttonIndex, HFONT hFont);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    public:
        void RegisterRadioButton(CNewUIRadioButton* button);
        void UnRegisterRadioButton();

    public:
        const int GetCurButtonIndex();

    public:
        int UpdateMouseEvent();

    public:
        bool Render();

    private:
        void SetCurButtonIndex(int index);
        void Initialize();
        void Destroy();

    private:
        typedef std::list<CNewUIRadioButton*>      RadioButtonList;

    private:
        RadioButtonList				m_RadioList;
        DWORD						m_CurButtonIndex;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        int							m_iButtonDistance;			// 버튼과 버튼사이의 간격
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    };

    inline
        void CNewUIRadioGroupButton::SetCurButtonIndex(int index)
    {
        m_CurButtonIndex = index;
    }

    inline
        const int CNewUIRadioGroupButton::GetCurButtonIndex()
    {
        return m_CurButtonIndex;
    }

    class CNewUICheckBox
    {
    public:
        CNewUICheckBox();
        virtual ~CNewUICheckBox();
        void CheckBoxImgState(int imgindex);
        void RegisterBoxState(bool eventstate);
        void ChangeText(std::wstring btname);
        void CheckBoxInfo(int x, int y, int sx, int sy);
        bool GetBoxState();

        void Render();
        bool UpdateMouseEvent();
    private:
        int						s_ImgIndex;
        POINT					m_Pos;
        POINT					m_Size;
       std::wstring		m_Name;
        HFONT					m_hTextFont;
        DWORD					m_NameColor;
        DWORD					m_NameBackColor;
        float					m_ImgWidth;
        float					m_ImgHeight;
        bool					State;
    };
};

#endif // !defined(AFX_NEWUIBUTTON_H__7DC4490D_D859_4159_9EE5_FBC4ECDE209A__INCLUDED_)
