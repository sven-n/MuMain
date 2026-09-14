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

    // Same as CreateOkMessageBox, plus a bold title row above the body -- models
    // CMsgBoxIGSCommon::Initialize(pszTitle, pszText) (GameShop/MsgBoxIGSCommon.h), the one native
    // family with a genuine title/body split, used as a generic OK-only notice/error popup at
    // ~50 call sites (mostly WSclient.cpp's cash-shop response handlers).
    bool CreateOkMessageBoxWithTitle(const std::wstring& strTitle, const std::wstring& strMsg);

    int IsPurchaseShop();
#define g_IsPurchaseShop mu::ui::window::IsPurchaseShop()

    bool CheckMouseIn(int x, int y, int width, int height);

    void RenderImage(GLuint uiImageType, float x, float y, float width, float height);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, float uw, float vh, DWORD color = RGBA(255, 255, 255, 255));

    // Scaled sprite blit: maps source texels (sx,sy,sw,sh) onto dest pixels (x,y,width,height),
    // unlike RenderImage above which samples 1:1 so a smaller size just crops.
    void RenderImageStretch(GLuint uiImageType, float x, float y, float width, float height,
                            float sx, float sy, float sw, float sh, DWORD color = RGBA(255, 255, 255, 255));

    float RenderNumber(float x, float y, int iNum, float fScale = 1.0f);

    // Renders text with an explicit color/background, restoring g_pRenderText's previous
    // state afterward so callers don't leak state into whatever renders next.
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
