#pragma once

#include "Data/GameData/ModelData/ModelFiles.h"

#include <filesystem>
#include <map>
#include <string>

class BMD;

namespace Render::Models::Gltf
{
// Replaces the bones and meshes of `model` - opened from its BMD, textures
// not yet opened, its data not shared - with those of the glTF binary `file`
// and attaches the file's SkinnedModel
// (BMD::GltfModel). The BMD's actions stay: they are the MU timeline the
// file's clips are played on. `actions` names other clips than the default
// "actionNN" for some actions and `lod` when the file's LOD 1 and 2 are drawn
// (Data/Models/ModelFiles.json). LOD 1 and 2 are optional; each may need at
// most as many BMD meshes as LOD 0 (mesh i of a LOD stands in for BMD mesh i).
//
// Joints are matched to the BMD's bones by name (a dummy bone, which has no
// name in the BMD, is "dummy<index>"); the file's other joints are appended
// after them, parents first. Space: glTF (x, y, z) is MU (x, -z, y).
//
// Refuses the file - false, `model` unchanged, the reason in `reason` and in
// the render log - when it cannot be read, lacks a bone of the BMD, has more
// than MAX_BONES joints, is not one skin with inverse bind matrices, or breaks
// another rule of the conventions the loader depends on.
bool LoadGlbIntoBmd(BMD& model, const std::filesystem::path& file,
                    const std::map<int, Data::Models::ActionClip>& actions, std::string& reason,
                    const Data::Models::LodSettings& lod = {});
} // namespace Render::Models::Gltf
