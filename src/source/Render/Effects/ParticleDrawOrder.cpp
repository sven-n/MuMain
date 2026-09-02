#include "ParticleDrawOrder.h"

#include <algorithm>

namespace Render::Effects::DrawOrder
{
    namespace
    {
        void GroupStretch(Entry* entries, Entry* scratch, size_t count)
        {
            for (size_t width = 1; width < count; width *= 2)
            {
                for (size_t first = 0; first < count; first += width * 2)
                {
                    const size_t middle = std::min(first + width, count);
                    const size_t last = std::min(first + width * 2, count);
                    std::merge(entries + first, entries + middle, entries + middle, entries + last,
                        scratch + first, [](const Entry& left, const Entry& right)
                        {
                            return left.textureKey < right.textureKey;
                        });
                }
                std::copy_n(scratch, count, entries);
            }
        }
    }

    void GroupByTexture(Entry* entries, size_t count, Entry* scratch)
    {
        if (entries == nullptr || scratch == nullptr || count < 2) return;

        size_t stretchStart = 0;
        for (size_t i = 0; i <= count; i++)
        {
            const bool endOfStretch = (i == count) || !entries[i].reorderable;
            if (!endOfStretch) continue;

            // A stretch of one cannot be reordered into anything different.
            if (i - stretchStart >= 2)
            {
                GroupStretch(entries + stretchStart, scratch + stretchStart, i - stretchStart);
            }
            stretchStart = i + 1;
        }
    }
}
