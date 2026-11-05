// =============================================================
// Generated once from Data/Local/Eng/Dialog_eng.bmd by the removed DialogImporter tool (see git history).
// =============================================================

#pragma once

namespace GameLogic::Quests::Dialog {

inline constexpr int MaxAnswer = 10;
inline constexpr int MaxDialog = 200;

// One reply branch in a dialog entry. `link` is the dialog index
// the client jumps to when the player picks this reply; `returnCode`
// is the answer code the client sends back to the server; `text` is
// the reply label (an I18N::Dialog slot), null for an unused slot.
struct Branch {
    int link;
    int returnCode;
    const wchar_t* const* text;
};

// A dialog entry: its replies and the NPC line (an I18N::Dialog
// slot, null when the dialog has none).
struct Entry {
    int numAnswer;
    Branch answers[MaxAnswer];
    const wchar_t* const* text;
};

// Returns the entry for `dialogIndex` (0-based). Out-of-range
// indices return a zero-answer sentinel entry so callers can
// render an empty dialog rather than dereferencing past the table.
const Entry& GetEntry(int dialogIndex) noexcept;

// The NPC line of `dialogIndex` in the current locale; empty when
// the dialog has none or the index is out of range.
const wchar_t* GetText(int dialogIndex) noexcept;

// The label of reply `answerSlot` of `dialogIndex` in the current
// locale; empty when that reply has none or either index is out of range.
const wchar_t* GetAnswerText(int dialogIndex, int answerSlot) noexcept;

}  // namespace GameLogic::Quests::Dialog
