#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/ThemeFileInterface.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlNativeText.h"
#include "UI/RmlBridge/RmlScaleInputs.h"
#include "Data/GameConfig/GameConfig.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Options/OptionWindow.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Character/CharMakeWin.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Factory.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

extern bool g_bWndActive;

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override { return 0; }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override { return 0; }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 0; }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

struct Fixture
{
    Fixture()
    {
        root = std::filesystem::temp_directory_path() /
            ("rml_theme_file_interface_tests_" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Rml::SetFileInterface(&files);
        Rml::SetRenderInterface(&renderer);
        REQUIRE(Rml::Initialise());
        UI::RmlBridge::ClearThemeFontCache();
        context = Rml::CreateContext("theme-file-interface", {320, 240});
        REQUIRE(context != nullptr);
    }

    ~Fixture()
    {
        GameConfig::GetInstance().SetUIScalePercent(previousPercent);
        UI::RmlBridge::SetContextResolver({});
        if (!previousDirectory.empty())
        {
            std::filesystem::current_path(previousDirectory);
            UI::RmlBridge::SetActiveThemeName(previousTheme);
        }
        Rml::Shutdown();
        UI::RmlBridge::ClearThemeFontCache();
        UI::RmlBridge::SetSelectedFontFamily({});
        Rml::SetFileInterface(nullptr);
        Rml::SetRenderInterface(nullptr);
        std::filesystem::remove_all(root);
    }

    void ActivateRoot()
    {
        previousDirectory = std::filesystem::current_path();
        previousTheme = UI::RmlBridge::GetActiveThemeName();
        std::filesystem::current_path(root);
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
    }

    std::filesystem::path Theme(const char* name) const
    {
        return root / "Data/Interface/RmlUi/themes" / name;
    }

    void Write(const std::filesystem::path& path, const std::string& contents) const
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream(path, std::ios::binary) << contents;
    }

    Rml::Colourb LoadColor(const char* theme, const char* linkAttributes) const
    {
        const std::string markup = std::string("<rml><head><link ") + linkAttributes +
            "/></head><body><div id='colored'>text</div></body></rml>";
        const std::string source = (Theme(theme) / "view.rml").generic_string();
        Rml::ElementDocument* document = context->LoadDocumentFromMemory(markup, source);
        REQUIRE(document != nullptr);
        document->Show();
        context->Update();
        Rml::Element* colored = document->GetElementById("colored");
        REQUIRE(colored != nullptr);
        const Rml::Colourb result = colored->GetComputedValues().color();
        context->UnloadDocument(document);
        return result;
    }

    Rml::ElementDocument* LoadLegacyVariant(const char* theme, const char* name)
    {
        const std::string source = UI::RmlBridge::ThemedDocumentSourceUrl(name, theme);
        auto file = files.Open(source);
        if (!file)
            file = files.Open(std::string("Data/Interface/RmlUi/") + name);
        REQUIRE(file != 0);
        std::string markup(files.Length(file), '\0');
        files.Read(markup.data(), markup.size(), file);
        files.Close(file);
        const std::regex bindings(R"(\sdata-(?:model|for|if|class-[\w-]+|style-[\w-]+|attr-[\w-]+|event-[\w-]+)="[^"]*")");
        markup = std::regex_replace(markup, bindings, "");
        markup = std::regex_replace(markup, std::regex(R"(\{\{[^}]*\}\})"), "");
        auto* document = context->LoadDocumentFromMemory(markup, source);
        REQUIRE(document != nullptr);
        UI::RmlBridge::ApplyNativeTextSize(document);
        UI::RmlBridge::ApplyStackingDepth(document, name);
        document->Show();
        context->Update();
        return document;
    }

    std::filesystem::path root;
    std::filesystem::path previousDirectory;
    std::string previousTheme;
    int previousPercent = GameConfig::GetInstance().GetUIScalePercent();
    UI::RmlBridge::ThemeFileInterface files;
    NullRenderer renderer;
    Rml::Context* context = nullptr;
};
} // namespace

