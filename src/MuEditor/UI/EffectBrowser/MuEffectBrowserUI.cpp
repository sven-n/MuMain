#include "stdafx.h"

#ifdef _EDITOR

#include "MuEffectBrowserUI.h"

#include "EffectBrowserAssets.h"
#include "EffectBrowserLabels.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "I18N/All.h"
#include "Render/Effects/EffectRegistry.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <string_view>
#include <utility>

using Data::Effects::EffectKind;
using MuEditor::Effects::EffectTypeRef;

namespace
{
// The size of the window when it opens the first time, at UI scale 1, and the
// most of the screen it takes then.
constexpr float DefaultWidth = 1000.0f;
constexpr float DefaultHeight = 620.0f;
constexpr float MaxDefaultScreenShare = 0.8f;
// The share of the width the list takes the first time; its border can be
// dragged.
constexpr float ListShare = 0.5f;
} // namespace

CMuEffectBrowserUI& CMuEffectBrowserUI::GetInstance()
{
    static CMuEffectBrowserUI instance;
    return instance;
}

void CMuEffectBrowserUI::Render(bool* open)
{
    const float scale = g_MuEditorCore.GetUIScale();
    const ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(std::min(DefaultWidth * scale, screen.x * MaxDefaultScreenShare),
                                    std::min(DefaultHeight * scale, screen.y * MaxDefaultScreenShare)),
                             ImGuiCond_FirstUseEver);
    // The id after ### keeps the window's place when the language changes.
    char title[128];
    std::snprintf(title, sizeof(title), "%s###EffectBrowser", I18N::Editor::EffectBrowser);
    if (!ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }
    // The lists and the combos count as the window, so clicks there do not
    // reach the game.
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        g_MuEditorCore.SetHoveringUI(true);
    if (BuildWhenLoaded())
    {
        RefreshAssetsWhenMapChanges();
        RenderMapLine();
        RenderTabs();
    }
    ImGui::End();
}

void CMuEffectBrowserUI::AfterRender(bool open)
{
    m_worldPreview.AfterFrame(open, MuEditor::Effects::IsWorldReadyForPreview());
}

bool CMuEffectBrowserUI::BuildWhenLoaded()
{
    if (m_model.IsBuilt())
        return true;
    // The registry is built right after the catalogue; a lookup before that
    // would log an error.
    if (g_EffectTypeCatalogue.GetTypeCount(EffectKind::Effect) == 0)
    {
        ImGui::TextDisabled("%s", I18N::Editor::EffectTypesNotLoaded);
        return false;
    }
    m_model.Build(g_EffectTypeCatalogue, &Render::Effects::Lookup);
    return true;
}

void CMuEffectBrowserUI::RefreshAssetsWhenMapChanges()
{
    if (m_assetWorld != gMapManager.WorldActive)
        RefreshAssets();
}

void CMuEffectBrowserUI::RefreshAssets()
{
    m_assetWorld = gMapManager.WorldActive;
    m_model.RefreshAssets(MuEditor::Effects::ProbeLoadedAsset, *m_assetWorld);
    NameMap();
}

void CMuEffectBrowserUI::NameMap()
{
    m_mapName = MuEditor::Effects::Labels::MapName(*m_assetWorld);
    m_mapNameLocale = I18N::GetCurrentLocale();
}

void CMuEffectBrowserUI::RenderMapLine()
{
    // The login and character screens are named in the editor's language.
    if (m_mapNameLocale != I18N::GetCurrentLocale())
        NameMap();
    ImGui::Text("%s: %s (%d)", I18N::Editor::CurrentMap, m_mapName.c_str(), *m_assetWorld);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Refresh))
        RefreshAssets();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s\n%s", I18N::Editor::RefreshAssets, I18N::Editor::SlotsKeepModels);
}

void CMuEffectBrowserUI::RenderTabs()
{
    if (!ImGui::BeginTabBar("kinds"))
        return;
    // Taken before the tabs: a type clicked in this frame's details asks for
    // its tab in the next frame, also when that tab comes before this one.
    const std::optional<EffectKind> tabToSelect = std::exchange(m_tabToSelect, std::nullopt);
    for (const EffectKind kind : Data::Effects::EffectKinds)
    {
        // The id after ### is the kind, so the tab stays when the language
        // changes.
        const std::string_view id = Data::Effects::GetEffectKindName(kind);
        char label[128];
        std::snprintf(label, sizeof(label), "%s (%d)###%.*s", MuEditor::Effects::Labels::Kind(kind),
                      static_cast<int>(m_model.GetRows(kind).size()), static_cast<int>(id.size()), id.data());
        const ImGuiTabItemFlags flags = tabToSelect == kind ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
        if (ImGui::BeginTabItem(label, nullptr, flags))
        {
            RenderKind(kind);
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}

void CMuEffectBrowserUI::RenderKind(EffectKind kind)
{
    int& selected = m_selected[Data::Effects::ToIndex(kind)];
    const float listWidth = ImGui::GetContentRegionAvail().x * ListShare;
    if (ImGui::BeginChild("list", ImVec2(listWidth, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        const int clicked = m_list.Render(m_model, kind, selected);
        if (clicked >= 0)
            selected = clicked;
    }
    ImGui::EndChild();

    ImGui::SameLine();
    const std::optional<EffectTypeRef> current =
        selected >= 0 ? std::optional<EffectTypeRef>(EffectTypeRef{kind, selected}) : std::nullopt;
    // The world preview runs only for the type shown.
    m_worldPreview.KeepOnly(current);
    std::optional<EffectTypeRef> clicked;
    if (ImGui::BeginChild("details", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders))
        clicked = m_details.Render(m_model, current, m_worldPreview);
    ImGui::EndChild();
    if (clicked)
        Show(*clicked);
}

// Selects the type in its tab and scrolls the list to it.
void CMuEffectBrowserUI::Show(EffectTypeRef type)
{
    m_tabToSelect = type.kind;
    m_selected[Data::Effects::ToIndex(type.kind)] = type.type;
    m_list.Show(m_model, type);
}

#endif // _EDITOR
