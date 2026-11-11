#include "stdafx.h"

#include "ItemTextureFiles.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace Data::Items
{
namespace
{
struct StoredTextureExtension
{
    std::string_view textureExtension;
    std::string_view storedExtension;
};

constexpr std::array<StoredTextureExtension, 2> StoredTextureExtensions = {{{".jpg", ".OZJ"}, {".tga", ".OZT"}}};
constexpr std::string_view HiddenTexturePrefix = "hid";
} // namespace

bool EqualsIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
           std::equal(
               left.begin(), left.end(), right.begin(), [](char a, char b)
               { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

bool EndsWithIgnoringCase(std::string_view text, std::string_view ending)
{
    return text.size() >= ending.size() && EqualsIgnoringCase(text.substr(text.size() - ending.size()), ending);
}

bool IsHiddenTexture(std::string_view textureFileName)
{
    return textureFileName.starts_with(HiddenTexturePrefix);
}

std::optional<std::string> GetStoredTextureFileName(std::string_view textureFileName)
{
    const size_t dot = textureFileName.rfind('.');
    if (dot == std::string_view::npos)
    {
        return std::nullopt;
    }

    const std::string_view extension = textureFileName.substr(dot);
    for (const StoredTextureExtension& stored : StoredTextureExtensions)
    {
        if (EqualsIgnoringCase(extension, stored.textureExtension))
        {
            return std::string(textureFileName.substr(0, dot)) + std::string(stored.storedExtension);
        }
    }
    return std::nullopt;
}
} // namespace Data::Items
