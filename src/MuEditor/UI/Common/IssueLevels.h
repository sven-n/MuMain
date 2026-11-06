#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "imgui.h"

// How the editor shows the level of a data problem, the same in Data Checks
// and in the problem lists of the tools' saves.
namespace MuEditor::IssueLevels
{
// Fatal red, error orange, warning yellow.
ImVec4 GetColor(Data::Items::ItemDataIssueSeverity severity);

// "Fatal", "Error" or "Warning" in the editor language.
const char* GetName(Data::Items::ItemDataIssueSeverity severity);

// A problem in a tool's list: "<level>: <text>" in the level's color, wrapped.
void RenderLine(Data::Items::ItemDataIssueSeverity severity, const char* text);
} // namespace MuEditor::IssueLevels

#endif // _EDITOR
