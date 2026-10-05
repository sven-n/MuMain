#pragma once

#include "UI/Placement/PlacementParticipant.h"

#include <functional>
#include <string_view>

namespace Rml { class ElementDocument; }

namespace UI::RmlBridge
{
struct WorkspaceDocumentOptions
{
    // Keeps the slot while the document is hidden (an input shown only while typing).
    bool placedWhileHidden = false;
    // Preferred size in dp, from the owner's own numbers; the root's measured box when empty.
    std::function<UI::Placement::PlacementParticipant::Size()> measure;
    // After the root is placed, or with null when it returns to its own placement.
    std::function<void(const UI::Placement::PlacementParticipant::Box*)> placed;
};

// Registers a dp-sized root as a content-sized workspace participant. The getter follows reloads.
void RegisterWorkspaceDocument(std::string_view name, std::function<Rml::ElementDocument*()> document,
                               const char* rootId, WorkspaceDocumentOptions options = {});
}
