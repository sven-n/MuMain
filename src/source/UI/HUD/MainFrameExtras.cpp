#include "stdafx.h"
#include "MainFrameExtras.h"

#include "Audio/DSPlaySound.h"
#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "UI/Core/WindowSystem.h"
#include "Engine/Object/ZzzInventory.h"
#include "World/MapInfra/MapManager.h"

#include <RmlUi/Core/DataModelHandle.h>

#include <cwchar>
#include <string>

namespace UI::Hud::MainFrameExtras
{
namespace
{
// A bound text and the source it was made from, so a frame whose source has not changed costs a
// comparison instead of a conversion.
struct WideText
{
    std::wstring source;
    Rml::String text;
};

struct Model
{
    Rml::String coordX;
    Rml::String coordY;
    bool partyOpen = false;
    bool guildOpen = false;
    bool commandOpen = false;
    WideText partyTooltip;
    WideText guildTooltip;
    WideText commandTooltip;
    int heroX = -1;
    int heroY = -1;
};

Model s_model;
Rml::DataModelHandle s_handle;

void Toggle(mu::ui::window::INTERFACE_LIST window)
{
    g_pNewUISystem->Toggle(window);
    PlayBuffer(SOUND_CLICK01);
}

// The command window, under the D key's rules (HotKey.cpp): nothing in Chaos Castle, a notice in a
// battle zone.
void ToggleCommandWindow()
{
    if (gMapManager.InChaosCastle())
        return;
    if (::IsStrifeMap(gMapManager.WorldActive))
    {
        if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::TheCommandWindowCannotBeActivatedInBattleZone) == FALSE)
            g_pSystemLogBox->AddText(I18N::Game::TheCommandWindowCannotBeActivatedInBattleZone, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return;
    }
    Toggle(mu::ui::window::INTERFACE_COMMAND);
}

void SyncText(Rml::String& field, const char* name, Rml::String value)
{
    if (field == value)
        return;
    field = std::move(value);
    s_handle.DirtyVariable(name);
}

void SyncWide(WideText& field, const char* name, const wchar_t* source)
{
    if (source == nullptr || std::wcscmp(field.source.c_str(), source) == 0)
        return;
    field.source = source;
    field.text = StringUtils::WideToNarrow(source);
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
    constructor.Bind("command_open", &s_model.commandOpen);
    constructor.Bind("party_tooltip", &s_model.partyTooltip.text);
    constructor.Bind("guild_tooltip", &s_model.guildTooltip.text);
    constructor.Bind("command_tooltip", &s_model.commandTooltip.text);
    constructor.BindEventCallback("mainframe_command_click",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { ToggleCommandWindow(); });
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

    // The coordinates are formatted only when the hero moves to another tile.
    if (Hero != nullptr && (Hero->PositionX != s_model.heroX || Hero->PositionY != s_model.heroY))
    {
        s_model.heroX = Hero->PositionX;
        s_model.heroY = Hero->PositionY;
        SyncText(s_model.coordX, "coord_x", std::to_string(s_model.heroX));
        SyncText(s_model.coordY, "coord_y", std::to_string(s_model.heroY));
    }
    SyncBool(s_model.partyOpen, "party_open", g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_PARTY));
    SyncBool(s_model.guildOpen, "guild_open", g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUILDINFO));
    SyncBool(s_model.commandOpen, "command_open", g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_COMMAND));
    SyncWide(s_model.partyTooltip, "party_tooltip", I18N::Game::PartyP);
    SyncWide(s_model.guildTooltip, "guild_tooltip", I18N::Game::GuildG);
    SyncWide(s_model.commandTooltip, "command_tooltip", I18N::Game::CommandD);
}
} // namespace UI::Hud::MainFrameExtras
