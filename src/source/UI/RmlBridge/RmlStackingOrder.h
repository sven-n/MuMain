#pragma once

#include <optional>
#include <string_view>

// The stacking order of every RmlUi document: the layer depth of the original window (or render
// pass) the document replaces. Loading a themed document sets it as its z-index. The original
// drew its windows in ascending GetLayerDepth() order (CNewUIManager::Render()), then the notices,
// the scene windows (CUIMng) and the reconnect dialog; RmlUi sorts a context's documents by z-index
// and keeps show/focus order only among documents of equal depth, which reproduces that order.
// See docs/rmlui-ui-system/building-new-ui.md, "Stacking order".
namespace UI::RmlBridge
{
// The panels that stand over the HUD and its logs (the command windows).
inline constexpr float ForegroundPanelLayerDepth = 10.65f;

// The scene whose windows a document belongs to. The original drew CNewUIManager's windows only in
// the main scene, a document renders in every scene: outside the main scene the main-scene
// documents are suspended (RmlTheme.h, SuspendMainSceneDocumentsOutsideMainScene()). The login,
// character and loading scenes' documents, the notices and the reconnect dialog are shown and
// hidden by their own scene's enter/exit code, in whatever scene that is: Any.
enum class DocumentScene
{
    Main,
    Any,
    // Opened in every scene (the options window, from the login and character scenes' menu too):
    // at its depth in the main scene, over the scene windows outside it.
    Every,
};

// Depth for a document file name such as "chat_log.rml" (no directory); none for a name the
// table does not know (such a document keeps z-index:auto, under every listed one).
std::optional<float> StackingDepthForDocument(std::string_view documentName);

// Scene for a document file name, from the same table; none for a name the table does not know
// (such a document is never suspended).
std::optional<DocumentScene> SceneForDocument(std::string_view documentName);

// Depth outside the main scene for a DocumentScene::Every document; none for any other.
std::optional<float> StackingDepthOutsideMainScene(std::string_view documentName);
} // namespace UI::RmlBridge
