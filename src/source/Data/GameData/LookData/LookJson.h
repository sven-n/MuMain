#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <span>
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
// folders ("lookFolders", ColorFolders.h) are the editor's: the game only checks them, as MuEditor does.
namespace Data::Looks
{
constexpr int LookFileFormatVersion = 1;

// "RenderStyles.json".
std::string_view GetRenderStylesFileName();

// A render style file as the Style Editor has it: its looks and the folders it sorts them in.
struct RenderStylesFile
{
    std::vector<LookDefinition> looks;
    std::vector<Colors::ColorFolder> folders;

    bool operator==(const RenderStylesFile&) const = default;
};

// Reads a render style file; problems go to `issues`, and a fatal one makes the file unusable. A problem of the folders
// is a warning that leaves out what it names.
void ReadRenderStylesFile(std::string_view text, const std::string& source, RenderStylesFile& file,
                          std::vector<Items::ItemDataIssue>& issues);
// ReadRenderStylesFile, appending the looks to `looks`.
void ReadRenderStylesJson(std::string_view text, const std::string& source, std::vector<LookDefinition>& looks,
                          std::vector<Items::ItemDataIssue>& issues);

// The name of each look of a render style file in file order, "" for one without a readable name: what a problem in
// "looks[3]" is about. Empty when the text is no JSON object.
std::vector<std::string> ReadLookNames(std::string_view text);

// The text of a render style file: the folders (each on a line), looks sorted by name, fields in a fixed order, values
// that are the item's left out.
std::string WriteRenderStylesJson(std::vector<LookDefinition> looks, std::span<const Colors::ColorFolder> folders = {});
} // namespace Data::Looks
