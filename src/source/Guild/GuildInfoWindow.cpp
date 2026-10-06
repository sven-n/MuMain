
#include "stdafx.h"
#include "GuildInfoWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Audio/DSPlaySound.h"
#include "GuildTypes.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Scenes/SceneCore.h" // g_iLengthAuthorityCode -- CGuildBreakPasswordMsgBoxLayout's own maxLength
#include "I18N/All.h"

#include "Character/CharacterManager.h"

#include "Core/Utilities/StringUtils.h"
#include "Guild/GuildMarkPalette.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Text/TextWrap.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

int	DeleteIndex = 0;
int AppointStatus = 0;
wchar_t DeleteID[100];

extern MARK_t GuildMark[MAX_MARKS];

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    // Was CGuildBreakPasswordMsgBoxLayout (CustomMessageBox.h) -- a masked (bIsPassword=true),
    // non-numeric-restricted Mode::Text WEBZEN.COM password entry, ported 2026-09-14. Shared across
    // this file's 3 call sites (guild-disband, leave-guild, kick-member), all of which already set
    // the global `DeleteIndex` right before showing this. Unlike the other masked-password dialogs
    // in this batch, native's own ProcessOk does NOT return CALLBACK_CONTINUE on empty input -- it
    // always closes, just logs an error message -- so this one deliberately never calls KeepOpen().
    void ShowGuildBreakPasswordDialog()
    {
        GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::IfYouWantToLeaveYourGuild, false },
            { I18N::Game::PleaseEnterYourWEBZENCOMPassword, false },
        };
        cfg.input = GenericDialogConfig::InputField{};
        cfg.input->mode = GenericDialogConfig::InputField::Mode::Text;
        cfg.input->maxLength = g_iLengthAuthorityCode;
        cfg.input->masked = true;
        cfg.onPrimary = []
        {
            const std::wstring strText = g_pGenericConfirmDialog->GetInputText();
            if (!strText.empty())
            {
                SocketClient->ToGameServer()->SendGuildKickPlayerRequest(MU_C16(GuildList[DeleteIndex].Name), MU_C16(strText.c_str()));
            }
            else
            {
                g_pSystemLogBox->AddText(I18N::Game::ThePasswordYouHaveEnteredIsIncorrect, mu::ui::window::TYPE_ERROR_MESSAGE);
            }
        };
        g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

namespace
{
// The mark cells of a 64-byte guild mark, as CreateGuildMark() builds the texture.
std::vector<Rml::String> MarkCells(const BYTE* mark)
{
    std::vector<Rml::String> cells;
    cells.reserve(Guild::MarkPalette::CellCount);
    for (int i = 0; i < Guild::MarkPalette::CellCount; ++i)
        cells.push_back(Guild::MarkPalette::CellColor(mark[i]));
    return cells;
}

} // namespace

int mu::ui::window::CGuildInfoWindow::GetGuildMemberIndex(const wchar_t* szName)
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && !wcscmp(GuildList[i].Name, szName))
            return i;
    }

    return -1;
}

mu::ui::window::CGuildInfoWindow::CGuildInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_nCurrentTab = static_cast<int>(GuildConstants::GuildTab::MEMBERS);
    m_EventState = EVENT_NONE;
    m_Tot_Notice = 0;

    m_bRequestUnionList = false;
}

mu::ui::window::CGuildInfoWindow::~CGuildInfoWindow()
{
    Release();
}

bool mu::ui::window::CGuildInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GUILDINFO, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CGuildInfoWindow::OpenningProcess()
{
    m_nCurrentTab = static_cast<int>(GuildConstants::GuildTab::MEMBERS);

    SocketClient->ToGameServer()->SendGuildListRequest();
}

void mu::ui::window::CGuildInfoWindow::ClosingProcess()
{
    m_bRequestUnionList = false;
}

void mu::ui::window::CGuildInfoWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CGuildInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CGuildInfoWindow::UpdateMouseEvent()
{
    bool ret = true;

    if (mu::ui::window::IsPress(VK_LBUTTON))
    {
        ret = Check_Mouse(MouseX, MouseY);
        if (ret == false)
        {
            PlayBuffer(SOUND_CLICK01);
        }
    }

    // Top-right corner close "X" (shared frame): hides + swallows the click. The buttons are
    // RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GUILDINFO))
    {
        m_EventState = EVENT_NONE;
        return false;
    }

    float panelWidth = static_cast<float>(GUILDINFO_WIDTH);
    float panelHeight = static_cast<float>(GUILDINFO_HEIGHT);
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                       static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        return false;
    }

    return ret;
}

