#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterPreviewUI.h"

#include "MonsterAnimationFacts.h"
#include "MonsterEdits.h"
#include "MonsterModelValues.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterSoundEventsEditor.h"
#include "MonsterSoundPreview.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"
#include "MuMonsterModelUI.h"
#include "MuMonsterSoundsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewInput.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewSubject.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Common/ScopedOffscreenCapture.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelCue;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSoundSet;

// The view's height to its width, and its height before the editor UI scale.
constexpr float ViewAspect = 0.75f;
constexpr float MinViewHeight = 200.0f;
constexpr float MaxViewHeight = 480.0f;
// Pixel widths before the editor UI scale.
constexpr float AnimationWidth = 160.0f;
constexpr float FrameWidth = 260.0f;
// Degrees the model turns per frame at the game's frame rate.
constexpr float TurnDegreesPerFrame = 0.6f;
// The end of an animation that holds its last frame (BMD::PlayAnimation).
constexpr float HeldFrameMargin = 0.01f;
constexpr ImU32 ImpactColor = IM_COL32(255, 90, 60, 255);
constexpr ImU32 CueColor = IM_COL32(159, 213, 159, 255);
constexpr float MarkerThickness = 2.0f;
constexpr float CueFrameWidth = 70.0f;
constexpr const char* AddSoundPopup = "##AddASoundAtThisFrame";

// The frames the animation plays through before it starts again: one less
// for an animation that ends where it starts (LockPositions), as
// BMD::PlayAnimation counts.
int PlayedFrames(const Action_t& action)
{
    return action.LockPositions ? action.NumAnimationKeys - 1 : action.NumAnimationKeys;
}

// The moment of the game whose sound events the preview plays when the
// animation starts, or nullopt: SetPlayerStop sets stop1, SetPlayerAttack
// an attack, SetPlayerShock the shock and SetPlayerDie the death (a walk's
// play frame by frame, in Advance).
std::optional<Data::Monsters::MonsterSoundMoment> MomentOf(int animation)
{
    switch (animation)
    {
    case MONSTER01_STOP1:
    case MONSTER01_STOP2:
        return Data::Monsters::MonsterSoundMoment::Stand;
    case MONSTER01_ATTACK1:
    case MONSTER01_ATTACK2:
    case MONSTER01_ATTACK3:
    case MONSTER01_ATTACK4:
    case MONSTER01_ATTACK5:
        return Data::Monsters::MonsterSoundMoment::Attack;
    case MONSTER01_SHOCK:
        return Data::Monsters::MonsterSoundMoment::Hit;
    case MONSTER01_DIE:
        return Data::Monsters::MonsterSoundMoment::Death;
    default:
        return std::nullopt;
    }
}

// The values the preview plays: the Monster Model tool's, with an edit that
// is not stored yet, else the model list's. The cues are GetCues'.
const MonsterModelDefinition* ValuesOf(int slot)
{
    if (const MonsterModelDefinition* draft = g_MuMonsterModelUI.GetShownDraft(slot))
    {
        return draft;
    }
    return g_MonsterModelDatabase.Find(slot);
}

std::string FrameText(float frame)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.2f", static_cast<double>(frame));
    return text;
}

// What the cue's events play, for its tag: "Jelint_move01.wav (set Raymond)".
std::string CueSounds(const MonsterSoundSet& set, const std::string& cue)
{
    std::string sounds;
    for (const Data::Monsters::MonsterSoundEvent& event : set.soundEvents)
    {
        if (std::find(event.cues.begin(), event.cues.end(), cue) == event.cues.end())
        {
            continue;
        }
        for (const std::string& sound : event.sounds)
        {
            sounds += (sounds.empty() ? "" : ", ") + sound;
        }
    }
    return sounds;
}

// The first event of the set on the cue.
std::optional<size_t> FirstEventOn(const MonsterSoundSet& set, const std::string& cue)
{
    for (size_t index = 0; index < set.soundEvents.size(); ++index)
    {
        const std::vector<std::string>& cues = set.soundEvents[index].cues;
        if (std::find(cues.begin(), cues.end(), cue) != cues.end())
        {
            return index;
        }
    }
    return std::nullopt;
}

