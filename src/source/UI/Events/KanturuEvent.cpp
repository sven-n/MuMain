
#include "stdafx.h"
#include "UI/Events/KanturuEvent.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Effects/ZzzEffect.h"
#include "I18N/All.h"

#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "Audio/DSPlaySound.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDigitCells.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cstring>
#include <string>

namespace
{
// RenderText() shrinks a text wider than its box to fit it: the size it drew `text` at.
float KanturuTextPxInBox(UI::Scaling::FontRole role, const UI::Scaling::Transform& transform, const wchar_t* text,
                         float boxWidth)
{
    g_pRenderText->SetFont(role == UI::Scaling::FontRole::Bold ? g_hFontBold : g_hFont);
    const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
    return UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width), boxWidth);
}
} // namespace

mu::ui::window::CKanturu2ndEnterNpc::CKanturu2ndEnterNpc()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_pNpcObject = NULL;
    m_dwRefreshTime = 0;
    m_dwRefreshButtonGapTime = 0;

    Initialize();
}

mu::ui::window::CKanturu2ndEnterNpc::~CKanturu2ndEnterNpc()
{
    Release();
}

void mu::ui::window::CKanturu2ndEnterNpc::Initialize()
{
    m_bNpcAnimation = false;
    m_bEnterRequest = false;

    m_iStateTextNum = 0;
    ZeroMemory(m_strSubject, sizeof(m_strSubject));
    for (int i = 0; i < KANTURU2ND_STATETEXT_MAX; i++)
    {
        ZeroMemory(m_strStateText[i], sizeof(m_strStateText[i]));
    }
}

bool mu::ui::window::CKanturu2ndEnterNpc::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CKanturu2ndEnterNpc::Release()
{
    m_RmlView.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CKanturu2ndEnterNpc::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CKanturu2ndEnterNpc::UpdateMouseEvent()
{
    // The Refresh, Enter and Close buttons are RmlUi's (see Update()); the window keeps the pointer.
    float panelWidth = KANTURU2ND_ENTER_WINDOW_WIDTH;
    float panelHeight = KANTURU2ND_ENTER_WINDOW_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CKanturu2ndEnterNpc::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CKanturu2ndEnterNpc::Update()
{
    // The Refresh button unlocks a second after its click (the original's BtnProcess()).
    if (m_RefreshLocked && timeGetTime() - m_dwRefreshButtonGapTime > KANTURU2ND_REFRESHBUTTON_GAPTIME)
    {
        m_RefreshLocked = false;
    }

    SyncRmlModel();

    // Clicks RmlUi reported, handled like the original's BtnProcess().
    const bool refresh = m_PendingRefresh;
    const bool enter = m_PendingEnter;
    const bool close = m_PendingClose;
    m_PendingRefresh = m_PendingEnter = m_PendingClose = false;

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC) == true)
    {
        if (refresh && !m_RefreshLocked)
        {
            ProcessRefresh();
        }
        else if (enter && !m_EnterLocked)
        {
            ProcessEnter();
        }
        else if (close)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC);
        }
    }

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC) == true)
    {
        if (timeGetTime() - m_dwRefreshTime > KANTURU2ND_REFRESH_GAPTIME)
        {
            SendRequestKanturu3rdInfo();
        }
    }

    return true;
}

bool mu::ui::window::CKanturu2ndEnterNpc::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

float mu::ui::window::CKanturu2ndEnterNpc::GetLayerDepth()
{
    return 10.1f;
}

void mu::ui::window::CKanturu2ndEnterNpc::SetNpcObject(OBJECT* pObj)
{
    m_pNpcObject = pObj;
}

bool mu::ui::window::CKanturu2ndEnterNpc::IsNpcAnimation()
{
    return m_bNpcAnimation;
}

void mu::ui::window::CKanturu2ndEnterNpc::SetNpcAnimation(bool bValue)
{
    m_bNpcAnimation = bValue;
}

bool mu::ui::window::CKanturu2ndEnterNpc::IsEnterRequest()
{
    return m_bEnterRequest;
}