bool mu::ui::window::CGuildInfoWindow::Check_Btn(int button)
{
    if (button == BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUILDINFO);
        m_EventState = EVENT_NONE;
        return false;
    }

    if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO))
    {
        if (button == BUTTON_GUILD_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
                {
                    // First proof case for CGenericConfirmDialog (see UI/Dialogs/GenericConfirmDialog.h) --
                    // was CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuildOutPerson)), an OK-only informational box.
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.lines.push_back({ I18N::Game::AllianceMasterCanTDisbandTheGuild, true });
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
                else
                {
                    DeleteIndex = GetGuildMemberIndex(Hero->ID);
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.showCancel = true;
                    cfg.lines = {
                        { I18N::Game::OnceYouDisbandTheGuild, true },
                        { I18N::Game::AllTheItemsAndZenInTheGuildVaultWillDisappear, true },
                        { I18N::Game::AlsoTheGuildRankingInformationWillDisappear, true },
                        { I18N::Game::WouldYouLikeToDisbandTheGuild, true },
                    };
                    cfg.onPrimary = []
                    {
                        ShowGuildBreakPasswordDialog();
                    };
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
            }
            else
            {
                DeleteIndex = GetGuildMemberIndex(Hero->ID);
                ShowGuildBreakPasswordDialog();
            }
        }
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS))
    {
        if (button == BUTTON_GET_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (const MemberEntry* pText = SelectedMember())
                {
                    if (pText->guildStatus != G_MASTER)
                    {
                        {
                            DeleteIndex = GetGuildMemberIndex(pText->name.c_str());
                            wchar_t szNameText[300];
                            mu_swprintf(szNameText, I18N::Game::CharacterS, GuildList[DeleteIndex].Name);
                            mu::ui::window::GenericDialogConfig cfg;
                            cfg.showCancel = true;
                            cfg.lines = {
                                { szNameText, true },
                                { I18N::Game::WouldYouLikeToRelease, true },
                            };
                            cfg.onPrimary = []
                            {
                                ShowGuildBreakPasswordDialog();
                            };
                            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                        }
                    }
                }
            }
        }
        else if (button == BUTTON_GET_POSITION)
        {
            if (const MemberEntry* pText = SelectedMember())
            {
                if (Hero->GuildStatus == G_MASTER)
                {
                    if (pText->guildStatus != G_MASTER)
                    {
                        {
                            AppointStatus = (GUILD_STATUS)pText->guildStatus;
                            DeleteIndex = GetGuildMemberIndex(pText->name.c_str());
                            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuild_ToPerson_PositionLayout));
                        }
                    }
                }
            }
        }
        else if (button == BUTTON_FREE_POSITION)
        {
            if (const MemberEntry* pText = SelectedMember())
            {
                if (pText->guildStatus == G_SUB_MASTER || pText->guildStatus == G_BATTLE_MASTER)
                {
                    {
                        AppointStatus = (GUILD_STATUS)pText->guildStatus;
                        DeleteIndex = GetGuildMemberIndex(pText->name.c_str());
                        wchar_t strText[256];
                        mu_swprintf(strText, I18N::Game::CharacterS, pText->name.c_str());
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = {
                            { strText, false },
                            { I18N::Game::WouldYouLikeToCancelTheRanking, false },
                        };
                        cfg.onPrimary = []
                        {
                            SocketClient->ToGameServer()->SendGuildRoleAssignRequest(G_PERSON, MU_C16(GuildList[DeleteIndex].Name), 0x03);
                        };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    }
                }
            }
        }
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::UNION))
    {
        if (button == BUTTON_UNION_CREATE)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (const UnionEntry* pText = SelectedUnion())
                {
                    if (wcscmp(pText->name.c_str(), GuildMark[Hero->GuildMarkIndex].GuildName))
                    {
                        wcscpy(DeleteID, pText->name.c_str());
                        wchar_t szAllianceText[256];
                        mu_swprintf(szAllianceText, I18N::Game::SGuildFromTheAlliance, DeleteID);
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = {
                            { szAllianceText, false },
                            { I18N::Game::WouldYouLikeToRelease, false },
                        };
                        cfg.onPrimary = [] { SocketClient->ToGameServer()->SendRemoveAllianceGuildRequest(MU_C16(DeleteID)); };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    }
                }
            }
        }
        else if (button == BUTTON_UNION_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                const bool isUnionMaster = wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName) == 0;
                if (isUnionMaster)
                {
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.lines.push_back({ I18N::Game::AllianceMasterCanTWithdrawTheGuild, true });
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
                else
                {
                    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Alliance, GuildRequestType::Leave, Hero->Key);
                }
            }
        }
    }
    return true;
}

