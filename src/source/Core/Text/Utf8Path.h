// A filesystem path from UTF-8 text. Apart from Utf8.h, which much of the game includes for its string
// conversions, so that only the files that build paths compile <filesystem> for it.
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace Core::Text
{
// A path from UTF-8 text on every platform, as std::filesystem::u8path made
// it before C++20 deprecated that function.
[[nodiscard]] inline std::filesystem::path PathFromUtf8(std::string_view text)
{
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}
} // namespace Core::Text
