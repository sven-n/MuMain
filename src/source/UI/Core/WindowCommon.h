//////////////////////////////////////////////////////////////////////
// WindowCommon.h: shared free functions (message boxes, image blitting,
// key-state queries) for the mu::ui::window tier.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
#define AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_

#pragma once

namespace mu::ui::window
{
    bool CreateOkMessageBox(const std::wstring& strMsg, DWORD dwColor = 0xffffffff, float fPriority = 3.f);

    int IsPurchaseShop();
#define g_IsPurchaseShop mu::ui::window::IsPurchaseShop()

    bool CheckMouseIn(int x, int y, int width, int height);

    void RenderImage(GLuint uiImageType, float x, float y, float width, float height);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, float uw, float vh, DWORD color = RGBA(255, 255, 255, 255));

    // Scaled sprite blit: maps a source region (sx,sy,sw,sh, in texels) onto the
    // dest rect (x,y,width,height, in pixels). Unlike RenderImage above — where
    // width/height also set the sampled texel extent (1:1, so a smaller size just
    // crops) — this lets a fixed-size sprite be drawn larger or smaller.
    void RenderImageStretch(GLuint uiImageType, float x, float y, float width, float height,
                            float sx, float sy, float sw, float sh, DWORD color = RGBA(255, 255, 255, 255));

    float RenderNumber(float x, float y, int iNum, float fScale = 1.0f);

    // Renders text with an explicit color/background, restoring g_pRenderText's previous
    // color/background afterward so callers don't leak state into whatever renders next.
    // Shared by CButton and CTooltip (Widgets/Window/{Button,Tooltip}.h) -- both draw a run of
    // text in a color that differs from whatever the shared renderer was last set to.
    void RenderTextWithColors(const wchar_t* text, int x, int y, int width, int height, HFONT font,
                              DWORD color, DWORD backColor, int sort);

    bool IsNone(int iVirtKey);
    bool IsRelease(int iVirtKey);
    bool IsPress(int iVirtKey);
    bool IsRepeat(int iVirtKey);

    class CNewKeyInput
    {
        struct INPUTSTATEINFO
        {
            BYTE byKeyState;
        } m_pInputInfo[256];

#ifndef ASG_FIX_ACTIVATE_APP_INPUT
        void Init();
#endif

    public:
        enum KEY_STATE
        {
            KEY_NONE = 0,
            KEY_RELEASE,
            KEY_PRESS,
            KEY_REPEAT
        };
        ~CNewKeyInput();

        static CNewKeyInput* GetInstance();
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
        void Init();
#endif
        void ScanAsyncKeyState();

        bool IsNone(int iVirtKey);
        bool IsRelease(int iVirtKey);
        bool IsPress(int iVirtKey);
        bool IsRepeat(int iVirtKey);
        void SetKeyState(int iVirtKey, KEY_STATE KeyState);

    protected:
        CNewKeyInput();
    };
}

#define g_pNewKeyInput	mu::ui::window::CNewKeyInput::GetInstance()

#endif // !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
