#include "SdlTtfFontSet.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <filesystem>
#include <string>

namespace Render::Text
{
namespace
{
#ifdef NDEBUG
inline constexpr bool kAllowSystemFontFallback = false;
#else
inline constexpr bool kAllowSystemFontFallback = true;
#endif

constexpr std::size_t RoleIndex(SdlTtfFontRole role)
{
    return static_cast<std::size_t>(role);
}

constexpr std::string_view RoleName(std::size_t role)
{
    constexpr std::string_view kRoleNames[SdlTtfFontSet::kRoleCount] = {"normal", "bold", "big-bold", "fixed"};
    return kRoleNames[role];
}

constexpr bool IsBoldRole(std::size_t role)
{
    return role == RoleIndex(SdlTtfFontRole::Bold) || role == RoleIndex(SdlTtfFontRole::Big);
}

[[nodiscard]] std::string BundledFontPath(const char* relativePath)
{
    return ResolveBundledFontPath(relativePath).string();
}

[[nodiscard]] std::string FindDeveloperFontPath()
{
#ifndef NDEBUG
    static const char* const k_SystemFontPaths[] = {
#ifdef __APPLE__
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial Unicode.ttf",
#elif defined(__linux__)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#else // Windows
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
#endif
    };

    for (const char* path : k_SystemFontPaths)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }
#endif

    return {};
}

void CloseTtfFont(TTF_Font*& font)
{
    if (!font)
        return;
    TTF_CloseFont(font);
    font = nullptr;
}

[[nodiscard]] TTF_Font* OpenDeveloperFont(std::string_view family, std::string_view role, float pointSize)
{
    if (!kAllowSystemFontFallback)
        return nullptr;

    const std::string fallbackPath = FindDeveloperFontPath();
    if (fallbackPath.empty())
        return nullptr;

    mu::log::Get("render")->warn("SDL_ttf -- NON-PARITY developer font fallback family='{}' role='{}' path='{}'",
                                 family, role, fallbackPath);
    TTF_Font* fallback = TTF_OpenFont(fallbackPath.c_str(), pointSize);
    if (!fallback)
    {
        mu::log::Get("render")->error(
            "SDL_ttf -- NON-PARITY developer font fallback family='{}' role='{}' path='{}' failed: {}", family, role,
            fallbackPath, SDL_GetError());
    }
    return fallback;
}

[[nodiscard]] TTF_Font* OpenTtfFontRole(std::string_view family, std::string_view role, const char* relativePath,
                                        float pointSize)
{
    const std::string packagedPath = BundledFontPath(relativePath);
    if (TTF_Font* font = TTF_OpenFont(packagedPath.c_str(), pointSize))
    {
        mu::log::Get("render")->info("SDL_ttf -- bundled family='{}' role='{}' path='{}'", family, role, packagedPath);
        return font;
    }

    mu::log::Get("render")->error("SDL_ttf -- bundled family='{}' role='{}' path='{}' failed: {}", family, role,
                                  packagedPath, SDL_GetError());
    return OpenDeveloperFont(family, role, pointSize);
}

[[nodiscard]] const char* FallbackPath(const BundledFont& fallbackFont, std::size_t role)
{
    return IsBoldRole(role) ? fallbackFont.bold : fallbackFont.regular;
}

[[nodiscard]] TTF_Font* OpenTtfFallbackRole(const BundledFont& fallbackFont, std::size_t role, TTF_Font* primary)
{
    const std::string packagedPath = BundledFontPath(FallbackPath(fallbackFont, role));
    TTF_Font* font = TTF_OpenFont(packagedPath.c_str(), TTF_GetFontSize(primary));
    if (!font)
    {
        mu::log::Get("render")->error("SDL_ttf -- bundled fallback family='{}' role='{}' path='{}' failed: {}",
                                      fallbackFont.family, RoleName(role), packagedPath, SDL_GetError());
        return nullptr;
    }

    // A family without a bold file uses SDL_ttf's synthetic bold.
    if (IsBoldRole(role) && std::string_view(fallbackFont.bold) == fallbackFont.regular)
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    mu::log::Get("render")->info("SDL_ttf -- bundled fallback family='{}' role='{}' path='{}'", fallbackFont.family,
                                 RoleName(role), packagedPath);
    return font;
}

// The UI layout was tuned for DejaVu Sans. A role font with a taller ascent
// (Noto Sans TC: 1.16 em against 0.93 em) would sit low in every text box, so
// its text is drawn higher by the difference.
[[nodiscard]] int MeasureLayoutLift(TTF_Font* font, const char* relativePath, std::size_t role)
{
    const char* referencePath = FallbackPath(kDejaVuSansFont, role);
    if (std::string_view(relativePath) == referencePath)
        return 0;

    TTF_Font* reference = TTF_OpenFont(BundledFontPath(referencePath).c_str(), TTF_GetFontSize(font));
    if (!reference)
    {
        mu::log::Get("render")->warn("SDL_ttf -- layout reference '{}' failed: {}", referencePath, SDL_GetError());
        return 0;
    }
    const int lift = std::max(0, TTF_GetFontAscent(font) - TTF_GetFontAscent(reference));
    TTF_CloseFont(reference);
    return lift;
}

[[nodiscard]] bool AttachTtfFallback(TTF_Font* font, TTF_Font* fallback, std::string_view role)
{
    if (TTF_AddFallbackFont(font, fallback))
        return true;

    mu::log::Get("render")->error("SDL_ttf -- fallback attach failed for role='{}': {}", role, SDL_GetError());
    return false;
}
} // namespace

