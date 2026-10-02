#include "stdafx.h"

#include "TextFile.h"

#include <fstream>
#include <iterator>
#include <string_view>

namespace Data
{
namespace
{
constexpr std::string_view Utf8ByteOrderMark = "\xEF\xBB\xBF";
} // namespace

std::optional<std::string> ReadTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return std::nullopt;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (text.starts_with(Utf8ByteOrderMark))
    {
        text.erase(0, Utf8ByteOrderMark.size());
    }
    return text;
}
} // namespace Data
