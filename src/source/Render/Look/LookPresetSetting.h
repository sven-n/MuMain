#pragma once

#include "Render/Look/LookPreset.h"

#include <string>
#include <string_view>

// The client's look preset and developer overrides (Render/Look/LookPreset.h).
namespace Render::Look
{

// The client's preset: saves [Render] Look and switches the five features. Start-up, the options
// window and `$look classic|enhanced` all go through this.
void ApplyLookPreset(LookPreset preset);
[[nodiscard]] LookPreset ActiveLookPreset();
// `$look classic|enhanced`, in every build; false when the text is not that command.
bool HandleLookPresetCommand(std::wstring_view command);

#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
// The per-feature developer commands as session overrides; false when the text is none of them.
bool HandleFeatureOverrideCommand(std::wstring_view command);
// The active overrides, for `$look show`.
[[nodiscard]] std::string DescribeFeatureOverrides();
#endif

} // namespace Render::Look
