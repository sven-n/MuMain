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

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original's exit button (newui_exit_00, 36 x 29).
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

bool CGoldBowmanLena::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng) {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA, this);

    m_View.SetItemDrawer([this](int, const Rml::Vector2f& offset, const Rml::Vector2f& size) { Render3D(offset, size); });
    m_View.SetWindowId(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA);
    m_View.Build();

    Show(false);

    return true;
}

void CGoldBowmanLena::Release()
{
    m_View.Release();
}

void CGoldBowmanLena::OpeningProcess()
{
}

void CGoldBowmanLena::ClosingProcess()
{
    g_bEventChipDialogEnable = 0;
    g_shEventChipCount = 0;
    SocketClient->ToGameServer()->SendEventChipExitDialog();
}

bool CGoldBowmanLena::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA) == false) {
        return true;
    }

    // Register, the exit button and the corner close are RmlUi's (see Update()).
    if (m_View.IsPointerOver() &&
        mu::ui::window::IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
    }

    return false;
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
        const float width = INVENTORY_WIDTH;
        std::vector<EventItemEntryView::Text> texts;
        texts.push_back({getMonsterName(236), width});
        for (int i = 0; i < 3; ++i)
            texts.push_back({Formatted(I18N::Game::Lookup(700 + i)), width});

        wchar_t count[100] = {};
        const int registerItem = g_pMyInventory->GetInventoryCtrl()->GetItemCount(ITEM_POTION + 21, 0);
        texts.push_back({I18N::Game::NumberOfRenaYouHaveCollected, width});
        mu_swprintf(count, L"    X    %d", registerItem);
        texts.push_back({count, width});
        texts.push_back({I18N::Game::NumberOfRegisteredRena, width});
        mu_swprintf(count, L"    X    %d", g_shEventChipCount);
        texts.push_back({count, width});
        for (int j = 0; j < 2; ++j)
            texts.push_back({Formatted(I18N::Game::Lookup(703 + j)), width});
        m_View.SetTexts(std::move(texts));

        m_View.SetButtons({{.label = I18N::Game::RegisteringRena,
                            .width = MSGBOX_BTN_EMPTY_WIDTH,
                            .height = MSGBOX_BTN_EMPTY_HEIGHT,
                            .hint = I18N::Game::RegisteringRena},
                           {.width = kExitWidth, .height = kExitHeight, .hint = I18N::Game::Close388}});
    }
    m_View.Sync(IsVisible());
}

bool CGoldBowmanLena::Render()
{
    return true;
}

float CGoldBowmanLena::GetLayerDepth()	// 3.4f
{
    return 3.4f;
}

// A Rena into each of the theme's two .entry-item-box. The original drew them at (640 - 120, 200)
// and 42 below from its column-one place at x 450: 70 into the panel.
void CGoldBowmanLena::Render3D(const Rml::Vector2f& offset, const Rml::Vector2f& size)
{
    EnableAlphaTest();
    DisableAlphaBlend();
    RenderItem3D(offset.x, offset.y, size.x, size.y, ITEM_POTION + 21, 0, 0, 0, false);
}
