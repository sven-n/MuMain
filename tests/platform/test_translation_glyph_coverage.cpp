#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <doctest.h>

#include "Core/Platform/BundledFonts.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Text/SdlTtfFontSet.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using Render::Text::SdlTtfFontRole;
using Render::Text::SdlTtfFontSet;

namespace
{
constexpr float kPointSize = 16.0f;
constexpr Uint32 kFirstPrintableCodepoint = 0x20;
constexpr SdlTtfFontRole kRoles[] = {SdlTtfFontRole::Normal, SdlTtfFontRole::Bold, SdlTtfFontRole::Big,
                                     SdlTtfFontRole::Fixed};

// Noto Sans TC has the Traditional Chinese forms only; 88 Japanese kanji
// (乗, 価, 値, ...) need Noto Sans JP.
constexpr std::string_view kLocalesWithKnownGaps[] = {"ja"};

constexpr Uint32 kFullwidthLatinH = 0xFF28; // Ｈ: on the baseline, only in Noto Sans TC
constexpr const char* kLatinAndFullwidthH = "H\xEF\xBC\xA8";
constexpr Uint8 kInkAlpha = 32; // counts the thin anti-aliased stems of small glyphs

struct XmlEntity
{
    std::string_view name;
    char value;
};

constexpr XmlEntity kXmlEntities[] = {
    {"&amp;", '&'}, {"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '"'}, {"&apos;", '\''},
};

// Runs the font set where the client runs: fonts/ resolves against the
// working directory.
class AssetDirectoryScope
{
public:
    explicit AssetDirectoryScope(const std::filesystem::path& directory) : m_previous(std::filesystem::current_path())
    {
        mu::log::Init();
        std::filesystem::current_path(directory);
        m_initialized = TTF_Init();
    }
    ~AssetDirectoryScope()
    {
        if (m_initialized)
            TTF_Quit();
        std::filesystem::current_path(m_previous);
    }
    AssetDirectoryScope(const AssetDirectoryScope&) = delete;
    AssetDirectoryScope& operator=(const AssetDirectoryScope&) = delete;

