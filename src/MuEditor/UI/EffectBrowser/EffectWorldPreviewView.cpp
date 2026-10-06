#include "stdafx.h"

#ifdef _EDITOR

#include "EffectWorldPreviewView.h"

#include "EffectBrowserLabels.h"
#include "EffectBrowserLayout.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "I18N/All.h"
#include "UI/DevEditor/DevEditorUI.h"
#include "UI/NewUI/NewUISystem.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

using Data::Effects::EffectKind;
using MuEditor::Effects::EffectCallSite;
using MuEditor::Effects::EffectWorldPreview;
using MuEditor::Effects::WorldPreviewRequest;
using MuEditor::Effects::WorldPreviewTarget;
namespace Layout = MuEditor::Effects::Layout;

namespace
{
constexpr float ValueWidth = 90.0f;
constexpr int VisibleGameCalls = 8;

// The sources the editor was built from (src/source), for the game's calls.
#ifdef MU_EDITOR_SOURCE_DIR
constexpr const char* SourceDirectory = MU_EDITOR_SOURCE_DIR;
#else
constexpr const char* SourceDirectory = "";
#endif

void TextDisabledWrapped(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
}

void Tooltip(const char* text)
{
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", text);
}

const char* WorldNoteText(MuEditor::Effects::WorldPreviewNote note)
{
    switch (note)
    {
    case MuEditor::Effects::WorldNoteNothingCreated:
        return I18N::Editor::WorldNothingCreated;
    case MuEditor::Effects::WorldNoteEndedAtOnce:
        return I18N::Editor::WorldEndedAtOnce;
    case MuEditor::Effects::WorldNoteNoCharacterNear:
        return I18N::Editor::WorldNoCharacterNear;
    case MuEditor::Effects::WorldNoteOwnPlace:
        return I18N::Editor::WorldOwnPlace;
    case MuEditor::Effects::WorldNoteFollowsOwner:
        return I18N::Editor::WorldFollowsOwner;
    case MuEditor::Effects::WorldNoteOwnSize:
        return I18N::Editor::WorldOwnSize;
    case MuEditor::Effects::WorldNoteOwnLight:
        return I18N::Editor::WorldOwnLight;
    case MuEditor::Effects::WorldNoteRefused:
        break;
    }
    return I18N::Editor::WorldRefused;
}

const char* TargetLabel(WorldPreviewTarget target)
{
    switch (target)
    {
    case WorldPreviewTarget::None:
        return I18N::Editor::CallTargetNone;
    case WorldPreviewTarget::NearestCharacter:
        return I18N::Editor::CallTargetNearest;
    case WorldPreviewTarget::Character:
        break;
    }
    return I18N::Editor::CallTargetCharacter;
}

// What a game call writes besides its place: SubType, size, owner.
std::string DescribeCall(const EffectCallSite& call)
{
    std::string text = std::string(I18N::Editor::SubType) + " " + (call.subType.empty() ? "0" : call.subType);
    if (!call.scale.empty())
        text += "  " + std::string(I18N::Editor::CallSize) + " " + call.scale;
    text += "  " + std::string(call.kind == EffectKind::Joint ? I18N::Editor::CallTarget : I18N::Editor::CallOwner) +
            " " + (call.withoutOwner ? I18N::Editor::CallTargetNone : call.owner);
    if (call.randomTypes > 1)
    {
        char random[64];
        std::snprintf(random, sizeof(random), I18N::Editor::CallRandomTypes, call.randomTypes);
        text += "  (" + std::string(random) + ")";
    }
    return text;
}
} // namespace

std::optional<int> CEffectWorldPreviewView::Render(EffectWorldPreview& world, EffectKind kind, int type, int subType)
{
    if (m_selected != MuEditor::Effects::EffectTypeRef{kind, type})
        Select(kind, type);
    ImGui::SeparatorText(I18N::Editor::InTheWorld);
    const bool ready = MuEditor::Effects::IsWorldReadyForPreview();
    const WorldPreviewRequest request{kind, type, subType, m_call};
    RenderButtons(world, request, ready);
    RenderCallValues(kind);
    const std::optional<int> used = RenderGameCalls();
    // What runs creates with the values as they are now.
    world.UpdateRunning({kind, type, used.value_or(subType), m_call});
    RenderRunning(world, request);
    RenderNotes(world, ready);
    return used;
}

