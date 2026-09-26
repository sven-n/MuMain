#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <doctest.h>

#include "Core/Platform/BundledFonts.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
constexpr float kPointSize = 16.0f;
constexpr Uint32 kFirstPrintableCodepoint = 0x20;

// Noto Sans TC has the Traditional Chinese forms only; 88 Japanese kanji
// (乗, 価, 値, ...) need Noto Sans JP.
constexpr std::string_view kLocalesWithKnownGaps[] = {"ja"};

struct XmlEntity
{
    std::string_view name;
    char value;
};

constexpr XmlEntity kXmlEntities[] = {
    {"&amp;", '&'}, {"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '"'}, {"&apos;", '\''},
};

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string DecodeXmlEntities(std::string_view text)
{
    std::string decoded;
    decoded.reserve(text.size());
    for (std::size_t i = 0; i < text.size();)
    {
        bool replaced = false;
        for (const XmlEntity& entity : kXmlEntities)
        {
            if (text.substr(i, entity.name.size()) == entity.name)
            {
                decoded.push_back(entity.value);
                i += entity.name.size();
                replaced = true;
                break;
            }
        }
        if (!replaced)
            decoded.push_back(text[i++]);
    }
    return decoded;
}

// The text of every <value> element of a .resx file.
std::vector<std::string> ReadResxValues(const std::filesystem::path& path)
{
    constexpr std::string_view kOpen = "<value>";
    constexpr std::string_view kClose = "</value>";
    const std::string xml = ReadFile(path);
    std::vector<std::string> values;
    for (std::size_t open = xml.find(kOpen); open != std::string::npos; open = xml.find(kOpen, open))
    {
        const std::size_t start = open + kOpen.size();
        const std::size_t close = xml.find(kClose, start);
        if (close == std::string::npos)
            break;
        values.push_back(DecodeXmlEntities(std::string_view(xml).substr(start, close - start)));
        open = close;
    }
    return values;
}

// "Game.zh-TW.resx" -> "zh-TW"
std::string LocaleOf(const std::filesystem::path& path)
{
    const std::string stem = path.stem().string();
    return stem.substr(stem.find('.') + 1);
}

bool HasKnownGaps(std::string_view locale)
{
    for (std::string_view known : kLocalesWithKnownGaps)
    {
        if (locale == known)
            return true;
    }
    return false;
}

std::set<Uint32> MissingCodepoints(TTF_Font* font, const std::vector<std::string>& values)
{
    std::set<Uint32> missing;
    for (const std::string& value : values)
    {
        const char* text = value.c_str();
        size_t length = value.size();
        while (length > 0)
        {
            const Uint32 codepoint = SDL_StepUTF8(&text, &length);
            if (codepoint >= kFirstPrintableCodepoint && !TTF_FontHasGlyph(font, codepoint))
                missing.insert(codepoint);
        }
    }
    return missing;
}

std::string FormatCodepoints(const std::set<Uint32>& codepoints)
{
    std::ostringstream text;
    text << std::hex << std::uppercase;
    for (Uint32 codepoint : codepoints)
        text << "U+" << codepoint << ' ';
    return text.str();
}

// One selectable UI family with the bundled fallbacks attached, the way the
// renderer loads a role.
struct FontWithFallbacks
{
    FontWithFallbacks(const char* primaryPath, bool bold)
    {
        const std::filesystem::path assets = MU_TEST_ASSET_SOURCE;
        primary = TTF_OpenFont((assets / primaryPath).string().c_str(), kPointSize);
        for (const BundledFont& fallbackFont : kBundledFallbackFonts)
        {
            const char* fallbackPath = bold ? fallbackFont.bold : fallbackFont.regular;
            TTF_Font* fallback = TTF_OpenFont((assets / fallbackPath).string().c_str(), kPointSize);
            if (fallback != nullptr && primary != nullptr && TTF_AddFallbackFont(primary, fallback))
                fallbacks.push_back(fallback);
            else
                allAttached = false;
        }
    }
    ~FontWithFallbacks()
    {
        if (primary != nullptr)
            TTF_CloseFont(primary);
        for (TTF_Font* fallback : fallbacks)
            TTF_CloseFont(fallback);
    }

    TTF_Font* primary = nullptr;
    std::vector<TTF_Font*> fallbacks;
    bool allAttached = true;
};

void CheckTranslationsDrawableWith(const char* primaryPath, bool bold)
{
    FontWithFallbacks font(primaryPath, bold);
    REQUIRE(font.primary != nullptr);
    REQUIRE(font.allAttached);

    for (const auto& entry : std::filesystem::directory_iterator(MU_TEST_LOCALIZATION_DIR))
    {
        if (entry.path().extension() != ".resx" || HasKnownGaps(LocaleOf(entry.path())))
            continue;

        const std::set<Uint32> missing = MissingCodepoints(font.primary, ReadResxValues(entry.path()));
        CAPTURE(primaryPath);
        CAPTURE(entry.path().filename().string());
        CHECK_MESSAGE(missing.empty(), "no glyph for " << FormatCodepoints(missing));
    }
}
} // namespace

TEST_CASE("every translation can be drawn with each bundled UI font [platform][bundled_font]")
{
    REQUIRE(TTF_Init());
    for (const BundledFont& family : kBundledFonts)
    {
        CheckTranslationsDrawableWith(family.regular, false);
        CheckTranslationsDrawableWith(family.bold, true);
    }
    CheckTranslationsDrawableWith(kBundledFixedFont.regular, false);
    TTF_Quit();
}
