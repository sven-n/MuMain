#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSoundPreview.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Audio/DSPlaySound.h"
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