bool mu::ui::window::CGuildInfoWindow::Check_Mouse(int mx, int my)
{
    if (mx > m_Pos.x && mx < (m_Pos.x + GUILDINFO_WIDTH) && my > m_Pos.y && my < (m_Pos.y + GUILDINFO_HEIGHT))
    {
        for (int i = 0; i < 3; i++)
        {
            int Tab_Pos = i * 56;
            if (mx > (m_Pos.x + 12 + Tab_Pos) && mx < (m_Pos.x + 12 + Tab_Pos + 56) && my > m_Pos.y && my < (m_Pos.y + 90))
            {
                m_nCurrentTab = i;
                switch (m_nCurrentTab)
                {
                case static_cast<int>(GuildConstants::GuildTab::INFO):
                    break;
                case static_cast<int>(GuildConstants::GuildTab::MEMBERS):
                {
                    SocketClient->ToGameServer()->SendGuildListRequest();
                }
                break;
                case static_cast<int>(GuildConstants::GuildTab::UNION):
                {
                    if (m_bRequestUnionList == false
                        && GuildMark[Hero->GuildMarkIndex].UnionName[0] != 0)
                    {
                        SocketClient->ToGameServer()->SendRequestAllianceList();
                        m_bRequestUnionList = true;
                    }
                }
                break;
                }
                return false;
            }
        }
    }
    // Both lists are RmlUi scroll panes now and own their own thumbs.
    return true;
}

