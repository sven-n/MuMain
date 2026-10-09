
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/HUD/CommandWindow.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "Guild/GuildTypes.h"
#include "Engine/AI/ZzzAI.h"
#include "World/MapInfra/MapManager.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The target box the original drew 5 units right of and below the pointer (newui_cursorid_wnd).
constexpr int kTargetBoxOffset = 5;
constexpr int kTitleBoxWidth = 72;

// The button labels, in COMMAND_TYPE order.
const wchar_t* const* const kCommandLabels[COMMAND_END] = {
    &I18N::Game::Trade,     &I18N::Game::Buy1124,  &I18N::Game::Party,          &I18N::Game::Whisper,
    &I18N::Game::Guild,     &I18N::Game::Alliance, &I18N::Game::HostilityGuild, &I18N::Game::SuspendHostilities,
    &I18N::Game::AddFriend, &I18N::Game::Follow,   &I18N::Game::Duel,           &I18N::Game::SpecialCommands,
};

} // namespace

mu::ui::window::CCommandWindow::CCommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
    m_bSelectedChar = false;
    m_bCanCommand = false;
}

mu::ui::window::CCommandWindow::~CCommandWindow()
{
    Release();
}

bool mu::ui::window::CCommandWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_COMMAND, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CCommandWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CCommandWindow::OpenningProcess()
{
    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
    m_PendingCommand = COMMAND_NONE;
}

void mu::ui::window::CCommandWindow::ClosingProcess()
{
    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
    m_PendingCommand = COMMAND_NONE;
}

void mu::ui::window::CCommandWindow::PressCommandButton(int command)
{
    // The chat commands don't act on the selected character, they open their own list instead.
    if (command == COMMAND_SPECIAL)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_COMMAND_LIST);
        PlayBuffer(SOUND_CLICK01);
        return;
    }

    if (g_CursedTemple->GetInterfaceState(static_cast<int>(mu::ui::window::INTERFACE_COMMAND), command))
        m_iCurSelectCommand = command;
}

bool mu::ui::window::CCommandWindow::UpdateMouseEvent()
{
    // The buttons, the exit button and the corner close are RmlUi's.
    if (UI::RmlBridge::IsPointerOver(m_RmlView.Document()))
    {
        SetMouseCursor(CURSOR_NORMAL);
        return false;
    }

    if (m_iCurSelectCommand != COMMAND_NONE)
        SetMouseCursor(CURSOR_IDSELECT);

    return true;
}

bool mu::ui::window::CCommandWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_COMMAND) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CCommandWindow::Update()
{
    if (IsVisible())
    {
        if (m_PendingCommand != COMMAND_NONE)
        {
            const int command = m_PendingCommand;
            m_PendingCommand = COMMAND_NONE;
            PressCommandButton(command);
        }
        RunCommand();
    }

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CCommandWindow::Render()
{
    // Nothing native left: frame, buttons, title, exit button and the target box at the pointer
    // are RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CCommandWindow::BindRmlModel(Rml::DataModelConstructor& c, CommandWindowRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("big_text_px", &model.bigTextPx);
    c.Bind("title_text_px", &model.titleTextPx);
    c.Bind("title_line_px", &model.titleLinePx);
    c.Bind("title_text", &model.titleText);
    c.Bind("exit_tooltip", &model.exitTooltip);

    auto button = c.RegisterStruct<CommandButtonEntry>();
    button.RegisterMember("label", &CommandButtonEntry::label);
    button.RegisterMember("index", &CommandButtonEntry::index);
    button.RegisterMember("selected", &CommandButtonEntry::selected);
    button.RegisterMember("label_line_px", &CommandButtonEntry::labelLinePx);
    button.RegisterMember("label_text_px", &CommandButtonEntry::labelTextPx);
    c.RegisterArray<std::vector<CommandButtonEntry>>();
    c.Bind("buttons", &model.buttons);

    c.Bind("target_visible", &model.targetVisible);
    c.Bind("target_left", &model.targetLeft);
    c.Bind("target_top", &model.targetTop);
    c.Bind("target_name", &model.targetName);
    c.Bind("target_in_range", &model.targetInRange);

    c.BindEventCallback("command_press",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PendingCommand = arguments[0].Get<int>(COMMAND_NONE);
                        });
    c.BindEventCallback("command_exit",
                        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND);
                            PlayBuffer(SOUND_CLICK01);
                        });

    model.titleText = StringUtils::WideToNarrow(I18N::Game::CommandWindow);
    wchar_t exitText[256] = {};
    mu_swprintf(exitText, I18N::Game::CloseS, L"D");
    model.exitTooltip = StringUtils::WideToNarrow(exitText);
    model.buttons.clear();
    for (int i = COMMAND_TRADE; i < COMMAND_END; ++i)
        model.buttons.push_back({StringUtils::WideToNarrow(*kCommandLabels[i]), i});
}

