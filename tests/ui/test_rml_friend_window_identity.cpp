#include <doctest.h>

#include "UI/RmlBridge/RmlModelBinder.h"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>

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

struct WindowRow
{
    int id;
    Rml::String text;
};

struct WindowModel
{
    Rml::Vector<WindowRow> windows;
};

Rml::String Selection(Rml::ElementFormControlInput* field)
{
    Rml::String text;
    field->GetSelection(nullptr, nullptr, &text);
    return text;
}

struct Fixture
{
    NullRenderer renderer;
    Rml::Context* context = nullptr;
    RmlModelBinder<WindowModel> binder;

    Fixture()
    {
        Rml::SetRenderInterface(&renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("friend-identity", {800, 600});
        REQUIRE(context);
        REQUIRE(binder.Create(context, "windows", [](Rml::DataModelConstructor& constructor, WindowModel& model)
        {
            auto row = constructor.RegisterStruct<WindowRow>();
            row.RegisterMember("id", &WindowRow::id);
            row.RegisterMember("text", &WindowRow::text);
            constructor.RegisterArray<Rml::Vector<WindowRow>>();
            constructor.Bind("windows", &model.windows);
        }));
    }

    ~Fixture()
    {
        context->UnloadAllDocuments();
        context->Update();
        binder.Destroy(context);
        Rml::RemoveContext("friend-identity");
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    void Update()
    {
        binder.MarkDirty("windows");
        context->Update();
    }

    Rml::ElementDocument* Repeater()
    {
        binder.GetModel().windows = {{11, "first"}, {22, "second"}, {33, "third"}};
        auto* document = context->LoadDocumentFromMemory(
            "<rml><head><style>div { display: block; }"
            ".window { display: block; width: 200px; height: 110px; }"
            "input { width: 180px; height: 20px; }"
            ".pane { width: 180px; height: 60px; overflow: hidden scroll; }"
            ".content { height: 300px; }"
            "</style></head><body data-model='windows'>"
            "<div id='windows'><div class='window' data-for='window : windows' data-attr-window-id='window.id'>"
            "<input data-value='window.text'/><div class='pane'><div class='content'/></div>"
            "</div></div></body></rml>");
        REQUIRE(document);
        document->Show();
        Update();
        return document;
    }
};
}

TEST_CASE("Window repeater preserves existing fields when appending [ui][friend-identity]")
{
    Fixture fixture;
    auto* document = fixture.Repeater();
    auto* window = document->GetElementById("windows")->GetChild(1);
    auto* field = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(window->GetChild(0));
    auto* pane = window->GetChild(1);
    REQUIRE(field);
    REQUIRE(field->Focus());
    field->SetSelectionRange(1, 3);
    pane->SetScrollTop(35);
    REQUIRE(pane->GetScrollTop() == doctest::Approx(35));

    fixture.binder.GetModel().windows.push_back({44, "fourth"});
    fixture.binder.GetModel().windows.front().text = "updated";
    fixture.Update();

    CHECK(fixture.context->GetFocusElement() == field);
    CHECK(window->GetAttribute<int>("window-id", 0) == 22);
    CHECK(field->GetValue() == "second");
    CHECK(Selection(field) == "ec");
    CHECK(pane->GetScrollTop() == doctest::Approx(35));
}

TEST_CASE("Compacting a window repeater reassigns focused DOM state by index [ui][friend-identity]")
{
    Fixture fixture;
    auto* document = fixture.Repeater();
    auto* window = document->GetElementById("windows")->GetChild(1);
    auto* field = window->GetChild(0);
    auto* pane = window->GetChild(1);
    REQUIRE(field->Focus());
    pane->SetScrollTop(35);
    REQUIRE(pane->GetScrollTop() == doctest::Approx(35));

    auto& windows = fixture.binder.GetModel().windows;
    windows.erase(windows.begin());
    fixture.Update();

    CHECK(fixture.context->GetFocusElement() == field);
    CHECK(window->GetAttribute<int>("window-id", 0) == 33);
    CHECK(document->GetElementById("windows")->GetChild(0)->GetAttribute<int>("window-id", 0) == 22);
    CHECK(pane->GetScrollTop() == doctest::Approx(35));
}

TEST_CASE("Separate window documents retain field state when a sibling closes [ui][friend-identity]")
{
    Fixture fixture;
    constexpr const char* markup =
        "<rml><head><style>div { display:block; } input { width:180px; height:20px; }"
        "#pane { width:180px; height:60px; overflow:hidden scroll; }"
        "#content { height:300px; }</style></head><body>"
        "<input id='field' value='second'/><div id='pane'><div id='content'/></div></body></rml>";
    auto* first = fixture.context->LoadDocumentFromMemory(markup);
    auto* second = fixture.context->LoadDocumentFromMemory(markup);
    REQUIRE(first);
    REQUIRE(second);
    first->Show();
    second->Show();
    fixture.context->Update();
    auto* field = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(second->GetElementById("field"));
    auto* pane = second->GetElementById("pane");
    REQUIRE(field);
    REQUIRE(field->Focus());
    field->SetSelectionRange(1, 3);
    pane->SetScrollTop(35);
    REQUIRE(pane->GetScrollTop() == doctest::Approx(35));

    fixture.context->UnloadDocument(first);
    auto* third = fixture.context->LoadDocumentFromMemory(markup);
    REQUIRE(third);
    third->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
    second->PullToFront();
    fixture.context->Update();

    CHECK(fixture.context->GetFocusElement() == field);
    CHECK(field->GetValue() == "second");
    CHECK(Selection(field) == "ec");
    CHECK(pane->GetScrollTop() == doctest::Approx(35));
    field->Blur();
    fixture.context->UnloadDocument(second);
    fixture.context->Update();
    CHECK(fixture.context->GetFocusElement() != field);
}
