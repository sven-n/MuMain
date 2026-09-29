#include "stdafx.h"
#include "UI/Events/GoldBowmanLena.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "I18N/All.h"

#include "GameLogic/Items/MixMgr.h"
#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original's CButtons: Register (newui_btn_empty, 108 x 29) at (45, 285) and the exit button
// (newui_exit_00, 36 x 29) at (13, 392), both with a tooltip above them.
constexpr float kRegisterX = 45.f;
constexpr float kRegisterY = 285.f;
constexpr float kExitX = 13.f;
constexpr float kExitY = 392.f;
constexpr float kExitWidth = 36.f;
constexpr float kExitHeight = 29.f;

// The original printed its notes through mu_swprintf() as formats.
std::wstring Formatted(const wchar_t* format)
{
    wchar_t text[100] = {};
    mu_swprintf(text, format);
    return text;
}
} // namespace

CGoldBowmanLena::CGoldBowmanLena()
{
}

CGoldBowmanLena::~CGoldBowmanLena()
{
    Release();
}

bool CGoldBowmanLena::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng) {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA, this);

    SetPos(x, y);

    m_RegisterTooltip.SetText(&I18N::Game::RegisteringRena);
    m_RegisterTooltip.SetAnchorAbove(true);
    m_ExitTooltip.SetText(&I18N::Game::Close388);
    m_ExitTooltip.SetAnchorAbove(true);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { m_View.ReloadTheme(); });

    Show(false);

    return true;
}

void CGoldBowmanLena::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    HideTooltips();
}

void CGoldBowmanLena::HideTooltips()
{
    UI::RmlBridge::Tooltip::Hide(&m_RegisterTooltip);
    UI::RmlBridge::Tooltip::Hide(&m_ExitTooltip);
}

void CGoldBowmanLena::OpeningProcess()
{
}

void CGoldBowmanLena::ClosingProcess()
{
    HideTooltips();
    g_bEventChipDialogEnable = 0;
    g_shEventChipCount = 0;
    SocketClient->ToGameServer()->SendEventChipExitDialog();
}

bool CGoldBowmanLena::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA) == false) {
        return true;
    }

    // Top-right corner close "X" (shared frame): hides + swallows the click. Register and the exit
    // button are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA))
    {
        return false;
    }

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON)) {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (mu::ui::window::IsNone(VK_LBUTTON) == false) {
            return false;
        }
        return false;
    }
    else
    {
        if (mu::ui::window::IsNone(VK_LBUTTON) == false) {
            return false;
        }
        return false;
    }

    return true;
}

bool CGoldBowmanLena::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA) == false) {
        return true;
    }

    if (mu::ui::window::IsPress(VK_ESCAPE) == true) {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA);
        return false;
    }

    return true;
}

bool CGoldBowmanLena::Update()
{
    SyncView();

    // A click RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (pressed == BUTTON_REGISTER)
    {
        int registerItem = g_pMyInventory->GetInventoryCtrl()->GetItemCount(ITEM_POTION + 21, 0);
        if (registerItem != 0)
        {
            int index = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_POTION + 21, 0);
            if (index != -1)
            {
                SocketClient->ToGameServer()->SendEventChipRegistrationRequest(0, index);
            }
        }
    }
    else if (pressed == BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA);
    }
    return true;
}

