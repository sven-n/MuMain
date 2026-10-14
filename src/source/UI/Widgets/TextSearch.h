#pragma once

#include <string_view>

namespace UI::TextSearch
{

[[nodiscard]] bool Contains(std::wstring_view text, std::wstring_view token, bool firstOnly = false);

} // namespace UI::TextSearch
