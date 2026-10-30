#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterPreviewUI.h"

#include "MonsterAnimationFacts.h"
#include "MonsterModelValues.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterSoundPreview.h"
#include "MonsterToolWindow.h"
#include "MuMonsterModelUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewInput.h"
#include "../MuEditor/UI/EffectBrowser/EffectPreviewSubject.h"
#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/MonsterSlotSound.h"
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
using Data::Monsters::MonsterModelDefinition;

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
// The sound slots (BMD::Sounds): idle 0 and 1, attack 2 and 3, die 4.
constexpr int IdleSlot = 0;
constexpr int AttackSlot = 2;
constexpr int DieSlot = 4;
constexpr ImU32 ImpactColor = IM_COL32(255, 90, 60, 255);
constexpr float MarkerThickness = 2.0f;

// The frames the animation plays through before it starts again: one less
// for an animation that ends where it starts (LockPositions), as
// BMD::PlayAnimation counts.
int PlayedFrames(const Action_t& action)
{
    return action.LockPositions ? action.NumAnimationKeys - 1 : action.NumAnimationKeys;
}

// The sound slot the game plays when the animation starts, or nullopt:
// SetPlayerStop and SetPlayerWalk an idle sound (at random while it plays),
// SetPlayerAttack and SetPlayerShock an attack sound, SetPlayerDie the die
// sound. Of a pair the game picks one at random.
std::optional<int> SoundSlotOf(int animation)
{
    switch (animation)
    {
    case MONSTER01_STOP1:
    case MONSTER01_STOP2:
    case MONSTER01_WALK:
        return IdleSlot + std::rand() % 2;
    case MONSTER01_ATTACK1:
    case MONSTER01_ATTACK2:
    case MONSTER01_ATTACK3:
    case MONSTER01_ATTACK4:
    case MONSTER01_ATTACK5:
    case MONSTER01_SHOCK:
        return AttackSlot + std::rand() % 2;
    case MONSTER01_DIE:
        return DieSlot;
    default:
        return std::nullopt;
    }
}

// The values the preview plays: the Monster Model tool's, with an edit that
// is not stored yet, else the model list's.
const MonsterModelDefinition* ValuesOf(int slot)
{
    if (const MonsterModelDefinition* draft = g_MuMonsterModelUI.GetShownDraft(slot))
    {
        return draft;
    }
    return g_MonsterModelDatabase.Find(slot);
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
        const std::optional<int> slot = monster != nullptr ? ShownModel::FindSlot(*monster) : std::nullopt;
        ShownModel::RenderSelectedMonster(number, monster);
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
// there, the others start again, and the game plays the slot's sound when the
// animation starts.
void MuMonsterPreviewUI::Advance(int slot, const MonsterModelDefinition& values)
{
    const Action_t& action = Models[slot].Actions[m_animation];
    if (action.NumAnimationKeys <= 1)
    {
        return;
    }
    // SetPlayerWalk plays a walk's sound events every frame.
    if (m_sounds && m_animation == MONSTER01_WALK)
    {
        SoundPreview::PlayEvents(values, m_animation, m_frame);
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
// (a walk's play frame by frame, in Advance), then the slot's, as the game
// plays them (PlayMonsterSound first).
void MuMonsterPreviewUI::Restart(int slot)
{
    m_frame = 0.0f;
    if (!m_sounds)
    {
        return;
    }
    const MonsterModelDefinition* values = ValuesOf(slot);
    if (values != nullptr && m_animation != MONSTER01_WALK)
    {
        SoundPreview::PlayEvents(*values, m_animation, m_frame);
    }
    if (const std::optional<int> soundSlot = SoundSlotOf(m_animation))
    {
        if (const std::optional<ESound> sound = Engine::Object::MonsterSlotSound::Find(Models[slot], *soundSlot))
        {
            SoundPreview::Play(*sound);
        }
    }
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