void mu::ui::window::CCommandWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::bigTextPx, "big_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Big));
    SyncTitle();

    SyncButtons();
    SyncTarget();
}

void mu::ui::window::CCommandWindow::SyncTitle()
{
    g_pRenderText->SetFont(g_hFontBold);
    const int titleWidth = g_pRenderText->MeasureText(I18N::Game::CommandWindow, lstrlen(I18N::Game::CommandWindow)).cx;
    const float titlePx = UI::RmlBridge::NativeTextPxInBox(UI::Scaling::FontRole::Bold, static_cast<float>(titleWidth),
                                                           static_cast<float>(kTitleBoxWidth));
    // The shrunk text's box shrinks with it: its top stays at y + 12.
    const float shrink = titlePx / UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::titleTextPx, "title_text_px", titlePx);
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::titleLinePx, "title_line_px",
              CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Bold) * shrink);
}

void mu::ui::window::CCommandWindow::SyncButtons()
{
    // CButton::Render(): the label, bold when selected, centred on its button.
    const float normalHeight = CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Normal);
    const float boldHeight = CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Bold);
    const float normalPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal);
    const float boldPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);

    CommandWindowRmlModel& model = m_RmlView.GetModel();
    bool changed = false;
    for (CommandButtonEntry& button : model.buttons)
    {
        CommandButtonEntry updated = button;
        updated.selected = button.index == m_iCurSelectCommand;
        updated.labelLinePx = updated.selected ? boldHeight : normalHeight;
        updated.labelTextPx = updated.selected ? boldPx : normalPx;

        changed = changed || updated.selected != button.selected ||
                  updated.labelLinePx != button.labelLinePx || updated.labelTextPx != button.labelTextPx;
        button = updated;
    }
    if (changed)
        m_RmlView.MarkDirty("buttons");
}

void mu::ui::window::CCommandWindow::SyncTarget()
{
    const CHARACTER* target = (m_iCurMouseCursor == CURSOR_IDSELECT && m_bSelectedChar && SelectedCharacter >= 0)
                                  ? &CharactersClient[SelectedCharacter]
                                  : nullptr;
    const bool visible = target != nullptr && target->Object.Kind == KIND_PLAYER && target != Hero &&
                         (target->Object.Type == MODEL_PLAYER || target->Change);

    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::targetVisible, "target_visible", visible);
    if (!visible)
        return;

    // Beside the pointer, in the panel's own units.
    Rml::Vector2f pointer;
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr || !UI::RmlBridge::PointerIn(document->GetElementById("panel"), pointer))
        return;
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::targetLeft, "target_left", pointer.x + kTargetBoxOffset);
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::targetTop, "target_top", pointer.y + kTargetBoxOffset);
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::targetName, "target_name",
              Rml::String(StringUtils::WideToNarrow(target->ID)));
    SyncField(m_RmlView.Binder(), &CommandWindowRmlModel::targetInRange, "target_in_range", m_bCanCommand);
}

void mu::ui::window::CCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

