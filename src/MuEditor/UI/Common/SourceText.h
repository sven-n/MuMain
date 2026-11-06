#pragma once

#ifdef _EDITOR

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// Reading the C++ sources the editor was built from, for the tools that look
// into them: the effect browser's create calls and the item tools' lines that
// name an item.
namespace MuEditor::SourceText
{
// A .cpp or .h file below a source folder.
struct SourceFile
{
    std::filesystem::path path;
    // Relative to the folder, with '/'.
    std::string relative;
};

bool IsIdentifierChar(char c);

// The text without the spaces and line breaks around it.
std::string Trimmed(std::string_view text);

// The text with comments and string and character literals blanked out;
// line breaks stay, so positions keep their lines.
std::string Blank(std::string_view source);

// The whole file; empty when it cannot be read.
std::string ReadFile(const std::filesystem::path& path);

// The .cpp and .h files below the folder, sorted by their relative path. The
// walk stops at the first folder it cannot read.
std::vector<SourceFile> FindSourceFiles(const std::filesystem::path& folder);
} // namespace MuEditor::SourceText

#endif // _EDITOR
