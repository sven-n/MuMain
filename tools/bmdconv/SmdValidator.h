#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Tools::BmdConv
{
enum class SmdKind
{
    Reference,
    Animation
};

struct SmdStats
{
    int version = 0;
    int nodeCount = 0;
    int frameCount = 0;
    int triangleCount = 0;
    std::vector<std::string> nodeNames;
    std::vector<std::string> materials;
};

struct SmdValidation
{
    SmdStats stats;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    bool ok() const
    {
        return errors.empty();
    }
};

// Checks an SMD text file against what the engine's SMD parser (Render/Models/SMD.cpp)
// and the BMD limits (Render/Models/ZzzBMD.h) accept. The engine parser has no error
// reporting and fixed-size buffers, so everything it would mis-read or overflow is
// rejected here first, with a message that names the offending line.
SmdValidation ValidateSmd(const std::filesystem::path& file, SmdKind kind);
} // namespace Tools::BmdConv
