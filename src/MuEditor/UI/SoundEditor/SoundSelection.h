#pragma once

#ifdef _EDITOR

// The sound selected in the Sound Editor and the Sound Mixer (sound editor
// design, Q-SE3): a click in either selects it in both; a window shows it
// when it draws, and scrolls to it once after it changed, without opening or
// raising itself.
namespace MuEditor::SoundSelection
{
// The selected sound number; -1 for none.
int Get();
void Set(int number);
// Changes with every Set of another number.
int GetVersion();
} // namespace MuEditor::SoundSelection

#endif // _EDITOR
