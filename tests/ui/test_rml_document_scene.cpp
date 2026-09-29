#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core.h>

extern EGameScene SceneFlag;

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

constexpr const char* kDocument = "<rml><head></head><body></body></rml>";
} // namespace

// A window open at logout (the move list, the inventory, the Blood Castle entry) kept rendering
// over character selection: its document is only synced from its window's Update(), which stops
// with the main scene. Outside the main scene the main-scene documents are suspended and keep
// their own visibility for the return.
TEST_CASE("main-scene documents are suspended outside the main scene [ui][rml-document-scene]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("rml-document-scene", {320, 240});
    REQUIRE(context != nullptr);

    auto* inventory = context->LoadDocumentFromMemory(kDocument);
    auto* moveList = context->LoadDocumentFromMemory(kDocument);
    auto* characterList = context->LoadDocumentFromMemory(kDocument);
    REQUIRE(inventory != nullptr);
    REQUIRE(moveList != nullptr);
    REQUIRE(characterList != nullptr);
    UI::RmlBridge::ApplyDocumentScene(inventory, "my_inventory.rml");
    UI::RmlBridge::ApplyDocumentScene(moveList, "move_command.rml");
    UI::RmlBridge::ApplyDocumentScene(characterList, "char_sel_main.rml");

    const EGameScene previousScene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    inventory->Show();
    moveList->Show();
    UI::RmlBridge::SuspendMainSceneDocumentsOutsideMainScene();
    context->Update();
    CHECK(inventory->IsVisible());
    CHECK(moveList->IsVisible());

    // Logout: the windows stop updating; the character list is shown by its own scene.
    SceneFlag = CHARACTER_SCENE;
    characterList->Show();
    UI::RmlBridge::SuspendMainSceneDocumentsOutsideMainScene();
    context->Update();
    CHECK_FALSE(inventory->IsVisible());
    CHECK_FALSE(moveList->IsVisible());
    CHECK(characterList->IsVisible());

    // A Show() while suspended has no effect.
    inventory->Show();
    UI::RmlBridge::SuspendMainSceneDocumentsOutsideMainScene();
    context->Update();
    CHECK_FALSE(inventory->IsVisible());

    // Back in the world: the move list's window was closed meanwhile (it hides its document in its
    // Update(), right after the resume); the inventory keeps its own state.
    SceneFlag = MAIN_SCENE;
    characterList->Hide();
    UI::RmlBridge::SuspendMainSceneDocumentsOutsideMainScene();
    context->Update();
    CHECK_FALSE(inventory->IsVisible());
    UI::RmlBridge::ResumeMainSceneDocuments();
    moveList->Hide();
    context->Update();
    CHECK(inventory->IsVisible());
    CHECK_FALSE(moveList->IsVisible());

    SceneFlag = previousScene;
    Rml::RemoveContext("rml-document-scene");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}