std::string AnimationLabel(int animation)
{
    return std::to_string(animation) + " " + Data::Monsters::GetMonsterAnimationName(animation);
}
} // namespace

MuMonsterPreviewUI& MuMonsterPreviewUI::GetInstance()
{
    static MuMonsterPreviewUI instance;
    return instance;
}

void MuMonsterPreviewUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterPreview, WindowIds::MonsterPreview, open))
    {
        const int number = g_MonsterSelection.GetNumber();
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        ShownModel::RenderSelectedMonster(number, monster);
        if (monster != nullptr && !monster->randomVariants.empty())
        {
            RenderVariants(*monster);
        }
        const std::optional<int> slot = monster != nullptr ? ShownModel::FindShownSlot(*monster) : std::nullopt;
        if (monster != nullptr && !slot)
        {
            ShownModel::RenderWhyNoModel(*monster);
        }
        else if (slot)
        {
            ImGui::PushID(*slot);
            RenderPreview(*slot);
            ImGui::PopID();
        }
    }
    ToolWindow::End();
}

// The looks of a monster with random variants (Q-MN83): the model shown and
// the set it plays.
void MuMonsterPreviewUI::RenderVariants(const MonsterDefinition& monster)
{
    const int shown = ShownModel::GetShownVariant(monster);
    ImGui::TextUnformatted(I18N::Editor::Variant);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::TheEntry, shown < 0))
    {
        ShownModel::SetShownVariant(monster.number, -1);
    }
    for (size_t index = 0; index < monster.randomVariants.size(); ++index)
    {
        ImGui::SameLine();
        ImGui::PushID(static_cast<int>(index));
        const std::string label = I18N::Format(I18N::Editor::Variant0, {std::to_string(index + 1)});
        if (ImGui::RadioButton(label.c_str(), shown == static_cast<int>(index)))
        {
            ShownModel::SetShownVariant(monster.number, static_cast<int>(index));
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", monster.randomVariants[index].model.c_str());
        }
        ImGui::PopID();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", I18N::Editor::RandomVariantNote);
}

const MonsterSoundSet* MuMonsterPreviewUI::ShownSet(int slot) const
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(g_MonsterSelection.GetNumber());
    if (monster == nullptr)
    {
        return nullptr;
    }
    const std::string& name = Data::Monsters::GetSoundSetWithModel(*monster, slot);
    return name.empty() ? nullptr : g_MonsterSoundSetDatabase.Find(name);
}

std::span<const MonsterModelCue> MuMonsterPreviewUI::GetCues(int slot) const
{
    if (const MonsterModelDefinition* draft = g_MuMonsterSoundsUI.GetShownModelDraft(slot))
    {
        return draft->cues;
    }
    const MonsterModelDefinition* values = ValuesOf(slot);
    return values != nullptr ? std::span<const MonsterModelCue>(values->cues) : std::span<const MonsterModelCue>();
}

void MuMonsterPreviewUI::PlaySounds(int slot, int animation, float frame,
                                    std::optional<Data::Monsters::MonsterSoundMoment> moment)
{
    const MonsterSoundSet* set = ShownSet(slot);
    if (set == nullptr || ValuesOf(slot) == nullptr)
    {
        return;
    }
    if (animation >= 0)
    {
        SoundPreview::PlayEvents(*set, GetCues(slot), animation, frame);
    }
    if (moment)
    {
        SoundPreview::PlayMoment(*set, *moment);
    }
}

