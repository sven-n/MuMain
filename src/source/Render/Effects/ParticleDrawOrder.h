#pragma once

#include <cstddef>

// Builds the order particle draws are submitted in.
//
// Particles are stored in a fixed slot array and were historically drawn in slot order,
// which is allocation order -- textures interleave, so the ImmediateRenderer's batch
// (GLP-19) breaks on nearly every sprite and each one becomes its own draw call. Grouping
// consecutive draws by texture lets those batches actually merge.
//
// Reordering draws is only sound where the pixels cannot depend on the order, so this
// never reorders freely: the caller classifies each particle, and only particles marked
// reorderable are grouped, and only within a stretch that contains no barrier. Everything
// else keeps its original relative position.
namespace Render::Effects::DrawOrder
{
    // One particle's place in the submission order, as classified by the caller.
    struct Entry
    {
        // Index into the caller's particle array. Carried through untouched -- some draw
        // cases derive their look from the slot, so it must stay bound to the particle.
        int particleIndex;

        // Texture this particle binds. Particles that share it can merge into one batch,
        // so it is what reorderable stretches get grouped by.
        int textureKey;

        // False for anything whose pixels could change if it moved relative to its
        // neighbours: a non-additive blend mode (additive accumulation is commutative,
        // alpha blending and alpha testing are not), a draw case that switches blend
        // mode for itself, or a case that toggles the sticky depth-test state and so
        // changes how every particle after it is drawn.
        bool reorderable;
    };

    // Reorders entries in place: each maximal stretch of reorderable entries is grouped by
    // textureKey, and non-reorderable entries stay exactly where they are, acting as
    // barriers that no draw may cross. Grouping is stable, so particles sharing a texture
    // keep their relative order within the stretch.
    //
    // With every entry non-reorderable this is a no-op, which is what makes the classifier
    // free to be as conservative as it likes.
    void GroupByTexture(Entry* entries, size_t count);
}
