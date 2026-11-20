#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct BuffEntry
{
    Rml::String decorator;
    float progress = 0.f;
    bool hasTimer = false;
};

struct BuffStripModel
{
    std::vector<BuffEntry> buffs;
};

void RegisterBuffStripModel(Rml::DataModelConstructor& constructor, BuffStripModel& model)
{
    auto entry = constructor.RegisterStruct<BuffEntry>();
    entry.RegisterMember("decorator", &BuffEntry::decorator);
    entry.RegisterMember("progress", &BuffEntry::progress);
    entry.RegisterMember("has_timer", &BuffEntry::hasTimer);
    constructor.RegisterArray<std::vector<BuffEntry>>();
    constructor.Bind("buffs", &model.buffs);
}

std::string ReadText(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    REQUIRE(file.is_open());
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

// The real document, with its theme-relative stylesheet links made absolute: a document loaded
// from memory has no URL of its own for them to resolve against.
std::string ThemeMarkup(const std::string& theme)
{
    const std::filesystem::path root = MU_RMLUI_DIR;
    const std::filesystem::path themeDirectory = root / "themes" / theme;
    std::string markup = ReadText(root / "buff_strip.rml");
    for (const char* name : {"base.rcss", "buff_strip.rcss"})
    {
        const std::string from = std::string("href=\"") + name + "\"";
        const std::string to = "href=\"" + (themeDirectory / name).generic_string() + "\"";
        const size_t at = markup.find(from);
        REQUIRE(at != std::string::npos);
        markup.replace(at, from.size(), to);
    }
    return markup;
}

void CheckTheme(const char* theme)
{
    CAPTURE(std::string(theme));
    UI::Tests::RmlLayoutFixture fixture;

    RmlModelBinder<BuffStripModel> binder;
    REQUIRE(binder.Create(fixture.context, "buff_strip", RegisterBuffStripModel));
    binder.GetModel().buffs = {{"", 1.f, true}, {"", 0.5f, true}, {"", 0.f, false}};
    binder.MarkDirty("buffs");

    Rml::ElementDocument* document = fixture.LoadMarkup(ThemeMarkup(theme));
    Rml::ElementList fills;
    Rml::ElementList tracks;
    document->QuerySelectorAll(fills, ".buff-time-fill");
    document->QuerySelectorAll(tracks, ".buff-time-track");
    REQUIRE(fills.size() == 3);
    REQUIRE(tracks.size() == 3);

    // The fill is a share of the track: full at 1.0, half at 0.5, and the untracked buff's track
    // is display:none through the .hidden class the model's has_timer drives.
    const float trackWidth = tracks[0]->GetOffsetWidth();
    REQUIRE(trackWidth > 0.f);
    CHECK(fills[0]->GetOffsetWidth() == doctest::Approx(trackWidth));
    CHECK(fills[1]->GetOffsetWidth() == doctest::Approx(trackWidth * 0.5f).epsilon(0.02f));
    CHECK(fills[1]->GetOffsetHeight() == doctest::Approx(tracks[0]->GetOffsetHeight()));
    CHECK(fills[1]->GetComputedValues().background_color().alpha > 0);
    CHECK(tracks[2]->GetComputedValues().display() == Rml::Style::Display::None);
    CHECK(fills[2]->GetOffsetWidth() == 0.f);

    binder.Destroy(fixture.context);
}
} // namespace

// The strip's duration bar is the model's remaining share: full at 1.0, half at 0.5, and absent
// for a buff whose duration the client does not track. Both bundled themes must style it.
TEST_CASE("the buff strip's duration bar follows the model [ui][rml-buff-strip]")
{
    CheckTheme("legacy");
    CheckTheme("modern");
}
