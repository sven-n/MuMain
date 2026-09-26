// Curated UI fonts shipped in the client's ./fonts directory.
//
// Single source of truth shared by every platform: the non-Windows GdiText shim
// reads bundled files directly, while Windows privately registers the same files
// with GDI. Family names match the options UI and config; paths are relative to
// the runtime directory normalized during startup.
#pragma once

#include <filesystem>
#include <string_view>

struct BundledFont
{
    const char* family;
    const char* regular;
    const char* bold;
};

// Traditional Chinese (zh-TW): selectable as the UI font and the first
// missing-glyph fallback. Simplified Chinese (zh-CN) would need Noto Sans SC,
// because the glyph forms differ.
inline constexpr BundledFont kNotoSansTcFont{"Noto Sans TC", "fonts/NotoSansTC-Regular.otf",
                                             "fonts/NotoSansTC-Bold.otf"};

inline constexpr BundledFont kDejaVuSansFont{"DejaVu Sans", "fonts/DejaVuSans.ttf", "fonts/DejaVuSans-Bold.ttf"};

inline constexpr BundledFont kBundledFonts[] = {
    {"Liberation Sans", "fonts/LiberationSans-Regular.ttf", "fonts/LiberationSans-Bold.ttf"},
    kDejaVuSansFont,
    kNotoSansTcFont,
};

inline constexpr std::string_view kDefaultBundledFontFamily = "DejaVu Sans";
inline constexpr BundledFont kBundledFixedFont{
    "Cousine", "fonts/Cousine-Regular.ttf", "fonts/Cousine-Regular.ttf"};
// Missing-glyph fallbacks behind every SDL_ttf role, in the order SDL_ttf tries
// them; it takes a glyph from the first one that has it.
// - DejaVu Sans: the Latin, Greek and Cyrillic letters Liberation Sans, Noto
//   Sans TC and Cousine lack. It has no CJK, so it takes nothing from the others.
// - Noto Sans TC: Han, kana and CJK punctuation. It comes before Nanum Gothic so
//   that Chinese text keeps its centered punctuation (，。「」) instead of the
//   Korean forms. It also has the Hangul jamo (ㅋ), but no Hangul syllables.
// - Nanum Gothic: Hangul syllables.
// ponytail: one Hangul face; SDL_ttf synthesizes bold, bundle NanumGothic-Bold if metric parity requires it.
inline constexpr BundledFont kBundledFallbackFonts[] = {
    kDejaVuSansFont,
    kNotoSansTcFont,
    {"Nanum Gothic", "fonts/NanumGothic-Regular.ttf", "fonts/NanumGothic-Regular.ttf"},
};

[[nodiscard]] inline std::filesystem::path ResolveBundledFontPath(const std::filesystem::path& relativePath)
{
    std::error_code error;
    const std::filesystem::path runtimeDirectory = std::filesystem::current_path(error);
    return error ? relativePath : runtimeDirectory / relativePath;
}

[[nodiscard]] constexpr const BundledFont& ResolveBundledFont(std::string_view configuredFamily)
{
    for (const BundledFont& font : kBundledFonts)
    {
        if (configuredFamily == font.family)
            return font;
    }
    for (const BundledFont& font : kBundledFonts)
    {
        if (kDefaultBundledFontFamily == font.family)
            return font;
    }
    return kBundledFonts[0];
}
