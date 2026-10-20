#pragma once

#include "Data/GameData/MapData/LookProfile.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// The developer `$` commands of the look profile. Only builds compiled with MU_ENABLE_CONTROL_SOCKET
// or _EDITOR offer them (Render::Look::HandleDeveloperCommand).
//
//   $look show [<name>]            effective values and where each comes from
//   $look set [map] <name> <v>     the default profile, or the current map's entry
//   $look reset [<name>]           back to the values loaded from the files
//   $look load [<candidate>]       Data/Render/Candidates/<candidate>/, or the shipped files
//   $look save                     writes Data/Render/Look.json and Data/Maps/LightMood.json
//
// Aliases of `$look set <name>` (the default profile):
//   $lighting rim <v>              lighting.rim.strength
//   $lighting specular <v>         lighting.specular.scale
//   $lighting diffuse <s> <a> <f>  lighting.diffuse.scale, .ambient, .floor
//   $lightmood knee <k>            lighting.highlightKnee
//   $dynlights wrap <w>            dynamic.wrap
//   $dynlights smooth <s>          dynamic.mapSmoothing
//   $dynlights spell <g> <r> <f>   dynamic.effectGain, .effectRange, .effectFade
//   $dynlights cap <c>             dynamic.effectCap
//   $dynlights reach <m>           dynamic.effectRangeMax
//   $bloom falloff <f>             bloom.levelFalloff
//   $bloom <v>                     bloom.strength
namespace Render::Look
{

struct LookAssignment
{
    Data::Maps::LookValue value = Data::Maps::LookValue::Count;
    double newValue = 0.0;
};

struct LookTuningCommand
{
    static constexpr std::size_t MaxAssignments = 3;
    std::array<LookAssignment, MaxAssignments> assignments{};
    std::size_t count = 0;
};

// The values an alias sets; nothing when it is none of the aliases above or its numbers do not
// match. Ranges are not checked here.
[[nodiscard]] std::optional<LookTuningCommand> ParseLookTuningCommand(std::wstring_view command);

enum class LookVerb : std::uint8_t
{
    Show,
    Set,
    Reset,
    Load,
    Save,
};

struct LookCommand
{
    LookVerb verb = LookVerb::Show;
    // Set: the current map's entry instead of the default.
    bool map = false;
    // Show, set, reset: the value's name (empty: every value); load: the candidate (empty: the
    // shipped files).
    std::string name;
    double value = 0.0;
};

// A `$look show|set|reset|load|save` command; nothing for any other text (also `$look classic`).
// A candidate name is letters, digits, '-', '_' and '.', never "." or "..".
[[nodiscard]] std::optional<LookCommand> ParseLookCommand(std::wstring_view command);

} // namespace Render::Look