void CGoldBowmanLena::SyncView()
{
    if (IsVisible())
    {
        // The original's RenderTexts(), all in the normal font on the 190-unit window: the notes
        // centred, the two captions light blue from x 20, the counts ("    X    n", spaces kept)
        // centred from x 5 beside the 3D Rena, the closing notes purple.
        const DWORD white = 0xFFFFFFFF;
        const DWORD caption = 0xFF47DFFA;
        const DWORD notice = 0xFFFA47D6;
        const float width = INVENTORY_WIDTH;
        std::vector<EventItemEntryView::Text> texts;
        texts.push_back({getMonsterName(236), 0.f, 15.f, width, false, white});
        for (int i = 0; i < 3; ++i)
            texts.push_back({Formatted(I18N::Game::Lookup(700 + i)), 0.f, 100.f + static_cast<float>(i) * 15.f, width,
                             false, white});

        wchar_t count[100] = {};
        const int registerItem = g_pMyInventory->GetInventoryCtrl()->GetItemCount(ITEM_POTION + 21, 0);
        texts.push_back({I18N::Game::NumberOfRenaYouHaveCollected, 20.f, 180.f, width, false, caption, true});
        mu_swprintf(count, L"    X    %d", registerItem);
        texts.push_back({count, 5.f, 202.f, width, false, white});
        texts.push_back({I18N::Game::NumberOfRegisteredRena, 20.f, 225.f, width, false, caption, true});
        mu_swprintf(count, L"    X    %d", g_shEventChipCount);
        texts.push_back({count, 5.f, 245.f, width, false, white});
        for (int j = 0; j < 2; ++j)
            texts.push_back({Formatted(I18N::Game::Lookup(703 + j)), 0.f, 350.f + static_cast<float>(j) * 15.f, width,
                             false, notice});
        m_View.SetTexts(std::move(texts));

        m_View.SetButtons({{I18N::Game::RegisteringRena, kRegisterX, kRegisterY, false, MSGBOX_BTN_EMPTY_WIDTH,
                            MSGBOX_BTN_EMPTY_HEIGHT, false, "wide"},
                           {L"", kExitX, kExitY, false, kExitWidth, kExitHeight, false, "exit"}});
    }
    m_View.Sync(IsVisible(), m_Pos);

    if (!IsVisible())
        HideTooltips();
}

bool CGoldBowmanLena::Render()
{
    // The frame, the texts and the buttons are RmlUi (the frame in the background context, under
    // the Rena); the two Rena stay native 3D, over the frame as before, in the render state the
    // original's 2D pass left them.
    EnableAlphaTest();
    DisableAlphaBlend();
    Render3D();

    // The buttons' hover tooltips (the shared RmlUi one), shown from here as the original's
    // CButton::Render() showed them, after the hover checks of the windows under it.
    m_RegisterTooltip.Render(m_Pos.x + static_cast<int>(kRegisterX), m_Pos.y + static_cast<int>(kRegisterY),
                             static_cast<int>(MSGBOX_BTN_EMPTY_WIDTH), static_cast<int>(MSGBOX_BTN_EMPTY_HEIGHT));
    m_ExitTooltip.Render(m_Pos.x + static_cast<int>(kExitX), m_Pos.y + static_cast<int>(kExitY),
                         static_cast<int>(kExitWidth), static_cast<int>(kExitHeight));
    return true;
}

float CGoldBowmanLena::GetLayerDepth()	// 3.4f
{
    return 3.4f;
}

// Pre-panel proj/view snapshot, restored around Render3D()'s matrix push/pop -- same shape as RenderDisplayItems().
static float s_PreGBLProj[16];
static float s_PreGBLView[16];

void CGoldBowmanLena::Render3D()
{
    EndBitmap();

    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    SetRenderViewport(0, 0, WindowWidth, WindowHeight);
    gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
    EnableDepthTest();
    EnableDepthMask();

    int Type = ITEM_POTION + 21;
    int Level = 0;
    float x = (float)REFERENCE_WIDTH - 120.f;
    float y = 200.f;
    float Width = (float)ItemAttribute[Type].Width * INVENTORY_SCALE;
    float Height = (float)ItemAttribute[Type].Height * INVENTORY_SCALE;
    RenderItem3D(x, y, Width, Height, Type, Level, 0, 0, false);
    RenderItem3D(x, y + 42, Width, Height, Type, Level, 0, 0, false);

    UpdateMousePositionn();

    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PopMatrix();

    BeginBitmap();
}
