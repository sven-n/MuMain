#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <RmlUi/Core.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override
    {
        return 0;
    }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override
    {
        return 0;
    }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override
    {
        return 0;
    }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

struct ViewTestModel
{
    Rml::String title;
};

void RegisterViewTestModel(Rml::DataModelConstructor& constructor, ViewTestModel& model)
{
    constructor.Bind("title", &model.title);
}

constexpr const char* kTitleDocument =
    "<rml><head></head><body data-model='view_test'><div id='title'>{{title}}</div></body></rml>";
constexpr const char* kPlainDocument = "<rml><head></head><body><div id='plain'>plain</div></body></rml>";

// RmlUi running headless, with a folder for the test's documents.
class Fixture
{
public:
    Fixture()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("rml-themed-view", {320, 240});
        REQUIRE(context != nullptr);
        m_Folder = std::filesystem::temp_directory_path() / "rml_themed_view_tests";
        std::filesystem::remove_all(m_Folder);
        std::filesystem::create_directories(m_Folder);
    }

    ~Fixture()
    {
        if (!m_ShutDown)
            ShutDown();
        std::filesystem::remove_all(m_Folder);
    }

    void ShutDown()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
        m_ShutDown = true;
    }

    std::string Path(const char* name) const { return (m_Folder / name).generic_string(); }

    std::string Write(const char* name, const char* markup) const
    {
        std::ofstream(m_Folder / name, std::ios::binary) << markup;
        return Path(name);
    }

    UI::RmlBridge::ThemedDocumentSpec Spec(const std::string& path) const
    {
        Rml::Context* target = context;
        return {path, [target] { return target; }};
    }

    Rml::String Title(Rml::ElementDocument* document) const
    {
        context->Update();
        return document->GetElementById("title")->GetInnerRML();
    }

    Rml::Context* context = nullptr;

private:
    NullRenderer m_Renderer;
    std::filesystem::path m_Folder;
    bool m_ShutDown = false;
};
} // namespace

// A theme switch rebuilds every document. Before the shared owner each window copied the fields it
// cared about out and back by hand, or lost them, and re-showed its document only if it remembered to.
TEST_CASE("a theme switch keeps the model and shows again what was visible [ui][rml-themed-view]")
{
    Fixture fixture;
    bool shownWhenReloaded = false;
    UI::RmlBridge::ThemedView<ViewTestModel> view("view_test", RegisterViewTestModel,
                                                  {fixture.Spec(fixture.Write("title.rml", kTitleDocument))});
    view.SetAfterReload([&] { shownWhenReloaded = view.Document()->IsVisible(); });
    REQUIRE(view.Ensure());
    CHECK_FALSE(shownWhenReloaded);
    view.GetModel().title = "Typed";
    view.MarkDirty("title");
    view.Document()->Show();
    CHECK(fixture.Title(view.Document()) == "Typed");

    Rml::ElementDocument* before = view.Document();
    UI::RmlBridge::ReloadAllThemedDocuments();
    REQUIRE(view.Document() != nullptr);
    CHECK(view.Document() != before);
    CHECK(view.Document()->IsVisible());
    CHECK(shownWhenReloaded);
    CHECK(fixture.Title(view.Document()) == "Typed");

    view.Document()->Hide();
    fixture.context->Update();
    UI::RmlBridge::ReloadAllThemedDocuments();
    fixture.context->Update();
    CHECK_FALSE(view.Document()->IsVisible());
}

// A document that fails to load (a missing file, a theme without it) comes back on a later frame,
// and a theme switch can bring it back too: the view registered on its first attempt.
TEST_CASE("a failed build retries and still follows theme switches [ui][rml-themed-view]")
{
    Fixture fixture;
    int builds = 0;
    UI::RmlBridge::ThemedViewOptions options;
    options.afterBuild = [&builds] { ++builds; };
    UI::RmlBridge::ThemedView<ViewTestModel> view("view_test", RegisterViewTestModel,
                                                  {fixture.Spec(fixture.Path("late.rml"))}, options);
    CHECK_FALSE(view.Ensure());
    CHECK(view.Document() == nullptr);
    CHECK(builds == 0);

    fixture.Write("late.rml", kTitleDocument);
    UI::RmlBridge::ReloadAllThemedDocuments();
    CHECK(view.IsBuilt());
    CHECK(builds == 1);
    CHECK(view.Ensure());
    CHECK(builds == 1);
}

// The background contexts exist only once the runtime made them.
TEST_CASE("a view waits for its context [ui][rml-themed-view]")
{
    Fixture fixture;
    Rml::Context* available = nullptr;
    UI::RmlBridge::ThemedView<> view({{fixture.Write("plain.rml", kPlainDocument), [&available] { return available; }}});
    CHECK_FALSE(view.Ensure());
    available = fixture.context;
    CHECK(view.Ensure());
    CHECK(view.Document()->GetContext() == fixture.context);
}