void MuMonsterPreviewUI::RenderPreview(int slot)
{
    ShownModel::OpenWhenShown(slot);
    const MonsterModelDefinition* values = ValuesOf(slot);
    if (!ShownModel::IsOpen(slot) || values == nullptr || Models[slot].NumActions <= 0)
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::TheModelIsNotOpenItShowsWhenItIs);
        ImGui::PopTextWrapPos();
        return;
    }
    if (slot != m_slot)
    {
        m_slot = slot;
        m_animation = 0;
        m_once = false;
        Restart(slot);
    }
    // An edit may leave the model with fewer animations.
    if (m_animation >= Models[slot].NumActions)
    {
        m_animation = 0;
        m_once = false;
        Restart(slot);
    }
    ImGui::TextDisabled("%s", std::string(Data::Monsters::GetMonsterModelSymbol(slot)).c_str());
    RenderControls(slot, *values);
    if (m_playing)
    {
        Advance(slot, *values);
    }
    RenderView(slot, *values);
}

void MuMonsterPreviewUI::RenderControls(int slot, const MonsterModelDefinition& values)
{
    const float scale = g_MuEditorCore.GetUIScale();
    const BMD& model = Models[slot];
    ImGui::SetNextItemWidth(AnimationWidth * scale);
    if (ImGui::BeginCombo(I18N::Editor::Animation, AnimationLabel(m_animation).c_str()))
    {
        for (int animation = 0; animation < model.NumActions; ++animation)
        {
            if (ImGui::Selectable(AnimationLabel(animation).c_str(), animation == m_animation))
            {
                m_animation = animation;
                m_once = false;
                Restart(slot);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button(m_playing ? I18N::Editor::Pause : I18N::Editor::PlayAnimation))
    {
        m_playing = !m_playing;
        m_once = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::FromTheStart))
    {
        Restart(slot);
    }
    RenderFrameSlider(slot, model.Actions[m_animation].NumAnimationKeys);
    ImGui::SameLine();
    RenderAddSound(slot);
    RenderCueTags(slot, model.Actions[m_animation].NumAnimationKeys,
                  std::max(static_cast<float>(model.Actions[m_animation].NumAnimationKeys) - HeldFrameMargin, 0.0f));

    const float speed = ModelValues::GetSpeed(values, slot, m_animation);
    ImGui::Text("%s", I18N::Format(I18N::Editor::Speed0PerFrame, {std::to_string(speed)}).c_str());
    const std::string inGame = AnimationFacts::DescribeInGame(slot, m_animation);
    if (!inGame.empty())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::InGame0, {inGame}).c_str());
    }
    if (ModelValues::HoldsLastFrame(values, slot, m_animation))
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::HoldsItsLastFrame);
    }

    ImGui::Checkbox(I18N::Editor::Turn, &m_turn);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::PlayTheSounds, &m_sounds);
    ImGui::SameLine();
    SoundPreview::RenderVolume();
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::GameSpeedFactors, &m_gameFactors);
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_gameFactors);
    ImGui::Checkbox(I18N::Editor::AsATransformedPlayer, &m_transformed);
    ImGui::EndDisabled();
}

// The slider of the frame, with the impact frame marked on it.
void MuMonsterPreviewUI::RenderFrameSlider(int slot, int keys)
{
    const float last = std::max(static_cast<float>(keys) - HeldFrameMargin, 0.0f);
    ImGui::SetNextItemWidth(FrameWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::SliderFloat(I18N::Editor::Frame, &m_frame, 0.0f, last, "%.2f"))
    {
        // Going frame by frame.
        m_playing = false;
    }
    const std::optional<float> impact = AnimationFacts::GetImpactFrame(slot, m_animation);
    if (!impact || last <= 0.0f)
    {
        return;
    }
    // The grab's centre goes from one end to the other less the grab's width.
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const float sliderWidth = FrameWidth * g_MuEditorCore.GetUIScale();
    const float grab = ImGui::GetStyle().GrabMinSize;
    const float x = min.x + grab * 0.5f + (sliderWidth - grab) * std::min(*impact / last, 1.0f);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), ImpactColor, MarkerThickness);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s",
                          I18N::Format(I18N::Editor::HitAtFrame0, {std::to_string(static_cast<int>(*impact))}).c_str());
    }
}

