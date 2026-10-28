#pragma once

#include "Data/GameData/ModelData/ModelFiles.h"

#include <filesystem>
#include <span>
#include <string>

class BMD;

// The model file table of the running game (Data/Models/ModelFiles.json, the
// developer overlay MU_DATA_OVERLAY first) and how an opened BMD gets the file
// the table names for it.
namespace Data::Models
{
// The table for MU_DATA_OVERLAY's value `overlayValue` (nullptr: none) and the
// game's Data/ directory `game`, the overlay's issues included.
[[nodiscard]] ModelFiles ReadModelFileTable(const char* overlayValue, const std::filesystem::path& game);

// Reads MU_DATA_OVERLAY and the game's table once, on the loading screen, and
// logs every issue. False, with the message that stops the start in
// `errorMessage`, when the table has errors (as for the effect data).
bool LoadModelFileTable(std::string& errorMessage);

// The table LoadModelFileTable read; no models before.
[[nodiscard]] const ModelFiles& GetModelFileTable();

// Replaces the bones and meshes of `model`, just opened from its BMD into
// Models[type], with those of the file `table` names for `type`
// (Render::Models::Gltf::LoadGlbIntoBmd). True when it did; a model without an
// entry, or whose file is refused (logged), keeps its BMD.
bool ApplyModelFile(BMD& model, int type, const ModelFiles& table);

// The texture name prefix of the images the model file of `entry` carries:
// its path below Data with backslashes ("Data\Monster\Monster14.glb").
[[nodiscard]] std::wstring ModelFileTextureName(const ModelFileEntry& entry);

// Opens the textures of Models[type] after ApplyModelFile replaced its meshes:
// by name (the meshes' extras.texture) from `subFolders` below Data\, then the
// images the file carries for the meshes that have one. A texture found
// nowhere is logged, never shown: the mesh draws without it.
void OpenModelFileTextures(int type, std::span<const std::wstring> subFolders, const ModelFiles& table);
} // namespace Data::Models