void mu::ui::window::CKanturu2ndEnterNpc::SetEnterRequest(bool bValue)
{
    m_bEnterRequest = bValue;
}

void mu::ui::window::CKanturu2ndEnterNpc::CreateMessageBox(MSGBOX_TYPE result)
{
    wchar_t strMessage[256];
    if (result == POPUP_FAILED || result == POPUP_FAILED2)
    {
        wcscpy(strMessage, I18N::Game::FailedToEnter);
    }
    else if (result == POPUP_UNIRIA)
    {
        wcscpy(strMessage, I18N::Game::YouCannotWarpWhileRidingOnAUnicorn);
    }
    else if (result == POPUP_CHANGERING)
    {
        wcscpy(strMessage, I18N::Game::YouCanTWarpWearingTheRingOfTransformation);
    }
    else if (result == POPUP_NOT_HELPER)
    {
        wcscpy(strMessage, I18N::Game::YouCanOnlyWarpRidingA);
    }
    else
    {
        wcscpy(strMessage, I18N::Game::Lookup(2170 + static_cast<int>(result)));
    }

    mu::ui::window::CreateOkMessageBox(strMessage);
}

void mu::ui::window::CKanturu2ndEnterNpc::ReceiveKanturu3rdInfo(UI::Kanturu::Stage stage, UI::Kanturu::Detail detail, bool canEnter,
                                                                 BYTE userCount, int remainingSeconds)
{
    if (m_pNpcObject && m_pNpcObject->CurrentAction == KANTURU2ND_NPC_ANI_ROT)
    {
        return;
    }

    if (g_MessageBox->IsEmpty() == false)
    {
        return;
    }

    Initialize();

    m_byState = static_cast<BYTE>(stage);

    m_EnterLocked = !canEnter;

    if (stage == UI::Kanturu::Stage::Tower)
    {
        if (detail == UI::Kanturu::Detail::TowerRevitalization || detail == UI::Kanturu::Detail::TowerNotify)
        {
            wcscpy(m_strSubject, I18N::Game::YouMayNowProceedToTheRefineryTower);
            wcscpy(m_strStateText[0], I18N::Game::PathToTheRefineryTowerIsNowOpened);
            mu_swprintf(m_strStateText[1], I18N::Game::PathToTheRefineryTowerWillBeClosedInDHours, remainingSeconds / 3600);
            m_iStateTextNum = 2;
        }
        else
        {
            wcscpy(m_strSubject, I18N::Game::YouCanTWarpToTheRefineryTower);
            wcscpy(m_strStateText[0], I18N::Game::DefeatTheNightmareThatControllingThe);
            wcscpy(m_strStateText[1], I18N::Game::EntranceIsRestrictedToEnsureThe);
            m_iStateTextNum = 2;
        }
    }
    else if (stage == UI::Kanturu::Stage::MayaBattle)
    {
        if (detail != UI::Kanturu::Detail::MayaStandby1
            && detail != UI::Kanturu::Detail::MayaStandby2
            && detail != UI::Kanturu::Detail::MayaStandby3)
        {
            wcscpy(m_strSubject, I18N::Game::BattleWithMayaIsOngoing);
            mu_swprintf(m_strStateText[0], I18N::Game::DPlayersAreTryingToOpen, userCount);
        }
        else
        {
            wcscpy(m_strSubject, I18N::Game::MorePlayersAreNeededToOpenThePathToTheTower);

            if (detail == UI::Kanturu::Detail::MayaStandby1)
            {
                if (userCount < 15)
                {
                    wcscpy(m_strStateText[0], I18N::Game::YouMayNowEnter);
                }
                else if (userCount == 15)
                {
                    wcscpy(m_strStateText[0], I18N::Game::MoonstonePendantAuthenticationHasFailed);
                }
                else
                {
                    if (!m_EnterLocked)
                    {
                        wcscpy(m_strStateText[0], I18N::Game::YouMayNowEnter);
                    }
                    else
                    {
                        wcscpy(m_strStateText[0], I18N::Game::MoonstonePendantAuthenticationHasFailed);
                    }
                }
                m_iStateTextNum = 1;
            }
            else if (detail == UI::Kanturu::Detail::MayaStandby2)
            {
                if (userCount < 15)
                {
                    mu_swprintf(m_strStateText[0], I18N::Game::NightmareHasLostTheControlOf, userCount);
                    mu_swprintf(m_strStateText[1], I18N::Game::MorePowerFromDPlayersAreNeeded, 15 - userCount);
                    m_iStateTextNum = 2;
                }
                else if (userCount == 15)
                {
                    wcscpy(m_strStateText[0], I18N::Game::NightmareHasLostTheControlOfMayaSLeftHand);
                    m_iStateTextNum = 1;
                }
            }
            else if (detail == UI::Kanturu::Detail::MayaStandby3)
            {
                if (userCount < 15)
                {
                    mu_swprintf(m_strStateText[0], I18N::Game::NightmareHasLostTheControlOf2166, userCount);
                    mu_swprintf(m_strStateText[1], I18N::Game::MorePowerFromDPlayersAreNeeded, 15 - userCount);
                    m_iStateTextNum = 2;
                }
                else if (userCount == 15)
                {
                    wcscpy(m_strStateText[0], I18N::Game::NightmareHasLostTheControlOfMayaSLeftHand);
                    m_iStateTextNum = 1;
                }
            }
            else
            {
                if (!m_EnterLocked)
                {
                    wcscpy(m_strStateText[0], I18N::Game::YouMayNowEnter);

                    m_iStateTextNum = 1;
                }
            }
        }

        if (detail == UI::Kanturu::Detail::MayaNotify || detail == UI::Kanturu::Detail::MayaMonster1 || detail == UI::Kanturu::Detail::Maya1
            || detail == UI::Kanturu::Detail::MayaEnd1 || detail == UI::Kanturu::Detail::MayaEndCycle1)
        {
            mu_swprintf(m_strStateText[1], I18N::Game::CurrentlyDPlayersAreInBattleWithMayaSLefeHand, userCount);
            m_iStateTextNum = 2;
        }
        else if (detail == UI::Kanturu::Detail::MayaMonster2 || detail == UI::Kanturu::Detail::Maya2
            || detail == UI::Kanturu::Detail::MayaEnd2 || detail == UI::Kanturu::Detail::MayaEndCycle2)
        {
            mu_swprintf(m_strStateText[1], I18N::Game::CurrentlyDPlayersAreInBattleWithMayaSRightHand, userCount);
            m_iStateTextNum = 2;
        }
        else if (detail == UI::Kanturu::Detail::MayaMonster3 || detail == UI::Kanturu::Detail::Maya3
            || detail == UI::Kanturu::Detail::MayaEnd3 || detail == UI::Kanturu::Detail::MayaEndCycle3)
        {
            mu_swprintf(m_strStateText[1], I18N::Game::CurrentlyDPlayersAreInBattleWithMayaSBothHands, userCount);
            m_iStateTextNum = 2;
        }
        else if (detail == UI::Kanturu::Detail::None || detail == UI::Kanturu::Detail::MayaEnd
            || detail == UI::Kanturu::Detail::MayaEndCycle)
        {
            m_iStateTextNum = 1;
        }
    }
    else if (stage == UI::Kanturu::Stage::NightmareBattle)
    {
        wcscpy(m_strSubject, I18N::Game::BattleWithMayaIsOngoing);
        mu_swprintf(m_strStateText[0], I18N::Game::DPlayersAreTryingToOpen, userCount);
        mu_swprintf(m_strStateText[1], I18N::Game::CurrentlyDPlayersAreInBattleWithNightmare, userCount);
        m_iStateTextNum = 2;
    }
    else if (stage == UI::Kanturu::Stage::Standby)
    {
        wcscpy(m_strSubject, I18N::Game::BossBattleWillStartSoon);
        if (detail == UI::Kanturu::Detail::StandbyStart)
        {
            mu_swprintf(m_strStateText[0], I18N::Game::ForceOfTheNightmareHasInvaded, remainingSeconds / 60);
        }
        else // STANBY_NONE || STANBY_NOTIFY || STANBY_END || STANBY_ENDCYCLE
        {
            mu_swprintf(m_strStateText[0], I18N::Game::YouWillBeAbleToApproachMayaShortly);
        }
        mu_swprintf(m_strStateText[1], I18N::Game::DefeatTheNightmareThatControllingThe);
        mu_swprintf(m_strStateText[2], I18N::Game::EntranceIsRestrictedToEnsureThe);
        m_iStateTextNum = 3;
    }
    else
    {
        wcscpy(m_strSubject, I18N::Game::FailedToEnter);
    }

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC) == false)
    {
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC);
    }
}