bool mu::ui::window::CGuildInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUILDINFO) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUILDINFO);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}
bool mu::ui::window::CGuildInfoWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in Check_Btn()).
    const int button = m_PendingButton;
    m_PendingButton = -1;
    if (IsVisible() && button >= 0)
    {
        PlayBuffer(SOUND_CLICK01);
        Check_Btn(button);
    }

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CGuildInfoWindow::Render()
{
    // Nothing native left: the frame, the tabs, the lists' lines, the marks and the buttons are
    // RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CGuildInfoWindow::BindRmlModel(Rml::DataModelConstructor& c, GuildInfoRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("panel_width", &model.panelWidth);
    c.Bind("text_px", &model.textPx);
    c.Bind("no_guild", &model.noGuild);
    c.Bind("tab", &model.tab);
    c.Bind("union_shown", &model.unionShown);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("mark_cells", &model.markCells);

    auto lineType = c.RegisterStruct<GuildLine>();
    lineType.RegisterMember("text", &GuildLine::text);
    lineType.RegisterMember("text_px", &GuildLine::textPx);
    c.Bind("hint_title", &model.hintTitle);
    c.Bind("hint_line1", &model.hintLine1);
    c.Bind("hint_line2", &model.hintLine2);
    c.Bind("hint_line3", &model.hintLine3);
    c.Bind("title", &model.title);
    c.Bind("guild_name", &model.guildName);
    c.Bind("tab_info", &model.tabInfo);
    c.Bind("tab_members", &model.tabMembers);
    c.Bind("tab_union", &model.tabUnion);
    c.Bind("notice_label", &model.noticeLabel);
    c.Bind("created", &model.created);
    c.Bind("score", &model.score);
    c.Bind("member_count", &model.memberCount);
    c.Bind("rival", &model.rival);
    c.Bind("header_name", &model.headerName);
    c.Bind("header_position", &model.headerPosition);
    c.Bind("header_server", &model.headerServer);
    c.Bind("header_union_name", &model.headerUnionName);
    c.Bind("header_union_members", &model.headerUnionMembers);
    c.RegisterArray<std::vector<GuildLine>>();
    c.Bind("alliance_lines", &model.allianceLines);
    auto noticeRow = c.RegisterStruct<GuildNoticeRow>();
    noticeRow.RegisterMember("text", &GuildNoticeRow::text);
    c.RegisterArray<std::vector<GuildNoticeRow>>();
    c.Bind("notice_rows", &model.noticeRows);
    auto memberRow = c.RegisterStruct<GuildMemberRow>();
    memberRow.RegisterMember("name", &GuildMemberRow::name);
    memberRow.RegisterMember("role", &GuildMemberRow::role);
    memberRow.RegisterMember("role_text_px", &GuildMemberRow::roleTextPx);
    memberRow.RegisterMember("server", &GuildMemberRow::server);
    memberRow.RegisterMember("selected", &GuildMemberRow::selected);
    memberRow.RegisterMember("officer", &GuildMemberRow::officer);
    c.RegisterArray<std::vector<GuildMemberRow>>();
    c.Bind("member_rows", &model.memberRows);
    auto unionRow = c.RegisterStruct<GuildUnionRow>();
    unionRow.RegisterMember("name", &GuildUnionRow::name);
    unionRow.RegisterMember("member_count", &GuildUnionRow::memberCount);
    unionRow.RegisterMember("count_text_px", &GuildUnionRow::countTextPx);
    unionRow.RegisterMember("mark_cells", &GuildUnionRow::markCells);
    unionRow.RegisterMember("selected", &GuildUnionRow::selected);
    c.RegisterArray<std::vector<GuildUnionRow>>();
    c.Bind("union_rows", &model.unionRows);
    auto actionButton = c.RegisterStruct<GuildActionButton>();
    actionButton.RegisterMember("label", &GuildActionButton::label);
    actionButton.RegisterMember("shown", &GuildActionButton::shown);
    c.Bind("guild_out_button", &model.guildOutButton);
    c.Bind("get_position_button", &model.getPositionButton);
    c.Bind("free_position_button", &model.freePositionButton);
    c.Bind("get_out_button", &model.getOutButton);
    c.Bind("union_create_button", &model.unionCreateButton);
    c.Bind("union_out_button", &model.unionOutButton);
    c.Bind("exit_tooltip", &model.exitTooltip);
    c.Bind("label_line_px", &model.labelLinePx);
    c.BindEventCallback("guild_info_button",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PendingButton = arguments[0].Get<int>(-1);
                        });
    c.BindEventCallback("guild_info_select_union",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                SelectUnion(arguments[0].Get<int>(-1));
                        });
    c.BindEventCallback("guild_info_select_member",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                SelectMember(arguments[0].Get<int>(-1));
                        });
}

void mu::ui::window::CGuildInfoWindow::OnRmlReloaded()
{
    m_ListsDirty = true;
}

void mu::ui::window::CGuildInfoWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CGuildInfoWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 4.5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_Pos);
    UI::RmlBridge::SyncPanelWidth(m_RmlView.Binder(), m_RmlView.Document());
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncContent();
}

