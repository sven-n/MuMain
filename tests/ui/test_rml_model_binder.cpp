#include <doctest.h>

#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core.h>

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

struct Row
{
    Rml::String text;
};

struct BinderTestModel
{
    Rml::String title;
    Rml::Vector<Row> rows;
};

int s_registrations = 0;

void RegisterBinderTestModel(Rml::DataModelConstructor& constructor, BinderTestModel& model)
{
    ++s_registrations;
    constructor.Bind("title", &model.title);
    if (auto row = constructor.RegisterStruct<Row>())
        row.RegisterMember("text", &Row::text);
    constructor.RegisterArray<Rml::Vector<Row>>();
    constructor.Bind("rows", &model.rows);
}

constexpr const char* kBinderTestDocument =
    "<rml><head></head><body data-model='binder_test'>"
    "<div id='title'>{{title}}</div>"
    "<div id='rows'><span data-for='row : rows'>{{row.text}}</span></div>"
    "</body></rml>";
} // namespace

// A window's build step creates its model, then loads its document; when the load fails it runs
// again on a later frame (or after a theme switch). The second Create() used to fail on the name
// the first one registered and free the type register that model still used, so the window never
// came back.
TEST_CASE("RmlModelBinder::Create keeps its model when the build step runs again [ui][rml-model-binder]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("rml-model-binder", {320, 240});
    REQUIRE(context != nullptr);
    s_registrations = 0;

    RmlModelBinder<BinderTestModel> binder;
    REQUIRE(binder.Create(context, "binder_test", RegisterBinderTestModel));
    CHECK(s_registrations == 1);

    // The document failed to load; the next build step creates the model again.
    CHECK(binder.Create(context, "binder_test", RegisterBinderTestModel));
    CHECK(s_registrations == 1);

    // Another binder cannot take the name, and its failed attempt leaves the first model intact.
    RmlModelBinder<BinderTestModel> other;
    CHECK_FALSE(other.Create(context, "binder_test", RegisterBinderTestModel));
    other.MarkDirty("title");

    binder.GetModel().title = "Title";
    binder.GetModel().rows = {{"first"}, {"second"}};
    binder.MarkDirty("title");
    binder.MarkDirty("rows");
    auto* document = context->LoadDocumentFromMemory(kBinderTestDocument);
    REQUIRE(document != nullptr);
    document->Show();
    context->Update();
    CHECK(document->GetElementById("title")->GetInnerRML() == "Title");
    const Rml::String rows = document->GetElementById("rows")->GetInnerRML();
    CHECK(rows.find("first") != Rml::String::npos);
    CHECK(rows.find("second") != Rml::String::npos);

    // A theme reload destroys the model; the next Create() registers it again.
    context->UnloadDocument(document);
    context->Update();
    binder.Destroy(context);
    REQUIRE(binder.Create(context, "binder_test", RegisterBinderTestModel));
    CHECK(s_registrations == 2);
    binder.Destroy(context);

    Rml::RemoveContext("rml-model-binder");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}
