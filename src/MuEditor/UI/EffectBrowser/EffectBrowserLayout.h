#pragma once

#ifdef _EDITOR

// Rows of controls in the effect browser that go on in the next line when
// the pane is too narrow for them.
namespace MuEditor::Effects::Layout
{
// Goes on in the same line when `width` fits there, else in the next one.
void SameLineIfFits(float width);

// The widths of controls with their labels, at the UI scale.
float LabeledComboWidth(const char* label, float comboWidth);
float CheckboxWidth(const char* label);
float ButtonWidth(const char* label);
} // namespace MuEditor::Effects::Layout

#endif // _EDITOR
