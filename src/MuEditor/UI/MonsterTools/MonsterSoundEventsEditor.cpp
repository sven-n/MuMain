#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSoundEventsEditor.h"

#include "MonsterSoundPreview.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor::Monsters::SoundEventsEditor
{
namespace
{
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterModelSound;
using Data::Monsters::MonsterSoundChance;
using Data::Monsters::MonsterSoundDraw;
using Data::Monsters::MonsterSoundEvent;

// Pixel widths before the editor UI scale.
constexpr float AnimationsWidth = 150.0f;
constexpr float FrameWidth = 50.0f;
constexpr float OneInWidth = 70.0f;
constexpr float DrawWidth = 140.0f;
constexpr float SoundWidth = 260.0f;
// The animations offered while the model is not open: up to attack 5.
constexpr int NamedAnimations = MONSTER01_ATTACK5 + 1;
constexpr int MaxOneIn = 100000;
constexpr size_t PickSounds = 2;
// A new chance is one in this.
constexpr int NewChance = 2;
// A new frame range is this many frames long.
constexpr float NewFrames = 1.0f;

std::string AnimationsText(const std::vector<int>& animations)
{
    std::string text;
    for (const int animation : animations)
    {
        text += (text.empty() ? "" : ", ") + Data::Monsters::GetMonsterAnimationName(animation);
    }
    return text;
}

const char* DrawLabel(MonsterSoundDraw draw)
{
    return draw == MonsterSoundDraw::Rand ? I18N::Editor::SoundDrawRand : I18N::Editor::SoundDrawRandFps;
}

std::vector<int> OfferedAnimations(int slot, bool modelOpen)
{
    const int count = modelOpen && Models != nullptr ? Models[slot].NumActions : NamedAnimations;
    std::vector<int> animations;
    for (int animation = 0; animation < count; ++animation)
    {
        animations.push_back(animation);
    }
    return animations;
}

// Plays the event's sounds at the preview volume; a sound of a model that is
// not open is loaded as the model loads it.
void PlayEvent(const MonsterModelDefinition& model, const MonsterSoundEvent& event, bool modelOpen)
{
    for (const std::string& name : event.sounds)
    {
        const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, name);
        if (!number)
        {
            continue;
        }
        const auto sound =
            std::find_if(model.sounds.begin(), model.sounds.end(), [&](const MonsterModelSound& candidate)
                         { return Data::Monsters::SoundNameOf(candidate) == name; });
        if (!modelOpen && sound != model.sounds.end())
        {
            Data::Monsters::ModelLoader::LoadSound(*sound);
        }
        SoundPreview::Play(*number);
        if (event.pick)
        {
            // A pick plays one of them: the first is heard.
            break;
        }
    }
}

// A combo of the model's sounds, with the current one also when the model
// does not load it; true when another was picked.
bool SoundCombo(const char* id, const MonsterModelDefinition& model, std::string& value, float width)
{
    bool picked = false;
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(id, value.c_str()))
    {
        bool listed = false;
        for (const MonsterModelSound& sound : model.sounds)
        {
            const std::string& name = Data::Monsters::SoundNameOf(sound);
            listed = listed || name == value;
            if (ImGui::Selectable(name.c_str(), name == value) && name != value)
            {
                value = name;
                picked = true;
            }
        }
        if (!listed && !value.empty())
        {
            ImGui::Selectable(value.c_str(), true);
        }
        ImGui::EndCombo();
    }
    return picked;
}

class EventRow
{
public:
    EventRow(MonsterModelDefinition& model, int slot, bool modelOpen, ModelDraft& draft)
        : m_model(model), m_slot(slot), m_modelOpen(modelOpen), m_draft(draft),
          m_label(StepLabel::Model(model.slot, I18N::Editor::StepWhatSoundEvents)), m_scale(g_MuEditorCore.GetUIScale())
    {
    }

