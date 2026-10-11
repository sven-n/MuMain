#include "stdafx.h"
#include "MainFrameExtras.h"

#include "Audio/DSPlaySound.h"
#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/Core/WindowSystem.h"

#include <RmlUi/Core/DataModelHandle.h>

#include <string>

namespace UI::Hud::MainFrameExtras
{
namespace
{
struct Model
{
    Rml::String coordX;
    Rml::String coordY;
    bool partyOpen = false;
    bool guildOpen = false;
    bool leftCovered = false;
    bool rightCovered = false;
    Rml::String partyTooltip;
    Rml::String guildTooltip;
};

Model s_model;
Rml::DataModelHandle s_handle;

void Toggle(mu::ui::window::INTERFACE_LIST window)
{
    g_pNewUISystem->Toggle(window);
    PlayBuffer(SOUND_CLICK01);
}

void SyncText(Rml::String& field, const char* name, Rml::String value)
{
    if (field == value)
        return;
    field = std::move(value);
    s_handle.DirtyVariable(name);
}

void SyncBool(bool& field, const char* name, bool value)
{
    if (field == value)
        return;
    field = value;
    s_handle.DirtyVariable(name);
}
} // namespace

void BindModel(Rml::DataModelConstructor& constructor)
{
    constructor.Bind("coord_x", &s_model.coordX);
    constructor.Bind("coord_y", &s_model.coordY);
    constructor.Bind("party_open", &s_model.partyOpen);
    constructor.Bind("guild_open", &s_model.guildOpen);
    constructor.Bind("left_covered", &s_model.leftCovered);
    constructor.Bind("right_covered", &s_model.rightCovered);
    constructor.Bind("party_tooltip", &s_model.partyTooltip);
    constructor.Bind("guild_tooltip", &s_model.guildTooltip);
    constructor.BindEventCallback("mainframe_party_click",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Toggle(mu::ui::window::INTERFACE_PARTY); });
    constructor.BindEventCallback("mainframe_guild_click",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Toggle(mu::ui::window::INTERFACE_GUILDINFO); });
    s_handle = constructor.GetModelHandle();
}

void SyncModel()
{
    if (!s_handle)
        return;

    if (Hero != nullptr)
    {
        SyncText(s_model.coordX, "coord_x", std::to_string(Hero->PositionX));
        SyncText(s_model.coordY, "coord_y", std::to_string(Hero->PositionY));
    }
    SyncBool(s_model.partyOpen, "party_open", g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_PARTY));
    SyncBool(s_model.guildOpen, "guild_open", g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUILDINFO));
    // A window docked on a side covers that side's dragon in 0.97 (the HUD document draws over the
    // windows here, so the dragon hides instead).
    SyncBool(s_model.leftCovered, "left_covered", UI::Placement::UncoveredWorldLeft() > 0.5f);
    SyncBool(s_model.rightCovered, "right_covered",
             UI::Placement::UncoveredWorldRight() < static_cast<float>(WindowWidth) - 0.5f);
    SyncText(s_model.partyTooltip, "party_tooltip", StringUtils::WideToNarrow(I18N::Game::PartyP));
    SyncText(s_model.guildTooltip, "guild_tooltip", StringUtils::WideToNarrow(I18N::Game::GuildG));
}
} // namespace UI::Hud::MainFrameExtras