SdlTtfFontSet::~SdlTtfFontSet()
{
    Close();
}

bool SdlTtfFontSet::Load(std::string_view configuredFamily, float normalPointSize, float bigPointSize,
                         float fixedPointSize)
{
    SdlTtfFontSet loaded;
    if (!loaded.Open(configuredFamily, {normalPointSize, normalPointSize, bigPointSize, fixedPointSize}))
        return false;

    std::swap(m_primary, loaded.m_primary);
    std::swap(m_fallbacks, loaded.m_fallbacks);
    return true;
}

void SdlTtfFontSet::Close()
{
    for (TTF_Font*& font : m_primary)
        CloseTtfFont(font);
    for (RoleFonts& fallbacks : m_fallbacks)
    {
        for (TTF_Font*& font : fallbacks)
            CloseTtfFont(font);
    }
}

TTF_Font* SdlTtfFontSet::Get(SdlTtfFontRole role) const
{
    return m_primary[RoleIndex(role)];
}

bool SdlTtfFontSet::Open(std::string_view configuredFamily, const RoleSizes& pointSizes)
{
    const BundledFont& family = ResolveBundledFont(configuredFamily);
    const char* const rolePaths[kRoleCount] = {family.regular, family.bold, family.bold, kBundledFixedFont.regular};
    for (std::size_t role = 0; role < kRoleCount; ++role)
    {
        const bool fixed = role == RoleIndex(SdlTtfFontRole::Fixed);
        const std::string_view familyName = fixed ? kBundledFixedFont.family : family.family;
        m_primary[role] = OpenTtfFontRole(familyName, RoleName(role), rolePaths[role], pointSizes[role]);
        if (!m_primary[role])
            return false;

        const int lift = MeasureLayoutLift(m_primary[role], rolePaths[role], role);
        SDL_SetNumberProperty(TTF_GetFontProperties(m_primary[role]), kLayoutLiftProperty, lift);
    }

    // SDL_ttf tries the fallbacks in the order they were attached.
    for (std::size_t i = 0; i < std::size(kBundledFallbackFonts); ++i)
    {
        if (!OpenFallbacks(kBundledFallbackFonts[i], rolePaths, m_fallbacks[i]))
            return false;
    }
    return true;
}

bool SdlTtfFontSet::OpenFallbacks(const BundledFont& fallbackFont, const char* const (&rolePaths)[kRoleCount],
                                  RoleFonts& fallbacks)
{
    for (std::size_t role = 0; role < kRoleCount; ++role)
    {
        // The role font itself can never supply a glyph it lacks.
        if (std::string_view(FallbackPath(fallbackFont, role)) == rolePaths[role])
            continue;

        fallbacks[role] = OpenTtfFallbackRole(fallbackFont, role, m_primary[role]);
        if (!fallbacks[role] || !AttachTtfFallback(m_primary[role], fallbacks[role], RoleName(role)))
            return false;
    }
    return true;
}

int LayoutLift(TTF_Font* font)
{
    if (font == nullptr)
        return 0;
    return static_cast<int>(SDL_GetNumberProperty(TTF_GetFontProperties(font), kLayoutLiftProperty, 0));
}
} // namespace Render::Text
