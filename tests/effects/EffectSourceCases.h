#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The case labels of the legacy switches of ZzzEffect.cpp, read from the
// source text: the switch after the registry lookup in CreateEffect,
// MoveEffect and RenderEffects. For the tests that check which stages of an
// effect type are still a case.
namespace EffectSourceCases
{
enum class Stage
{
    Create,
    Move,
    Render,
};

// Whether the macro of an #ifdef or #ifndef in the three functions is defined.
struct MacroState
{
    std::string_view name;
    bool defined = false;
};

struct SwitchLabels
{
    // The labels at the top level of the switch, without spaces
    // ("MODEL_SKILL_FURY_STRIKE+1"), in the order of the source.
    std::vector<std::string> labels;
    // What the scan cannot read: the function or the switch not found, an
    // #if or a macro not in the list, a label below the top level of the
    // switch.
    std::vector<std::string> problems;
};

// The text of ZzzEffect.cpp with comments and string literals blanked out.
std::string ReadEffectSource(const std::filesystem::path& zzzEffect);

SwitchLabels ReadSwitchLabels(const std::string& source, Stage stage, std::span<const MacroState> macros);
} // namespace EffectSourceCases