float mu::ui::window::CCommandWindow::GetLayerDepth()
{
    return LayerDepth;
}

void mu::ui::window::CCommandWindow::RunCommand()
{
    if (MouseLButtonPush && m_iCurMouseCursor != CURSOR_IDSELECT)
        SetMouseCursor(CURSOR_PUSH);

    if (m_iCurSelectCommand == COMMAND_NONE)
        return;

    int Selectindex = -1;
    CHARACTER* pSelectedCha = NULL;
    int distX, distY;
    m_bSelectedChar = false;
    m_bCanCommand = false;

    if (SelectedCharacter >= 0)
    {
        pSelectedCha = &CharactersClient[SelectedCharacter];
        m_bSelectedChar = true;
        if (pSelectedCha != NULL)
        {
            distX = abs((pSelectedCha->PositionX) - (Hero->PositionX));
            distY = abs((pSelectedCha->PositionY) - (Hero->PositionY));
            if (pSelectedCha->Object.Kind == KIND_PLAYER && pSelectedCha != Hero && (pSelectedCha->Object.Type == MODEL_PLAYER || pSelectedCha->Change) && (distX <= MAX_DISTANCE_TILE && distY <= MAX_DISTANCE_TILE))
            {
                if ((pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_DEER) && (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SNOWMAN) && (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SSANTA))
                {
                    Selectindex = SelectedCharacter;
                    m_bCanCommand = true;
                }
            }
        }
    }

    if (MouseRButtonPush)
    {
        MouseRButtonPush = false;
        MouseRButton = false;

        SetMouseCursor(CURSOR_NORMAL);

        if (Selectindex >= 0)
        {
            switch (m_iCurSelectCommand)
            {
            case COMMAND_TRADE:
            {
                CommandTrade(pSelectedCha);
            }break;

            case COMMAND_PURCHASE:
            {
                CommandPurchase(pSelectedCha);
            }break;

            case COMMAND_PARTY:
            {
                CommandParty(pSelectedCha->Key);
            }break;

            case COMMAND_WHISPER:
            {
                CommandWhisper(pSelectedCha);
            }break;

            case COMMAND_GUILD:
            {
                CommandGuild(pSelectedCha);
            }break;

            case COMMAND_GUILDUNION:
            {
                CommandGuildUnion(pSelectedCha);
            }break;

            case COMMAND_RIVAL:
            {
                CommandGuildRival(pSelectedCha);
            }break;

            case COMMAND_RIVALOFF:
            {
                CommandCancelGuildRival(pSelectedCha);
            }break;

            case COMMAND_ADD_FRIEND:
            {
                CommandAddFriend(pSelectedCha);
            }break;

            case COMMAND_FOLLOW:
            {
                CommandFollow(Selectindex);
            }break;

            case COMMAND_BATTLE:
            {
                CommandDual(pSelectedCha);
            }break;
            }
        }
        m_iCurSelectCommand = COMMAND_NONE;
    }
}
int mu::ui::window::CCommandWindow::GetCurCommandType()
{
    return m_iCurSelectCommand;
}

void mu::ui::window::CCommandWindow::SetMouseCursor(int iCursorType)
{
    m_iCurMouseCursor = iCursorType;
}

int mu::ui::window::CCommandWindow::GetMouseCursor()
{
    return m_iCurMouseCursor;
}

bool mu::ui::window::CCommandWindow::CommandTrade(CHARACTER* pSelectedCha)
{
    if (pSelectedCha == NULL)
        return false;

    int level = CharacterAttribute->Level;

    if (level < TRADELIMITLEVEL)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCanUseTheTradeCommandAtCharacterLevel6, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (IsShopInViewport(pSelectedCha))
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotTradeRightNow, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendTradeRequest(pSelectedCha->Key);

    return true;
}

bool mu::ui::window::CCommandWindow::CommandPurchase(CHARACTER* pSelectedCha)
{
    if (pSelectedCha == nullptr)
        return false;

    SocketClient->ToGameServer()->SendPlayerShopItemListRequest(pSelectedCha->Key, MU_C16(pSelectedCha->ID));

    return true;
}

