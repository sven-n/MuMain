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

    // Was CSystemMenuMsgBoxLayout (CustomMessageBox.h) -- proof-of-concept port onto
    // CGenericMenuDialog (UI/Dialogs/GenericMenuDialog.h), the sibling N-button-menu primitive.
    // Two call sites (HotKey.cpp's Esc handler, WindowMenu.cpp's menu-item 0), neither passes any
    // parameters, so this is a plain free function like CreateOkMessageBox() above.
    void ShowSystemMenuDialog();

    // Batch of 9 more CustomMessageBox.h "multi-option menu" classes ported onto the same
    // CGenericMenuDialog primitive, same reasoning/shape as ShowSystemMenuDialog() above -- see
    // docs/rmlui-ui-system/dialog-migration-plan.md's "Multi-option menus" entry. All plain free
    // functions (no parameters; each reads the globals it needs directly, same as the native
    // classes it replaces).
    void ShowChaosMixMenuDialog();
    void ShowTrainerMenuDialog();
    void ShowTrainerRecoverDialog();
    void ShowSeedMasterMenuDialog();
    void ShowSeedInvestigatorMenuDialog();
    void ShowResetCharacterPointDialog();
    void ShowDelgardoMainMenuDialog();
    void ShowLuckyTradeMenuDialog();
    // No live callers -- same as its native predecessor CCherryBlossomMsgBox (grep-confirmed zero
    // CreateMessageBox call sites for it even before this port). Kept for parity with the other 8.
    void ShowCherryBlossomMenuDialog();

    // CGemIntegrationMsgBox/CGemIntegrationUnityMsgBox ported onto CGenericMenuDialog as 3 chained
    // free functions instead of 1:1 class replacement -- native's single CGemIntegrationUnityMsgBox
    // swapped its own button set in place (ResetWndSize()) between a jewel-type grid and a
    // mix-amount grid; CGenericMenuDialog's buttons always close on click, so that in-place swap
    // becomes "close this menu, open a different one" via the same reentrant-Show()-during-click
    // chaining ShowTrainerMenuDialog()/ShowTrainerRecoverDialog() already prove. COMGEM
    // (GameLogic/Items/CComGem.h) is the shared state the 3 phases read/write, same as native.
    // CGemIntegrationDisjointMsgBox stays native (embedded live inventory list-selection widget,
    // a different problem chaining doesn't solve) -- see dialog-migration-plan.md.
    void ShowGemIntegrationMenuDialog();  // entry selector: Unity / Disjoint / Cancel
    void ShowGemIntegrationJewelDialog(); // Unity phase 1: pick a jewel type
    void ShowGemIntegrationMixDialog();   // Unity phase 2: pick a mix-amount tier

    // CElpisMsgBox ported onto CGenericMenuDialog -- unlike the other consumers above, the button
    // set here never changes; only the body text above it does (native's own m_iMessageType), so
    // this is the same reentrant-Show()-during-click chaining reused to swap `lines`, not buttons.
    // iMessageType selects which blurb to show (0 = default prompt; otherwise one of the
    // MSGBOX_EVENT_USER_CUSTOM_ELPIS_* values, MessageBox.h) -- the "About" buttons' onClick calls
    // this same function again with a different value instead of opening a second dialog.
    void ShowElpisMenuDialog(int iMessageType = 0);

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