namespace
{
void CompareLegacyElements(Rml::Element& original, Rml::Element& inherited)
{
    CAPTURE(original.GetTagName());
    CAPTURE(original.GetId());
    CHECK(original.GetInnerRML() == inherited.GetInnerRML());
    CHECK(original.GetAbsoluteLeft() == doctest::Approx(inherited.GetAbsoluteLeft()));
    CHECK(original.GetAbsoluteTop() == doctest::Approx(inherited.GetAbsoluteTop()));
    CHECK(original.GetOffsetWidth() == doctest::Approx(inherited.GetOffsetWidth()));
    CHECK(original.GetOffsetHeight() == doctest::Approx(inherited.GetOffsetHeight()));
    for (const char* property : {"color", "background-color", "font-family", "font-size", "font-weight",
                                 "display", "visibility", "opacity", "decorator", "transform"})
    {
        CAPTURE(property);
        const auto* expected = original.GetProperty(property);
        const auto* actual = inherited.GetProperty(property);
        REQUIRE((expected != nullptr) == (actual != nullptr));
        if (expected)
            CHECK(expected->ToString() == actual->ToString());
    }
    REQUIRE(original.GetNumChildren() == inherited.GetNumChildren());
    for (int index = 0; index < original.GetNumChildren(); ++index)
        CompareLegacyElements(*original.GetChild(index), *inherited.GetChild(index));
}

void CompareLegacyVariant(Fixture& fixture, const char* name)
{
    UI::RmlBridge::SetActiveThemeName("legacy");
    auto* original = fixture.LoadLegacyVariant("legacy", name);
    UI::RmlBridge::SetActiveThemeName("legacy-inherited-test");
    CHECK(UI::RmlBridge::ThemeUsesNativeTextSize());
    auto* child = fixture.LoadLegacyVariant("legacy-inherited-test", name);
    CompareLegacyElements(*original, *child);
    fixture.context->UnloadAllDocuments();
    fixture.context->Update();
}

struct FontEnvironment
{
    static constexpr const char* Name = "MU_THEME_FONT_TEST_ROOT";
    std::string previous = std::getenv(Name) ? std::getenv(Name) : "";
    FontEnvironment() { Set(MU_FONT_DIR); }
    ~FontEnvironment() { Set(previous); }
    static void Set(const std::string& value)
    {
#ifdef _WIN32
        _putenv_s(Name, value.c_str());
#else
        if (value.empty())
            unsetenv(Name);
        else
            setenv(Name, value.c_str(), 1);
#endif
    }
};
}

TEST_CASE("saved theme recovery validates ancestry before choosing a fallback [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("legacy") / "base.rcss", "body { color: white; }");
    fixture.Write(fixture.Theme("child") / "theme.ini", "[Theme]\nBase=legacy\n");
    fixture.Write(fixture.Theme("broken") / "theme.ini", "[Theme]\nBase=missing\n");
    fixture.Write(fixture.Theme("cycle") / "theme.ini", "[Theme]\nBase=cycle\n");
    fixture.ActivateRoot();
    CHECK(UI::RmlBridge::ResolveStartupThemeName("CHILD") == "child");
    for (const char* name : {"missing", "broken", "cycle", "../legacy", ""})
        CHECK(UI::RmlBridge::ResolveStartupThemeName(name) == "legacy");
    std::filesystem::remove(fixture.Theme("legacy") / "base.rcss");
    CHECK(UI::RmlBridge::ResolveStartupThemeName("missing").empty());
}