TEST_CASE("release removes the documents, the model and the registration [ui][rml-themed-view]")
{
    Fixture fixture;
    UI::RmlBridge::ThemedView<ViewTestModel> view("view_test", RegisterViewTestModel,
                                                  {fixture.Spec(fixture.Write("title.rml", kTitleDocument))});
    REQUIRE(view.Ensure());
    view.GetModel().title = "Typed";
    view.Release();
    fixture.context->Update();
    CHECK(view.Document() == nullptr);
    CHECK(fixture.context->GetNumDocuments() == 0);
    CHECK(view.GetModel().title.empty());

    // The name is free again, and a theme switch no longer builds the released view.
    RmlModelBinder<ViewTestModel> other;
    CHECK(other.Create(fixture.context, "view_test", RegisterViewTestModel));
    other.Destroy(fixture.context);
    UI::RmlBridge::ReloadAllThemedDocuments();
    CHECK(view.Document() == nullptr);

    // A released window can be created again (the scene windows are, on every visit).
    CHECK(view.Ensure());
    CHECK(view.Document() != nullptr);
}

// The UI is released before RmlUi shuts down, but a static owner (the tooltip, the notices) is
// destroyed after it, at exit.
TEST_CASE("a view outliving RmlUi tears down without touching it [ui][rml-themed-view]")
{
    Fixture fixture;
    auto view = std::make_unique<UI::RmlBridge::ThemedView<ViewTestModel>>(
        "view_test", RegisterViewTestModel,
        std::vector<UI::RmlBridge::ThemedDocumentSpec>{fixture.Spec(fixture.Write("title.rml", kTitleDocument))});
    REQUIRE(view->Ensure());
    Rml::Context* context = fixture.context;
    CHECK(UI::RmlBridge::IsContextAlive(context));
    fixture.ShutDown();
    CHECK_FALSE(UI::RmlBridge::IsContextAlive(context));
    // A static window's destructor calling its Release(), which hides.
    view->Hide();
    CHECK(view->Document() == nullptr);
    view.reset();
}

// The friend family's chat rooms and letters: one document and one model per instance.
TEST_CASE("per-instance views bind their own models [ui][rml-themed-view]")
{
    Fixture fixture;
    const std::string path = fixture.Write("room.rml", kTitleDocument);
    UI::RmlBridge::ThemedViewOptions options;
    options.modelPlaceholder = "view_test";
    UI::RmlBridge::ThemedView<ViewTestModel> first("room_1", RegisterViewTestModel, {fixture.Spec(path)}, options);
    UI::RmlBridge::ThemedView<ViewTestModel> second("room_2", RegisterViewTestModel, {fixture.Spec(path)}, options);
    REQUIRE(first.Ensure());
    REQUIRE(second.Ensure());
    first.GetModel().title = "First";
    first.MarkDirty("title");
    second.GetModel().title = "Second";
    second.MarkDirty("title");
    CHECK(fixture.Title(first.Document()) == "First");
    CHECK(fixture.Title(second.Document()) == "Second");
}

// The main frame's two documents show one model.
TEST_CASE("documents sharing one model all bind it [ui][rml-themed-view]")
{
    Fixture fixture;
    UI::RmlBridge::ThemedView<ViewTestModel> view(
        "view_test", RegisterViewTestModel,
        {fixture.Spec(fixture.Write("main.rml", kTitleDocument)), fixture.Spec(fixture.Write("top.rml", kTitleDocument))});
    REQUIRE(view.Ensure());
    view.GetModel().title = "Shared";
    view.MarkDirty("title");
    CHECK(fixture.Title(view.Document(0)) == "Shared");
    CHECK(fixture.Title(view.Document(1)) == "Shared");

    UI::RmlBridge::ReloadAllThemedDocuments();
    CHECK(fixture.Title(view.Document(1)) == "Shared");
}

// A reloaded document goes on top of its context, so the order the owners reload in is the order
// they stack in afterwards: the order they were first built in, not wherever a hash put them.
TEST_CASE("a theme switch keeps the documents' stacking [ui][rml-themed-view]")
{
    Fixture fixture;
    std::vector<std::unique_ptr<UI::RmlBridge::ThemedView<>>> views;
    for (int i = 0; i < 8; ++i)
    {
        const std::string name = "plain" + std::to_string(i) + ".rml";
        views.push_back(std::make_unique<UI::RmlBridge::ThemedView<>>(
            std::vector<UI::RmlBridge::ThemedDocumentSpec>{fixture.Spec(fixture.Write(name.c_str(), kPlainDocument))}));
        REQUIRE(views.back()->Ensure());
    }

    UI::RmlBridge::ReloadAllThemedDocuments();
    REQUIRE(fixture.context->GetNumDocuments() == static_cast<int>(views.size()));
    for (size_t i = 0; i < views.size(); ++i)
        CHECK(fixture.context->GetDocument(static_cast<int>(i)) == views[i]->Document());
}

// Unloading drops the focus without a blur; a focused field that never blurs leaves the client's
// text-input latch set, and every hotkey stays dead.
TEST_CASE("release blurs a focused field it owns [ui][rml-themed-view]")
{
    struct BlurCounter : Rml::EventListener
    {
        int blurs = 0;
        void ProcessEvent(Rml::Event&) override { ++blurs; }
    };

    Fixture fixture;
    UI::RmlBridge::ThemedView<> view({fixture.Spec(fixture.Write(
        "field.rml", "<rml><head></head><body><input id='field' type='text'/></body></rml>"))});
    REQUIRE(view.Ensure());
    view.Document()->Show();
    Rml::Element* field = view.Document()->GetElementById("field");
    BlurCounter counter;
    field->AddEventListener(Rml::EventId::Blur, &counter);
    REQUIRE(field->Focus());
    fixture.context->Update();

    view.Release();
    CHECK(counter.blurs == 1);
    fixture.context->Update();
}
