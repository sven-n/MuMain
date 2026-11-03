#pragma once

#ifdef _EDITOR

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Mesh numbers as the model tool writes them in a text field: "1, 3".
namespace MuEditor::Items::NumberList
{
std::string Format(const std::vector<int>& numbers);
// nullopt when the text has something that is not a number 0 or more.
std::optional<std::vector<int>> Parse(std::string_view text);
} // namespace MuEditor::Items::NumberList

#endif // _EDITOR
