#pragma once

#include <optional>
#include <string>
#include <string_view>

// File name rules of the models and their textures, shared by the loader
// messages, the model file checks and the tests.
namespace Data::Items
{
bool EqualsIgnoringCase(std::string_view left, std::string_view right);
bool EndsWithIgnoringCase(std::string_view text, std::string_view ending);

// Meshes whose texture name starts with "hid" are not drawn; their texture
// is never loaded.
bool IsHiddenTexture(std::string_view textureFileName);

// The game loads .jpg and .tga textures from encrypted copies with the same
// name: "Sword01.jpg" is read from "Sword01.OZJ", "hair.tga" from
// "hair.OZT". Returns that file name, or nothing for other file types.
std::optional<std::string> GetStoredTextureFileName(std::string_view textureFileName);
} // namespace Data::Items