// The cues of the shown animation as tags under the frame slider (Q-MN80):
// hovered, a tag says its frames and what it plays; clicked, Monster Sounds
// shows its event.
void MuMonsterPreviewUI::RenderCueTags(int slot, int keys, float last)
{
    const std::span<const MonsterModelCue> cues = GetCues(slot);
    if (cues.empty() || keys <= 0 || last <= 0.0f)
    {
        return;
    }
    const MonsterSoundSet* set = ShownSet(slot);
    const float scale = g_MuEditorCore.GetUIScale();
    const float sliderWidth = FrameWidth * scale;
    const float grab = ImGui::GetStyle().GrabMinSize;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    bool any = false;
    for (const MonsterModelCue& cue : cues)
    {
        if (cue.animation != m_animation)
        {
            continue;
        }
        any = true;
        const float x = start.x + grab * 0.5f + (sliderWidth - grab) * std::min(cue.frames.first / last, 1.0f);
        const ImVec2 size = ImGui::CalcTextSize(cue.name.c_str());
        ImGui::SetCursorScreenPos(ImVec2(x - size.x * 0.5f, start.y));
        ImGui::PushID(cue.name.c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, CueColor);
        const bool clicked = ImGui::SmallButton(cue.name.c_str());
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
        {
            const std::string sounds = set != nullptr ? CueSounds(*set, cue.name) : std::string();
            ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::CueTag0123,
                                                 {cue.name, FrameText(cue.frames.first), FrameText(cue.frames.second),
                                                  sounds.empty() ? std::string(I18N::Editor::CuePlaysNothing)
                                                                 : sounds + " (" + set->name + ")"})
                                        .c_str());
        }
        if (clicked && set != nullptr)
        {
            g_MuMonsterSoundsUI.ShowEvent(set->name, FirstEventOn(*set, cue.name));
        }
        ImGui::PopID();
        // The marker on the slider above.
        ImGui::GetWindowDrawList()->AddLine(ImVec2(x, start.y - ImGui::GetFrameHeightWithSpacing()),
                                            ImVec2(x, start.y - ImGui::GetStyle().ItemSpacing.y), CueColor,
                                            MarkerThickness);
    }
    ImGui::SetCursorScreenPos(start);
    if (any)
    {
        ImGui::Dummy(ImVec2(sliderWidth, ImGui::GetFrameHeight()));
    }
}

// Add a sound at this frame (Q-MN82): a cue at the paused frame and an event
// on it in the monster's set, then Monster Sounds shows the event to pick its
// sound.
void MuMonsterPreviewUI::RenderAddSound(int slot)
{
    const MonsterSoundSet* set = ShownSet(slot);
    const MonsterModelDefinition* values = ValuesOf(slot);
    const bool possible = set != nullptr && !set->sounds.empty() && values != nullptr;
    ImGui::BeginDisabled(!possible);
    if (ImGui::SmallButton(I18N::Editor::AddASoundAtThisFrame) && possible)
    {
        m_playing = false;
        m_newCue = MuEditor::Monsters::SoundEventsEditor::MakeCue(*values, m_animation, m_frame);
        ImGui::OpenPopup(AddSoundPopup);
    }
    ImGui::EndDisabled();
    if (!possible && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", set == nullptr ? I18N::Editor::AddASoundNeedsASet : I18N::Editor::AddASoundNeedsAFile);
    }
    RenderAddSoundPopup(slot);
}