TEST_CASE("focused confirmation input claims its document and accepts Enter and Escape [ui][dialog-input]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("legacy") / "base.rcss", "body { color: white; }");
    fixture.Write(fixture.root / "Data/Interface/RmlUi/generic_confirm_dialog.rml",
        "<rml><head></head><body data-model='generic_confirm_dialog'>"
        "<input id='gcd_input' type='text' data-value='input_value'/></body></rml>");
    fixture.ActivateRoot();
    UI::RmlBridge::SetActiveThemeName("legacy");
    mu::ui::window::CManager manager;
    mu::ui::window::CGenericConfirmDialog dialog;
    dialog.Create(&manager);
    int accepted = 0;
    int cancelled = 0;
    mu::ui::window::GenericDialogConfig config;
    config.showCancel = true;
    config.input = mu::ui::window::GenericDialogConfig::InputField{};
    config.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
    config.onPrimary = [&] { ++accepted; };
    config.onCancel = [&] { ++cancelled; };
    const bool wasActive = g_bWndActive;
    g_bWndActive = true;
    for (int key : {VK_RETURN, VK_ESCAPE})
    {
        dialog.Show(config);
        fixture.context->Update();
        dialog.Update();
        auto* field = fixture.context->GetFocusElement();
        REQUIRE(field != nullptr);
        CHECK(field->GetId() == "gcd_input");
        mu::ui::window::CObject* owner = &dialog;
        REQUIRE(owner->TakesTypingFrom(field->GetOwnerDocument()));
        CHECK_FALSE(owner->TakesTypingFrom(nullptr));
        g_pNewKeyInput->SetKeyState(key, mu::ui::window::CNewKeyInput::KEY_PRESS);
        CHECK_FALSE(owner->UpdateKeyEvent());
        g_pNewKeyInput->SetKeyState(key, mu::ui::window::CNewKeyInput::KEY_NONE);
        CHECK_FALSE(dialog.IsVisible());
    }
    CHECK(accepted == 1);
    CHECK(cancelled == 1);
    g_bWndActive = wasActive;
    dialog.Release();
}

TEST_CASE("options keep displayed theme identities and refresh only on open or switch [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("legacy") / "base.rcss", "body { color: white; }");
    fixture.Write(fixture.Theme("modern") / "base.rcss", "body { color: white; }");
    fixture.Write(fixture.root / "Data/Interface/RmlUi/option_window.rml",
        "<rml><head></head><body data-model='option_window'>"
        "<div id='panel' class='fullscreen'><span id='selected'>{{ theme_value_label }}</span>"
        "<span id='index'>{{ theme_index }}</span><div id='choices'>"
        "<span data-for='label : theme_labels'>{{ label }}</span></div>"
        "<button id='choose' data-event-click='option_dropdown_option_click(2, 1)'>Choose</button>"
        "</div></body></rml>");
    fixture.ActivateRoot();
    UI::RmlBridge::SetActiveThemeName("legacy");
    mu::ui::window::CManager manager;
    mu::ui::window::COptionWindow options;
    REQUIRE(options.Create(&manager));
    options.Show(true);
    auto update = [&] { options.Update(); fixture.context->Update(); };
    auto document = [&] { return fixture.context->GetDocument(0); };
    for (int frame = 0; frame < 5; ++frame)
        update();
    REQUIRE(document() != nullptr);
    CHECK(document()->GetElementById("selected")->GetInnerRML() == "Legacy");
    const auto originalChoices = document()->GetElementById("choices")->GetInnerRML();
    fixture.Write(fixture.Theme("aaa") / "base.rcss", "body { color: white; }");
    for (int frame = 0; frame < 5; ++frame)
        update();
    CHECK(document()->GetElementById("choices")->GetInnerRML() == originalChoices);
    // Index 1 belongs to Modern in the displayed list, even though discovery now includes aaa.
    document()->GetElementById("choose")->DispatchEvent(Rml::EventId::Click, {});
    update();
    CHECK(UI::RmlBridge::GetActiveThemeName() == "modern");
    CHECK(document()->GetElementById("selected")->GetInnerRML() == "Modern");
    CHECK(document()->GetElementById("index")->GetInnerRML() == "2");
    fixture.Write(fixture.Theme("aaa") / "theme.ini", "[Meta]\nDisplayName=Added theme\n");
    options.OpenningProcess();
    update();
    CHECK(document()->GetElementById("choices")->GetInnerRML().find("Added theme") != std::string::npos);
    // A deleted pending theme must not silently turn into another entry at its old index.
    for (int frame = 0; frame < 5; ++frame)
        update();
    document()->GetElementById("choose")->DispatchEvent(Rml::EventId::Click, {});
    std::filesystem::remove(fixture.Theme("legacy") / "base.rcss");
    update();
    CHECK(UI::RmlBridge::GetActiveThemeName() == "modern");
    CHECK(document()->GetElementById("index")->GetInnerRML() == "1");
}

