#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserList.h"

#include "EffectBrowserLabels.h"
#include "EffectBrowserLayout.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>

using Data::Effects::EffectKind;
using MuEditor::Effects::EffectBrowserModel;
using MuEditor::Effects::EffectBrowserRow;
namespace Layout = MuEditor::Effects::Layout;

namespace
{
// Widths at UI scale 1.
constexpr float SearchWidth = 180.0f;
constexpr float StageComboWidth = 140.0f;
constexpr float NumberColumnWidth = 60.0f;

constexpr int BasicColumnCount = 3;
constexpr int EffectColumnCount = 6;

// A label in front of a combo that sets `value` to one of `stages` or to any.
template <typename Stage, size_t Count>
void RenderStageCombo(const char* id, const char* label, std::optional<Stage>& value,
                      const std::array<Stage, Count>& stages)
{
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(StageComboWidth * g_MuEditorCore.GetUIScale());
    const char* preview = value ? MuEditor::Effects::Labels::Stage(*value) : I18N::Editor::Any;
    if (!ImGui::BeginCombo(id, preview))
        return;
    if (ImGui::Selectable(I18N::Editor::Any, !value.has_value()))
        value.reset();
    for (const Stage stage : stages)
    {
        if (ImGui::Selectable(MuEditor::Effects::Labels::Stage(stage), value == stage))
            value = stage;
    }
    ImGui::EndCombo();
}

// Returns whether the row was clicked.
bool RenderRow(const EffectBrowserRow& row, bool withStages, bool selected)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(row.type);
    const bool clicked = ImGui::Selectable(row.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns);
    ImGui::PopID();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(row.code.data(), row.code.data() + row.code.size());
    ImGui::TableNextColumn();
    ImGui::Text("%d", row.type);
    if (!withStages)
        return clicked;
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(MuEditor::Effects::Labels::Stage(row.stages.create));
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(MuEditor::Effects::Labels::Stage(row.stages.move));
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(MuEditor::Effects::Labels::Stage(row.stages.render));
    for (const char* suffix : {MuEditor::Effects::Labels::GroundSuffix(row.stages),
                               MuEditor::Effects::Labels::AfterCharactersSuffix(row.stages)})
    {
        if (suffix[0] == '\0')
            continue;
        ImGui::SameLine();
        ImGui::TextUnformatted(suffix);
    }
    return clicked;
}

// Ctrl+C anywhere in the browser, unless a text field has the keyboard.
void CopySelectedName(const EffectBrowserModel& model, EffectKind kind, int selectedType)
{
    const ImGuiIO& io = ImGui::GetIO();
    const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (!focused || io.WantTextInput || !io.KeyCtrl)
        return;
    // From the next frame on the game gets no keys while Ctrl is held, so the
    // C of Ctrl+C does not open the character window too.
    ImGui::SetNextFrameWantCaptureKeyboard(true);
    if (selectedType < 0 || !ImGui::IsKeyPressed(ImGuiKey_C, false))
        return;
    if (const EffectBrowserRow* row = model.FindRow(kind, selectedType))
        ImGui::SetClipboardText(row->name.c_str());
}
} // namespace

int CEffectBrowserList::Render(const EffectBrowserModel& model, EffectKind kind, int selectedType)
{
    RenderFilters(kind);
    UpdateListed(model, kind);
    ImGui::Text("%s: %d / %d", I18N::Editor::Listed, static_cast<int>(m_listed.size()),
                static_cast<int>(model.GetRows(kind).size()));
    return RenderTable(model, kind, selectedType);
}

void CEffectBrowserList::Show(const EffectBrowserModel& model, MuEditor::Effects::EffectTypeRef type)
{
    const EffectBrowserRow* row = model.FindRow(type.kind, type.type);
    if (row == nullptr)
        return;
    if (!EffectBrowserModel::IsListed(type.kind, *row, m_filter))
    {
        m_filter = {};
        m_search[0] = '\0';
    }
    m_scrollTo = type;
}

void CEffectBrowserList::RenderFilters(EffectKind kind)
{
    ImGui::TextUnformatted(I18N::Editor::Search);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(SearchWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::InputText("##search", m_search, sizeof(m_search)))
        m_filter.search = MuEditor::Effects::ToSearchText(m_search);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::LoadedNow, &m_filter.onlyLoaded);
    if (kind != EffectKind::Effect)
        return;
    RenderStageCombo("##create", I18N::Editor::Creation, m_filter.create, MuEditor::Effects::CreateStages);
    Layout::SameLineIfFits(Layout::LabeledComboWidth(I18N::Editor::Move, StageComboWidth));
    RenderStageCombo("##move", I18N::Editor::Move, m_filter.move, MuEditor::Effects::MoveStages);
    Layout::SameLineIfFits(Layout::LabeledComboWidth(I18N::Editor::Drawing, StageComboWidth));
    RenderStageCombo("##render", I18N::Editor::Drawing, m_filter.render, MuEditor::Effects::RenderStages);
}

// Lists again only when the kind, the filter or the assets changed.
void CEffectBrowserList::UpdateListed(const EffectBrowserModel& model, EffectKind kind)
{
    if (m_listedKind == kind && m_listedFilter == m_filter && m_listedAssetGeneration == model.GetAssetGeneration())
        return;
    m_listed = model.Filter(kind, m_filter);
    m_listedKind = kind;
    m_listedFilter = m_filter;
    m_listedAssetGeneration = model.GetAssetGeneration();
}

int CEffectBrowserList::RenderTable(const EffectBrowserModel& model, EffectKind kind, int selectedType)
{
    const bool withStages = kind == EffectKind::Effect;
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
    if (!ImGui::BeginTable("types", withStages ? EffectColumnCount : BasicColumnCount, flags,
                           ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        return -1;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(I18N::Editor::Name);
    ImGui::TableSetupColumn(I18N::Editor::Code);
    ImGui::TableSetupColumn(I18N::Editor::Number, ImGuiTableColumnFlags_WidthFixed,
                            NumberColumnWidth * g_MuEditorCore.GetUIScale());
    if (withStages)
    {
        ImGui::TableSetupColumn(I18N::Editor::Creation);
        ImGui::TableSetupColumn(I18N::Editor::Move);
        ImGui::TableSetupColumn(I18N::Editor::Drawing);
    }
    ImGui::TableHeadersRow();
    const int clicked = RenderRows(model.GetRows(kind), kind, selectedType);
    ImGui::EndTable();
    CopySelectedName(model, kind, selectedType);
    return clicked;
}

int CEffectBrowserList::RenderRows(std::span<const EffectBrowserRow> rows, EffectKind kind, int selectedType)
{
    int clicked = -1;
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_listed.size()));
    const int scrollRow = TakeScrollRow(rows, kind);
    if (scrollRow >= 0)
        clipper.IncludeItemByIndex(scrollRow);
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const EffectBrowserRow& row = rows[m_listed[i]];
            if (RenderRow(row, kind == EffectKind::Effect, row.type == selectedType))
                clicked = row.type;
            if (i == scrollRow)
                ImGui::SetScrollHereY(0.5f);
        }
    }
    return clicked;
}

int CEffectBrowserList::TakeScrollRow(std::span<const EffectBrowserRow> rows, EffectKind kind)
{
    if (!m_scrollTo || m_scrollTo->kind != kind)
        return -1;
    const int type = m_scrollTo->type;
    m_scrollTo.reset();
    const auto found = std::find_if(m_listed.begin(), m_listed.end(), [&](int row) { return rows[row].type == type; });
    return found != m_listed.end() ? static_cast<int>(found - m_listed.begin()) : -1;
}

#endif // _EDITOR
