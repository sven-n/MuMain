#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSoundEventsEditor.h"

#include "MonsterEdits.h"
#include "MonsterSoundPreview.h"
#include "MonsterStepLabel.h"
#include "MuMonsterPreviewUI.h"

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
#include <cstdio>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace MuEditor::Monsters::SoundEventsEditor
{
namespace
{
using Data::Monsters::MonsterModelCue;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSound;
using Data::Monsters::MonsterSoundChance;
using Data::Monsters::MonsterSoundDraw;
using Data::Monsters::MonsterSoundEvent;
using Data::Monsters::MonsterSoundMoment;
using Data::Monsters::MonsterSoundSet;

// Pixel widths before the editor UI scale.
constexpr float WhenWidth = 170.0f;
constexpr float CueAnimationWidth = 90.0f;
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
// A new cue is this many frames long; a cue at least this.
constexpr float NewCueFrames = 1.0f;
constexpr float MinCueFrames = 0.1f;
constexpr ImU32 FocusColor = IM_COL32(90, 140, 60, 110);
constexpr ImVec4 MissingCueColor = ImVec4(1.0f, 0.45f, 0.4f, 1.0f);

// The removal that waits for the question whether the cue goes too.
struct PendingRemoval
{
    std::string set;
    size_t event = 0;
    std::vector<std::string> cues;
    int slot = -1;
};
std::optional<PendingRemoval> g_pendingRemoval;
constexpr const char* RemoveCuePopup = "##RemoveTheCue";

std::string AnimationsText(const std::vector<int>& animations)
{
    std::string text;
    for (const int animation : animations)
    {
        text += (text.empty() ? "" : ", ") + Data::Monsters::GetMonsterAnimationName(animation);
    }
    return text;
}

// Takes the value out of the list, or puts it in (sorted when `sorted`);
// false for the last one.
template <typename T> bool Toggle(std::vector<T>& values, const T& value, bool sorted = true)
{
    const auto found = std::find(values.begin(), values.end(), value);
    if (found == values.end())
    {
        values.push_back(value);
        if (sorted)
        {
            std::sort(values.begin(), values.end());
        }
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

std::vector<int> OfferedAnimations(const Context& context)
{
    const int count =
        context.modelOpen && Models != nullptr && context.slot >= 0 ? Models[context.slot].NumActions : NamedAnimations;
    std::vector<int> animations;
    for (int animation = 0; animation < count; ++animation)
    {
        animations.push_back(animation);
    }
    return animations;
}

MonsterModelCue* FindCue(MonsterModelDefinition* model, const std::string& name)
{
    if (model == nullptr)
    {
        return nullptr;
    }
    const auto found = std::find_if(model->cues.begin(), model->cues.end(),
                                    [&](const MonsterModelCue& cue) { return cue.name == name; });
    return found != model->cues.end() ? &*found : nullptr;
}

std::string FrameText(float frame)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.2f", static_cast<double>(frame));
    return text;
}

// Plays the event's sounds at the preview volume, one of a pick's at random.
void PlayEvent(const MonsterSoundSet& set, const MonsterSoundEvent& event)
{
    for (const std::string& name : PreviewSounds(event, Random::RangeInt(0, 1)))
    {
        SoundPreview::PlaySoundOfSet(set, name);
    }
}

// A combo of the set's sounds, with the current one also when the set does
// not have it; true when another was picked.
bool SoundCombo(const char* id, const MonsterSoundSet& set, std::string& value, float width)
{
    bool picked = false;
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(id, value.c_str()))
    {
        bool listed = false;
        for (const MonsterSound& sound : set.sounds)
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
    EventRow(MonsterSoundSet& set, SetDraft& draft, const Context& context)
        : m_set(set), m_draft(draft), m_context(context),
          m_label(StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundEvents)),
          m_scale(g_MuEditorCore.GetUIScale())
    {
    }

    // Draws the row; true when the event is to be removed.
    bool Render(MonsterSoundEvent& event)
    {
        ImGui::TableNextColumn();
        RenderWhen(event);
        ImGui::TableNextColumn();
        RenderCueFrames(event);
        ImGui::TableNextColumn();
        RenderFlag("##Armed", event.deathEffectArmed, I18N::Editor::SoundEventDeathEffectNote);
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
            PlayEvent(m_set, event);
        }
        ImGui::SameLine();
        return ImGui::SmallButton(I18N::Editor::Remove);
    }

private:
    // The moments, the animations, then the model's cues; picking one of
    // another kind makes the event one of it.
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
        for (const int animation : OfferedAnimations(m_context))
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
        RenderCueChoices(event);
        ImGui::EndCombo();
    }

    void RenderCueChoices(MonsterSoundEvent& event)
    {
        if (m_context.model == nullptr)
        {
            return;
        }
        ImGui::Separator();
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::SoundEventCuesOf0, {m_context.model->slot}).c_str());
        for (const MonsterModelCue& cue : m_context.model->cues)
        {
            const bool chosen = std::find(event.cues.begin(), event.cues.end(), cue.name) != event.cues.end();
            ImGui::PushID(cue.name.c_str());
            ImGui::BeginDisabled(chosen && event.cues.size() == 1);
            if (ImGui::Selectable(cue.name.c_str(), chosen, ImGuiSelectableFlags_DontClosePopups))
            {
                m_draft.AfterChange(ToggleCue(event, cue.name), m_label);
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        // A cue at the frame the preview shows (Q-MN82).
        const int animation = g_MuMonsterPreviewUI.GetAnimation();
        const float frame = g_MuMonsterPreviewUI.GetFrame();
        const std::string label = I18N::Format(I18N::Editor::SoundEventNewCueAt01,
                                               {Data::Monsters::GetMonsterAnimationName(animation), FrameText(frame)});
        if (ImGui::Selectable(label.c_str()))
        {
            const MonsterModelCue cue = MakeCue(*m_context.model, animation, frame);
            const std::string step =
                StepLabel::Model(m_context.model->slot, I18N::Format(I18N::Editor::StepWhatCue0, {cue.name}));
            g_MonsterEdits.BeginGroup(step);
            m_context.model->cues.push_back(cue);
            m_context.modelDraft->StoreNow(step);
            event.animations.clear();
            event.moments.clear();
            event.cues = {cue.name};
            m_draft.StoreNow(step);
            g_MonsterEdits.EndGroup();
        }
    }

    // On a cue's event: the cue's animation and frames, which are the model's
    // and change for every monster of it (Q-MN81).
    void RenderCueFrames(MonsterSoundEvent& event)
    {
        // The cues are those of the selected monster's look, which plays
        // another set (or none).
        if (m_context.model == nullptr)
        {
            if (!event.cues.empty())
            {
                ImGui::TextDisabled("%s", I18N::Editor::CueFramesNeedAMonsterOfTheSet);
            }
            return;
        }
        for (const std::string& name : event.cues)
        {
            ImGui::PushID(name.c_str());
            MonsterModelCue* cue = FindCue(m_context.model, name);
            if (cue == nullptr)
            {
                ImGui::TextColored(MissingCueColor, "%s", I18N::Format(I18N::Editor::SoundEventNoCue0, {name}).c_str());
                ImGui::PopID();
                continue;
            }
            const std::string label =
                StepLabel::Model(m_context.model->slot, I18N::Format(I18N::Editor::StepWhatCue0, {cue->name}));
            ImGui::TextUnformatted(cue->name.c_str());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(CueAnimationWidth * m_scale);
            if (ImGui::BeginCombo("##CueAnimation", Data::Monsters::GetMonsterAnimationName(cue->animation).c_str()))
            {
                for (const int animation : OfferedAnimations(m_context))
                {
                    const std::string animationName = Data::Monsters::GetMonsterAnimationName(animation);
                    if (ImGui::Selectable(animationName.c_str(), animation == cue->animation) &&
                        animation != cue->animation)
                    {
                        cue->animation = animation;
                        m_context.modelDraft->AfterChange(true, label);
                    }
                }
                ImGui::EndCombo();
            }
            auto& [from, to] = cue->frames;
            ImGui::SameLine();
            ImGui::SetNextItemWidth(FrameWidth * m_scale);
            // The inputs keep the cue at least 0.1 frames long.
            if (ImGui::InputFloat("##From", &from, 0.0f, 0.0f, "%.2f"))
            {
                from = std::max(from, 0.0f);
                to = std::max(to, from + MinCueFrames);
            }
            m_context.modelDraft->AfterField(label);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(FrameWidth * m_scale);
            if (ImGui::InputFloat("##To", &to, 0.0f, 0.0f, "%.2f"))
            {
                to = std::max(to, from + MinCueFrames);
            }
            m_context.modelDraft->AfterField(label);
            ImGui::SameLine();
            if (ImGui::SmallButton(I18N::Editor::UseThePreviewFrame))
            {
                const float length = to - from;
                cue->animation = g_MuMonsterPreviewUI.GetAnimation();
                from = g_MuMonsterPreviewUI.GetFrame();
                to = from + length;
                m_context.modelDraft->AfterChange(true, label);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::UseThePreviewFrameNote);
            }
            ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::SoundEventCueOf01,
                                                   {m_context.model->slot, std::to_string(m_context.modelMonsters)})
                                          .c_str());
            ImGui::PopID();
        }
    }

    void RenderFlag(const char* id, bool& value, const char* note)
    {
        m_draft.AfterChange(ImGui::Checkbox(id, &value), m_label);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", note);
        }
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
            if (SoundCombo("##Sound", m_set, event.sounds[index], SoundWidth * m_scale))
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
        if (SoundCombo("##AddSound", m_set, added, SoundWidth * m_scale))
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

    MonsterSoundSet& m_set;
    SetDraft& m_draft;
    const Context& m_context;
    std::string m_label;
    float m_scale;
};