void mu::ui::window::CKanturu2ndEnterNpc::ReceiveKanturu3rdEnter(UI::Kanturu::EntryResult result)
{
    m_bEnterRequest = false;
    CreateMessageBox(static_cast<MSGBOX_TYPE>(result));

    // The original dereferenced the gateway NPC unchecked: an entry answer before the client had
    // seen the NPC (the pointer is set when the NPC enters the viewport) crashed it.
    m_bNpcAnimation = false;
    if (m_pNpcObject)
    {
        m_pNpcObject->AnimationFrame = 0;
        SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_STOP);
    }

    DeleteJoint(BITMAP_JOINT_ENERGY, NULL);

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC);
}

void mu::ui::window::CKanturu2ndEnterNpc::SendRequestKanturu3rdInfo()
{
    SocketClient->ToGameServer()->SendKanturuInfoRequest();
    m_dwRefreshTime = timeGetTime();
}

void mu::ui::window::CKanturu2ndEnterNpc::SendRequestKanturu3rdEnter()
{
    SocketClient->ToGameServer()->SendKanturuEnterRequest();
    m_bEnterRequest = true;
}

void mu::ui::window::CKanturu2ndEnterNpc::ProcessRefresh()
{
    SendRequestKanturu3rdInfo();

    m_RefreshLocked = true;
    m_dwRefreshButtonGapTime = timeGetTime();
}