TEST_CASE("scene resizing preserves open creation and message actions [ui][scene-resize]")
{
    // Avoid scene initialization: it needs live character models and native GPU textures.
    class Scene : public CSceneUICoordinator
    {
    public:
        void SetScene(int scene) { m_nScene = scene; }
    } scene;
    const unsigned int previousWidth = WindowWidth;
    const unsigned int previousHeight = WindowHeight;
    const bool creationShown = g_CharMakeWin.IsVisible();
    WindowWidth = 1280;
    WindowHeight = 720;
    wchar_t message[] = L"Resize must keep this message";
    g_MsgWin.PopUp(MESSAGE_FREE_MSG_NOT_BTN, message);
    scene.SetScene(UIM_SCENE_LOGIN);
    scene.RepositionSceneUI();
    CHECK(g_MsgWin.IsVisible());
    CHECK(g_MsgWin.PendingMessageCode() == MESSAGE_FREE_MSG_NOT_BTN);
    g_CharMakeWin.Show(true);
    scene.SetScene(UIM_SCENE_CHARACTER);
    WindowWidth = 1920;
    WindowHeight = 1080;
    scene.RepositionSceneUI();
    CHECK(g_CharMakeWin.IsVisible());
    CHECK(g_MsgWin.IsVisible());
    CHECK(g_MsgWin.PendingMessageCode() == MESSAGE_FREE_MSG_NOT_BTN);
    g_CharMakeWin.Show(creationShown);
    g_MsgWin.Show(false);
    WindowWidth = previousWidth;
    WindowHeight = previousHeight;
}

TEST_CASE("themed external RCSS resolves tokens regardless of link attribute order [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("alpha") / "tokens.ini", "[Tokens]\naccent=#123456\n");
    fixture.Write(fixture.Theme("alpha") / "paint.rcss", "#colored { color: token(accent); }");
    fixture.Write(fixture.Theme("beta") / "tokens.ini", "[Tokens]\naccent=#abcdef\n");
    fixture.Write(fixture.Theme("beta") / "paint.rcss", "#colored { color: token(accent); }");

    CHECK(fixture.LoadColor("alpha", "href='paint.rcss' type='text/rcss'") == Rml::Colourb(0x12, 0x34, 0x56));
    CHECK(fixture.LoadColor("beta", "type='text/rcss' href='paint.rcss'") == Rml::Colourb(0xab, 0xcd, 0xef));
    CHECK(fixture.LoadColor("alpha", "type='text/rcss' href='paint.rcss'") == Rml::Colourb(0x12, 0x34, 0x56));
}

TEST_CASE("theme file interface passes through ordinary files and plain RCSS [ui][rml-theme]")
{
    Fixture fixture;
    const auto plain = fixture.Theme("alpha") / "plain.rcss";
    const auto other = fixture.root / "fonts" / "sample.bin";
    fixture.Write(plain, "#colored { color: #987654; }");
    fixture.Write(other, "abcdef");

    CHECK(fixture.LoadColor("alpha", "href='plain.rcss' type='text/rcss'") == Rml::Colourb(0x98, 0x76, 0x54));
    Rml::FileHandle file = fixture.files.Open(other.generic_string());
    REQUIRE(file != 0);
    CHECK(fixture.files.Length(file) == 6);
    CHECK(fixture.files.Seek(file, 2, SEEK_SET));
    char bytes[3] = {};
    CHECK(fixture.files.Read(bytes, 2, file) == 2);
    CHECK(std::string(bytes, 2) == "cd");
    fixture.files.Close(file);
    CHECK(fixture.files.Open((fixture.root / "missing.rcss").generic_string()) == 0);
}

