#pragma once

#include "Core/Platform/BundledFonts.h"

#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>

struct TTF_Font;

namespace Render::Text
{
enum class SdlTtfFontRole : std::size_t
{
    Normal,
    Bold,
    Big,
    Fixed,
};

// The SDL_ttf fonts of every UI text role: the selected family (normal, bold,
// big bold), the fixed-width face, and the bundled missing-glyph fallbacks
// attached behind each of them.
class SdlTtfFontSet final
{
public:
    static constexpr std::size_t kRoleCount = 4;

    SdlTtfFontSet() = default;
    ~SdlTtfFontSet();
    SdlTtfFontSet(const SdlTtfFontSet&) = delete;
    SdlTtfFontSet& operator=(const SdlTtfFontSet&) = delete;

    // Opens every role at the given sizes. On failure the current fonts stay
    // loaded and false is returned.
    [[nodiscard]] bool Load(std::string_view configuredFamily, float normalPointSize, float bigPointSize,
                            float fixedPointSize);
    void Close();

    [[nodiscard]] TTF_Font* Get(SdlTtfFontRole role) const;

private:
    using RoleFonts = std::array<TTF_Font*, kRoleCount>;
    using RoleSizes = std::array<float, kRoleCount>;

    [[nodiscard]] bool Open(std::string_view configuredFamily, const RoleSizes& pointSizes);
    [[nodiscard]] bool OpenFallbacks(const BundledFont& fallbackFont, const char* const (&rolePaths)[kRoleCount],
                                     RoleFonts& fallbacks);

    RoleFonts m_primary{};
    std::array<RoleFonts, std::size(kBundledFallbackFonts)> m_fallbacks{};
};
// SDL property of each role font: how many pixels higher its text is drawn so
// that it sits where DejaVu Sans text would. The layout was tuned for DejaVu
// Sans; the value is 0 unless the selected family has a taller ascent.
inline constexpr const char* kLayoutLiftProperty = "MuMain.SDL_ttf.font.layout_lift";

[[nodiscard]] int LayoutLift(TTF_Font* font);
} // namespace Render::Text