void mu::ui::window::CKanturu2ndEnterNpc::ProcessEnter()
{
    if (m_pNpcObject)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC);

        if (m_byState == KANTURU_STATE_TOWER)
        {
            SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
            m_bNpcAnimation = true;
            return;
        }

        ITEM *pItemHelper, *pItemRingLeft, *pItemRingRight, *pItemWing;
        pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
        pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
        pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        pItemWing = &CharacterMachine->Equipment[EQUIPMENT_WING];

        if (pItemHelper->Type == ITEM_HORN_OF_UNIRIA)
        {
            CreateMessageBox(POPUP_UNIRIA);
            return;
        }

        if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) ||
            g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
        {
            CreateMessageBox(POPUP_CHANGERING);
            return;
        }

        if (!((pItemWing->Type >= ITEM_WINGS_OF_ELF && pItemWing->Type <= ITEM_WINGS_OF_DARKNESS) ||
              (pItemWing->Type >= ITEM_WING_OF_STORM && pItemWing->Type <= ITEM_WING_OF_DIMENSION) ||
              (ITEM_WING + 130 <= pItemWing->Type && pItemWing->Type <= ITEM_WING + 134) ||
              pItemHelper->Type == ITEM_HORN_OF_DINORANT || pItemHelper->Type == ITEM_DARK_HORSE_ITEM ||
              pItemWing->Type == ITEM_CAPE_OF_LORD || pItemHelper->Type == ITEM_HORN_OF_FENRIR ||
              (pItemWing->Type >= ITEM_CAPE_OF_FIGHTER && pItemWing->Type <= ITEM_CAPE_OF_OVERRULE) ||
              (pItemWing->Type == ITEM_WING + 135)))
        {
            CreateMessageBox(POPUP_NOT_HELPER);
            return;
        }

        if (pItemRingLeft->Type == ITEM_MOONSTONE_PENDANT || pItemRingRight->Type == ITEM_MOONSTONE_PENDANT)
        {
            SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
            m_bNpcAnimation = true;
        }
        else
        {
            CreateMessageBox(POPUP_NOT_MUNSTONE);
            return;
        }
    }
}

