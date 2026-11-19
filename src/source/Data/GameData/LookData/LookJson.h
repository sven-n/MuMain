#pragma once

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <map>
#include <set>
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

// A reflection's motion as the files name it ("fastSweep"), and the motion of a name (none: no motion has it); for
// the other files that name motions (the shines below +3, Shines.h).
const char* MotionName(Motion motion);
std::optional<Motion> MotionNamed(std::string_view name);
// "slowDrift, fastSweep, ...", for messages.
std::string MotionNameList();
// The file of a kind of looks: "RenderStyles.json" or "ItemEffects.json".
std::string_view GetLookFileName(LookKind kind);

// A look file as the Style Editor has it: its looks and the folders it sorts them in.
struct LookFile
{
    std::vector<LookDefinition> looks;
    std::vector<Colors::ColorFolder> folders;

    bool operator==(const LookFile&) const = default;
};
using RenderStylesFile = LookFile;

// Reads a look file of that kind (its "kind" must say so); problems go to `issues`, and a fatal one makes the file
// unusable. A part of the other kind's looks is fatal (design L23); a problem of the folders is a warning that leaves
// out what it names.
void ReadLookFile(std::string_view text, const std::string& source, LookKind kind, LookFile& file,
                  std::vector<Items::ItemDataIssue>& issues);

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

// Whether a look has a field each item sets (design L12).
bool SetsEachItem(const LookDefinition& look);
// The names of the values each item can set for the look.
std::set<std::string> EachItemNames(const LookDefinition& look);
// The look as an item that sets values of it draws it (design L12): each field that each item sets takes the item's
// value of its name (ItemModelDefinition::renderStyleValues), read as the field reads it in the file; a field whose
// value the item doesn't set keeps the look's. A value that can't be the field's is a fatal problem in `issues`, and
// the item then draws with the look's own values.
LookDefinition ResolveForItem(const LookDefinition& look, const std::map<std::string, std::string>& values,
                              const std::string& source, std::vector<Items::ItemDataIssue>& issues,
                              LookKind kind = LookKind::RenderStyle);

// The text of a render style file: the folders (each on a line), looks sorted by name, fields in a fixed order, values
// that are the item's left out.
std::string WriteRenderStylesJson(std::vector<LookDefinition> looks, std::span<const Colors::ColorFolder> folders = {});
// The text of a look file of that kind, as WriteRenderStylesJson; an item effect writes its item values before the
// drawing, one entry a line, and its object light.
std::string WriteLookFileJson(LookKind kind, std::vector<LookDefinition> looks,
                              std::span<const Colors::ColorFolder> folders = {});
} // namespace Data::Looks