TEST_CASE("inherited stylesheets use child tokens and whole-file overrides [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("parent") / "tokens.ini", "[Tokens]\naccent=#123456\n");
    fixture.Write(fixture.Theme("parent") / "paint.rcss", "#colored { color: token(accent); }");
    fixture.Write(fixture.Theme("child") / "theme.ini", "[Theme]\nBase=parent\n");
    fixture.Write(fixture.Theme("child") / "tokens.ini", "[Tokens]\naccent=#abcdef\n");
    CHECK(fixture.LoadColor("child", "href='paint.rcss' type='text/rcss'") == Rml::Colourb(0xab, 0xcd, 0xef));
    CHECK(fixture.LoadColor("parent", "href='paint.rcss' type='text/rcss'") == Rml::Colourb(0x12, 0x34, 0x56));
    fixture.Write(fixture.Theme("child") / "paint.rcss", "#colored { color: #987654; }");
    Rml::Factory::ClearStyleSheetCache();
    CHECK(fixture.LoadColor("child", "href='paint.rcss' type='text/rcss'") == Rml::Colourb(0x98, 0x76, 0x54));
}

TEST_CASE("optional fonts inherit their declaring path and missing families fall back [ui][rml-theme]")
{
    Fixture fixture;
    const auto parent = fixture.Theme("parent");
    fixture.Write(parent / "theme.ini", "[Fonts]\nregular=fonts/body.ttf\nmissing=%MU_THEME_MISSING_ENV%/missing.ttf\n");
    fixture.Write(parent / "tokens.ini", "[Tokens]\nfont-body=Liberation Sans\n");
    std::filesystem::create_directories(parent / "fonts");
    std::filesystem::copy_file(std::filesystem::path(MU_FONT_DIR) / "LiberationSans-Regular.ttf", parent / "fonts/body.ttf");
    fixture.Write(fixture.Theme("child") / "theme.ini", "[Theme]\nBase=parent\n");
    fixture.Write(fixture.Theme("child") / "tokens.ini", "[Tokens]\nfont-body=Unavailable Test Family\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    UI::RmlBridge::LoadThemeFonts(theme);
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body") == std::optional<std::string>("Liberation Sans"));
    fixture.Write(parent / "paint.rcss", "#colored { font-family: token(font-body); font-size: 16px; }");
    auto* document = fixture.context->LoadDocumentFromMemory(
        "<rml><head><link type='text/rcss' href='paint.rcss'/></head><body><div id='colored'>Visible</div></body></rml>",
        (fixture.Theme("child") / "view.rml").generic_string());
    REQUIRE(document != nullptr);
    document->Show();
    fixture.context->Update();
    auto* text = document->GetElementById("colored");
    REQUIRE(text != nullptr);
    CHECK(text->GetFontFaceHandle() != 0);
    CHECK(text->GetComputedValues().font_family() == "liberation sans");
}

TEST_CASE("sparse themes load parent markup with child templates and reload tokens [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("parent") / "base.rcss", "#colored { color: token(accent); }");
    fixture.Write(fixture.Theme("parent") / "tokens.ini", "[Tokens]\naccent=#123456\n");
    fixture.Write(fixture.Theme("parent") / "theme.ini", "[Capabilities]\nNativeTextSize=1\n");
    fixture.Write(fixture.Theme("parent") / "msg_win.rml",
        "<rml><head><link type='text/rcss' href='base.rcss'/>"
        "<link type='text/template' href='nested/frame.rml'/></head><body template='test-frame'></body></rml>");
    fixture.Write(fixture.Theme("parent") / "nested/frame.rml",
        "<template name='test-frame' content='content'><head></head><body><div id='colored'>Parent</div>"
        "<div id='content'></div></body></template>");
    fixture.Write(fixture.Theme("alpha") / "theme.ini", "[Theme]\nBase=parent\n[Capabilities]\nNativeTextSize=0\n");
    fixture.Write(fixture.Theme("alpha") / "tokens.ini", "[Tokens]\naccent=#abcdef\n");
    fixture.Write(fixture.Theme("alpha") / "nested/frame.rml",
        "<template name='test-frame' content='content'><head></head><body><div id='colored'>Child</div>"
        "<div id='content'></div></body></template>");
    fixture.Write(fixture.Theme("beta") / "theme.ini", "[Theme]\nBase=parent\n");
    fixture.ActivateRoot();
    REQUIRE(UI::RmlBridge::ThemeExists("alpha"));
    CHECK_FALSE(UI::RmlBridge::ThemeExists("../parent"));
    CHECK(UI::RmlBridge::DiscoverAvailableThemes().size() == 3);
    UI::RmlBridge::SetActiveThemeName("alpha");
    CHECK_FALSE(UI::RmlBridge::ThemeUsesNativeTextSize());
    UI::RmlBridge::ThemedView<> view({{"Data/Interface/RmlUi/msg_win.rml"}});
    REQUIRE(view.Ensure());
    view.Document()->Show();
    fixture.context->Update();
    REQUIRE(view.Document()->GetElementById("colored"));
    CHECK(view.Document()->GetElementById("colored")->GetInnerRML() == "Child");
    CHECK(view.Document()->GetElementById("colored")->GetComputedValues().color() == Rml::Colourb(0xab, 0xcd, 0xef));
    UI::RmlBridge::SetActiveThemeName("beta");
    UI::RmlBridge::ReloadAllThemedDocuments();
    CHECK(UI::RmlBridge::ThemeUsesNativeTextSize());
    REQUIRE(view.Document());
    fixture.context->Update();
    CHECK(view.Document()->IsVisible());
    REQUIRE(view.Document()->GetElementById("colored"));
    CHECK(view.Document()->GetElementById("colored")->GetInnerRML() == "Parent");
    CHECK(view.Document()->GetElementById("colored")->GetComputedValues().color() == Rml::Colourb(0x12, 0x34, 0x56));
    fixture.Write(fixture.Theme("alpha") / "tokens.ini", "[Tokens]\naccent=#987654\n");
    UI::RmlBridge::SetActiveThemeName("alpha");
    UI::RmlBridge::ReloadAllThemedDocuments();
    fixture.context->Update();
    REQUIRE(view.Document()->GetElementById("colored"));
    CHECK(view.Document()->GetElementById("colored")->GetComputedValues().color() == Rml::Colourb(0x98, 0x76, 0x54));
}

TEST_CASE("system font paths expand environment variables and missing files remain optional [ui][rml-theme]")
{
    Fixture fixture;
    FontEnvironment environment;
    fixture.Write(fixture.Theme("child") / "theme.ini",
        "[Fonts]\nregular=%MU_THEME_FONT_TEST_ROOT%/LiberationSans-Regular.ttf\n"
        "missing=%MU_THEME_FONT_TEST_ROOT%/missing.ttf\n");
    fixture.Write(fixture.Theme("child") / "tokens.ini", "[Tokens]\nfont-body=Liberation Sans\nfont-title=Missing Family\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    UI::RmlBridge::LoadThemeFonts(theme);
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body") == std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-title") == std::optional<std::string>("Liberation Sans"));
}

