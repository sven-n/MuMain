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

inline constexpr BundledFont kBundledFonts[] = {
    { "Liberation Sans", "fonts/LiberationSans-Regular.ttf", "fonts/LiberationSans-Bold.ttf" },
    { "DejaVu Sans",     "fonts/DejaVuSans.ttf",             "fonts/DejaVuSans-Bold.ttf" },
};

inline constexpr std::string_view kDefaultBundledFontFamily = "DejaVu Sans";
inline constexpr BundledFont kBundledFixedFont{
    "Cousine", "fonts/Cousine-Regular.ttf", "fonts/Cousine-Regular.ttf"};

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
