#include "stdafx.h"
#include "UI/Events/GoldBowmanWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlKeyboardFocus.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>

#define MAXGOLDBOWMANSESERIAL 12

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original's exit button (newui_exit_00, CButton at (13, 392), 36 x 29).
constexpr float kExitX = 13.f;
constexpr float kExitY = 392.f;
constexpr float kExitWidth = 36.f;
constexpr float kExitHeight = 29.f;

// The original printed its notes through mu_swprintf() as formats ("100%%" shows "100%").
std::wstring Formatted(const wchar_t* format)
{
    wchar_t text[100] = {};
    mu_swprintf(text, format);
    return text;
}
} // namespace

CGoldBowmanWindow::CGoldBowmanWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    ZeroMemory(g_strGiftName, sizeof(g_strGiftName));
}

CGoldBowmanWindow::~CGoldBowmanWindow()
{
    Release();
}

bool CGoldBowmanWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GOLD_BOWMAN, this);

    SetPos(x, y);

    m_ExitTooltip.SetText(&I18N::Game::Close388);
    m_ExitTooltip.SetAnchorAbove(true);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this,
                                          [this]
                                          {
                                              // A new document: its field starts empty and unfocused.
                                              m_View.ReloadTheme();
                                              m_SerialFocusPending = IsVisible();
                                          });

    Show(false);

    return true;
}

void CGoldBowmanWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    UI::RmlBridge::Tooltip::Hide(&m_ExitTooltip);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

Rml::Element* CGoldBowmanWindow::GetSerialField() const
{
    return m_View.GetElementById("serial_field");
}

void CGoldBowmanWindow::ClearSerialField()
{
    m_View.SetInputValue(Rml::String());
    if (Rml::Element* field = GetSerialField())
        field->Blur();
}

void CGoldBowmanWindow::OpeningProcess()
{
    ZeroMemory(g_strGiftName, sizeof(g_strGiftName));
    ClearSerialField();
    m_SerialFocusPending = true;
}

void CGoldBowmanWindow::ClosingProcess()
{
    ZeroMemory(g_strGiftName, sizeof(g_strGiftName));
    ClearSerialField();
    m_SerialFocusPending = false;
    SetRelatedWnd(g_hWnd);
    UI::RmlBridge::Tooltip::Hide(&m_ExitTooltip);
    SocketClient->ToGameServer()->SendEventChipExitDialog();
}

bool CGoldBowmanWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN) == false) {
        return true;
    }

    // Top-right corner close "X" (shared frame): hides + swallows the click. Register and the exit
    // button are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GOLD_BOWMAN))
    {
        return false;
    }

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    m_View.RefreshPanelSize(panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                      static_cast<int>(panelHeight))
            .Contains(MouseX, MouseY))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (mu::ui::window::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }
    else
    {
        if (mu::ui::window::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CGoldBowmanWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN) == false) {
        return true;
    }

    if (mu::ui::window::IsPress(VK_ESCAPE) == true)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GOLD_BOWMAN);
        return false;
    }

    return true;
}

bool CGoldBowmanWindow::Update()
{
    SyncView();

    // A click RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (pressed == BUTTON_SERIAL)
    {
        SendSerial();
    }
    else if (pressed == BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GOLD_BOWMAN);
    }
    return true;
}

void CGoldBowmanWindow::SendSerial()
{
    mu::ui::window::CInventoryCtrl* pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
    if (pNewInventoryCtrl->FindEmptySlot(2, 4) == -1)
    {
        mu::ui::window::CreateOkMessageBox(I18N::Game::LeaveAtLeastOneEmptySlotInYourInventory);
        return;
    }

    // The number as three groups of four characters, empty groups where it is shorter.
    // The model, not the element: data-value writes the typed serial back into it.
    std::wstring serial = StringUtils::NarrowToWide(m_View.InputValue());
    serial.resize(MAXGOLDBOWMANSESERIAL, L'\0');

    wchar_t strSerial1[5] = {};
    wmemcpy(strSerial1, serial.c_str(), 4);

    wchar_t strSerial2[5] = {};
    wmemcpy(strSerial2, serial.c_str() + 4, 4);

    wchar_t strSerial3[5] = {};
    wmemcpy(strSerial3, serial.c_str() + 8, 4);

    SocketClient->ToGameServer()->SendLuckyNumberRequest(MU_C16(strSerial1), MU_C16(strSerial2), MU_C16(strSerial3));
}