TEST_CASE("a loaded font selection replaces the body and title tokens only [ui][rml-theme]")
{
    Fixture fixture;
    for (const char* name : {"LiberationSans-Regular.ttf", "DejaVuSans.ttf"})
        REQUIRE(Rml::LoadFontFace((std::filesystem::path(MU_FONT_DIR) / name).generic_string()));
    fixture.Write(fixture.Theme("child") / "tokens.ini",
                  "[Tokens]\nfont-body=DejaVu Sans\nfont-title=DejaVu Sans\nfont-icon=DejaVu Sans\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body") == std::optional<std::string>("DejaVu Sans"));
    UI::RmlBridge::SetSelectedFontFamily("Liberation Sans");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body") == std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-title") == std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-icon") == std::optional<std::string>("DejaVu Sans"));
    UI::RmlBridge::SetSelectedFontFamily("Unavailable Test Family");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body") == std::optional<std::string>("DejaVu Sans"));
}

TEST_CASE("the native font token names the native text renderer's family [ui][rml-theme]")
{
    Fixture fixture;
    for (const char* name : {"LiberationSans-Regular.ttf", "DejaVuSans.ttf"})
        REQUIRE(Rml::LoadFontFace((std::filesystem::path(MU_FONT_DIR) / name).generic_string()));
    fixture.Write(fixture.Theme("child") / "tokens.ini", "[Tokens]\nfont-body=Liberation Sans\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-native") == std::optional<std::string>("DejaVu Sans"));
    UI::RmlBridge::SetSelectedFontFamily("Liberation Sans");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-native") == std::optional<std::string>("Liberation Sans"));
}

