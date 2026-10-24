#include "stdafx.h"

#ifdef _EDITOR

#include "CodeReferenceList.h"

#include "I18N/All.h"
#include "imgui.h"

#include <string>

namespace MuEditor::CodeReferenceList
{
namespace
{
// How many source lines are listed; the count says how many there are.
constexpr size_t ListedCodeLines = 12;
constexpr float CodeListLines = 8.0f;
} // namespace

void Render(bool known, std::span<const CodeReference> references, const char* notFoundText)
{
    if (!known)
    {
        ImGui::BulletText("%s", notFoundText);
        return;
    }
    if (references.empty())
    {
        return;
    }
    ImGui::BulletText("%s", I18N::Format(I18N::Editor::NamedInCode0Lines, {std::to_string(references.size())}).c_str());
    if (ImGui::BeginListBox("##Code", ImVec2(-FLT_MIN, CodeListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        for (size_t index = 0; index < references.size() && index < ListedCodeLines; ++index)
        {
            const CodeReference& reference = references[index];
            ImGui::Text("%s:%d  %s", reference.file.c_str(), reference.line, reference.text.c_str());
        }
        if (references.size() > ListedCodeLines)
        {
            ImGui::TextDisabled("...");
        }
        ImGui::EndListBox();
    }
}
} // namespace MuEditor::CodeReferenceList

#endif // _EDITOR