void RenderMaps(MonsterSoundSet& set, SetDraft& draft)
{
    ImGui::TextUnformatted(I18N::Editor::SoundEventMaps);
    ImGui::SameLine();
    if (MapPicker::Render("SoundEventMaps", set.maps, I18N::Editor::SoundEventEveryMap))
    {
        draft.AfterChange(true, StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundEvents));
    }
}

// The cues of the event that no other event plays on with the model.
std::vector<std::string> CuesOnlyThisEventUses(const MonsterSoundSet& set, size_t index, int slot)
{
    std::vector<std::string> alone;
    for (const std::string& cue : set.soundEvents[index].cues)
    {
        if (!IsCueUsed(slot, cue, set.name, index))
        {
            alone.push_back(cue);
        }
    }
    return alone;
}

void RemoveEvent(MonsterSoundSet& set, size_t index, SetDraft& draft)
{
    set.soundEvents.erase(set.soundEvents.begin() + static_cast<std::ptrdiff_t>(index));
    draft.AfterChange(true, StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundEvents));
}

// The question of Q-MN82: removing a cue's last event asks whether the cue
// goes too.
void RenderRemoveCuePopup(MonsterSoundSet& set, SetDraft& draft, const Context& context)
{
    if (g_pendingRemoval && !ImGui::IsPopupOpen(RemoveCuePopup))
    {
        ImGui::OpenPopup(RemoveCuePopup);
    }
    if (!ImGui::BeginPopup(RemoveCuePopup))
    {
        return;
    }
    const bool valid = g_pendingRemoval && g_pendingRemoval->set == set.name &&
                       g_pendingRemoval->event < set.soundEvents.size() && context.model != nullptr &&
                       g_pendingRemoval->slot == context.slot;
    if (!valid)
    {
        g_pendingRemoval.reset();
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }
    std::string cues;
    for (const std::string& cue : g_pendingRemoval->cues)
    {
        cues += (cues.empty() ? "" : ", ") + cue;
    }
    ImGui::TextUnformatted(I18N::Editor::RemoveTheCue);
    ImGui::Separator();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::RemoveTheCueQuestion01, {cues, context.model->slot}).c_str());
    ImGui::PopTextWrapPos();
    if (ImGui::Button(I18N::Editor::RemoveTheCueToo))
    {
        const std::string step = StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundEvents);
        g_MonsterEdits.BeginGroup(step);
        std::erase_if(context.model->cues,
                      [&](const MonsterModelCue& cue)
                      {
                          return std::find(g_pendingRemoval->cues.begin(), g_pendingRemoval->cues.end(), cue.name) !=
                                 g_pendingRemoval->cues.end();
                      });
        context.modelDraft->StoreNow(step);
        RemoveEvent(set, g_pendingRemoval->event, draft);
        g_MonsterEdits.EndGroup();
        g_pendingRemoval.reset();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::KeepTheCue))
    {
        RemoveEvent(set, g_pendingRemoval->event, draft);
        g_pendingRemoval.reset();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}
} // namespace

