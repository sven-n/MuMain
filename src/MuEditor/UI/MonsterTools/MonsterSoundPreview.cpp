#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSoundPreview.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace MuEditor::Monsters::SoundPreview
{
namespace
{
// Pixel width before the editor UI scale.
constexpr float SliderWidth = 160.0f;
constexpr int FullPercent = 100;

float g_volume = 1.0f;
} // namespace

float GetVolume()
{
    return g_volume;
}

void SetVolume(float volume)
{
    g_volume = std::clamp(volume, 0.0f, 1.0f);
}

void Play(int number)
{
    PlayBufferAtLevel(static_cast<ESound>(number), g_volume);
}

void PlayEvents(const Data::Monsters::MonsterSoundSet& set, std::span<const Data::Monsters::MonsterModelCue> cues,
                int animation, float frame)
{
    GameLogic::Monsters::SoundEvents::Situation situation;
    situation.animation = animation;
    situation.frame = frame;
    situation.deathEffectArmed = true;
    GameLogic::Monsters::SoundEvents::PlayEvents(set, cues, situation, [](int number, bool) { Play(number); });
}

void PlayMoment(const Data::Monsters::MonsterSoundSet& set, Data::Monsters::MonsterSoundMoment moment)
{
    GameLogic::Monsters::SoundEvents::Situation situation;
    situation.moment = moment;
    situation.deathEffectArmed = true;
    GameLogic::Monsters::SoundEvents::PlayEvents(set, {}, situation, [](int number, bool) { Play(number); });
}

void PlaySoundOfSet(const Data::Monsters::MonsterSoundSet& set, const std::string& name)
{
    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(set, name);
    if (!number)
    {
        return;
    }
    for (const Data::Monsters::MonsterSound& sound : set.sounds)
    {
        if (Data::Monsters::SoundNameOf(sound) == name)
        {
            Data::Monsters::ModelLoader::LoadSound(sound);
            break;
        }
    }
    Play(*number);
}

void RenderVolume()
{
    int percent = static_cast<int>(std::lround(g_volume * FullPercent));
    ImGui::SetNextItemWidth(SliderWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::SliderInt(I18N::Editor::PreviewVolume, &percent, 0, FullPercent, "%d%%"))
    {
        SetVolume(static_cast<float>(percent) / FullPercent);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::PreviewVolumeNote);
    }
}
} // namespace MuEditor::Monsters::SoundPreview

#endif // _EDITOR