    // Draws the row; true when the event is to be removed.
    bool Render(MonsterSoundEvent& event)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderAnimations(event);
        ImGui::TableNextColumn();
        RenderFrames(event);
        ImGui::TableNextColumn();
        m_draft.AfterChange(ImGui::Checkbox("##Armed", &event.deathEffectArmed), m_label);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundEventDeathEffectNote);
        }
        ImGui::TableNextColumn();
        RenderChance(event);
        ImGui::TableNextColumn();
        RenderPick(event);
        ImGui::TableNextColumn();
        RenderSounds(event);
        ImGui::TableNextColumn();
        if (ImGui::SmallButton(I18N::Editor::PlayThisSound))
        {
            PlayEvent(m_model, event, m_modelOpen);
        }
        ImGui::SameLine();
        return ImGui::SmallButton(I18N::Editor::Remove);
    }

private:
    void RenderAnimations(MonsterSoundEvent& event)
    {
        ImGui::SetNextItemWidth(AnimationsWidth * m_scale);
        if (!ImGui::BeginCombo("##Animations", AnimationsText(event.animations).c_str()))
        {
            return;
        }
        for (const int animation : OfferedAnimations(m_slot, m_modelOpen))
        {
            auto found = std::find(event.animations.begin(), event.animations.end(), animation);
            const bool chosen = found != event.animations.end();
            const std::string name = Data::Monsters::GetMonsterAnimationName(animation);
            // An event plays in one animation at least.
            ImGui::BeginDisabled(chosen && event.animations.size() == 1);
            if (ImGui::Selectable(name.c_str(), chosen, ImGuiSelectableFlags_DontClosePopups))
            {
                if (chosen)
                {
                    event.animations.erase(found);
                }
                else
                {
                    event.animations.push_back(animation);
                    std::sort(event.animations.begin(), event.animations.end());
                }
                m_draft.AfterChange(true, m_label);
            }
            ImGui::EndDisabled();
        }
        ImGui::EndCombo();
    }

    void RenderFrames(MonsterSoundEvent& event)
    {
        bool limited = event.frames.has_value();
        if (ImGui::Checkbox("##Frames", &limited))
        {
            if (limited)
            {
                event.frames = std::pair<float, float>{0.0f, NewFrames};
            }
            else
            {
                event.frames.reset();
            }
            m_draft.AfterChange(true, m_label);
        }
        if (!event.frames)
        {
            return;
        }
        auto& [from, to] = *event.frames;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(FrameWidth * m_scale);
        // The inputs keep the range at least 0.1 wide, but only when one is
        // edited: a narrower range of the file stays as it is.
        if (ImGui::InputFloat("##From", &from, 0.0f, 0.0f, "%.1f"))
        {
            from = std::max(from, 0.0f);
            to = std::max(to, from + 0.1f);
        }
        m_draft.AfterField(m_label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(FrameWidth * m_scale);
        if (ImGui::InputFloat("##To", &to, 0.0f, 0.0f, "%.1f"))
        {
            to = std::max(to, from + 0.1f);
        }
        m_draft.AfterField(m_label);
    }

    void RenderChance(MonsterSoundEvent& event)
    {
        bool chance = event.chance.has_value();
        if (ImGui::Checkbox("##Chance", &chance))
        {
            if (chance)
            {
                event.chance = MonsterSoundChance{NewChance, MonsterSoundDraw::Rand};
            }
            else
            {
                event.chance.reset();
            }
            m_draft.AfterChange(true, m_label);
        }
        if (!event.chance)
        {
            return;
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(I18N::Editor::SoundEventOneIn);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(OneInWidth * m_scale);
        ImGui::InputInt("##OneIn", &event.chance->oneIn, 0);
        event.chance->oneIn = std::clamp(event.chance->oneIn, 1, MaxOneIn);
        m_draft.AfterField(m_label);
        ImGui::SameLine();
        RenderDraw("##ChanceDraw", event.chance->draw);
    }

    void RenderDraw(const char* id, MonsterSoundDraw& draw)
    {
        ImGui::SetNextItemWidth(DrawWidth * m_scale);
        if (ImGui::BeginCombo(id, DrawLabel(draw)))
        {
            for (const MonsterSoundDraw candidate : {MonsterSoundDraw::Rand, MonsterSoundDraw::RandFps})
            {
                if (ImGui::Selectable(DrawLabel(candidate), candidate == draw) && candidate != draw)
                {
                    draw = candidate;
                    m_draft.AfterChange(true, m_label);
                }
            }
            ImGui::EndCombo();
        }
    }

    void RenderPick(MonsterSoundEvent& event)
    {
        // A pick chooses one of two sounds.
        ImGui::BeginDisabled(event.sounds.size() != PickSounds);
        bool pick = event.pick.has_value();
        if (ImGui::Checkbox("##Pick", &pick))
        {
            if (pick)
            {
                event.pick = MonsterSoundDraw::Rand;
            }
            else
            {
                event.pick.reset();
            }
            m_draft.AfterChange(true, m_label);
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundEventPickNote);
        }
        if (event.pick)
        {
            ImGui::SameLine();
            RenderDraw("##PickDraw", *event.pick);
        }
    }

    void RenderSounds(MonsterSoundEvent& event)
    {
        std::optional<size_t> removed;
        for (size_t index = 0; index < event.sounds.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            if (SoundCombo("##Sound", m_model, event.sounds[index], SoundWidth * m_scale))
            {
                m_draft.AfterChange(true, m_label);
            }
            if (event.sounds.size() > 1)
            {
                ImGui::SameLine();
                if (ImGui::SmallButton("x"))
                {
                    removed = index;
                }
            }
            ImGui::PopID();
        }
        if (removed)
        {
            event.sounds.erase(event.sounds.begin() + static_cast<std::ptrdiff_t>(*removed));
            if (event.sounds.size() != PickSounds)
            {
                event.pick.reset();
            }
            m_draft.AfterChange(true, m_label);
        }
        std::string added;
        if (SoundCombo("##AddSound", m_model, added, SoundWidth * m_scale))
        {
            event.sounds.push_back(added);
            if (event.sounds.size() != PickSounds)
            {
                event.pick.reset();
            }
            m_draft.AfterChange(true, m_label);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundEventAddSound);
        }
    }

    MonsterModelDefinition& m_model;
    int m_slot;
    bool m_modelOpen;
    ModelDraft& m_draft;
    std::string m_label;
    float m_scale;
};

