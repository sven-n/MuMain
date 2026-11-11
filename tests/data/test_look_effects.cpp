#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/LookData/LookEffects.h"

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// The effects the Style Editor shows a look's layers as (design L35, L40).
namespace
{
using namespace Data::Looks;

const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;

const LookDefinition& ShippedLook(const RenderStylesLoadResult& shipped, std::string_view name)
{
    for (const LookDefinition& look : shipped.looks)
    {
        if (look.name == name)
        {
            return look;
        }
    }
    FAIL("no shipped look " << name);
    return shipped.looks.front();
}

const char* NameOf(EffectKind kind)
{
    switch (kind)
    {
    case EffectKind::Base:
        return "base";
    case EffectKind::Glow:
        return "glow";
    case EffectKind::Pulse:
        return "pulse";
    case EffectKind::Shine:
        return "shine";
    case EffectKind::Overlay:
        return "overlay";
    case EffectKind::Flowing:
        return "flowing";
    case EffectKind::Color:
        return "color";
    case EffectKind::Frames:
        return "frames";
    }
    return "?";
}
} // namespace

TEST_CASE("The shipped looks' layers read as the gallery's effects [looks][effects]")
{
    const RenderStylesLoadResult shipped = LoadRenderStyleFile(DataDirectory / "Effects");
    REQUIRE(!shipped.looks.empty());

    // The knight blade: its own texture, a pulsing glow on the blade, a glow, a shine that flows.
    const LookDefinition& knightBlade = ShippedLook(shipped, "knightBlade");
    REQUIRE(knightBlade.layers.size() == 4);
    CHECK(DescribeEffect(knightBlade.layers[0]).kind == EffectKind::Base);
    CHECK(DescribeEffect(knightBlade.layers[1]) == LayerEffect{.kind = EffectKind::Pulse});
    CHECK(DescribeEffect(knightBlade.layers[2]) == LayerEffect{.kind = EffectKind::Glow});
    CHECK(DescribeEffect(knightBlade.layers[3]) ==
          LayerEffect{.kind = EffectKind::Shine, .glows = true, .flows = true});
    // chromeMesh0: a glowing shine on mesh 0.
    const LookDefinition& chrome = ShippedLook(shipped, "chromeMesh0");
    CHECK(DescribeEffect(chrome.layers[1]) == LayerEffect{.kind = EffectKind::Shine, .glows = true});
    // The archangel staff's overlay texture glows and flows.
    const LookDefinition& staff = ShippedLook(shipped, "archangelStaff");
    bool overlay = false;
    for (const Layer& layer : staff.layers)
    {
        overlay =
            overlay || DescribeEffect(layer) == LayerEffect{.kind = EffectKind::Overlay, .glows = true, .flows = true};
    }
    CHECK(overlay);

    // How many layers read as each effect, for the design's L40; the base apart by what it draws.
    std::map<std::string, int> counts;
    for (const LookDefinition& look : shipped.looks)
    {
        for (const Layer& layer : look.layers)
        {
            const EffectKind kind = DescribeEffect(layer).kind;
            std::string name = NameOf(kind);
            if (kind == EffectKind::Base)
            {
                name += layer.blend == Blend::AsDrawing               ? " (as the drawing asks)"
                        : layer.meshes.IsAll() || layer.meshes.allBut ? " (every part)"
                                                                      : " (some parts)";
            }
            ++counts[name];
        }
    }
    std::string text;
    for (const auto& [name, count] : counts)
    {
        text += name + " " + std::to_string(count) + ", ";
    }
    MESSAGE("layers by effect: " << text);
    CHECK(counts["base (every part)"] > 0);

    // A layer of the item's own texture added over the others brightens it: a glow; with its opacity waving, a pulse.
    Layer added;
    added.blend = Blend::Glow;
    added.meshes.meshes = {1};
    CHECK(DescribeEffect(added) == LayerEffect{.kind = EffectKind::Glow});
    added.opacity = Shape{.kind = ShapeKind::Wave, .from = 0.2, .to = 0.8, .seconds = 2.0};
    CHECK(DescribeEffect(added) == LayerEffect{.kind = EffectKind::Pulse});
}

TEST_CASE("Each effect of the gallery starts with a layer that reads as it [looks][effects]")
{
    REQUIRE(GetGalleryEffects().size() == 7);
    for (const EffectKind kind : GetGalleryEffects())
    {
        const std::string name = NameOf(kind);
        CAPTURE(name);
        const Layer layer = MakeEffectLayer(kind, {2});
        // A flowing texture and frames move their part's texture as its glowing mesh, as the moved looks do: they glow.
        CHECK(DescribeEffect(layer) == LayerEffect{.kind = kind, .glows = NeedsOnePart(kind)});
        CHECK(layer.meshes.meshes == std::vector<int>{2});
        if (NeedsOnePart(kind))
        {
            CHECK(layer.glowMesh == GlowMesh(2));
        }
    }
    CHECK(NeedsOnePart(EffectKind::Flowing));
    CHECK(NeedsOnePart(EffectKind::Frames));
    CHECK_FALSE(NeedsOnePart(EffectKind::Glow));
    // On every mesh, a glow makes every mesh glow.
    const Layer everywhere = MakeEffectLayer(EffectKind::Glow, {});
    CHECK(everywhere.meshes.IsAll());
    CHECK(std::holds_alternative<AllGlowMeshes>(everywhere.glowMesh));
    CHECK(DescribeEffect(everywhere) == LayerEffect{.kind = EffectKind::Glow});
}