    [[nodiscard]] bool Initialized() const
    {
        return m_initialized;
    }

private:
    std::filesystem::path m_previous;
    bool m_initialized = false;
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

// The code points of every <value> element of a .resx file.
void AddResxCodepoints(const std::filesystem::path& path, std::set<Uint32>& codepoints)
{
    constexpr std::string_view kOpen = "<value>";
    constexpr std::string_view kClose = "</value>";
    const std::string xml = ReadFile(path);
    for (std::size_t open = xml.find(kOpen); open != std::string::npos; open = xml.find(kOpen, open))
    {
        const std::size_t start = open + kOpen.size();
        const std::size_t close = xml.find(kClose, start);
        if (close == std::string::npos)
            break;

        const std::string value = DecodeXmlEntities(std::string_view(xml).substr(start, close - start));
        const char* text = value.c_str();
        size_t length = value.size();
        while (length > 0)
        {
            const Uint32 codepoint = SDL_StepUTF8(&text, &length);
            if (codepoint >= kFirstPrintableCodepoint)
                codepoints.insert(codepoint);
        }
        open = close;
    }
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

std::map<std::string, std::set<Uint32>> TranslationCodepointsByLocale()
{
    std::map<std::string, std::set<Uint32>> codepoints;
    for (const auto& entry : std::filesystem::directory_iterator(MU_TEST_LOCALIZATION_DIR))
    {
        const std::string locale = LocaleOf(entry.path());
        if (entry.path().extension() == ".resx" && !HasKnownGaps(locale))
            AddResxCodepoints(entry.path(), codepoints[locale]);
    }
    return codepoints;
}

std::string MissingCodepoints(TTF_Font* font, const std::set<Uint32>& codepoints)
{
    std::ostringstream missing;
    missing << std::hex << std::uppercase;
    for (Uint32 codepoint : codepoints)
    {
        if (!TTF_FontHasGlyph(font, codepoint))
            missing << "U+" << codepoint << ' ';
    }
    return missing.str();
}

// The lowest row with ink in the columns [firstColumn, endColumn).
int LowestInkRow(SDL_Surface* surface, int firstColumn, int endColumn)
{
    for (int y = surface->h - 1; y >= 0; --y)
    {
        for (int x = firstColumn; x < endColumn; ++x)
        {
            Uint8 red = 0, green = 0, blue = 0, alpha = 0;
            SDL_ReadSurfacePixel(surface, x, y, &red, &green, &blue, &alpha);
            if (alpha >= kInkAlpha)
                return y;
        }
    }
    return -1;
}
} // namespace

TEST_CASE("every translation can be drawn with each bundled UI font [platform][bundled_font]")
{
    AssetDirectoryScope assets(MU_TEST_ASSET_SOURCE);
    REQUIRE(assets.Initialized());
    const auto codepointsByLocale = TranslationCodepointsByLocale();
    REQUIRE_FALSE(codepointsByLocale.empty());

    for (const BundledFont& family : kBundledFonts)
    {
        SdlTtfFontSet fonts;
        REQUIRE(fonts.Load(family.family, kPointSize, kPointSize, kPointSize));
        for (SdlTtfFontRole role : kRoles)
        {
            for (const auto& [locale, codepoints] : codepointsByLocale)
            {
                CAPTURE(family.family);
                CAPTURE(static_cast<int>(role));
                CAPTURE(locale);
                const std::string missing = MissingCodepoints(fonts.Get(role), codepoints);
                CHECK_MESSAGE(missing.empty(), "no glyph for " << missing);
            }
        }
    }
}

TEST_CASE("fallback glyphs sit on the role font's baseline [platform][bundled_font]")
{
    AssetDirectoryScope assets(MU_TEST_ASSET_SOURCE);
    REQUIRE(assets.Initialized());
    for (float pointSize : {11.0f, 13.0f, 16.0f, 24.0f})
    {
        SdlTtfFontSet fonts;
        REQUIRE(fonts.Load("DejaVu Sans", pointSize, pointSize, pointSize));
        for (SdlTtfFontRole role : kRoles)
        {
            CAPTURE(pointSize);
            CAPTURE(static_cast<int>(role));
            TTF_Font* font = fonts.Get(role);
            REQUIRE(TTF_FontHasGlyph(font, kFullwidthLatinH));

            // "H" comes from the role font, the fullwidth "Ｈ" from Noto Sans TC.
            int latinWidth = 0;
            REQUIRE(TTF_GetStringSize(font, "H", 0, &latinWidth, nullptr));
            SDL_Surface* surface = TTF_RenderText_Blended(font, kLatinAndFullwidthH, 0, SDL_Color{255, 255, 255, 255});
            REQUIRE(surface != nullptr);
            CHECK(LowestInkRow(surface, latinWidth, surface->w) == LowestInkRow(surface, 0, latinWidth));
            SDL_DestroySurface(surface);
        }
    }
}

TEST_CASE("only a family taller than DejaVu Sans lifts its text [platform][bundled_font]")
{
    AssetDirectoryScope assets(MU_TEST_ASSET_SOURCE);
    REQUIRE(assets.Initialized());
    constexpr float kBigPointSize = 32.0f;

    SdlTtfFontSet dejaVu;
    REQUIRE(dejaVu.Load("DejaVu Sans", kPointSize, kBigPointSize, kPointSize));
    SdlTtfFontSet liberation;
    REQUIRE(liberation.Load("Liberation Sans", kPointSize, kBigPointSize, kPointSize));
    SdlTtfFontSet noto;
    REQUIRE(noto.Load("Noto Sans TC", kPointSize, kBigPointSize, kPointSize));

    for (SdlTtfFontRole role : kRoles)
    {
        CAPTURE(static_cast<int>(role));
        CHECK(Render::Text::LayoutLift(dejaVu.Get(role)) == 0);
        CHECK(Render::Text::LayoutLift(liberation.Get(role)) == 0);
    }
    // Noto Sans TC's ascent is 1.16 em against DejaVu Sans' 0.93 em.
    CHECK(Render::Text::LayoutLift(noto.Get(SdlTtfFontRole::Normal)) == 4);
    CHECK(Render::Text::LayoutLift(noto.Get(SdlTtfFontRole::Big)) == 8);
    CHECK(Render::Text::LayoutLift(noto.Get(SdlTtfFontRole::Fixed)) == 0); // Cousine
}

TEST_CASE("a failed reload keeps the current fonts [platform][bundled_font]")
{
    AssetDirectoryScope assets(MU_TEST_ASSET_SOURCE);
    REQUIRE(assets.Initialized());
    SdlTtfFontSet fonts;
    REQUIRE(fonts.Load("DejaVu Sans", kPointSize, kPointSize, kPointSize));
    TTF_Font* loaded = fonts.Get(SdlTtfFontRole::Normal);

    // No fonts/ folder here, so no bundled font can be opened.
    const std::filesystem::path empty = std::filesystem::temp_directory_path() / "mu_font_set_without_fonts";
    std::filesystem::create_directories(empty);
    std::filesystem::current_path(empty);
    CHECK_FALSE(fonts.Load("DejaVu Sans", kPointSize, kPointSize, kPointSize));
    CHECK(fonts.Get(SdlTtfFontRole::Normal) == loaded);
    CHECK(TTF_FontHasGlyph(loaded, 0x4E2D)); // 中, still from the attached fallback
}
