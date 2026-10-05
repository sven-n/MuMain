#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserDetails.h"

#include "EffectBrowserLabels.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectTypesJson.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

using Data::Effects::EffectKind;
using MuEditor::Effects::EffectBrowserModel;
using MuEditor::Effects::EffectBrowserRow;
using MuEditor::Effects::EffectCreateTable;
using MuEditor::Effects::EffectTypeRef;

namespace
{
// At most this many types are listed at once; the list scrolls.
constexpr int MaxListedRows = 8;

// A section that starts open; its id stays when the language changes.
bool BeginSection(const char* id, const char* label)
{
    return ImGui::TreeNodeEx(id, ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen, "%s", label);
}

std::string JoinSubTypes(const std::vector<int>& subTypes)
{
    std::string text;
    for (const int subType : subTypes)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(subType);
    }
    return text;
}

std::string CatalogueFile(EffectKind kind)
{
    const std::string fileName(Data::Effects::GetEffectTypesFileName(kind));
    return (Data::Effects::GetEffectDataDirectory() / fileName).generic_string();
}

// Particles, joints and sprites have no registry; their stages are code.
const char* DescribeCodeOf(EffectKind kind)
{
    switch (kind)
    {
    case EffectKind::Particle:
        return I18N::Editor::ParticleCode;
    case EffectKind::Joint:
        return I18N::Editor::JointCode;
    default:
        break;
    }
    return I18N::Editor::SpriteCode;
}

// The names of the effect types `types` in a box of fixed height, under a
// node with their count. Returns the type clicked.
std::optional<int> RenderEffectList(const char* id, const char* label, const EffectBrowserModel& model,
                                    const std::vector<int>& types)
{
    if (types.empty() || !ImGui::TreeNode(id, "%s (%d)", label, static_cast<int>(types.size())))
        return std::nullopt;
    std::optional<int> clicked;
    const int rows = std::min(static_cast<int>(types.size()), MaxListedRows);
    const ImVec2 size(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * rows + ImGui::GetStyle().FramePadding.y * 2.0f);
    if (ImGui::BeginListBox("##types", size))
    {
        for (const int type : types)
        {
            const EffectBrowserRow* row = model.FindRow(EffectKind::Effect, type);
            ImGui::PushID(type);
            if (row != nullptr && ImGui::Selectable(row->name.c_str()))
                clicked = type;
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::TreePop();
    return clicked;
}

// "-" for an unset value. A value of a variant that is the row's is dimmed, so
// the values of the variant stand out.
void RenderValue(const std::string& value, bool sameAsRow)
{
    const bool dim = value.empty() || sameAsRow;
    if (dim)
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", value.empty() ? "-" : value.c_str());
    if (dim)
        ImGui::PopStyleColor();
}

void RenderCreateTable(const EffectCreateTable& table, const std::vector<std::string>& variantSubTypes)
{
    const int columns = static_cast<int>(variantSubTypes.size()) + 2;
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
    if (!ImGui::BeginTable("values", columns, flags))
        return;
    ImGui::TableSetupColumn(I18N::Editor::Field);
    ImGui::TableSetupColumn(variantSubTypes.empty() ? I18N::Editor::AllSubTypes : I18N::Editor::OtherSubTypes);
    for (const std::string& subTypes : variantSubTypes)
    {
        char header[128];
        std::snprintf(header, sizeof(header), "%s %s", I18N::Editor::SubType, subTypes.c_str());
        ImGui::TableSetupColumn(header);
    }
    ImGui::TableHeadersRow();
    for (const EffectCreateTable::Line& line : table.lines)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(line.field.c_str());
        for (size_t column = 0; column < line.values.size(); ++column)
        {
            ImGui::TableNextColumn();
            RenderValue(line.values[column], column > 0 && line.values[column] == line.values[0]);
        }
    }
    ImGui::EndTable();
}
} // namespace

std::optional<EffectTypeRef> CEffectBrowserDetails::Render(const EffectBrowserModel& model,
                                                           std::optional<EffectTypeRef> selected)
{
    const EffectBrowserRow* row = selected ? model.FindRow(selected->kind, selected->type) : nullptr;
    if (row == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::SelectAnEffectType);
        return std::nullopt;
    }
    Refresh(model, *selected);
    RenderIdentity(*row, selected->kind);
    // FX1.7 adds the preview here.
    std::optional<EffectTypeRef> clicked = RenderSameNumber(model);
    RenderAsset(*row);
    if (const std::optional<EffectTypeRef> sharing = RenderStages(model, *row, selected->kind))
        clicked = sharing;
    RenderCreationValues(selected->kind);
    RenderUsedBy(selected->kind);
    return clicked;
}

void CEffectBrowserDetails::Refresh(const EffectBrowserModel& model, EffectTypeRef selected)
{
    if (m_ref == selected)
        return;
    m_ref = selected;
    m_details = model.Describe(selected);
    m_variantSubTypes.clear();
    if (!m_details.creation)
        return;
    for (const std::vector<int>& subTypes : m_details.creation->variantSubTypes)
    {
        m_variantSubTypes.push_back(JoinSubTypes(subTypes));
    }
}