void mu::ui::window::CGuildInfoWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    const float x0 = static_cast<float>(m_Pos.x);
    const float y0 = static_cast<float>(m_Pos.y);

    // One of the window's own lines: the document places it, so only what it says and the size the
    // native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, bool boldFont, float boxWidth) -> GuildLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(boldFont ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = boldFont ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            boxWidth > 0.f
                ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), boxWidth)
                : UI::Scaling::NativeTextPixelSize(role, transform);
        return {StringUtils::WideToNarrow(text), px};
    };
    const bool noGuild = Hero->GuildStatus == G_NONE;
    std::vector<Rml::String> markCells;
    std::vector<GuildLine> allianceLines;
    GuildLine hintTitle, hintLine1, hintLine2, hintLine3;
    GuildLine title, guildName, tabInfo, tabMembers, tabUnion;
    GuildLine noticeLabel, created, score, memberCount, rival;
    GuildLine headerName, headerPosition, headerServer, headerUnionName, headerUnionMembers;
    GuildActionButton guildOutButton, getPositionButton, freePositionButton, getOutButton;
    GuildActionButton unionCreateButton, unionOutButton;
    bool unionShown = false;
    wchar_t Text[300] = {};

    if (noGuild)
    {
        // The original's RenderNoneGuild().
        hintTitle = line(I18N::Game::Guild, true, 190.f);
        hintLine1 = line(I18N::Game::TypeGuildInFrontOf, false, 0.f);
        hintLine2 = line(I18N::Game::TheGuildMasterYouWantToJoin, false, 0.f);
        hintLine3 = line(I18N::Game::AndYouCanJoinTheGuild, false, 0.f);
    }
    else
    {
        // The original's Render_Text() and each tab's Render_*().
        const float tabWidth = static_cast<float>(GuildConstants::UILayout::TAB_WIDTH);
        title = line(I18N::Game::Guild, false, 190.f);
        mu_swprintf(Text, L"%ls ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
        guildName = line(Text, false, 120.f);
        tabInfo = line(I18N::Game::Guild, false, tabWidth);
        tabMembers = line(I18N::Game::Members, false, tabWidth);
        tabUnion = line(I18N::Game::Alliance, false, tabWidth);

        if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO))
        {
            markCells = MarkCells(GuildMark[Hero->GuildMarkIndex].Mark);
            guildOutButton = {StringUtils::WideToNarrow(Hero->GuildStatus == G_MASTER ? I18N::Game::Disband
                                                                                      : I18N::Game::Leave),
                              true};
            noticeLabel = line(I18N::Game::GuildAnnouncement, false, 40.f);

            mu_swprintf(Text, L"%ls :", I18N::Game::GuildCreationDate);
            created = line(Text, false, 40.f);
            mu_swprintf(Text, I18N::Game::GuildScoreD, GuildTotalScore);
            score = line(Text, false, 80.f);
            if (Hero->GuildStatus == G_MASTER)
            {
                const int Class = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
                if (Class == CLASS_DARK_LORD)
                {
                    int nCount = CharacterAttribute->Level / 10 + CharacterAttribute->Charisma / 10;
                    if (nCount > 80)
                        nCount = 80;
                    mu_swprintf(Text, I18N::Game::GuildMembersDD, g_nGuildMemberCount, nCount);
                }
                else
                {
                    mu_swprintf(Text, I18N::Game::GuildMembersDD, g_nGuildMemberCount, CharacterAttribute->Level / 10);
                }
            }
            else
            {
                mu_swprintf(Text, I18N::Game::GuildMemberD, g_nGuildMemberCount);
            }
            memberCount = line(Text, false, 80.f);
            mu_swprintf(Text, L"%ls : %ls", I18N::Game::HostilityGuild,
                        m_RivalGuildName[0] ? m_RivalGuildName : I18N::Game::None);
            rival = line(Text, false, 0.f);
        }
        else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS))
        {
            headerName = line(I18N::Game::Name, false, 40.f);
            headerPosition = line(I18N::Game::Position, false, 40.f);
            headerServer = line(I18N::Game::Server, false, 40.f);

            if (Hero->GuildStatus == G_MASTER)
            {
                getPositionButton = {StringUtils::WideToNarrow(I18N::Game::Position), true};
                freePositionButton = {StringUtils::WideToNarrow(I18N::Game::Dissolve), true};
                getOutButton = {StringUtils::WideToNarrow(I18N::Game::Release), true};
            }
        }
        else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::UNION))
        {
            unionShown = GuildMark[Hero->GuildMarkIndex].UnionName[0] != 0;
            if (unionShown)
            {
                headerUnionName = line(I18N::Game::NAME, false, 40.f);
                headerUnionMembers = line(I18N::Game::Members, false, 40.f);


                unionCreateButton = {StringUtils::WideToNarrow(I18N::Game::DisbandAlliance), true};
                unionOutButton = {StringUtils::WideToNarrow(I18N::Game::DisbandGuildAlliance), true};
            }
            else
            {
                // Render_Guild_Info()'s explanation.
                const wchar_t* lines[] = {
                    I18N::Game::ToMakeTheAlliance,
                    I18N::Game::FaceTheGuildMaster,
                    I18N::Game::OfDesiredGuildForGuildAlliance,
                    I18N::Game::EnterAllianceOrGuildAlliance,
                    I18N::Game::ButtonInCommandWindow,
                    I18N::Game::IfTheOppositeIsNotAGuild,
                    I18N::Game::AllianceOppositeAllianceShould,
                    I18N::Game::BeTheMainAllianceForCreating,
                    I18N::Game::GuildAllianceRequestThe,
                    I18N::Game::RegistrationToOppositeAlliance,
                    I18N::Game::IfTheOppositeIsGuildAlliance,
                };
                for (const wchar_t* text : lines)
                    allianceLines.push_back(line(text, false, 0.f));
            }
        }
    }

    GuildInfoRmlModel& model = m_RmlView.GetModel();
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::noGuild, "no_guild", noGuild);
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::tab, "tab", m_nCurrentTab);
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::unionShown, "union_shown", unionShown);
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::markCells, "mark_cells", std::move(markCells));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::hintTitle, "hint_title", std::move(hintTitle));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::hintLine1, "hint_line1", std::move(hintLine1));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::hintLine2, "hint_line2", std::move(hintLine2));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::hintLine3, "hint_line3", std::move(hintLine3));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::title, "title", std::move(title));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::guildName, "guild_name", std::move(guildName));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::tabInfo, "tab_info", std::move(tabInfo));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::tabMembers, "tab_members", std::move(tabMembers));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::tabUnion, "tab_union", std::move(tabUnion));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::noticeLabel, "notice_label", std::move(noticeLabel));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::created, "created", std::move(created));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::score, "score", std::move(score));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::memberCount, "member_count", std::move(memberCount));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::rival, "rival", std::move(rival));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::headerName, "header_name", std::move(headerName));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::headerPosition, "header_position", std::move(headerPosition));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::headerServer, "header_server", std::move(headerServer));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::headerUnionName, "header_union_name", std::move(headerUnionName));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::headerUnionMembers, "header_union_members",
              std::move(headerUnionMembers));
    SyncListContent();
    if (model.allianceLines != allianceLines)
    {
        model.allianceLines = std::move(allianceLines);
        m_RmlView.MarkDirty("alliance_lines");
    }
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::guildOutButton, "guild_out_button", std::move(guildOutButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::getPositionButton, "get_position_button",
              std::move(getPositionButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::freePositionButton, "free_position_button",
              std::move(freePositionButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::getOutButton, "get_out_button", std::move(getOutButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::unionCreateButton, "union_create_button",
              std::move(unionCreateButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::unionOutButton, "union_out_button", std::move(unionOutButton));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::exitTooltip, "exit_tooltip",
              StringUtils::WideToNarrow(I18N::Game::Close388));

    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::labelLinePx, "label_line_px",
              static_cast<float>(lineHeight) * transform.scaleY);
}

void mu::ui::window::CGuildInfoWindow::SyncListContent()
{
    const auto& model = m_RmlView.GetModel();
    if (model.rootScale != m_ListScale || model.textPx != m_ListTextPx)
        m_ListsDirty = true;
    if (!m_ListsDirty)
        return;
    m_ListScale = model.rootScale;
    m_ListTextPx = model.textPx;
    m_ListsDirty = false;

    std::vector<GuildNoticeRow> noticeRows;
    noticeRows.reserve(m_NoticeLines.size());
    for (const auto& text : m_NoticeLines)
        noticeRows.push_back({StringUtils::WideToNarrow(text.c_str())});
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::noticeRows, "notice_rows", std::move(noticeRows));
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::memberRows, "member_rows", BuildMemberRows());
    SyncField(m_RmlView.Binder(), &GuildInfoRmlModel::unionRows, "union_rows", BuildUnionRows());
}

