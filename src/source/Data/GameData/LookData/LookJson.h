#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <string>
#include <string_view>
#include <vector>

// The render style file, Data/Effects/RenderStyles.json (docs/item-data.md, "Render styles and item effects"):
//
//   { "formatVersion": 1, "kind": "renderStyle",
//     "looks": [ { "name": "chromeMesh0",
//                  "layers": [ { "kind": "texture" },
//                              { "kind": "reflection", "blend": "glow", "meshes": [0], "glowMesh": 0 } ] } ] }
//
// Looks are sorted by name. A value left out is the item's (for a layer's meshes: every mesh). The Style Editor's
// folders ("lookFolders") are the editor's; the game skips them.
namespace Data::Looks
{
constexpr int LookFileFormatVersion = 1;

// "RenderStyles.json".
std::string_view GetRenderStylesFileName();

// Reads the looks of a render style file into `looks`; problems go to `issues`, and a fatal one makes the file
// unusable.
void ReadRenderStylesJson(std::string_view text, const std::string& source, std::vector<LookDefinition>& looks,
                          std::vector<Items::ItemDataIssue>& issues);

// The text of a render style file: looks sorted by name, fields in a fixed order, values that are the item's left
// out.
std::string WriteRenderStylesJson(std::vector<LookDefinition> looks);
} // namespace Data::Looks