void MuMonsterPreviewUI::RenderAddSoundPopup(int slot)
{
    if (!ImGui::BeginPopup(AddSoundPopup))
    {
        return;
    }
    const MonsterSoundSet* set = ShownSet(slot);
    const std::optional<MonsterModelDefinition> model = g_MonsterDataHandler.ReadModel(slot);
    if (set == nullptr || set->sounds.empty() || !model)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::TextUnformatted(I18N::Editor::AddASoundAtThisFrame);
    ImGui::Separator();
    ImGui::TextUnformatted(I18N::Editor::CueName);
    ImGui::SameLine();
    Items::InputString("##CueName", m_newCue.name);
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::CueOfTheModel0, {model->slot}).c_str());
    ImGui::TextUnformatted(Data::Monsters::GetMonsterAnimationName(m_newCue.animation).c_str());
    ImGui::SameLine();
    ImGui::SetNextItemWidth(CueFrameWidth * scale);
    ImGui::InputFloat("##From", &m_newCue.frames.first, 0.0f, 0.0f, "%.2f");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(CueFrameWidth * scale);
    ImGui::InputFloat("##To", &m_newCue.frames.second, 0.0f, 0.0f, "%.2f");
    m_newCue.frames.first = std::max(m_newCue.frames.first, 0.0f);
    m_newCue.frames.second = std::max(m_newCue.frames.second, m_newCue.frames.first + 0.1f);
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::InTheSoundSet0, {set->name}).c_str());
    const bool taken = std::any_of(model->cues.begin(), model->cues.end(),
                                   [&](const MonsterModelCue& cue) { return cue.name == m_newCue.name; });
    ImGui::BeginDisabled(m_newCue.name.empty() || taken);
    if (ImGui::Button(I18N::Editor::Add))
    {
        const std::string label =
            StepLabel::Model(model->slot, I18N::Format(I18N::Editor::StepWhatCue0, {m_newCue.name}));
        MonsterModelDefinition changedModel = *model;
        changedModel.cues.push_back(m_newCue);
        MonsterSoundSet changedSet = *set;
        Data::Monsters::MonsterSoundEvent event;
        event.cues = {m_newCue.name};
        event.sounds = {Data::Monsters::SoundNameOf(set->sounds.front())};
        changedSet.soundEvents.push_back(event);
        const int monster = g_MonsterSelection.GetNumber();
        g_MonsterEdits.BeginGroup(label);
        g_MonsterEdits.SetModel(label, changedModel, monster);
        g_MonsterEdits.SetSoundSet(label, changedSet, monster);
        g_MonsterEdits.EndGroup();
        g_MuMonsterSoundsUI.ShowEvent(changedSet.name, changedSet.soundEvents.size() - 1);
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    if (taken && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::CueNameTaken);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

float MuMonsterPreviewUI::SpeedFactor(int slot) const
{
    if (!m_gameFactors)
    {
        return 1.0f;
    }
    float factor = 1.0f;
    for (const AnimationFacts::SpeedFactor& gameFactor : AnimationFacts::GetSpeedFactors(slot, m_animation))
    {
        using When = AnimationFacts::SpeedFactor::When;
        const bool applies = gameFactor.when == When::Always ||
                             (gameFactor.when == When::AfterFrame6 && m_frame > 6.0f) ||
                             (gameFactor.when == When::Transformed && m_transformed);
        if (applies)
        {
            factor *= gameFactor.factor;
        }
    }
    return factor;
}

// As BMD::PlayAnimation plays it: an animation that holds its last frame stops
// there, the others start again, and the game plays the sound events when the
// animation starts.
void MuMonsterPreviewUI::Advance(int slot, const MonsterModelDefinition& values)
{
    const Action_t& action = Models[slot].Actions[m_animation];
    if (action.NumAnimationKeys <= 1)
    {
        return;
    }
    // SetPlayerWalk plays a walk's sound events every frame, then those of
    // the walk moment.
    if (m_sounds && m_animation == MONSTER01_WALK)
    {
        PlaySounds(slot, m_animation, m_frame, Data::Monsters::MonsterSoundMoment::Walk);
    }
    const float speed = std::max(ModelValues::GetSpeed(values, slot, m_animation), 0.0f) * SpeedFactor(slot);
    m_frame += speed * FPS_ANIMATION_FACTOR;
    if (ModelValues::HoldsLastFrame(values, slot, m_animation))
    {
        // BMD::PlayAnimation ends a held animation when it reaches its last
        // frame, so an attack that holds it ends there too.
        const bool ended = m_frame >= static_cast<float>(action.NumAnimationKeys);
        m_frame = std::min(m_frame, static_cast<float>(action.NumAnimationKeys) - HeldFrameMargin);
        if (m_once && ended)
        {
            StandAfterAttack(slot);
        }
        return;
    }
    const int played = PlayedFrames(action);
    if (played > 0 && m_frame >= static_cast<float>(played))
    {
        if (m_once)
        {
            StandAfterAttack(slot);
            return;
        }
        m_frame = std::fmod(m_frame, static_cast<float>(played));
        Restart(slot);
    }
}

// SetPlayerStop after the attack Play started.
void MuMonsterPreviewUI::StandAfterAttack(int slot)
{
    m_once = false;
    m_animation = MONSTER01_STOP1;
    Restart(slot);
}

void MuMonsterPreviewUI::PlayAttack(int slot, int animation, std::optional<int> sound)
{
    if (!ShownModel::IsOpen(slot) || animation < 0 || animation >= Models[slot].NumActions)
    {
        return;
    }
    m_slot = slot;
    m_animation = animation;
    m_playing = true;
    m_once = true;
    Restart(slot);
    if (m_sounds && sound)
    {
        SoundPreview::Play(*sound);
    }
}

// The animation from the start, with its sounds: the sound events of its start
// (a walk's play frame by frame, in Advance), then those of its moment, as
// the game plays them (PlayMonsterSound first).
void MuMonsterPreviewUI::Restart(int slot)
{
    m_frame = 0.0f;
    if (!m_sounds)
    {
        return;
    }
    PlaySounds(slot, m_animation != MONSTER01_WALK ? m_animation : -1, m_frame, MomentOf(m_animation));
}

void MuMonsterPreviewUI::RenderView(int slot, const MonsterModelDefinition& values)
{
    const float width = ImGui::GetContentRegionAvail().x;
    if (width < 1.0f)
    {
        return;
    }
    const float scale = g_MuEditorCore.GetUIScale();
    // The width without the vertical scrollbar, so the height does not make
    // the scrollbar show and hide by turns (as the item preview's view).
    const ImGuiStyle& style = ImGui::GetStyle();
    const float widthWithoutScrollbar = ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize;
    const float height = std::clamp(widthWithoutScrollbar * ViewAspect, MinViewHeight * scale, MaxViewHeight * scale);
    Effects::PreviewInput::RenderView("##MonsterView", width, height, m_camera);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    if (m_framedSlot != slot)
    {
        m_framedSlot = slot;
        const Effects::PreviewFraming framing = m_scene.Frame(slot);
        m_camera.Frame(framing.center, framing.radius);
    }
    if (m_turn)
    {
        m_turnDegrees = std::fmod(m_turnDegrees + TurnDegreesPerFrame * FPS_ANIMATION_FACTOR, 360.0f);
    }
    if (!ImGui::IsItemVisible())
    {
        return;
    }
    std::uint32_t texture = 0;
    if (mu::GetRenderer().IsFrameActive())
    {
        // The density of the OS window the view is in; ImGui leaves it 0
        // while the editor's windows stay in the game window.
        const float viewportScale = ImGui::GetWindowViewport()->FramebufferScale.x;
        const float framebufferScale = viewportScale > 0.0f ? viewportScale : ImGui::GetIO().DisplayFramebufferScale.x;
        const Effects::PreviewTextureSize size = Effects::ChoosePreviewTextureSize(width, height, framebufferScale);
        texture = m_target.Begin(size.width, size.height, !ImGui::IsMouseDown(ImGuiMouseButton_Left));
        if (texture == 0)
        {
            return;
        }
        const MuEditor::ScopedOffscreenCapture endCapture;
        const float priorFrame = AnimationFacts::GetPriorFrame(m_frame, PlayedFrames(Models[slot].Actions[m_animation]),
                                                               ModelValues::HoldsLastFrame(values, slot, m_animation));
        m_scene.Draw(slot, m_animation, m_frame, priorFrame, m_turnDegrees, m_camera, width / height);
    }
    else
    {
        // The game skipped this frame; the editor's windows may still be drawn.
        texture = m_target.KeepLastPicture();
    }
    if (void* pointer = texture != 0 ? mu::GetRenderer().GetTexturePointer(texture) : nullptr)
    {
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)pointer, min, max);
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
