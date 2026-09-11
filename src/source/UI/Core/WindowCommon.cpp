
#include "stdafx.h"
#include "Core/Input/KeyState.h"

#include "UI/Core/WindowCommon.h"
#include "UI/Widgets/Window/RenderNumber.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Widgets/UIControls.h"  // g_pRenderText
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#include "../MuEditor/Core/MuEditorCore.h"
#endif

extern int MouseX, MouseY;
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
extern bool g_bWndActive;
#endif // ASG_FIX_ACTIVATE_APP_INPUT

bool mu::ui::window::CreateOkMessageBox(const std::wstring& strMsg, DWORD dwColor, float fPriority)
{
    CCommonMessageBox* pMsgBox = g_MessageBox->NewMessageBox(MSGBOX_CLASS(CCommonMessageBox));
    if (pMsgBox)
    {
        return pMsgBox->Create(MSGBOX_COMMON_TYPE_OK, strMsg, dwColor);
    }
    return false;
}

int mu::ui::window::IsPurchaseShop()
{
    if (g_pMyShopInventory->IsVisible())
    {
        return 1;
    }
    else if (g_pPurchaseShopInventory->IsVisible())
    {
        return 2;
    }

    return -1;
}

bool mu::ui::window::CheckMouseIn(int x, int y, int width, int height)
{
    if (MouseX >= x && MouseX < x + width && MouseY >= y && MouseY < y + height)
        return true;
    return false;
}

void mu::ui::window::RenderImage(GLuint uiImageType, float x, float y, float width, float height)
{
    BITMAP_t* pImage = &Bitmaps[uiImageType];

    float u, v, uw, vh;

    u = 0.5f / (float)pImage->Width;
    v = 0.5f / (float)pImage->Height;
    uw = (width - 0.5f) / (float)pImage->Width;
    vh = (height - 0.5f) / (float)pImage->Height;

    RenderBitmap(uiImageType, x, y, width, height, u, v, uw - u, vh - v);
}

void mu::ui::window::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,
                           float uw, float vh, DWORD color)
{
    RenderColorBitmap(uiImageType, x, y, width, height, su, sv, uw, vh, color);
}

void mu::ui::window::RenderImageStretch(GLuint uiImageType, float x, float y, float width, float height,
                                  float sx, float sy, float sw, float sh, DWORD color)
{
    BITMAP_t* pImage = &Bitmaps[uiImageType];

    // Normalize source texels to UV; GL scales (sw x sh) onto the (width x height) quad.
    float u  = (sx + 0.5f) / (float)pImage->Width;
    float v  = (sy + 0.5f) / (float)pImage->Height;
    float uw = (sw - 1.0f) / (float)pImage->Width;
    float vh = (sh - 1.0f) / (float)pImage->Height;

    RenderColorBitmap(uiImageType, x, y, width, height, u, v, uw, vh, color);
}

void mu::ui::window::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv)
{
    BITMAP_t* pImage = &Bitmaps[uiImageType];

    float u, v, uw, vh;
    u = ((su + 0.5f) / (float)pImage->Width);
    v = ((sv + 0.5f) / (float)pImage->Height);
    uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
    vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

    RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh);
}

void mu::ui::window::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,
                           DWORD color)
{
    BITMAP_t* pImage = &Bitmaps[uiImageType];

    float u, v, uw, vh;
    u = ((su + 0.5f) / (float)pImage->Width);
    v = ((sv + 0.5f) / (float)pImage->Height);
    uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
    vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

    RenderColorBitmap(uiImageType, x, y, width, height, u, v, uw, vh, color);
}

float mu::ui::window::RenderNumber(float x, float y, int iNum, float fScale)
{
    return g_RenderNumber->RenderNumber(x, y, iNum, fScale);
}

void mu::ui::window::RenderTextWithColors(const wchar_t* text, int x, int y, int width, int height,
                                          HFONT font, DWORD color, DWORD backColor, int sort)
{
    g_pRenderText->SetFont(font);

    DWORD backupTextColor = g_pRenderText->GetTextColor();
    DWORD backupBgColor = g_pRenderText->GetBgColor();

    g_pRenderText->SetTextColor(color);
    g_pRenderText->SetBgColor(backColor);
    g_pRenderText->RenderText(x, y, text, width, height, sort);

    g_pRenderText->SetTextColor(backupTextColor);
    g_pRenderText->SetBgColor(backupBgColor);
}

bool mu::ui::window::IsNone(int iVirtKey)
{
    return g_pNewKeyInput->IsNone(iVirtKey);
}