TEST_CASE("an empty legacy child preserves real theme resources and layout [ui][rml-theme]")
{
    Fixture fixture;
    const std::filesystem::path assets = MU_RMLUI_DIR;
    std::filesystem::create_directories(fixture.root / "Data/Interface");
    std::filesystem::copy(assets, fixture.root / "Data/Interface/RmlUi", std::filesystem::copy_options::recursive);
    fixture.Write(fixture.Theme("legacy-inherited-test") / "theme.ini", "[Theme]\nBase=legacy\n");
    fixture.ActivateRoot();
    for (const char* name : {"LiberationSans-Regular.ttf", "LiberationSans-Bold.ttf", "DejaVuSans.ttf", "DejaVuSans-Bold.ttf"})
        REQUIRE(Rml::LoadFontFace((std::filesystem::path(MU_FONT_DIR) / name).generic_string()));
    const UI::RmlBridge::ThemeInheritance legacy(fixture.Theme("legacy"));
    const UI::RmlBridge::ThemeInheritance inherited(fixture.Theme("legacy-inherited-test"));
    REQUIRE(inherited.IsValid());
    for (const auto& entry : std::filesystem::recursive_directory_iterator(fixture.Theme("legacy")))
    {
        if (entry.is_regular_file() && entry.path().filename() != "theme.ini")
            CHECK(inherited.ResolveFile(entry.path().lexically_relative(fixture.Theme("legacy"))) == entry.path());
    }
    CHECK(inherited.Section("Tokens").size() == legacy.Section("Tokens").size());
    for (const auto& [key, value] : legacy.Section("Tokens"))
        CHECK(inherited.GetSetting("Tokens", key)->value == value.value);
    for (const Rml::Vector2i viewport : {Rml::Vector2i{1024, 768}, Rml::Vector2i{1920, 1080}})
    {
        fixture.context->SetDimensions(viewport);
        for (const int percent : {75, 100, 150})
        {
            GameConfig::GetInstance().SetUIScalePercent(percent);
            fixture.context->SetDensityIndependentPixelRatio(UI::Scaling::TypographyScale(viewport.x, viewport.y));
            UI::RmlBridge::ApplyScaleInputs(fixture.context);
            for (const char* name : {"login.rml", "char_make.rml", "msg_win.rml", "main_frame.rml",
                                     "chat_log.rml", "friend_shell.rml", "my_inventory.rml"})
            {
                CAPTURE(viewport.x); CAPTURE(viewport.y); CAPTURE(percent); CAPTURE(name);
                CompareLegacyVariant(fixture, name);
            }
        }
    }
}

TEST_CASE("map-name font tokens preserve normal families and inherited fallback [ui][map-name]")
{
    Fixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf"));
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/DejaVuSans.ttf"));
    const auto parent = fixture.Theme("parent");
    const auto child = fixture.Theme("child");
    fixture.Write(child / "theme.ini", "[Theme]\nBase=parent\n");
    fixture.Write(parent / "tokens.ini", "[Tokens]\nfont-map-name=Liberation Sans\n");
    fixture.Write(child / "tokens.ini", "[Tokens]\nfont-map-name=DejaVu Sans\n");
    const auto resolve = [&](const char* token)
    { return UI::RmlBridge::ResolveThemeFontToken(UI::RmlBridge::ThemeInheritance(child), token); };
    CHECK(resolve("font-map-name") == std::optional<std::string>("DejaVu Sans"));
    CHECK(resolve("font-map-name-style") == std::optional<std::string>("normal"));

    fixture.Write(parent / "tokens.ini", "[Tokens]\nfont-map-name=DejaVu Sans\n");
    fixture.Write(child / "tokens.ini", "[Tokens]\nfont-map-name=Unavailable Test Family\n");
    CHECK(resolve("font-map-name") == std::optional<std::string>("DejaVu Sans"));
    CHECK(resolve("font-map-name-style") == std::optional<std::string>("normal"));

    fixture.Write(parent / "tokens.ini", "[Tokens]\nfont-map-name=Unavailable Parent Family\n");
    CHECK(resolve("font-map-name") == std::optional<std::string>("Liberation Sans"));
    CHECK(resolve("font-map-name-style") == std::optional<std::string>("normal"));
}

