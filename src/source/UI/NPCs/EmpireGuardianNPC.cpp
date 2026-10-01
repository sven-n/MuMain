
#include "stdafx.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/NPCs/EmpireGuardianNPC.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"

#include "Audio/DSPlaySound.h"
#include "UI/Widgets/UIControls.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Text/TextWrap.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CEmpireGuardianNPC::CEmpireGuardianNPC()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_bCanClick = true;
}

CEmpireGuardianNPC::~CEmpireGuardianNPC()
{
    Release();
}

bool CEmpireGuardianNPC::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { m_View.ReloadTheme(); });

    Show(false);

    return true;
}

void CEmpireGuardianNPC::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CEmpireGuardianNPC::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CEmpireGuardianNPC::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, NPC_WINDOW_WIDTH, NPC_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CEmpireGuardianNPC::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CEmpireGuardianNPC::Update()
{
    SyncView();

    // A click RmlUi reported (the original's button handling in BtnProcess()).
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (pressed == 0)
    {
        SocketClient->ToGameServer()->SendEnterEmpireGuardianEvent();
        ::PlayBuffer(SOUND_INTERFACE01);
        m_bCanClick = false;
    }
    else if (pressed == 1)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC);
    }
    return true;
}

bool CEmpireGuardianNPC::IsVisible() const
{
    return CObject::IsVisible();
}

bool CEmpireGuardianNPC::Render()
{
    // Nothing native left but the 3D preview (Render3D()): the frame, the texts and the buttons
    // are RmlUi. Kept because CObject requires the override.
    return true;
}

void CEmpireGuardianNPC::SyncView()
{
    if (IsVisible())
    {
        // The original's RenderFrame()/Render(): the title and the texts in (220, 220, 220),
        // Gaion's Order bold yellow, the warning bold red. Both cut texts share one buffer, the
        // second over the first, as the original's did.
        const float centreX = static_cast<float>(NPC_WINDOW_WIDTH) / 2;
        wchar_t szTextOut[2][300] = {};
        std::vector<EventItemEntryView::Text> texts;
        texts.push_back({I18N::Game::JerintTheAssistant, 110.f, true});
        texts.push_back({I18N::Game::WithoutGaionSOrder, 190.f});
        g_pRenderText->SetFont(g_hFont);
        CutStr(I18N::Game::YouCannotEnterTheFortressOfEmpireGuardians, szTextOut[0], 150, 2, 300);
        texts.push_back({szTextOut[0], 190.f});
        texts.push_back({szTextOut[1], 190.f});
        texts.push_back({I18N::Game::WillYouShowMeTheOrder, 190.f});
        texts.push_back({I18N::Game::GaionSOrder, 110.f, true});
        texts.push_back({I18N::Game::Warning2223, 110.f, true});
        texts.push_back({I18N::Game::TheRound7MapSundayCanOnly, 200.f});
        texts.push_back({I18N::Game::BeAccessedIfYouHaveA, 200.f});
        texts.push_back({I18N::Game::CompleteSecromicon2837, 200.f});
        g_pRenderText->SetFont(g_hFont);
        CutStr(I18N::Game::YouCanOnlyEnterAsAMemberOfAParty, szTextOut[0], 155, 2, 300);
        texts.push_back({szTextOut[0], 200.f});
        texts.push_back({szTextOut[1], 200.f});
        m_View.SetTexts(std::move(texts));

        m_View.SetButtons({{I18N::Game::Enter, false}, {I18N::Game::Close388, false}});
    }
    m_View.Sync(IsVisible(), m_Pos);
}

bool CEmpireGuardianNPC::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    // The Enter and Close buttons are RmlUi's (see Update()).
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC);

    return false;
}

float CEmpireGuardianNPC::GetLayerDepth()
{
    return 1.2f;
}

void CEmpireGuardianNPC::OpenningProcess()
{
}

void CEmpireGuardianNPC::ClosingProcess()
{
}

void CEmpireGuardianNPC::Render3D()
{
    RenderItem3D();
}

void CEmpireGuardianNPC::RenderItem3D()
{
    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };

    int nItemType = ITEM_GAIONS_ORDER;
    int nItemLevel = 0;

    ::RenderItem3D(ptOrigin.x + (190 - 20) / 2, ptOrigin.y + 70, 20.0f, 27.0f, nItemType, nItemLevel, 0, 0, false);
}
