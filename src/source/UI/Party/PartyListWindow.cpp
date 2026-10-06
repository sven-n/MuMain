
#include "stdafx.h"
#include "UI/Chat/Chat.h"

#include "UI/Party/PartyListWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"

#include "Engine/Object/ZzzInventory.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInterface.h"
#include "Camera/CameraProjection.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Party/PartyListLayout.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CPartyListWindow::CPartyListWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_bActive = false;
    m_iVal = UI::Party::List::CardSpacing;
    m_iSelectedCharacter = -1;

    for (int i = 0; i < MAX_PARTYS; i++)
    {
        m_iPartyListBGColor[i] = PARTY_LIST_BGCOLOR_DEFAULT;
        m_bPartyMemberoutofSight[i] = false;
    }
}

CPartyListWindow::~CPartyListWindow()
{
    Release();
}

bool CPartyListWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_PARTY_INFO_WINDOW, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(true);

    return true;
}

void CPartyListWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void CPartyListWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

// Right-aligned to the world the open windows leave uncovered, in this window's own units.
void CPartyListWindow::FollowUncoveredWorld()
{
    const float right = UI::Placement::UncoveredWorldRightIn(GetLayoutTransform());
    m_Pos.x = static_cast<int>(std::lround(right)) - (PARTY_LIST_WINDOW_WIDTH + 2);
}

int CPartyListWindow::GetSelectedCharacter()
{
    if (m_iSelectedCharacter == -1)
        return -1;

    return Party[m_iSelectedCharacter].index;
}

void CPartyListWindow::SetListBGColor()
{
    for (int i = 0; i < PartyNumber; i++)
    {
        m_iPartyListBGColor[i] = PARTY_LIST_BGCOLOR_DEFAULT;

        if (Party[i].index == -1)
        {
            m_iPartyListBGColor[i] = PARTY_LIST_BGCOLOR_RED;
        }

        if (Party[i].index > -1)
        {
            m_iPartyListBGColor[i] = PARTY_LIST_BGCOLOR_GREEN;
        }
    }
}

bool CPartyListWindow::BtnProcess()
{
    m_iSelectedCharacter = -1;

    for (int i = 0; i < PartyNumber; i++)
    {
        int iVal = i * m_iVal;

        // The leave buttons are RmlUi's (party_leave); the card hover stays here.
        if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y + iVal, PARTY_LIST_WINDOW_WIDTH, PARTY_LIST_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        {
            m_iSelectedCharacter = i;

            if (SelectedCharacter == -1) {
                CHARACTER* c = &CharactersClient[Party[i].index];
                if (c && c != Hero) {
                    UI::Chat::CreateChat(c->ID, L"", c);
                }
            }

            if (SelectCharacterInPartyList(&Party[i]))
            {
                return true;
            }
        }
    }

    return false;
}

bool CPartyListWindow::UpdateMouseEvent()
{
    FollowUncoveredWorld();
    if (!m_bActive)
        return true;

    if (true == BtnProcess())
        return false;

    if (PartyNumber > 0)
    {
        int iHeight = (PARTY_LIST_WINDOW_HEIGHT * PartyNumber) + (4 * (PartyNumber - 1));
        if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, PARTY_LIST_WINDOW_WIDTH, iHeight).Contains(MouseX, MouseY))
        {
            return false;
        }
    }

    return true;
}

bool CPartyListWindow::UpdateKeyEvent()
{
    return true;
}

bool CPartyListWindow::Update()
{
    FollowUncoveredWorld();
    if (m_PendingLeave >= 0)
    {
        const int member = m_PendingLeave;
        m_PendingLeave = -1;
        if (member < PartyNumber && CanLeave(member))
            g_pPartyInfoWindow->LeaveParty(member);
    }

    // Before the reset below: Party[].index still holds what last frame's character pass found,
    // which is what the original's Render() read.
    SyncRmlModel();

    if (PartyNumber <= 0)
    {
        m_bActive = false;
        return true;
    }

    m_bActive = true;

    for (int i = 0; i < PartyNumber; i++)
    {
        Party[i].index = -2;
    }

    return true;
}

bool CPartyListWindow::Render()
{
    // Nothing native left: the cards are RmlUi. Kept because CObject requires the override.
    return true;
}

bool CPartyListWindow::CanLeave(int member) const
{
    // The leader may remove anyone; everyone else only themselves.
    return !wcscmp(Party[0].Name, Hero->ID) || !wcscmp(Party[member].Name, Hero->ID);
}

void CPartyListWindow::BindRmlModel(Rml::DataModelConstructor& c, PartyListRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);

    auto card = c.RegisterStruct<PartyListCardEntry>();
    card.RegisterMember("name", &PartyListCardEntry::name);
    card.RegisterMember("name_text_px", &PartyListCardEntry::nameTextPx);
    card.RegisterMember("leader", &PartyListCardEntry::leader);
    card.RegisterMember("absent", &PartyListCardEntry::absent);
    card.RegisterMember("defense_buff", &PartyListCardEntry::defenseBuff);
    card.RegisterMember("selected", &PartyListCardEntry::selected);
    card.RegisterMember("show_leave", &PartyListCardEntry::showLeave);
    card.RegisterMember("health_length", &PartyListCardEntry::healthLength);
    card.RegisterMember("index", &PartyListCardEntry::index);
    c.RegisterArray<std::vector<PartyListCardEntry>>();
    c.Bind("cards", &model.cards);

    c.BindEventCallback("party_leave",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PendingLeave = arguments[0].Get<int>(-1);
                        });
}

void CPartyListWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CPartyListWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    const bool visible = IsVisible() && PartyNumber > 0;
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), visible);
    if (!visible)
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_Pos);
    SyncCards(UI::Scaling::GetActiveTransform());
}

