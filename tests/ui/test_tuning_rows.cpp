#include "stdafx.h"

#include "doctest.h"

#include "Render/Look/TuningControls.h"
#include "Render/Look/TuningSession.h"
#include "UI/Options/TuningRows.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <set>
#include <string>
#include <vector>

using Render::Look::Control;
using Render::Look::TuningControlId;
using Render::Look::TuningGroup;
using UI::Options::TuningLock;
using UI::Options::TuningRowState;

namespace
{
constexpr int GroupCount = static_cast<int>(Render::Look::TuningGroupCount);

std::string ReadFile(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    REQUIRE_MESSAGE(stream.good(), file.generic_string());
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

std::size_t Count(const std::string& text, const std::string& needle)
{
    std::size_t count = 0;
    for (std::size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
    {
        ++count;
    }
    return count;
}

// The opening tag that carries `attribute`.
std::vector<std::string> TagsWith(const std::string& markup, const std::string& attribute)
{
    std::vector<std::string> tags;
    const std::regex tag("<[a-z]+[^>]*>");
    for (auto it = std::sregex_iterator(markup.begin(), markup.end(), tag); it != std::sregex_iterator(); ++it)
    {
        if (it->str().find(attribute) != std::string::npos)
        {
            tags.push_back(it->str());
        }
    }
    return tags;
}

const std::filesystem::path Themes[] = {
    std::filesystem::path(MU_RMLUI_DIR) / "option_window.rml",
    std::filesystem::path(MU_RMLUI_DIR) / "themes/modern/option_window.rml",
};
} // namespace

TEST_CASE("every tuning control has a localized row with bounds, value, unit and state [ui][options][tuning]")
{
    std::set<std::string> labels;
    for (const Render::Look::TuningControl& control : Render::Look::TuningControls())
    {
        CAPTURE(static_cast<int>(control.id));
        const UI::Options::TuningRow row = UI::Options::BuildTuningRow(control, control.max, TuningRowState::Custom);
        CHECK_FALSE(row.label.empty());
        CHECK_FALSE(row.help.empty());
        CHECK(row.help != row.label);
        CHECK_FALSE(row.minText.empty());
        CHECK(row.maxText == row.valueText);
        CHECK(row.fillPercent == doctest::Approx(100.f));
        CHECK(row.stateText == "Custom");
        labels.insert(row.label);
        const UI::Options::TuningRow inherited =
            UI::Options::BuildTuningRow(control, control.min, TuningRowState::Inherited);
        CHECK(inherited.fillPercent == 0.f);
        CHECK(inherited.stateText == "Defaults");
    }
    CHECK(labels.size() == Render::Look::TuningControlCount);
}

TEST_CASE("tuning rows show units, No cap and Immediate at their zero [ui][options][tuning]")
{
    using UI::Options::FormatTuningText;
    CHECK(FormatTuningText(Control(TuningControlId::EffectGain), 1.5) == "1.5x");
    CHECK(FormatTuningText(Control(TuningControlId::EffectRange), 2.0) == "2.0x");
    CHECK(FormatTuningText(Control(TuningControlId::EffectFade), 0.5) == "0.50 s");
    CHECK(FormatTuningText(Control(TuningControlId::EffectRangeMax), 8.0) == "8 cells");
    CHECK(FormatTuningText(Control(TuningControlId::MoodStrength), 100.0) == "100 %");
    CHECK(FormatTuningText(Control(TuningControlId::GradingStrength), 0.0) == "0 %");
    // Zero overlap cap and zero maximum radius are No cap, not Off; zero fade/smoothing is Immediate.
    CHECK(FormatTuningText(Control(TuningControlId::EffectCap), 0.0) == "No cap");
    CHECK(FormatTuningText(Control(TuningControlId::EffectRangeMax), 0.0) == "No cap");
    CHECK(FormatTuningText(Control(TuningControlId::EffectFade), 0.0) == "Immediate");
    CHECK(FormatTuningText(Control(TuningControlId::MapSmoothing), 0.0) == "Immediate");
    // Zero skill intensity is a number: the light is gone, the sprites are not.
    CHECK(FormatTuningText(Control(TuningControlId::EffectGain), 0.0) == "0.0x");
    // The help says what each zero means.
    const auto help = [](TuningControlId id)
    { return UI::Options::BuildTuningRow(Control(id), 0.0, TuningRowState::Inherited).help; };
    CHECK(help(TuningControlId::EffectCap).find("No cap") != std::string::npos);
    CHECK(help(TuningControlId::EffectRangeMax).find("No cap") != std::string::npos);
    CHECK(help(TuningControlId::EffectGain).find("sprites") != std::string::npos);
    CHECK(help(TuningControlId::EffectRange).find("Maximum skill radius") != std::string::npos);
    CHECK(help(TuningControlId::GradingStrength).find("bloom stays on") != std::string::npos);
}

TEST_CASE("a row distinguishes defaults, global custom tuning and unsaved changes [ui][options][tuning]")
{
    Render::Look::TuningSession session({[](const Data::Config::PersonalTuning&, const Data::Config::PersonalTuning&)
                                         { return Data::Config::PersonalTuningSaveResult{false, "read-only"}; },
                                         {},
                                         {}},
                                        {});
    CHECK(UI::Options::TuningStateOf(session, TuningControlId::BloomStrength) == TuningRowState::Inherited);
    CHECK(UI::Options::TuningSaveMessage(session).empty());
    session.Preview(TuningControlId::BloomStrength, 0.5);
    CHECK(UI::Options::TuningStateOf(session, TuningControlId::BloomStrength) == TuningRowState::Unsaved);
    CHECK_FALSE(session.Commit());
    CHECK(UI::Options::TuningStateOf(session, TuningControlId::BloomStrength) == TuningRowState::Unsaved);
    const std::string message = UI::Options::TuningSaveMessage(session);
    CHECK(message.find("not saved") != std::string::npos);
    CHECK(message.find("read-only") != std::string::npos);

    Data::Config::PersonalTuning persisted;
    persisted.look[Data::Maps::LookValue::BloomStrength] = 0.5;
    const Render::Look::TuningSession saved({}, persisted);
    CHECK(UI::Options::TuningStateOf(saved, TuningControlId::BloomStrength) == TuningRowState::Custom);
}

TEST_CASE("a tuning group is locked with its reason and dynamic lights need Enhanced lighting [ui][options][tuning]")
{
    using UI::Options::TuningGroupLock;
    const UI::Options::TuningFeatureState on;
    CHECK(TuningGroupLock(TuningGroup::Lighting, true, true, on) == TuningLock::None);
    CHECK(TuningGroupLock(TuningGroup::Lighting, false, true, on) == TuningLock::ClassicLook);
    CHECK(TuningGroupLock(TuningGroup::DynamicLights, true, false, on) == TuningLock::NeedsLighting);
    CHECK(TuningGroupLock(TuningGroup::LightMood, true, false, on) == TuningLock::None);
    UI::Options::TuningFeatureState off;
    off.chosen = false;
    CHECK(TuningGroupLock(TuningGroup::PostProcessing, true, true, off) == TuningLock::FeatureOff);
    UI::Options::TuningFeatureState unavailable;
    unavailable.active = false;
    CHECK(TuningGroupLock(TuningGroup::PostProcessing, true, true, unavailable) == TuningLock::Unavailable);
    UI::Options::TuningFeatureState session;
    session.sessionOff = true;
    CHECK(TuningGroupLock(TuningGroup::Lighting, true, true, session) == TuningLock::SessionOff);
    UI::Options::TuningFeatureState sessionValue;
    sessionValue.sessionSet = true;
    CHECK(TuningGroupLock(TuningGroup::LightMood, true, true, sessionValue) == TuningLock::SessionSet);
    for (const TuningLock lock : {TuningLock::ClassicLook, TuningLock::FeatureOff, TuningLock::NeedsLighting,
                                  TuningLock::Unavailable, TuningLock::SessionOff, TuningLock::SessionSet})
    {
        CHECK_FALSE(UI::Options::TuningLockReason(lock).empty());
    }
    CHECK(UI::Options::TuningLockReason(TuningLock::None).empty());
    CHECK(UI::Options::TuningLockReason(TuningLock::NeedsLighting).find("Enhanced lighting") != std::string::npos);
}

TEST_CASE("both option themes bind the same tuning groups, rows, sliders and resets [ui][options][tuning]")
{
    for (const std::filesystem::path& file : Themes)
    {
        CAPTURE(file.generic_string());
        const std::string markup = ReadFile(file);
        for (int group = 0; group < GroupCount; ++group)
        {
            CAPTURE(group);
            const std::string g = std::to_string(group);
            // The label (with its chevron) expands; the checkbox alone switches the feature.
            const std::vector<std::string> expanders = TagsWith(markup, "option_toggle_tuning_group(" + g + ")");
            REQUIRE(expanders.size() == 1);
            CHECK(expanders[0].find("option_toggle_enhanced_feature") == std::string::npos);
            const std::vector<std::string> switches = TagsWith(markup, "option_toggle_enhanced_feature(" + g + ")");
            REQUIRE(switches.size() == 1);
            CHECK(switches[0].find("option_toggle_tuning_group") == std::string::npos);
            CHECK(switches[0].find("row") == std::string::npos); // on the checkbox, not the whole row
            CHECK(Count(markup, "data-class-expanded=\"tuning_expanded_" + g + "\"") == 1);
            CHECK(Count(markup, "data-class-hidden=\"!tuning_expanded_" + g + "\"") == 1);
            CHECK(Count(markup, "data-class-locked=\"tuning_locked_" + g + "\"") == 1);
            CHECK(Count(markup, "{{tuning_reason_" + g + "}}") == 1);
            CHECK(Count(markup, "data-for=\"row : tuning_rows_" + g + "\"") == 1);
            CHECK(Count(markup, "option_reset_tuning_group(" + g + ")") == 1);
            // "More settings" only where the group has specialist rows, expanded on its own.
            bool specialist = false;
            for (const Render::Look::TuningControl& control : Render::Look::TuningControls())
            {
                specialist = specialist || (control.specialist && static_cast<int>(control.group) == group);
            }
            CHECK(Count(markup, "data-for=\"row : tuning_more_rows_" + g + "\"") == (specialist ? 1u : 0u));
            CHECK(Count(markup, "option_toggle_tuning_more(" + g + ")") == (specialist ? 1u : 0u));
            CHECK(Count(markup, "data-class-hidden=\"!tuning_more_expanded_" + g + "\"") == (specialist ? 1u : 0u));
        }
        // One slider template per row list: pointer, drag, keys and every way an interaction ends.
        const std::size_t lists = Count(markup, "data-for=\"row : tuning_");
        CHECK(lists == 6);
        for (const char* binding :
             {"data-event-mousedown=\"option_tuning_pointer(row.control)\"",
              "data-event-drag=\"option_tuning_pointer(row.control)\"",
              "data-event-keydown=\"option_tuning_key(row.control)\"", "data-event-mouseup=\"option_tuning_commit\"",
              "data-event-dragend=\"option_tuning_commit\"", "data-event-keyup=\"option_tuning_commit\"",
              "data-event-blur=\"option_tuning_commit\"", "option_reset_tuning(row.control)", "{{row.label}}",
              "{{row.value_text}}", "{{row.min_text}}", "{{row.max_text}}", "{{row.state_text}}", "row.help",
              "row.fill_percent"})
        {
            CHECK_MESSAGE(Count(markup, binding) == lists, binding);
        }
        CHECK(Count(markup, "option_reset_all_tuning") == 1);
        CHECK(Count(markup, "{{tuning_save_message}}") == 1);
    }
}
