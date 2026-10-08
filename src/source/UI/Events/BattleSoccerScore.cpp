//*****************************************************************************
// File: NewUIBattleSoccerScore.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Events/BattleSoccerScore.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Textures/ZzzTexture.h"

#include "Core/Utilities/StringUtils.h"
#include "Guild/GuildMarkPalette.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <string>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
template <typename Model, typename T>
void Sync(RmlModelBinder<Model>& binder, T Model::* field, const char* name, T value)
{
    Model& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

// The 64 cell colours of GuildMark[markIndex]; none for an index ::CreateGuildMark() refuses.
std::vector<Rml::String> MarkCells(int markIndex)
{
    std::vector<Rml::String> cells;
    if (markIndex < 0 || markIndex >= MAX_MARKS)
        return cells;
    cells.reserve(Guild::MarkPalette::CellCount);
    for (int i = 0; i < Guild::MarkPalette::CellCount; ++i)
        cells.push_back(Guild::MarkPalette::CellColor(GuildMark[markIndex].Mark[i]));
    return cells;
}

bool SameTeams(const std::vector<BattleSoccerTeamEntry>& a, const std::vector<BattleSoccerTeamEntry>& b)
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (a[i].red != b[i].red || a[i].score != b[i].score || a[i].name != b[i].name || a[i].mark != b[i].mark)
            return false;
    }
    return true;
}
} // namespace

mu::ui::window::CBattleSoccerScore::CBattleSoccerScore()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

mu::ui::window::CBattleSoccerScore::~CBattleSoccerScore()
{
    Release();
}

bool mu::ui::window::CBattleSoccerScore::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BATTLE_SOCCER_SCORE, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CBattleSoccerScore::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CBattleSoccerScore::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CBattleSoccerScore::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CBattleSoccerScore::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CBattleSoccerScore::Update()
{
    SyncRmlModel();
    return true;
}

bool mu::ui::window::CBattleSoccerScore::Render()
{
    // Nothing native left: the back, the scores, the marks and the names are RmlUi. Kept because
    // CObject requires the override.
    return true;
}

void mu::ui::window::CBattleSoccerScore::BindRmlModel(Rml::DataModelConstructor& c, BattleSoccerScoreRmlModel& model)
{
    c.Bind("bold_text_px", &model.boldTextPx);

    c.RegisterArray<std::vector<Rml::String>>();
    auto team = c.RegisterStruct<BattleSoccerTeamEntry>();
    team.RegisterMember("red", &BattleSoccerTeamEntry::red);
    team.RegisterMember("score", &BattleSoccerTeamEntry::score);
    team.RegisterMember("name", &BattleSoccerTeamEntry::name);
    team.RegisterMember("mark", &BattleSoccerTeamEntry::mark);
    c.RegisterArray<std::vector<BattleSoccerTeamEntry>>();
    c.Bind("teams", &model.teams);
}

void mu::ui::window::CBattleSoccerScore::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CBattleSoccerScore::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 1.8: under the windows (RmlStackingOrder.cpp).
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    // CManager scopes LayoutMode::HudFrame around this window: the bottom HUD's uniform scale, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    Sync(m_RmlView.Binder(), &BattleSoccerScoreRmlModel::boldTextPx, "bold_text_px",
         UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
    SyncTeams();
}

void mu::ui::window::CBattleSoccerScore::SyncTeams()
{
    // The original's RenderContents(): our guild's war (our team's colour first), or the two teams
    // of a spectated match, else only the back.
    std::vector<BattleSoccerTeamEntry> teams;
    if (EnableGuildWar && Hero->GuildMarkIndex != -1)
    {
        const bool redFirst = HeroSoccerTeam == 0;
        teams.push_back({redFirst, std::to_string(GuildWarScore[0]),
                         StringUtils::WideToNarrow(GuildMark[Hero->GuildMarkIndex].GuildName),
                         MarkCells(Hero->GuildMarkIndex)});
        teams.push_back({!redFirst, std::to_string(GuildWarScore[1]), StringUtils::WideToNarrow(GuildWarName),
                         MarkCells(FindGuildMark(GuildWarName))});
    }
    else if (SoccerObserver)
    {
        teams.push_back({true, std::to_string(GuildWarScore[0]), StringUtils::WideToNarrow(SoccerTeamName[0]),
                         MarkCells(FindGuildMark(SoccerTeamName[0]))});
        teams.push_back({false, std::to_string(GuildWarScore[1]), StringUtils::WideToNarrow(SoccerTeamName[1]),
                         MarkCells(FindGuildMark(SoccerTeamName[1]))});
    }

    BattleSoccerScoreRmlModel& model = m_RmlView.GetModel();
    if (SameTeams(model.teams, teams))
        return;
    model.teams = std::move(teams);
    m_RmlView.MarkDirty("teams");
}

int mu::ui::window::CBattleSoccerScore::FindGuildMark(wchar_t* pszGuildName)
{
    for (int i = 0; i < MARK_EDIT; ++i)
    {
        MARK_t* p = &GuildMark[i];
        if (wcscmp(p->GuildName, pszGuildName) == 0)
        {
            return i;
        }
    }
    return 0;
}

float mu::ui::window::CBattleSoccerScore::GetLayerDepth()
{
    return 1.8f;
}