bool mu::ui::window::CCommandWindow::CommandParty(SHORT iChaKey)
{
    if (PartyNumber > 0 && wcscmp(Party[0].Name, Hero->ID) != 0)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouAreAlreadyInAParty, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendPartyInviteRequest(iChaKey);

    return true;
}

bool mu::ui::window::CCommandWindow::CommandWhisper(CHARACTER* pSelectedCha)
{
    g_pChatInputBox->SetWhsprID(pSelectedCha->ID);

    return true;
}

bool mu::ui::window::CCommandWindow::CommandGuild(CHARACTER* pSelectedChar)
{
    if (Hero->GuildStatus != G_NONE)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouAreAlreadyInAGuild, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if ((pSelectedChar->GuildMarkIndex < 0) || (pSelectedChar->GuildStatus != G_MASTER))
    {
        g_pSystemLogBox->AddText(I18N::Game::TheUserIsNotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendGuildJoinRequest(pSelectedChar->Key);

    return true;
}

bool mu::ui::window::CCommandWindow::CommandGuildUnion(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::NotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus == G_NONE)
    {
        g_pSystemLogBox->AddText(I18N::Game::ThisDoesNotBelongToTheGuild, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::TheUserIsNotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus == G_MASTER)
    {
        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Alliance, GuildRequestType::Join, pSelectedCha->Key);
        return true;
    }

    return false;
}

bool mu::ui::window::CCommandWindow::CommandGuildRival(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::NotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::TheUserIsNotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Join, pSelectedCha->Key);

    return true;
}

bool mu::ui::window::CCommandWindow::CommandCancelGuildRival(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::NotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(I18N::Game::TheUserIsNotAGuildMaster, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SetAction(&Hero->Object, PLAYER_RESPECT1);
    SendRequestAction(Hero->Object, AT_RESPECT1);
    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Leave, pSelectedCha->Key);
    return true;
}

bool mu::ui::window::CCommandWindow::CommandAddFriend(CHARACTER* pSelectedCha)
{
    if (g_pWindowMgr->IsServerEnable() == TRUE && pSelectedCha != nullptr)
    {
        SocketClient->ToGameServer()->SendFriendAddRequest(MU_C16(pSelectedCha->ID));
        return true;
    }

    return false;
}

bool mu::ui::window::CCommandWindow::CommandFollow(int iSelectedChaIndex)
{
    if (iSelectedChaIndex < 0)
    {
        return false;
    }

    g_iFollowCharacter = iSelectedChaIndex;

    return true;
}

int mu::ui::window::CCommandWindow::CommandDual(CHARACTER* pSelectedCha)
{
    int iLevel = CharacterAttribute->Level;
    if (iLevel < 30)
    {
        wchar_t szError[48] = L"";
        mu_swprintf(szError, I18N::Game::OpenOnlyForLevelDOrHigher, 30);
        g_pSystemLogBox->AddText(szError, mu::ui::window::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4)
    {
        g_pSystemLogBox->AddText(I18N::Game::DuelingIsNotPossibleInThisArea, mu::ui::window::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotEngageInDuelsWhileInLorenMarket, mu::ui::window::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (!g_DuelMgr.IsDuelEnabled())
    {
        SocketClient->ToGameServer()->SendDuelStartRequest(pSelectedCha->Key, MU_C16(pSelectedCha->ID));
        return 1;
    }
    else if (g_DuelMgr.IsDuelEnabled() && g_DuelMgr.IsDuelPlayer(pSelectedCha, DUEL_ENEMY))
    {
        SocketClient->ToGameServer()->SendDuelStopRequest();
        return 2;
    }
    else
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotChallengePlayerIsAlreadyInADuel, mu::ui::window::TYPE_ERROR_MESSAGE);
        return 3;
    }
    return 0;
}