std::vector<mu::ui::window::GuildMemberRow> mu::ui::window::CGuildInfoWindow::BuildMemberRows() const
{
    const auto transform = UI::Scaling::GetActiveTransform();
    // A list cell's own shrink box, for the two cells that had one.
    auto cellPx = [&](const wchar_t* text, float boxWidth)
    {
        g_pRenderText->SetFont(g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform,
                                                     static_cast<float>(measured), boxWidth);
    };

    std::vector<GuildMemberRow> memberRows;
    // Officers share the selected-row backdrop.
    memberRows.reserve(m_Members.size());
    for (auto it = m_Members.begin(); it != m_Members.end(); ++it)
    {
        const wchar_t* role = nullptr;
        if (it->guildStatus == G_MASTER)
            role = I18N::Game::Master;
        else if (it->guildStatus == G_SUB_MASTER)
            role = I18N::Game::AssistM;
        else if (it->guildStatus == G_BATTLE_MASTER)
            role = I18N::Game::BattleM;
        GuildMemberRow row;
        row.name = StringUtils::WideToNarrow(it->name.c_str());
        if (role != nullptr)
        {
            row.role = StringUtils::WideToNarrow(role);
            row.roleTextPx = cellPx(role, 70.f);
        }
        if (it->server != 255)
        {
            wchar_t server[16] = {};
            mu_swprintf(server, L"%d", it->server + 1);
            row.server = StringUtils::WideToNarrow(server);
        }
        row.selected = !m_SelectedMember.empty() && it->name == m_SelectedMember;
        row.officer = role != nullptr;
        memberRows.push_back(std::move(row));
    }

    return memberRows;
}