// The values stay for another type of the same kind, so parts of one look
// can be created with them; a kind starts with its own.
void CEffectWorldPreviewView::Select(EffectKind kind, int type)
{
    if (!m_selected || m_selected->kind != kind)
        m_call = MuEditor::Effects::DefaultWorldPreviewCall(kind);
    m_selected = MuEditor::Effects::EffectTypeRef{kind, type};
    m_typeCalls = m_gameCalls.IsLoaded() ? m_gameCalls.Find(kind, type) : std::vector<const EffectCallSite*>();
    if (m_call.target == WorldPreviewTarget::None &&
        std::none_of(m_typeCalls.begin(), m_typeCalls.end(),
                     [](const EffectCallSite* call) { return call->withoutOwner; }))
        m_call.target = WorldPreviewTarget::Character;
}

void CEffectWorldPreviewView::RenderButtons(EffectWorldPreview& world, const WorldPreviewRequest& request, bool ready)
{
    ImGui::BeginDisabled(!ready);
    if (ImGui::Button(I18N::Editor::CreateInWorld))
        world.Start(request);
    ImGui::EndDisabled();
    Tooltip(I18N::Editor::CreateInWorldTooltip);

    Layout::SameLineIfFits(Layout::ButtonWidth(I18N::Editor::StopPreview));
    ImGui::BeginDisabled(!world.IsRunning());
    if (ImGui::Button(I18N::Editor::StopPreview))
        world.Stop();
    ImGui::EndDisabled();

    Layout::SameLineIfFits(Layout::CheckboxWidth(I18N::Editor::RepeatPreview));
    bool repeat = world.GetRepeat();
    if (ImGui::Checkbox(I18N::Editor::RepeatPreview, &repeat))
        world.SetRepeat(repeat);
    Tooltip(I18N::Editor::RepeatPreviewTooltip);

    Layout::SameLineIfFits(Layout::CheckboxWidth(I18N::Editor::MuteSounds));
    bool mute = world.GetMute();
    if (ImGui::Checkbox(I18N::Editor::MuteSounds, &mute))
        world.SetMute(mute);
    Tooltip(I18N::Editor::MuteSoundsTooltip);
}

void CEffectWorldPreviewView::RenderCallValues(EffectKind kind)
{
    if (!ImGui::TreeNode("callValues", "%s", I18N::Editor::CallValues))
        return;
    const float width = ValueWidth * g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(I18N::Editor::CallDistance, &m_call.distance, 1.0f, 0.0f, 1500.0f, "%.0f");
    Tooltip(I18N::Editor::CallDistanceTooltip);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(I18N::Editor::CallHeight, &m_call.height, 1.0f, -300.0f, 1000.0f, "%.0f");
    ImGui::SetNextItemWidth(width);
    ImGui::DragFloat(I18N::Editor::CallSize, &m_call.scale, 0.05f, 0.0f, 200.0f, "%.2f");
    Tooltip(I18N::Editor::CallSizeTooltip);
    ImGui::SameLine();
    // Lightning gets the light only as a colour the call passes.
    const bool joint = kind == EffectKind::Joint;
    if (joint)
    {
        ImGui::Checkbox("##jointColour", &m_call.jointColour);
        Tooltip(I18N::Editor::CallColourTooltip);
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    }
    ImGui::BeginDisabled(joint && !m_call.jointColour);
    ImGui::ColorEdit3(I18N::Editor::CallLight, m_call.light.data(),
                      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    ImGui::EndDisabled();
    if (joint)
        Tooltip(I18N::Editor::CallColourTooltip);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::CallRandomAngle, &m_call.randomAngle);
    RenderTarget();
    if (kind == EffectKind::Joint)
    {
        ImGui::SetNextItemWidth(width);
        ImGui::InputInt(I18N::Editor::CallPk, &m_call.pk);
        Tooltip(I18N::Editor::CallPkTooltip);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(width);
        ImGui::InputInt(I18N::Editor::CallSkillIndex, &m_call.skillIndex);
        Tooltip(I18N::Editor::CallPkTooltip);
    }
    ImGui::TreePop();
}