void RenderMaps(MonsterModelDefinition& model, ModelDraft& draft)
{
    ImGui::TextUnformatted(I18N::Editor::SoundEventMaps);
    ImGui::SameLine();
    if (MapPicker::Render("SoundEventMaps", model.soundEventMaps, I18N::Editor::SoundEventEveryMap))
    {
        draft.AfterChange(true, StepLabel::Model(model.slot, I18N::Editor::StepWhatSoundEvents));
    }
}
} // namespace

void Render(MonsterModelDefinition& model, int slot, bool modelOpen, ModelDraft& draft)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::SoundEventsNote);
    ImGui::PopTextWrapPos();
    if (!model.soundEvents.empty())
    {
        constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("##SoundEvents", 7, flags))
        {
            ImGui::TableSetupColumn(I18N::Editor::Animations);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventFrames);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventDeathEffect);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventChance);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventPick);
            ImGui::TableSetupColumn(I18N::Editor::Sounds);
            ImGui::TableSetupColumn("##Buttons");
            ImGui::TableHeadersRow();
            EventRow row(model, slot, modelOpen, draft);
            std::optional<size_t> removed;
            for (size_t index = 0; index < model.soundEvents.size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                if (row.Render(model.soundEvents[index]))
                {
                    removed = index;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
            if (removed)
            {
                model.soundEvents.erase(model.soundEvents.begin() + static_cast<std::ptrdiff_t>(*removed));
                draft.AfterChange(true, StepLabel::Model(model.slot, I18N::Editor::StepWhatSoundEvents));
            }
        }
    }
    // An event plays the model's own sounds.
    ImGui::BeginDisabled(model.sounds.empty());
    if (ImGui::Button(I18N::Editor::AddASoundEvent))
    {
        MonsterSoundEvent event;
        event.animations = {MONSTER01_ATTACK1};
        event.sounds = {Data::Monsters::SoundNameOf(model.sounds.front())};
        model.soundEvents.push_back(event);
        draft.AfterChange(true, StepLabel::Model(model.slot, I18N::Editor::StepWhatSoundEvents));
    }
    ImGui::EndDisabled();
    if (model.sounds.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SoundEventNeedsASoundOfTheModel);
    }
    RenderMaps(model, draft);
}
} // namespace MuEditor::Monsters::SoundEventsEditor

#endif // _EDITOR