void CPartyListWindow::SyncCards(const UI::Scaling::Transform& transform)
{
    using namespace UI::Party::List;

    g_pRenderText->SetFont(g_hFont);

    std::vector<PartyListCardEntry> cards;
    cards.reserve(static_cast<size_t>(PartyNumber));
    for (int i = 0; i < PartyNumber; ++i)
    {
        const PARTY_t& member = Party[i];
        PartyListCardEntry card;
        card.index = i;
        card.name = StringUtils::WideToNarrow(member.Name);
        card.leader = i == 0;
        card.absent = member.index == -1;
        card.selected = !card.absent && m_iSelectedCharacter == i;
        card.showLeave = CanLeave(i);
        card.healthLength = HealthBarLength(member.stepHP);
        if (member.index >= 0 && member.index < MAX_CHARACTERS_CLIENT)
        {
            OBJECT* memberObject = &CharactersClient[member.index].Object;
            card.defenseBuff = g_isCharacterBuff(memberObject, eBuff_Defense);
        }

        const float nameBox = static_cast<float>(card.leader ? LeaderNameBoxWidth : MemberNameBoxWidth);
        const float nameWidth = static_cast<float>(g_pRenderText->MeasureText(member.Name, lstrlen(member.Name)).cx);
        card.nameTextPx =
            UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform, nameWidth, nameBox);

        cards.push_back(std::move(card));
    }

    PartyListRmlModel& model = m_RmlView.GetModel();
    const bool changed = cards.size() != model.cards.size() ||
                         !std::equal(cards.begin(), cards.end(), model.cards.begin(),
                                     [](const PartyListCardEntry& a, const PartyListCardEntry& b)
                                     {
                                         return a.name == b.name && a.nameTextPx == b.nameTextPx &&
                                                a.absent == b.absent && a.defenseBuff == b.defenseBuff &&
                                                a.selected == b.selected && a.showLeave == b.showLeave &&
                                                a.healthLength == b.healthLength;
                                     });
    if (!changed)
        return;

    model.cards = std::move(cards);
    m_RmlView.MarkDirty("cards");
}

void mu::ui::window::CPartyListWindow::RenderPartyHPOnHead()
{
    if (PartyNumber <= 0)
        return;

    float   Width = 38.f;
    wchar_t    Text[100];

    for (int j = 0; j < PartyNumber; ++j)
    {
        PARTY_t* p = &Party[j];

        if (p->index <= -1) continue;

        CHARACTER* c = &CharactersClient[p->index];
        OBJECT* o = &c->Object;
        vec3_t      Position;
        int         ScreenX, ScreenY;

        Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 100.f, Position);

        BeginOpengl();
        CameraProjection::WorldToScreen(g_Camera, Position, &ScreenX, &ScreenY);
        EndOpengl();

        ScreenX -= (int)(Width / 2);

        if ((MouseX >= ScreenX && MouseX < ScreenX + Width && MouseY >= ScreenY - 2 && MouseY < ScreenY + 6))
        {
            mu_swprintf(Text, L"HP : %d0%%", p->stepHP);
            g_pRenderText->SetTextColor(255, 230, 210, 255);
            g_pRenderText->RenderText(ScreenX, ScreenY - 6, Text);
        }

        EnableAlphaTest();
        RenderColorQuadARGB((float)(ScreenX + 1), (float)(ScreenY + 1), Width + 4.f, 5.f, 0x80000000u);

        EnableAlphaBlend();
        RenderColorQuadARGB((float)ScreenX, (float)ScreenY, Width + 4.f, 5.f, 0xFF330000u);
        RenderColorQuadARGB((float)(ScreenX + 2), (float)(ScreenY + 2), Width, 1.f, 0xFF320A00u);

        int stepHP = std::min<int>(10, p->stepHP);

        for (int k = 0; k < stepHP; ++k)
        {
            RenderColorQuadARGB((float)(ScreenX + 2 + (k * 4)), (float)(ScreenY + 2), 3.f, 2.f,
                0xFFFA0A00u);
        }
        DisableAlphaBlend();
    }
    DisableAlphaBlend();
}

float CPartyListWindow::GetLayerDepth()
{
    return 5.4f;
}

void CPartyListWindow::OpenningProcess()
{
}

void CPartyListWindow::ClosingProcess()
{
}

bool CPartyListWindow::SelectCharacterInPartyList(PARTY_t* pMember)
{
    auto HeroClass = gCharacterManager.GetBaseClass(Hero->Class);

    if (HeroClass == CLASS_ELF
        || HeroClass == CLASS_WIZARD
        || HeroClass == CLASS_SUMMONER
        )
    {
        auto Skill = CharacterAttribute->Skill[Hero->CurrentSkill];

        if (Skill == AT_SKILL_HEALING
            || Skill == AT_SKILL_HEALING_STR
            || Skill == AT_SKILL_DEFENSE
            || Skill == AT_SKILL_DEFENSE_STR
            || Skill == AT_SKILL_DEFENSE_MASTERY
            || Skill == AT_SKILL_ATTACK
            || Skill == AT_SKILL_ATTACK_STR
            || Skill == AT_SKILL_ATTACK_MASTERY
            || Skill == AT_SKILL_TELEPORT_ALLY
            || Skill == AT_SKILL_SOUL_BARRIER
            || Skill == AT_SKILL_SOUL_BARRIER_STR
            || Skill == AT_SKILL_SOUL_BARRIER_PROFICIENCY
            || Skill == AT_SKILL_ALICE_THORNS
            || Skill == AT_SKILL_RECOVER
            )
        {
            SelectedCharacter = pMember->index;
            return true;
        }
    }

    return false;
}