void mu::ui::window::CKanturu2ndEnterNpc::BindRmlModel(Rml::DataModelConstructor& c, KanturuEnterRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("text_px", &model.textPx);
    auto line = c.RegisterStruct<KanturuEnterLineEntry>();
    line.RegisterMember("text", &KanturuEnterLineEntry::text);
    line.RegisterMember("top", &KanturuEnterLineEntry::top);
    line.RegisterMember("text_px", &KanturuEnterLineEntry::textPx);
    line.RegisterMember("kind", &KanturuEnterLineEntry::kind);
    c.RegisterArray<std::vector<KanturuEnterLineEntry>>();
    c.Bind("lines", &model.lines);
    c.Bind("refresh_text", &model.refreshText);
    c.Bind("enter_text", &model.enterText);
    c.Bind("close_text", &model.closeText);
    c.Bind("refresh_locked", &model.refreshLocked);
    c.Bind("enter_locked", &model.enterLocked);
    c.Bind("label_line_px", &model.labelLinePx);
    c.BindEventCallback("kanturu_refresh", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingRefresh = true; });
    c.BindEventCallback("kanturu_enter", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingEnter = true; });
    c.BindEventCallback("kanturu_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingClose = true; });
}

void mu::ui::window::CKanturu2ndEnterNpc::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CKanturu2ndEnterNpc::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 10.1: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncContent();
}

void mu::ui::window::CKanturu2ndEnterNpc::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    KanturuEnterRmlModel updated = m_RmlView.GetModel();
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    updated.labelLinePx = static_cast<float>(lineHeight) * transform.scaleY;
    updated.refreshText = StringUtils::WideToNarrow(I18N::Game::Refresh);
    updated.enterText = StringUtils::WideToNarrow(I18N::Game::Enter);
    updated.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);
    updated.refreshLocked = m_RefreshLocked;
    updated.enterLocked = m_EnterLocked;

    // The original's RenderTexts(): the subject bold, split into lines of 52 characters, 12 units
    // apart from y 30; 20 units below it the state texts, the first in green, the others bright
    // yellow, 15 units between two texts.
    updated.lines.clear();
    auto addLine = [&](const wchar_t* text, float top, const char* kind)
    {
        // The subject is the one the original drew bold, so it is the one measured in that font.
        const auto role = std::strcmp(kind, "subject") == 0 ? UI::Scaling::FontRole::Bold
                                                            : UI::Scaling::FontRole::Normal;
        updated.lines.push_back({StringUtils::WideToNarrow(text), top,
                                 KanturuTextPxInBox(role, transform, text, KANTURU2ND_ENTER_WINDOW_WIDTH), kind});
    };
    float textY = 30.f;
    wchar_t separated[3][52] = {};
    int lineCount = SeparateTextIntoLines(m_strSubject, separated[0], 3, 52);
    for (int i = 0; i < lineCount; i++)
    {
        addLine(separated[i], textY, "subject");
        textY += 12.f;
    }
    textY += 20.f;
    for (int i = 0; i < m_iStateTextNum; i++)
    {
        ZeroMemory(separated, sizeof(separated));
        lineCount = SeparateTextIntoLines(m_strStateText[i], separated[0], 3, 52);
        for (int j = 0; j < lineCount; j++)
        {
            addLine(separated[j], textY, i == 0 ? "state" : "note");
            textY += 12.f;
        }
        textY += 15.f;
    }

    KanturuEnterRmlModel& model = m_RmlView.GetModel();
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::labelLinePx, "label_line_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::refreshText, "refresh_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::enterText, "enter_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::closeText, "close_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::refreshLocked, "refresh_locked", updated);
    SyncFieldFrom(m_RmlView.Binder(), &KanturuEnterRmlModel::enterLocked, "enter_locked", updated);
    const bool sameLines = model.lines == updated.lines;
    if (!sameLines)
    {
        model.lines = std::move(updated.lines);
        m_RmlView.MarkDirty("lines");
    }
}

mu::ui::window::CKanturuInfoWindow::CKanturuInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
}

mu::ui::window::CKanturuInfoWindow::~CKanturuInfoWindow()
{
    Release();
}