void CGoldBowmanWindow::SyncView()
{
    if (IsVisible())
    {
        // The original's RenderTexts(): every line in the normal font, centred on the 190-unit
        // window; the example number green, the registered gift's name (from the server's answer)
        // at y 330.
        const DWORD white = 0xFFFFFFFF;
        const float width = INVENTORY_WIDTH;
        std::vector<EventItemEntryView::Text> texts{
            {getMonsterName(236), 0.f, 15.f, width, false, white},
            {Formatted(I18N::Game::EnterThe12DigitLuckyNumber), 0.f, 80.f, width, false, white},
            {Formatted(I18N::Game::WrittenOnThe100WinningCard), 0.f, 95.f, width, false, white},
            {Formatted(I18N::Game::LuckyNumberRegistrationPeriod), 0.f, 110.f, width, false, white},
            {Formatted(I18N::Game::Oct282003Nov30), 0.f, 125.f, width, false, white},
            {Formatted(I18N::Game::EnterTheLuckyNumber), 0.f, 180.f, width, false, white},
            {Formatted(I18N::Game::ExAUS919DKL2J9), 0.f, 195.f, width, false, 0xFF18FF00},
            {Formatted(I18N::Game::PleaseMakeSureToDifferentiate), 0.f, 210.f, width, false, white},
            {Formatted(I18N::Game::AlphabetOAndNumber0AndAlphabetIAndNumber1), 0.f, 225.f, width, false, white}};
        if (wcscmp(g_strGiftName, L"") != 0)
            texts.push_back({g_strGiftName, 0.f, 330.f, width, false, 0xFFFFD200});
        m_View.SetTexts(std::move(texts));

        // Register: newui_btn_empty (108 x 29) at (45, 285) with a normal label; the exit button.
        m_View.SetButtons({{I18N::Game::LuckyNumberRegistered, 45.f, 285.f, false, MSGBOX_BTN_EMPTY_WIDTH,
                            MSGBOX_BTN_EMPTY_HEIGHT, false, "wide"},
                           {L"", kExitX, kExitY, false, kExitWidth, kExitHeight, false, "exit"}});
    }
    m_View.Sync(IsVisible(), m_Pos);

    if (!IsVisible())
    {
        UI::RmlBridge::Tooltip::Hide(&m_ExitTooltip);
        return;
    }

    Rml::Element* field = GetSerialField();
    if (field == nullptr)
        return;
    if (m_SerialFocusPending)
    {
        field->Focus();
        if (field->IsPseudoClassSet("focus"))
            m_SerialFocusPending = false;
    }
    // The original pointed its related window at the focused edit box, so Escape still closes the
    // window while the player types.
    UI::RmlBridge::ClaimKeyboardWhileTyping(*this, field->GetOwnerDocument());
}

bool CGoldBowmanWindow::Render()
{
    // Nothing native left: the frame, the texts, the field and the buttons are RmlUi. The exit
    // button's hover tooltip (the shared RmlUi one) is shown from here, where the original's
    // CButton::Render() showed it, after the hover checks of the windows under it.
    m_ExitTooltip.Render(m_Pos.x + static_cast<int>(kExitX), m_Pos.y + static_cast<int>(kExitY),
                         static_cast<int>(kExitWidth), static_cast<int>(kExitHeight));
    return true;
}

float CGoldBowmanWindow::GetLayerDepth()
{
    return 3.4f;
}