bool mu::ui::window::IsRelease(int iVirtKey)
{
    return g_pNewKeyInput->IsRelease(iVirtKey);
}

bool mu::ui::window::IsPress(int iVirtKey)
{
    return g_pNewKeyInput->IsPress(iVirtKey);
}

bool mu::ui::window::IsRepeat(int iVirtKey)
{
    return g_pNewKeyInput->IsRepeat(iVirtKey);
}

mu::ui::window::CNewKeyInput::CNewKeyInput()
{
    Init();
}

mu::ui::window::CNewKeyInput::~CNewKeyInput() {}

void mu::ui::window::CNewKeyInput::Init()
{
    memset(&m_pInputInfo, 0, sizeof(INPUTSTATEINFO) * 256);
}

mu::ui::window::CNewKeyInput* mu::ui::window::CNewKeyInput::GetInstance()
{
    static mu::ui::window::CNewKeyInput s_Instance;
    return &s_Instance;
}

void mu::ui::window::CNewKeyInput::ScanAsyncKeyState()
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
    if (!g_bWndActive)
        return;
#endif // ASG_FIX_ACTIVATE_APP_INPUT

#ifdef _EDITOR
    // While ImGui wants keyboard focus, block game keyboard input at the source.
    if (g_MuEditorCore.IsEnabled())
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard)
        {
            for (int key = 0; key < 256; key++)
            {
                m_pInputInfo[key].byKeyState = KEY_NONE;
            }
            return;
        }
    }
#endif // _EDITOR

    for (int key = 0; key < 256; key++)
    {
        if (Core::Input::IsKeyDown(key))
        {
            if (m_pInputInfo[key].byKeyState == KEY_NONE || m_pInputInfo[key].byKeyState == KEY_RELEASE)
            {
                // press event (key was up before but down now)
                m_pInputInfo[key].byKeyState = KEY_PRESS;
            }
            else if (m_pInputInfo[key].byKeyState == KEY_PRESS)
            {
                // drag event (key is still down)
                m_pInputInfo[key].byKeyState = KEY_REPEAT;
            }
        }
        else // Key is not currently pressed
        {
            if (m_pInputInfo[key].byKeyState == KEY_REPEAT || m_pInputInfo[key].byKeyState == KEY_PRESS)
            {
                // release event (key was down before but up now)
                m_pInputInfo[key].byKeyState = KEY_RELEASE;
            }
            else if (m_pInputInfo[key].byKeyState == KEY_RELEASE)
            {
                m_pInputInfo[key].byKeyState = KEY_NONE;
            }
        }
    }

    // Mouse press-edge flag is cleared in PollEvents(), not here -- this runs before
    // CInput::Update(), so clearing here would drop fast clicks (DOWN+UP in one batch).

#ifdef _EDITOR
    // Must stay inside the editor guard: outside the editor nothing sets
    // g_bEnterPressed=true, so this would clear every Enter press before chat-open
    // logic saw it (previously broke Enter-to-chat on non-editor SDL3 builds).
    if (IsPress(VK_RETURN) && IsEnterPressed() == false)
    {
        m_pInputInfo[VK_RETURN].byKeyState = KEY_NONE;
    }
    SetEnterPressed(false);
#endif
}

bool mu::ui::window::CNewKeyInput::IsNone(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
    if (!g_bWndActive)
        return false;
#endif // ASG_FIX_ACTIVATE_APP_INPUT
    return (m_pInputInfo[iVirtKey].byKeyState == KEY_NONE) ? true : false;
}

bool mu::ui::window::CNewKeyInput::IsRelease(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
    if (!g_bWndActive)
        return false;
#endif // ASG_FIX_ACTIVATE_APP_INPUT
    return (m_pInputInfo[iVirtKey].byKeyState == KEY_RELEASE) ? true : false;
}

bool mu::ui::window::CNewKeyInput::IsPress(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
    if (!g_bWndActive)
        return false;
#endif // ASG_FIX_ACTIVATE_APP_INPUT
    return (m_pInputInfo[iVirtKey].byKeyState == KEY_PRESS) ? true : false;
}

bool mu::ui::window::CNewKeyInput::IsRepeat(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
    if (!g_bWndActive)
        return false;
#endif // ASG_FIX_ACTIVATE_APP_INPUT
    return (m_pInputInfo[iVirtKey].byKeyState == KEY_REPEAT) ? true : false;
}

void mu::ui::window::CNewKeyInput::SetKeyState(int iVirtKey, KEY_STATE KeyState)
{
    m_pInputInfo[iVirtKey].byKeyState = KeyState;
}