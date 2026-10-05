#include "stdafx.h"

#ifdef _EDITOR

#include "EffectWorldPreviewView.h"

#include "EffectBrowserLabels.h"
#include "EffectBrowserLayout.h"
#include "I18N/All.h"
#include "UI/DevEditor/DevEditorUI.h"
#include "UI/NewUI/NewUISystem.h"
#include "imgui.h"

#include <cstdint>

using MuEditor::Effects::EffectWorldPreview;
using MuEditor::Effects::WorldPreviewRequest;
namespace Layout = MuEditor::Effects::Layout;

namespace
{
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
    case MuEditor::Effects::WorldNoteRefused:
        break;
    }
    return I18N::Editor::WorldRefused;
}
} // namespace

void CEffectWorldPreviewView::Render(EffectWorldPreview& world, const WorldPreviewRequest& request)
{
    ImGui::SeparatorText(I18N::Editor::InTheWorld);
    const bool ready = MuEditor::Effects::IsWorldReadyForPreview();
    RenderButtons(world, request, ready);
    RenderRunning(world, request);
    RenderNotes(world, ready);
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

// What the preview keeps alive in the world.
void CEffectWorldPreviewView::RenderRunning(const EffectWorldPreview& world, const WorldPreviewRequest& request) const
{
    if (!world.IsRunning())
        return;
    using Data::Effects::EffectKind;
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
