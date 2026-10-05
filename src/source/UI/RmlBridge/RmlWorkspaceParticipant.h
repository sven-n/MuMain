#pragma once

#include <functional>
#include <string_view>

namespace Rml { class ElementDocument; }

namespace UI::RmlBridge
{
// Registers a dp-sized root as a content-sized workspace participant. The getter follows reloads.
void RegisterWorkspaceDocument(std::string_view name, std::function<Rml::ElementDocument*()> document,
                               const char* rootId);
}