// None only for types the game creates without an owner somewhere: the
// creation code of others may read their owner without checking it.
void CEffectWorldPreviewView::RenderTarget()
{
    ImGui::SetNextItemWidth(2.5f * ValueWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo(I18N::Editor::CallTarget, TargetLabel(m_call.target)))
        return;
    if (ImGui::IsWindowAppearing())
        LoadGameCalls();
    const bool noneAllowed = std::any_of(m_typeCalls.begin(), m_typeCalls.end(),
                                         [](const EffectCallSite* call) { return call->withoutOwner; });
    constexpr std::array<WorldPreviewTarget, 3> targets = {WorldPreviewTarget::Character, WorldPreviewTarget::None,
                                                           WorldPreviewTarget::NearestCharacter};
    for (const WorldPreviewTarget target : targets)
    {
        const bool disabled = target == WorldPreviewTarget::None && !noneAllowed;
        if (ImGui::Selectable(TargetLabel(target), m_call.target == target,
                              disabled ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
            m_call.target = target;
        if (disabled)
            Tooltip(I18N::Editor::CallTargetNoneTooltip);
    }
    ImGui::EndCombo();
}

std::optional<int> CEffectWorldPreviewView::RenderGameCalls()
{
    char label[96];
    if (m_gameCalls.IsLoaded())
        std::snprintf(label, sizeof(label), "%s (%d)###gameCalls", I18N::Editor::GameCalls,
                      static_cast<int>(m_typeCalls.size()));
    else
        std::snprintf(label, sizeof(label), "%s###gameCalls", I18N::Editor::GameCalls);
    if (!ImGui::TreeNode(label))
        return std::nullopt;
    LoadGameCalls();
    std::optional<int> used;
    if (!m_gameCallsFound)
        TextDisabledWrapped(I18N::Editor::GameCallsNotFound);
    else if (m_typeCalls.empty())
        TextDisabledWrapped(I18N::Editor::NoGameCalls);
    else
    {
        const float rows = static_cast<float>(std::min<size_t>(m_typeCalls.size(), VisibleGameCalls));
        if (ImGui::BeginChild("gameCalls", ImVec2(0.0f, rows * ImGui::GetFrameHeightWithSpacing()),
                              ImGuiChildFlags_Borders))
        {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(m_typeCalls.size()));
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    const EffectCallSite& call = *m_typeCalls[static_cast<size_t>(i)];
                    ImGui::PushID(i);
                    if (ImGui::SmallButton(I18N::Editor::UseCall))
                        used = Use(call);
                    ImGui::SameLine();
                    ImGui::Text("%s:%d", call.file.c_str(), call.line);
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", DescribeCall(call).c_str());
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::TreePop();
    return used;
}

// Takes the values a game call writes out; the others stay.
std::optional<int> CEffectWorldPreviewView::Use(const EffectCallSite& call)
{
    m_call.scale = call.scaleValue.value_or(0.0f);
    m_call.light = call.light.value_or(MuEditor::Effects::PreviewVector{1.0f, 1.0f, 1.0f});
    if (call.withoutOwner)
        m_call.target = WorldPreviewTarget::None;
    else if (m_call.target == WorldPreviewTarget::None)
        m_call.target = WorldPreviewTarget::Character;
    if (call.kind == EffectKind::Joint)
    {
        m_call.jointColour = call.light.has_value();
        m_call.pk = call.pkValue.value_or(-1);
        m_call.skillIndex = call.skillIndexValue.value_or(0);
    }
    return call.subTypeValue;
}

void CEffectWorldPreviewView::LoadGameCalls()
{
    if (!m_gameCalls.IsLoaded())
        m_gameCallsFound = m_gameCalls.Load(SourceDirectory);
    if (m_selected)
        m_typeCalls = m_gameCalls.Find(m_selected->kind, m_selected->type);
}

// What the preview keeps alive in the world.
void CEffectWorldPreviewView::RenderRunning(const EffectWorldPreview& world, const WorldPreviewRequest& request) const
{
    if (!world.IsRunning())
        return;
    if (request.kind == EffectKind::Sprite)
    {
        TextDisabledWrapped(I18N::Editor::SpriteEveryFrame);
        return;
    }
    const MuEditor::Effects::EffectPreviewCounts counts = world.GetCounts();
    namespace Labels = MuEditor::Effects::Labels;
    ImGui::TextDisabled("%s %d  %s %d  %s %d", Labels::Kind(EffectKind::Effect), counts.effects,
                        Labels::Kind(EffectKind::Particle), counts.particles, Labels::Kind(EffectKind::Joint),
                        counts.joints);
}

// Why nothing shows: the game's state and what the last call did.
void CEffectWorldPreviewView::RenderNotes(const EffectWorldPreview& world, bool ready) const
{
    if (!ready)
        TextDisabledWrapped(I18N::Editor::WorldNotInGame);
    if (!g_pOption->GetRenderAllEffects())
        TextDisabledWrapped(I18N::Editor::WorldEffectsOff);
    if (!g_DevEditorUI.ShouldRenderEffects())
        TextDisabledWrapped(I18N::Editor::WorldEffectsHidden);
    const std::uint16_t notes = world.GetNotes();
    for (std::uint16_t bit = 1; bit != 0 && bit <= notes; bit = static_cast<std::uint16_t>(bit << 1))
    {
        if ((notes & bit) != 0)
            TextDisabledWrapped(WorldNoteText(static_cast<MuEditor::Effects::WorldPreviewNote>(bit)));
    }
}

#endif // _EDITOR