float mu::ui::window::CGuildInfoWindow::GetLayerDepth()
{
    return 4.5f;
}

std::vector<mu::ui::window::GuildUnionRow> mu::ui::window::CGuildInfoWindow::BuildUnionRows() const
{
    constexpr float kMemberCountBoxWidth = 60.f;
    const auto transform = UI::Scaling::GetActiveTransform();
    std::vector<GuildUnionRow> rows;
    rows.reserve(m_Unions.size());
    g_pRenderText->SetFont(g_hFont);
    for (const UnionEntry& entry : m_Unions)
    {
        const std::wstring count = std::to_wstring(entry.memberCount);
        const int measured = g_pRenderText->MeasureText(count.c_str(), static_cast<int>(count.size())).cx;
        GuildUnionRow row;
        row.name = StringUtils::WideToNarrow(entry.name.c_str());
        row.memberCount = StringUtils::WideToNarrow(count.c_str());
        row.countTextPx = UI::Scaling::NativeTextPixelSizeInBox(
            UI::Scaling::FontRole::Normal, transform, static_cast<float>(measured), kMemberCountBoxWidth);
        row.markCells = MarkCells(entry.mark);
        row.selected = entry.name == m_SelectedUnion;
        rows.push_back(std::move(row));
    }
    return rows;
}

void mu::ui::window::CGuildInfoWindow::AddGuildNotice(wchar_t* szText)
{
    m_ListsDirty = true;
    wchar_t szTemp[GuildConstants::UILayout::TEXT_MAX_LINES][MAX_TEXT_LENGTH + 1] = { {0}, {0}, {0}, {0}, {0} };
    CutText3(szText, szTemp[0], GuildConstants::UILayout::TEXT_SPLIT_WIDTH, GuildConstants::UILayout::TEXT_MAX_LINES, MAX_TEXT_LENGTH + 1);

    for (int i = 0; i < GuildConstants::UILayout::TEXT_MAX_LINES; ++i)
    {
        if (szTemp[i][0])
        {
            m_NoticeLines.emplace_back(szTemp[i]);
            m_Tot_Notice++;
        }
    }
}

void mu::ui::window::CGuildInfoWindow::AddGuildMember(GUILD_LIST_t* pInfo)
{
    m_ListsDirty = true;
    if (pInfo == nullptr || pInfo->Name[0] == L'\0')
        return;
    MemberEntry entry;
    entry.name = pInfo->Name;
    entry.number = pInfo->Number;
    entry.server = pInfo->Server;
    entry.guildStatus = pInfo->GuildStatus;
    m_Members.push_back(std::move(entry));
    m_SelectedMember = m_Members.front().name;
}

void mu::ui::window::CGuildInfoWindow::GuildClear()
{
    m_ListsDirty = true;
    m_Members.clear();
    m_SelectedMember.clear();
}

const mu::ui::window::CGuildInfoWindow::MemberEntry* mu::ui::window::CGuildInfoWindow::SelectedMember() const
{
    if (m_SelectedMember.empty())
        return nullptr;
    for (const MemberEntry& entry : m_Members)
    {
        if (entry.name == m_SelectedMember)
            return &entry;
    }
    return nullptr;
}

void mu::ui::window::CGuildInfoWindow::SelectMember(int displayIndex)
{
    m_ListsDirty = true;
    const auto& rows = m_RmlView.GetModel().memberRows;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(rows.size()))
        return;
    m_SelectedMember = StringUtils::NarrowToWide(rows[displayIndex].name);
    if (SelectedMember() == nullptr)
        m_SelectedMember.clear();
    m_RmlView.MarkDirty("member_rows");
}

void mu::ui::window::CGuildInfoWindow::UnionGuildClear()
{
    m_ListsDirty = true;
    m_Unions.clear();
    m_SelectedUnion.clear();
}

