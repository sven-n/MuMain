#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The case labels of the legacy switches of ZzzEffect.cpp, read from the
// source text: the switch after the registry lookup in CreateEffect,
// MoveEffect and RenderEffects, and the switches of RenderEffectShadows, which
// draws on the ground, and RenderAfterEffects, which draws again after the
// characters; those two have no registry lookup. For the tests that check
// which stages of an effect type are still a case. Also the conditions of the
// code MoveEffect runs after its switch, and the cases that return before it.
namespace EffectSourceCases
{
enum class Stage
{
    Create,
    Move,
    Render,
    Ground,
    AfterCharacters,
};

// Whether the macro of an #ifdef or #ifndef in the five functions is defined.
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

// The conditions of the code after MoveEffect's switch, without spaces:
// `skipped` holds the types that code leaves alone, `animated` the types of
// the others whose model it animates.
struct SharedMoveConditions
{
    std::string skipped;
    std::string animated;
    // What the scan cannot read: the function or the switch not found, or the
    // code after the switch no longer
    // `if (skipped) {} else { if (animated) { ... PlayAnimation(...) ... } ... }`.
    std::vector<std::string> problems;
};

SharedMoveConditions ReadSharedMoveConditions(const std::string& source, std::span<const MacroState> macros);

// The labels of the cases of the switch of `stage` whose statements always end
// with `return;`, so the code after the switch never runs for them. A label
// without statements counts as the case it falls through to.
SwitchLabels ReadCasesThatReturn(const std::string& source, Stage stage, std::span<const MacroState> macros);

// The labels of the cases of the switch of `stage` whose statements contain
// `text` (spaces left out on both sides).
SwitchLabels ReadCasesContaining(const std::string& source, Stage stage, std::span<const MacroState> macros,
                                 std::string_view text);
} // namespace EffectSourceCases
