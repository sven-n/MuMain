#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSoundEventsEditor.h"

#include "MonsterSoundPreview.h"
#include "MonsterStepLabel.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterSoundMoments.h"
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
using Data::Monsters::MonsterSoundMoment;

// Pixel widths before the editor UI scale.
constexpr float WhenWidth = 170.0f;
constexpr float SubTypeWidth = 70.0f;
constexpr float SubTypeValueWidth = 50.0f;
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
// A new SubType condition names the second set's SubType.
constexpr int NewSubType = 9;
// OBJECT::SubType is a short.
constexpr int MaxSubType = 32767;

std::string AnimationsText(const std::vector<int>& animations)
{
    std::string text;
    for (const int animation : animations)
    {
        text += (text.empty() ? "" : ", ") + Data::Monsters::GetMonsterAnimationName(animation);
    }
    return text;
}

const char* SubTypeLabel(SubTypeCondition condition)
{
    switch (condition)
    {
    case SubTypeCondition::Only:
        return I18N::Editor::SoundEventOnlySubType;
    case SubTypeCondition::Not:
        return I18N::Editor::SoundEventNotSubType;
    case SubTypeCondition::Any:
        break;
    }
    return I18N::Editor::SoundEventAnySubType;
}

// Takes the value out of the sorted list, or puts it in; false for the last
// one.
template <typename T> bool Toggle(std::vector<T>& values, T value)
{
    const auto found = std::find(values.begin(), values.end(), value);
    if (found == values.end())
    {
        values.push_back(value);
        std::sort(values.begin(), values.end());
        return true;
    }
    if (values.size() == 1)
    {
        return false;
    }
    values.erase(found);
    return true;
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

// Plays the event's sounds at the preview volume, one of a pick's at random;
// a sound of a model that is not open is loaded as the model loads it.
void PlayEvent(const MonsterModelDefinition& model, const MonsterSoundEvent& event, bool modelOpen)
{
    for (const std::string& name : PreviewSounds(event, Random::RangeInt(0, 1)))
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
        RenderWhen(event);
        ImGui::TableNextColumn();
        RenderFrames(event);
        ImGui::TableNextColumn();
        RenderFlag("##Armed", event.deathEffectArmed, I18N::Editor::SoundEventDeathEffectNote);
        ImGui::TableNextColumn();
        RenderSubType(event);
        ImGui::TableNextColumn();
        RenderFlag("##Anywhere", event.heardAnywhere, I18N::Editor::SoundEventHeardAnywhereNote);
        ImGui::TableNextColumn();
        RenderFlag("##AtMonster", event.atMonster, I18N::Editor::SoundEventAtTheMonsterNote);
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
    // The moments, then the animations; picking one of the other kind makes
    // the event one of it.
    void RenderWhen(MonsterSoundEvent& event)
    {
        ImGui::SetNextItemWidth(WhenWidth * m_scale);
        const bool open = ImGui::BeginCombo("##When", WhenText(event).c_str());
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundEventWhenNote);
        }
        if (!open)
        {
            return;
        }
        ImGui::TextDisabled("%s", I18N::Editor::SoundEventMoments);
        for (const MonsterSoundMoment moment : Data::Monsters::AllMonsterSoundMoments)
        {
            const bool chosen = std::find(event.moments.begin(), event.moments.end(), moment) != event.moments.end();
            const std::string name(Data::Monsters::GetMonsterSoundMomentName(moment));
            ImGui::PushID(name.c_str());
            // An event plays at one moment at least.
            ImGui::BeginDisabled(chosen && event.moments.size() == 1);
            if (ImGui::Selectable(name.c_str(), chosen, ImGuiSelectableFlags_DontClosePopups))
            {
                m_draft.AfterChange(ToggleMoment(event, moment), m_label);
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        ImGui::Separator();
        ImGui::TextDisabled("%s", I18N::Editor::Animations);
        for (const int animation : OfferedAnimations(m_slot, m_modelOpen))
        {
            const bool chosen =
                std::find(event.animations.begin(), event.animations.end(), animation) != event.animations.end();
            const std::string name = Data::Monsters::GetMonsterAnimationName(animation);
            ImGui::PushID(animation);
            // An event plays in one animation at least.
            ImGui::BeginDisabled(chosen && event.animations.size() == 1);
            if (ImGui::Selectable(name.c_str(), chosen, ImGuiSelectableFlags_DontClosePopups))
            {
                m_draft.AfterChange(ToggleAnimation(event, animation), m_label);
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    void RenderFlag(const char* id, bool& value, const char* note)
    {
        m_draft.AfterChange(ImGui::Checkbox(id, &value), m_label);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", note);
        }
    }

    void RenderSubType(MonsterSoundEvent& event)
    {
        const SubTypeCondition current = GetSubTypeCondition(event);
        ImGui::SetNextItemWidth(SubTypeWidth * m_scale);
        if (ImGui::BeginCombo("##SubType", SubTypeLabel(current)))
        {
            for (const SubTypeCondition condition :
                 {SubTypeCondition::Any, SubTypeCondition::Only, SubTypeCondition::Not})
            {
                if (ImGui::Selectable(SubTypeLabel(condition), condition == current) && condition != current)
                {
                    SetSubTypeCondition(event, condition);
                    m_draft.AfterChange(true, m_label);
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundEventSubTypeNote);
        }
        std::optional<int>& value = event.subType ? event.subType : event.notSubType;
        if (!value)
        {
            return;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(SubTypeValueWidth * m_scale);
        ImGui::InputInt("##SubTypeValue", &*value, 0);
        *value = std::clamp(*value, 0, MaxSubType);
        m_draft.AfterField(m_label);
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
        // A pick chooses one of two sounds, or one sound or none.
        ImGui::BeginDisabled(event.sounds.empty() || event.sounds.size() > PickSounds);
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
            m_draft.AfterChange(true, m_label);
        }
        std::string added;
        if (SoundCombo("##AddSound", m_model, added, SoundWidth * m_scale))
        {
            event.sounds.push_back(added);
            if (event.sounds.size() > PickSounds)
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

std::string WhenText(const MonsterSoundEvent& event)
{
    if (event.moments.empty())
    {
        return AnimationsText(event.animations);
    }
    std::string text = I18N::Editor::SoundEventOn;
    for (size_t index = 0; index < event.moments.size(); ++index)
    {
        text +=
            (index == 0 ? " " : ", ") + std::string(Data::Monsters::GetMonsterSoundMomentName(event.moments[index]));
    }
    return text;
}

bool ToggleMoment(MonsterSoundEvent& event, MonsterSoundMoment moment)
{
    if (!event.animations.empty())
    {
        event.animations.clear();
        event.moments = {moment};
        return true;
    }
    return Toggle(event.moments, moment);
}

bool ToggleAnimation(MonsterSoundEvent& event, int animation)
{
    if (!event.moments.empty())
    {
        event.moments.clear();
        event.animations = {animation};
        return true;
    }
    return Toggle(event.animations, animation);
}

SubTypeCondition GetSubTypeCondition(const MonsterSoundEvent& event)
{
    if (event.subType)
    {
        return SubTypeCondition::Only;
    }
    return event.notSubType ? SubTypeCondition::Not : SubTypeCondition::Any;
}

void SetSubTypeCondition(MonsterSoundEvent& event, SubTypeCondition condition)
{
    const int value = event.subType.value_or(event.notSubType.value_or(NewSubType));
    event.subType.reset();
    event.notSubType.reset();
    if (condition == SubTypeCondition::Only)
    {
        event.subType = value;
    }
    else if (condition == SubTypeCondition::Not)
    {
        event.notSubType = value;
    }
}

std::vector<std::string> PreviewSounds(const MonsterSoundEvent& event, int draw)
{
    if (!event.pick || event.sounds.empty())
    {
        return event.sounds;
    }
    const size_t index = static_cast<size_t>(draw) < event.sounds.size() ? static_cast<size_t>(draw) : 0;
    return {event.sounds[index]};
}

void Render(MonsterModelDefinition& model, int slot, bool modelOpen, ModelDraft& draft)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::SoundEventsNote);
    ImGui::PopTextWrapPos();
    if (!model.soundEvents.empty())
    {
        constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("##SoundEvents", 10, flags))
        {
            ImGui::TableSetupColumn(I18N::Editor::SoundEventWhen);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventFrames);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventDeathEffect);
            ImGui::TableSetupColumn(I18N::Editor::SubType);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventHeardAnywhere);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventAtTheMonster);
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
