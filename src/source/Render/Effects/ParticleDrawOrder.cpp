#include "ParticleDrawOrder.h"

#include <algorithm>

namespace Render::Effects::DrawOrder
{
    namespace
    {
        // Groups one stretch [first, last) of reorderable entries by texture.
        //
        // std::stable_sort, not sort: two particles sharing a texture must keep their
        // relative order. They are additive and so commutative in colour, but keeping the
        // order makes the result identical to the unsorted path rather than merely
        // equivalent, which is what makes an A/B comparison meaningful.
        void GroupStretch(Entry* first, Entry* last)
        {
            std::stable_sort(first, last, [](const Entry& a, const Entry& b)
            {
                return a.textureKey < b.textureKey;
            });
        }
    }

    void GroupByTexture(Entry* entries, size_t count)
    {
        if (entries == nullptr || count < 2) return;

        size_t stretchStart = 0;
        for (size_t i = 0; i <= count; i++)
        {
            const bool endOfStretch = (i == count) || !entries[i].reorderable;
            if (!endOfStretch) continue;

            // A stretch of one cannot be reordered into anything different.
            if (i - stretchStart >= 2)
            {
                GroupStretch(entries + stretchStart, entries + i);
            }
            stretchStart = i + 1;
        }
    }
}