void CEffectBrowserDetails::RenderIdentity(const EffectBrowserRow& row, EffectKind kind)
{
    ImGui::TextUnformatted(row.name.c_str());
    ImGui::Text("%s: %.*s", I18N::Editor::Code, static_cast<int>(row.code.size()), row.code.data());
    ImGui::Text("%s: %d", I18N::Editor::Number, row.type);
    ImGui::Text("%s: %s", I18N::Editor::Kind, MuEditor::Effects::Labels::Kind(kind));
    std::string& file = m_files[Data::Effects::ToIndex(kind)];
    if (file.empty())
        file = CatalogueFile(kind);
    ImGui::Text("%s: %s", I18N::Editor::DefinedIn, file.c_str());
}

std::optional<EffectTypeRef> CEffectBrowserDetails::RenderSameNumber(const EffectBrowserModel& model)
{
    if (!BeginSection("sameNumber", I18N::Editor::SameNumber))
        return std::nullopt;
    if (m_details.sameNumber.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoOtherKind);
        return std::nullopt;
    }
    std::optional<EffectTypeRef> clicked;
    for (const EffectTypeRef& other : m_details.sameNumber)
    {
        const EffectBrowserRow* row = model.FindRow(other.kind, other.type);
        char label[256];
        std::snprintf(label, sizeof(label), "%s: %s (%.*s)", MuEditor::Effects::Labels::Kind(other.kind),
                      row->name.c_str(), static_cast<int>(row->code.size()), row->code.data());
        ImGui::PushID(static_cast<int>(Data::Effects::ToIndex(other.kind)));
        if (ImGui::Selectable(label))
            clicked = other;
        ImGui::PopID();
    }
    return clicked;
}

void CEffectBrowserDetails::RenderAsset(const EffectBrowserRow& row)
{
    if (!BeginSection("asset", I18N::Editor::Asset))
        return;
    const char* slot = MuEditor::Effects::Labels::Slot(row.assetSlot);
    if (row.assetSlot == MuEditor::Effects::EffectAssetSlot::TextureChosenInCode)
        ImGui::TextUnformatted(slot);
    else if (row.asset.loaded)
        ImGui::Text("%s: %s", slot, row.asset.file.c_str());
    else
        ImGui::Text("%s: %s", slot, I18N::Editor::NothingLoaded);
    if (row.assetSlot == MuEditor::Effects::EffectAssetSlot::DefaultTexture)
        ImGui::TextDisabled("%s", I18N::Editor::SlotDefaultTexture);
    if (row.asset.loaded && row.assetSlot == MuEditor::Effects::EffectAssetSlot::Model)
        ImGui::TextDisabled("%s", I18N::Editor::SlotsKeepModels);
    if (MuEditor::Effects::IsWorldObjectSlot(row.assetSlot, row.type))
        ImGui::TextDisabled("%s", I18N::Editor::WorldObjectSlot);
}

std::optional<EffectTypeRef> CEffectBrowserDetails::RenderStages(const EffectBrowserModel& model,
                                                                 const EffectBrowserRow& row, EffectKind kind)
{
    if (!BeginSection("stages", I18N::Editor::Stages))
        return std::nullopt;
    if (kind != EffectKind::Effect)
    {
        ImGui::TextWrapped("%s", DescribeCodeOf(kind));
        return std::nullopt;
    }
    const char* creation = MuEditor::Effects::Labels::Stage(row.stages.create);
    if (m_variantSubTypes.empty())
        ImGui::Text("%s: %s", I18N::Editor::Creation, creation);
    else
        ImGui::Text("%s: %s (%d %s)", I18N::Editor::Creation, creation, static_cast<int>(m_variantSubTypes.size()),
                    I18N::Editor::Variants);
    ImGui::Text("%s: %s", I18N::Editor::Move, MuEditor::Effects::Labels::Stage(row.stages.move));
    ImGui::Text("%s: %s %s %s", I18N::Editor::Drawing, MuEditor::Effects::Labels::Stage(row.stages.render),
                MuEditor::Effects::Labels::GroundSuffix(row.stages),
                MuEditor::Effects::Labels::AfterCharactersSuffix(row.stages));

    std::optional<int> clicked =
        RenderEffectList("sameHook", I18N::Editor::SameCreationHook, model, m_details.sameCreateHook);
    if (const std::optional<int> sameMove =
            RenderEffectList("sameMove", I18N::Editor::SameMoveHandler, model, m_details.sameMoveHandler))
        clicked = sameMove;
    if (!clicked)
        return std::nullopt;
    return EffectTypeRef{EffectKind::Effect, *clicked};
}

void CEffectBrowserDetails::RenderCreationValues(EffectKind kind)
{
    if (kind != EffectKind::Effect || !BeginSection("creation", I18N::Editor::CreationValues))
        return;
    if (!m_details.creation)
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoCreationValues);
        return;
    }
    RenderCreateTable(*m_details.creation, m_variantSubTypes);
}

// The data users of a type (D26): in FX1 only the creation values of the
// effects name types.
void CEffectBrowserDetails::RenderUsedBy(EffectKind kind)
{
    if (!BeginSection("usedBy", I18N::Editor::UsedByData))
        return;
    if (!m_details.creation)
    {
        ImGui::TextDisabled("%s", I18N::Editor::NamedByNoData);
        return;
    }
    ImGui::Text("%s: %s", m_files[Data::Effects::ToIndex(kind)].c_str(), I18N::Editor::NamedByItsCreationValues);
}

#endif // _EDITOR
