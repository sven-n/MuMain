// Click sounds by style (sound editor design, Q-UI1): the themes' click-sound
// kinds, the kind of a click, and the context's listener.
#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "UI/RmlBridge/RmlClickSounds.h"

#include <RmlUi/Core.h>

#include <string>
#include <vector>

using UI::RmlBridge::ClickKindOf;

namespace
{
// A listener that stops the click, as a button's own handler may.
class StopsClicks final : public Rml::EventListener
{
public:
    void ProcessEvent(Rml::Event& event) override
    {
        event.StopPropagation();
    }
};
} // namespace

TEST_CASE("Click sounds: every theme gives the login, server list and character select buttons their click kinds "
          "(Q-UI1) [ui][rml-click-sounds]")
{
    UI::Tests::RmlLayoutFixture fixture;
    UI::RmlBridge::RegisterClickSoundProperty();
    // A second call keeps the one property.
    UI::RmlBridge::RegisterClickSoundProperty();
    for (const std::string theme : {"legacy", "classic", "modern"})
    {
        CAPTURE(theme);
        Rml::ElementDocument* login = fixture.Load(theme, "login");
        CHECK(ClickKindOf(login->QuerySelector(".btn-ok")) == "confirm");
        CHECK(ClickKindOf(login->QuerySelector(".btn-cancel")) == "cancel");
        // The rest of the window makes no sound.
        CHECK(ClickKindOf(login->QuerySelector(".checkbox-remember-me")).empty());
        login->Close();

        Rml::ElementDocument* servers = fixture.Load(theme, "server_select");
        Rml::Element* group = servers->QuerySelector(".server-group-btn");
        REQUIRE(group != nullptr);
        CHECK(ClickKindOf(group) == "select");
        // A click on the label inside the button is the button's.
        CHECK(ClickKindOf(group->QuerySelector(".button-label")) == "select");
        CHECK(ClickKindOf(servers->QuerySelector(".server-option-btn")) == "confirm");
        servers->Close();

        Rml::ElementDocument* characters = fixture.Load(theme, "char_sel_main");
        for (const char* id : {"btn_create", "btn_menu", "btn_delete"})
        {
            CAPTURE(id);
            CHECK(ClickKindOf(characters->GetElementById(id)) == "menu");
        }
        CHECK(ClickKindOf(characters->GetElementById("btn_connect")) == "confirm");
        characters->Close();
        fixture.Refresh();
    }
}

TEST_CASE("Click sounds: a click's kind is its element's or the nearest parent's, and none and a disabled button play "
          "nothing (Q-UI1) [ui][rml-click-sounds]")
{
    UI::Tests::RmlLayoutFixture fixture;
    UI::RmlBridge::RegisterClickSoundProperty();
    Rml::ElementDocument* document = fixture.LoadMarkup(R"(<rml><body>
        <div id="button" style="click-sound: menu;"><span id="label">Menu</span>
            <div id="quiet" style="click-sound: none;"><span id="inQuiet">x</span></div>
            <div id="off" class="disabled"><span id="inOff">x</span></div>
            <div id="own" style="click-sound: confirm;">x</div>
        </div>
        <div id="plain">x</div>
        </body></rml>)");
    CHECK(ClickKindOf(document->GetElementById("button")) == "menu");
    CHECK(ClickKindOf(document->GetElementById("label")) == "menu");
    CHECK(ClickKindOf(document->GetElementById("own")) == "confirm");
    CHECK(ClickKindOf(document->GetElementById("quiet")).empty());
    CHECK(ClickKindOf(document->GetElementById("inQuiet")).empty());
    CHECK(ClickKindOf(document->GetElementById("inOff")).empty());
    CHECK(ClickKindOf(document->GetElementById("plain")).empty());
    CHECK(ClickKindOf(nullptr).empty());
    document->GetElementById("button")->SetClass("disabled", true);
    CHECK(ClickKindOf(document->GetElementById("label")).empty());
}

TEST_CASE("Click sounds: the context's listener hears a click before the button's handler, also one that stops it "
          "(Q-UI1) [ui][rml-click-sounds]")
{
    UI::Tests::RmlLayoutFixture fixture;
    UI::RmlBridge::RegisterClickSoundProperty();
    std::vector<std::string> played;
    UI::RmlBridge::ClickSoundListener listener;
    listener.play = [&played](std::string_view kind) { played.emplace_back(kind); };
    fixture.context->AddEventListener("click", &listener, true);
    Rml::ElementDocument* document = fixture.LoadMarkup(R"(<rml><body>
        <div id="ok" style="click-sound: confirm;"><span id="label">OK</span></div>
        <div id="plain">x</div>
        </body></rml>)");
    StopsClicks stops;
    document->GetElementById("label")->AddEventListener("click", &stops);
    document->GetElementById("label")->Click();
    document->GetElementById("plain")->Click();
    CHECK(played == std::vector<std::string>{"confirm"});
    document->GetElementById("label")->RemoveEventListener("click", &stops);
    fixture.context->RemoveEventListener("click", &listener, true);
}