const mu::ui::window::CGuildInfoWindow::UnionEntry* mu::ui::window::CGuildInfoWindow::SelectedUnion() const
{
    if (m_SelectedUnion.empty())
        return nullptr;
    for (const UnionEntry& entry : m_Unions)
    {
        if (entry.name == m_SelectedUnion)
            return &entry;
    }
    return nullptr;
}

void mu::ui::window::CGuildInfoWindow::SelectUnion(int displayIndex)
{
    m_ListsDirty = true;
    const auto& rows = m_RmlView.GetModel().unionRows;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(rows.size()))
        return;
    m_SelectedUnion = StringUtils::NarrowToWide(rows[displayIndex].name);
    if (SelectedUnion() == nullptr)
        m_SelectedUnion.clear();
    m_RmlView.MarkDirty("union_rows");
}

void mu::ui::window::CGuildInfoWindow::NoticeClear()
{
    m_ListsDirty = true;
    m_NoticeLines.clear();
    m_Tot_Notice = 0;
}

void mu::ui::window::CGuildInfoWindow::SetRivalGuildName(wchar_t* szName)
{
    wcsncpy(m_RivalGuildName, szName, MAX_GUILDNAME);
    m_RivalGuildName[MAX_GUILDNAME] = 0;
}

void mu::ui::window::CGuildInfoWindow::AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount)
{
    m_ListsDirty = true;
    if (szGuildName == nullptr || szGuildName[0] == L'\0')
        return;

    UnionEntry entry;
    entry.name.assign(szGuildName, wcsnlen(szGuildName, MAX_GUILDNAME));
    entry.memberCount = nMemberCount;
    if (pGuildMark != nullptr)
        memcpy(entry.mark, pGuildMark, sizeof(entry.mark));
    m_Unions.push_back(std::move(entry));
    m_SelectedUnion = m_Unions.front().name;
    m_bRequestUnionList = false;
}

int mu::ui::window::CGuildInfoWindow::GetUnionCount()
{
    return static_cast<int>(m_Unions.size());
}

void mu::ui::window::CGuildInfoWindow::ReceiveGuildRelationShip(GuildRelationshipType byRelationShipType, GuildRequestType byRequestType,
    BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL)
{
    if (mu::ui::window::g_pGenericConfirmDialog->IsVisible())
    {
        SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
            byRelationShipType,
            byRequestType,
            0x00,
            MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
    }
    else
    {
        m_MessageInfo.s_byRelationShipType = byRelationShipType;
        m_MessageInfo.s_byRelationShipRequestType = byRequestType;
        m_MessageInfo.s_byTargetUserIndexH = byTargetUserIndexH;
        m_MessageInfo.s_byTargetUserIndexL = byTargetUserIndexL;

        int nCharKey = MAKEWORD(m_MessageInfo.s_byTargetUserIndexL, m_MessageInfo.s_byTargetUserIndexH);
        int nIndex = FindCharacterIndex(nCharKey);
        if (nIndex < 0 || nIndex >= MAX_CHARACTERS_CLIENT)
            return;
        CHARACTER* pPlayer = &CharactersClient[nIndex];

        wchar_t szText[3][64];
        ZeroMemory(szText, sizeof(szText));

        if (m_MessageInfo.s_byRelationShipType == GuildRelationshipType::Alliance)
        {
            if (m_MessageInfo.s_byRelationShipRequestType == GuildRequestType::Join)
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAGuildAlliance, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedARegistrationRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
            else										// Break Off
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAGuildAlliance, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedAWithdrawalRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
        }
        else if (m_MessageInfo.s_byRelationShipType == GuildRelationshipType::Hostility)
        {
            if (m_MessageInfo.s_byRelationShipRequestType == GuildRequestType::Join)
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAHostileGuild, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedApprovalRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
            else
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAHostileGuild, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedCancellationRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
        }

        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { szText[0], false },
            { szText[1], false },
            { szText[2], false },
        };
        cfg.onPrimary = [byRelationShipType, byRequestType, byTargetUserIndexH, byTargetUserIndexL]
        {
            SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
                byRelationShipType, byRequestType, 0x01,
                MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
        };
        cfg.onCancel = [byRelationShipType, byRequestType, byTargetUserIndexH, byTargetUserIndexL]
        {
            SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
                byRelationShipType, byRequestType, 0x00,
                MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}
