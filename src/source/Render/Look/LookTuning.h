#pragma once

#include "Render/Look/LookFiles.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Live tuning of the look profile and the light mood in developer builds: the `$look` commands
// (Render/Look/LookTuningCommand.h) and the DevEditor's Look panel call these. Every change
// applies from the next frame on and is logged; a refused one changes nothing and logs why.
#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
namespace Render::Look
{

// The live documents and values.
[[nodiscard]] const LookFiles& LiveFiles();
// Where the live values were loaded from: "shipped" or the candidate's name.
[[nodiscard]] const std::string& LiveSource();

// Every value (or one) on the active world, with where it comes from.
[[nodiscard]] std::vector<LookValueState> DescribeLive(std::optional<std::string_view> name = std::nullopt);

// Sets the values in the default profile, or in the active world's entry.
bool SetLive(std::span<const LookEdit> edits, bool currentMap);
// Back to the values loaded from the files: every value, or one name in the default and the
// active world's entry.
bool ResetLive(std::optional<std::string_view> name);
// Makes Data/Render/Candidates/<candidate>/ the live values, or the shipped files again (empty).
bool LoadLive(std::string_view candidate);
// Writes the live values into the shipped files.
bool SaveLive();
// How many times SaveLive wrote the files in this session, by `$look save` or the DevEditor's
// Save; Data Checks compares it to say that the looks changed since its last check.
[[nodiscard]] int SaveCount();

// `$look show|set|reset|load|save` and the aliases; false when the text is none of them.
bool HandleDeveloperCommand(std::wstring_view command);

} // namespace Render::Look
#endif