TEST_CASE("font style tokens stay paired with the resolved family and use supported faces [ui][rml-theme]")
{
    Fixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf"));
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf", "Liberation Sans",
                              Rml::Style::FontStyle::Italic, Rml::Style::FontWeight::Normal));
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/DejaVuSans.ttf"));
    const auto parent = fixture.Theme("parent");
    const auto child = fixture.Theme("child");
    fixture.Write(parent / "tokens.ini",
                  "[Tokens]\nfont-callout=Liberation Sans\nfont-callout-style=italic\nfont-title=DejaVu Sans\n");
    fixture.Write(child / "theme.ini", "[Theme]\nBase=parent\n");
    fixture.Write(child / "tokens.ini", "[Tokens]\nfont-callout=DejaVu Sans\nfont-callout-style=italic\n");
    const UI::RmlBridge::ThemeInheritance theme(child);
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-callout") == std::optional<std::string>("DejaVu Sans"));
    // DejaVu has no italic face, so its requested style falls back to regular.
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-callout-style") == std::optional<std::string>("normal"));

    fixture.Write(child / "tokens.ini", "[Tokens]\nfont-callout=Unavailable Child Family\n");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-callout") == std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-callout-style") == std::optional<std::string>("italic"));
}

TEST_CASE("unpaired theme families require a regular font face [ui][rml-theme]")
{
    Fixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf", "Liberation Sans",
                              Rml::Style::FontStyle::Italic, Rml::Style::FontWeight::Normal));
    fixture.Write(fixture.Theme("child") / "tokens.ini",
                  "[Tokens]\nfont-body=Liberation Sans\nfont-title=Liberation Sans\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    CHECK_FALSE(UI::RmlBridge::ResolveThemeFontToken(theme, "font-body"));
    CHECK_FALSE(UI::RmlBridge::ResolveThemeFontToken(theme, "font-title"));
}

TEST_CASE("a missing map-name font falls back to the token the theme names, in that token's style [ui][map-name]")
{
    Fixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf"));
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf", "Liberation Sans",
                              Rml::Style::FontStyle::Italic, Rml::Style::FontWeight::Normal));
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/DejaVuSans.ttf"));
    fixture.Write(fixture.Theme("child") / "tokens.ini",
                  "[Tokens]\nfont-map-name-fallback=font-title\nfont-title=DejaVu Sans\nfont-title-style=italic\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") == std::optional<std::string>("DejaVu Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") == std::optional<std::string>("normal"));
    UI::RmlBridge::SetSelectedFontFamily("Liberation Sans");
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name") ==
          UI::RmlBridge::ResolveThemeFontToken(theme, "font-title"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") ==
          UI::RmlBridge::ResolveThemeFontToken(theme, "font-title-style"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-map-name-style") == std::optional<std::string>("italic"));
}

TEST_CASE("font fallback tokens that name each other end at the bundled face [ui][rml-theme]")
{
    Fixture fixture;
    REQUIRE(Rml::LoadFontFace(std::string(MU_FONT_DIR) + "/LiberationSans-Regular.ttf"));
    fixture.Write(fixture.Theme("child") / "tokens.ini",
                  "[Tokens]\nfont-a=Unavailable A\nfont-a-fallback=font-b\nfont-b=Unavailable B\n"
                  "font-b-fallback=font-a\n");
    const UI::RmlBridge::ThemeInheritance theme(fixture.Theme("child"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-a") == std::optional<std::string>("Liberation Sans"));
    CHECK(UI::RmlBridge::ResolveThemeFontToken(theme, "font-a-style") == std::optional<std::string>("normal"));
}
