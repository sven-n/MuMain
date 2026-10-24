#pragma once

#include <cstdint>
#include <string>
#include <vector>

class BMD;

namespace Render::Models::Gltf
{
struct EmbeddedImage;

// An embedded image decoded to RGBA8, rows top first.
struct DecodedImage
{
    int width = 0;
    int height = 0;
    // The file has an alpha channel (the texture then blends, like a .tga one).
    bool hasAlpha = false;
    std::vector<std::uint8_t> rgba;
};

// Decodes a PNG image a model file carries. False, with the reason, for
// another type, a PNG that cannot be read or one above the bitmap size limit
// (Render/Sprites/BitmapTextureLimits.h).
bool DecodeEmbeddedImage(const EmbeddedImage& image, DecodedImage& decoded, std::string& reason);

// Gives every mesh of `model` (loaded from a glTF binary) whose material carries
// an image that image as texture (BMD::IndexTexture), loaded under the name
// "<modelFile>#<image name>". A mesh without one, or whose image cannot be
// used (logged), keeps the texture it has. Returns the number of meshes that
// got an embedded image. Also sets the textures of the coarser LODs' meshes
// (SkinnedModel::lodTextures): their image, else the texture of the BMD mesh
// with the same texture name, else that of the BMD mesh of the same index -
// so it runs after the BMD's textures were opened - and loads the normal maps
// of the meshes of every LOD (SkinnedModel::meshNormalTextures,
// lodNormalTextures).
int OpenEmbeddedTextures(BMD& model, const std::wstring& modelFile, int wrap, int filter);
} // namespace Render::Models::Gltf
