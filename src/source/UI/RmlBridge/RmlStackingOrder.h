#pragma once

#include <optional>
#include <string_view>

// The stacking order of every RmlUi document: the layer depth of the original window (or render
// pass) the document replaces. LoadThemedDocument() sets it as the document's z-index. The original
// drew its windows in ascending GetLayerDepth() order (CNewUIManager::Render()), then the notices,
// the scene windows (CUIMng) and the reconnect dialog; RmlUi sorts a context's documents by z-index
// and keeps show/focus order only among documents of equal depth, which reproduces that order in
// both the main and the background context. See docs/rmlui-ui-system/STATUS.md, "Stacking order".
namespace UI::RmlBridge
{
// Depth for a document file name such as "chat_log.rml" (no directory); none for a name the
// table does not know (such a document keeps z-index:auto, under every listed one).
std::optional<float> StackingDepthForDocument(std::string_view documentName);
} // namespace UI::RmlBridge