bool mu::ui::window::CKanturuInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_KANTURU_INFO, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CKanturuInfoWindow::Release()
{
    m_RmlView.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CKanturuInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CKanturuInfoWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CKanturuInfoWindow::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CKanturuInfoWindow::Update()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU_INFO))
    {
        if (M39Kanturu3rd::IsInKanturu3rd() == false)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_KANTURU_INFO);
        }
    }

    SyncView();
    return true;
}

bool mu::ui::window::CKanturuInfoWindow::Render()
{
    // Nothing native left: the frame, the texts and the digits are RmlUi (SyncView()). Kept
    // because CObject requires the override.
    return true;
}

namespace
{
template <typename T>
void SyncInfoField(RmlModelBinder<mu::ui::window::KanturuInfoRmlModel>& binder,
                   T mu::ui::window::KanturuInfoRmlModel::* field, const char* name, T value)
{
    auto& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

} // namespace

void mu::ui::window::CKanturuInfoWindow::BindRmlModel(Rml::DataModelConstructor& c, KanturuInfoRmlModel& model)
{
    c.Bind("scale_x", &model.scaleX);
    c.Bind("scale_y", &model.scaleY);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("panel_x", &model.panelX);
    c.Bind("panel_y", &model.panelY);
    c.Bind("users_text", &model.usersText);
    c.Bind("monsters_text", &model.monstersText);
    c.Bind("colon_visible", &model.colonVisible);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("minute_digits", &model.minuteDigits);
    c.Bind("second_digits", &model.secondDigits);
}

void mu::ui::window::CKanturuInfoWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CKanturuInfoWindow::SyncView()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    // CManager scopes LayoutMode::HudFrame around the window: the bottom HUD's uniform scale, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::boldTextPx, "bold_text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::panelX, "panel_x", static_cast<float>(m_Pos.x));
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::panelY, "panel_y", static_cast<float>(m_Pos.y));

    // The original's RenderInfo(): the characters, then the monsters or, while Maya fights, the boss.
    wchar_t strText[256];
    mu_swprintf(strText, I18N::Game::CharacterD, UserCount);
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::usersText, "users_text", StringUtils::WideToNarrow(strText));
    if (g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA1 ||
        g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA2 ||
        g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA3)
    {
        wcscpy(strText, I18N::Game::MonsterBoss2182);
    }
    else
    {
        mu_swprintf(strText, I18N::Game::MonsterD, MonsterCount);
    }
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::monstersText, "monsters_text", StringUtils::WideToNarrow(strText));

    // The time left since the last SetTime(). The original showed 0 seconds for the whole last
    // minute (it took the seconds modulo 60 * minutes); the seconds are taken modulo 60 here.
    const int iPastSecond = m_iSecond - static_cast<int>((GetTickCount() - m_dwSyncTime) / 1000);
    const int iRemaining = std::max(iPastSecond, 0);
    m_iMinute = iRemaining / 60;
    const int iSecond = iRemaining % 60;

    if (timeGetTime() - m_dwColonTime > 500)
    {
        m_dwColonTime = timeGetTime();
        m_bColonVisible = !m_bColonVisible;
    }
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::colonVisible, "colon_visible", m_bColonVisible);

    // RenderNumber()'s newui_number1 cells; the theme lays out each run.
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::minuteDigits, "minute_digits",
                  UI::RmlBridge::DigitCells(m_iMinute, 12.f, 14.f));
    SyncInfoField(m_RmlView.Binder(), &KanturuInfoRmlModel::secondDigits, "second_digits",
                  UI::RmlBridge::DigitCells(iSecond, 12.f, 14.f));
}

float mu::ui::window::CKanturuInfoWindow::GetLayerDepth()
{
    return 1.92f;
}

float mu::ui::window::CKanturuInfoWindow::GetKeyEventOrder()
{
    return 9.1f;
}

void mu::ui::window::CKanturuInfoWindow::SetTime(int iTimeLimit)
{
    m_iMinute = 0;
    m_iSecond = iTimeLimit / 1000;
    m_dwSyncTime = GetTickCount();
}
