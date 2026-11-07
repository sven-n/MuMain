#pragma once

#include <optional>
#include <string>

// The language a player reads, as MuError.log names it: once at start-up and again for every change after it, so a
// report shows the texts its player saw. The options window and the editor's language box switch the UI language,
// and the options window turns translated item names on and off.
namespace Core::Log::ReadingLanguage
{
struct State
{
    std::string uiLocale;
    bool itemNamesTranslated = false;
};

// The line for `now` after the line written for `logged`: the first one names both, e.g.
// "> UI language = de, item names translated."; a later one what changed, e.g. "> UI language changed to zh-CN." or
// "> Item names now in English.". Empty when nothing changed, as when the editor's language box applies the active
// language again.
std::string Describe(const std::optional<State>& logged, const State& now);

// Writes Describe's line for `now` into MuError.log and remembers `now` as what the log says.
void Write(const State& now);
} // namespace Core::Log::ReadingLanguage