SetDraft MakeSetDraft(SetDraft::Load load, SetDraft::Store store)
{
    return SetDraft(std::move(load), std::move(store), Items::ReloadAfterStore::NextFrame);
}

ModelDraft MakeModelDraft(ModelDraft::Load load, ModelDraft::Store store)
{
    return ModelDraft(std::move(load), std::move(store), Items::ReloadAfterStore::NextFrame);
}

std::string WhenText(const MonsterSoundEvent& event)
{
    if (!event.cues.empty())
    {
        std::string text = I18N::Editor::SoundEventCue;
        for (size_t index = 0; index < event.cues.size(); ++index)
        {
            text += (index == 0 ? " " : ", ") + event.cues[index];
        }
        return text;
    }
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
    if (event.moments.empty())
    {
        event.animations.clear();
        event.cues.clear();
        event.moments = {moment};
        return true;
    }
    return Toggle(event.moments, moment);
}

bool ToggleAnimation(MonsterSoundEvent& event, int animation)
{
    if (event.animations.empty())
    {
        event.moments.clear();
        event.cues.clear();
        event.animations = {animation};
        return true;
    }
    return Toggle(event.animations, animation);
}

bool ToggleCue(MonsterSoundEvent& event, const std::string& cue)
{
    if (event.cues.empty())
    {
        event.moments.clear();
        event.animations.clear();
        event.cues = {cue};
        return true;
    }
    return Toggle(event.cues, cue, false);
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

MonsterModelCue MakeCue(const MonsterModelDefinition& model, int animation, float frame)
{
    MonsterModelCue cue;
    cue.animation = animation;
    cue.frames = {frame, frame + NewCueFrames};
    const std::string base = Data::Monsters::GetMonsterAnimationName(animation) + " " + FrameText(frame);
    cue.name = base;
    for (int number = 2; std::any_of(model.cues.begin(), model.cues.end(),
                                     [&](const MonsterModelCue& other) { return other.name == cue.name; });
         ++number)
    {
        cue.name = base + " (" + std::to_string(number) + ")";
    }
    return cue;
}

bool IsCueUsed(int slot, const std::string& cue, const std::string& exceptSet, size_t except)
{
    for (const MonsterSoundSet* set : Data::Monsters::ModelLoader::FindSoundSetsOfSlot(slot))
    {
        for (size_t index = 0; index < set->soundEvents.size(); ++index)
        {
            const std::vector<std::string>& cues = set->soundEvents[index].cues;
            const bool skipped = set->name == exceptSet && index == except;
            if (!skipped && std::find(cues.begin(), cues.end(), cue) != cues.end())
            {
                return true;
            }
        }
    }
    return false;
}

void Render(MonsterSoundSet& set, SetDraft& draft, const Context& context)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::SoundEventsNote);
    ImGui::PopTextWrapPos();
    if (!set.soundEvents.empty())
    {
        constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("##SoundEvents", 9, flags))
        {
            ImGui::TableSetupColumn(I18N::Editor::SoundEventWhen);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventFrames);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventDeathEffect);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventHeardAnywhere);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventAtTheMonster);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventChance);
            ImGui::TableSetupColumn(I18N::Editor::SoundEventPick);
            ImGui::TableSetupColumn(I18N::Editor::Sounds);
            ImGui::TableSetupColumn("##Buttons");
            ImGui::TableHeadersRow();
            EventRow row(set, draft, context);
            std::optional<size_t> removed;
            for (size_t index = 0; index < set.soundEvents.size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                ImGui::TableNextRow();
                if (context.focusedEvent == index)
                {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, FocusColor);
                    ImGui::SetScrollHereY();
                }
                if (row.Render(set.soundEvents[index]))
                {
                    removed = index;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
            if (removed)
            {
                std::vector<std::string> alone = context.model != nullptr
                                                     ? CuesOnlyThisEventUses(set, *removed, context.slot)
                                                     : std::vector<std::string>();
                if (alone.empty())
                {
                    RemoveEvent(set, *removed, draft);
                }
                else
                {
                    g_pendingRemoval = PendingRemoval{set.name, *removed, std::move(alone), context.slot};
                }
            }
        }
    }
    RenderRemoveCuePopup(set, draft, context);
    // An event plays the set's own sounds.
    ImGui::BeginDisabled(set.sounds.empty());
    if (ImGui::Button(I18N::Editor::AddASoundEvent))
    {
        MonsterSoundEvent event;
        event.animations = {MONSTER01_ATTACK1};
        event.sounds = {Data::Monsters::SoundNameOf(set.sounds.front())};
        set.soundEvents.push_back(event);
        draft.AfterChange(true, StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundEvents));
    }
    ImGui::EndDisabled();
    if (set.sounds.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SoundEventNeedsASoundOfTheModel);
    }
    RenderMaps(set, draft);
}
} // namespace MuEditor::Monsters::SoundEventsEditor

#endif // _EDITOR
