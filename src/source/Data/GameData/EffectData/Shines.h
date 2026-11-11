#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The shines below +3 (Data/Effects/Shines.json, design L30):
//
//   { "formatVersion": 1, "shines": [
//     { "name": "seal", "light": 0.9, "passes": [
//       { "kind": "reflection", "motion": "fastSweep", "opacity": 1.5, "brightness": 1.5 },
//       { "kind": "reflection", "motion": "swirl" } ] } ] }
//
// The glow of an item model names one ("glow": {"shineBelowPlus3": "seal"}). At +0 to +2 the item is then drawn in
// its light times "light" (left out: in the light the model has), and after it each pass draws glowing in the item's
// shine color, on its shine meshes: a pass of kind "texture" with the meshes' own texture, one of kind "reflection"
// with a reflection that moves so. Its light is the shine color times the brightness and the opacity.
namespace Data::Effects
{
struct ShinePass
{
    // A reflection (with its motion), or the meshes' own texture.
    bool reflection = true;
    Looks::Motion motion = Looks::Motion::SlowDrift;
    double opacity = 1.0;
    double brightness = 1.0;

    bool operator==(const ShinePass&) const = default;
};

struct Shine
{
    std::string name;
    // The item's light is multiplied by this before the model is drawn; none: the light stays as it is.
    std::optional<double> light;
    std::vector<ShinePass> passes;

    bool operator==(const Shine&) const = default;
};

constexpr int ShinesFormatVersion = 1;

// The file, relative to the client folder.
constexpr const char* ShinesFile = "Data/Effects/Shines.json";

// Reads the shines of the file into `shines`.
void ReadShinesJson(std::string_view text, const std::string& source, std::vector<Shine>& shines,
                    std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these shines, in their order, one pass a line.
std::string WriteShinesJson(std::span<const Shine> shines);

// The shines by name. Built once at startup with the item models.
class ShineList
{
public:
    static ShineList& GetInstance();

    void Build(std::span<const Shine> shines);

    // How often the list was built: what keeps a shine of it must look it up again when this changes.
    int GetVersion() const
    {
        return m_version;
    }

    // Returns nullptr for names that are not in the list.
    const Shine* Find(std::string_view name) const;

    std::span<const Shine> GetShines() const
    {
        return m_shines;
    }

private:
    std::vector<Shine> m_shines;
    int m_version = 0;
};
} // namespace Data::Effects

#define g_Shines Data::Effects::ShineList::GetInstance()
