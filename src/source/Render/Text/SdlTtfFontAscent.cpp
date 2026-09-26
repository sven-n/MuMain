#include "SdlTtfFontAscent.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace Render::Text
{
namespace
{
// sfnt table directory and table field offsets, from the OpenType spec.
constexpr std::uint32_t kTagHead = 0x68656164; // 'head'
constexpr std::uint32_t kTagHhea = 0x68686561; // 'hhea'
constexpr std::uint32_t kTagOs2 = 0x4F532F32;  // 'OS/2'
constexpr std::int64_t kTableCountOffset = 4;
constexpr std::int64_t kTableRecordsOffset = 12;
constexpr std::int64_t kTableRecordSize = 16;
constexpr std::int64_t kTableRecordOffsetField = 8;
constexpr std::int64_t kHeadUnitsPerEmOffset = 18;
constexpr std::int64_t kHheaAscenderOffset = 4;
constexpr std::int64_t kOs2FsSelectionOffset = 62;
constexpr std::int64_t kOs2TypoAscenderOffset = 68;
// fsSelection bit 7: FreeType then takes the ascender from OS/2 instead of hhea.
constexpr std::uint16_t kUseTypoMetrics = 1u << 7;
constexpr std::int64_t kMissingTable = -1;

struct SfntTables
{
    std::int64_t head = kMissingTable;
    std::int64_t hhea = kMissingTable;
    std::int64_t os2 = kMissingTable;
};

[[nodiscard]] bool ReadU16At(SDL_IOStream* io, std::int64_t offset, std::uint16_t& value)
{
    return SDL_SeekIO(io, offset, SDL_IO_SEEK_SET) >= 0 && SDL_ReadU16BE(io, &value);
}

[[nodiscard]] bool ReadU32At(SDL_IOStream* io, std::int64_t offset, std::uint32_t& value)
{
    return SDL_SeekIO(io, offset, SDL_IO_SEEK_SET) >= 0 && SDL_ReadU32BE(io, &value);
}

[[nodiscard]] std::optional<SfntTables> FindSfntTables(SDL_IOStream* io)
{
    std::uint16_t tableCount = 0;
    if (!ReadU16At(io, kTableCountOffset, tableCount))
        return std::nullopt;

    SfntTables tables;
    for (std::uint16_t i = 0; i < tableCount; ++i)
    {
        const std::int64_t record = kTableRecordsOffset + i * kTableRecordSize;
        std::uint32_t tag = 0;
        std::uint32_t offset = 0;
        if (!ReadU32At(io, record, tag) || !ReadU32At(io, record + kTableRecordOffsetField, offset))
            return std::nullopt;

        if (tag == kTagHead)
            tables.head = offset;
        else if (tag == kTagHhea)
            tables.hhea = offset;
        else if (tag == kTagOs2)
            tables.os2 = offset;
    }
    return tables;
}

[[nodiscard]] bool UsesTypoMetrics(SDL_IOStream* io, const SfntTables& tables)
{
    std::uint16_t fsSelection = 0;
    return tables.os2 != kMissingTable && ReadU16At(io, tables.os2 + kOs2FsSelectionOffset, fsSelection) &&
           (fsSelection & kUseTypoMetrics) != 0;
}

// A read-only stream over a font file that returns `ascender` wherever the file
// stores the ascender FreeType reads.
struct AscenderPatchedStream
{
    SDL_IOStream* file = nullptr;
    SfntAscender ascender;
    std::uint8_t replacement[2] = {};
};

void OverlayAscender(const AscenderPatchedStream& stream, std::int64_t start, std::uint8_t* data, std::size_t length)
{
    const std::int64_t end = start + static_cast<std::int64_t>(length);
    for (int i = 0; i < stream.ascender.offsetCount; ++i)
    {
        for (std::int64_t byte = 0; byte < static_cast<std::int64_t>(sizeof(stream.replacement)); ++byte)
        {
            const std::int64_t position = stream.ascender.offsets[i] + byte;
            if (position >= start && position < end)
                data[position - start] = stream.replacement[byte];
        }
    }
}

Sint64 SDLCALL PatchedStreamSize(void* userdata)
{
    return SDL_GetIOSize(static_cast<AscenderPatchedStream*>(userdata)->file);
}

Sint64 SDLCALL PatchedStreamSeek(void* userdata, Sint64 offset, SDL_IOWhence whence)
{
    return SDL_SeekIO(static_cast<AscenderPatchedStream*>(userdata)->file, offset, whence);
}

size_t SDLCALL PatchedStreamRead(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
{
    const auto* stream = static_cast<AscenderPatchedStream*>(userdata);
    const Sint64 start = SDL_TellIO(stream->file);
    const size_t read = SDL_ReadIO(stream->file, ptr, size);
    if (read < size)
        *status = SDL_GetIOStatus(stream->file);
    if (start >= 0)
        OverlayAscender(*stream, start, static_cast<std::uint8_t*>(ptr), read);
    return read;
}

size_t SDLCALL PatchedStreamWrite(void*, const void*, size_t, SDL_IOStatus* status)
{
    *status = SDL_IO_STATUS_READONLY;
    return 0;
}

bool SDLCALL PatchedStreamFlush(void*, SDL_IOStatus*)
{
    return true;
}

bool SDLCALL PatchedStreamClose(void* userdata)
{
    const std::unique_ptr<AscenderPatchedStream> stream(static_cast<AscenderPatchedStream*>(userdata));
    return SDL_CloseIO(stream->file);
}

[[nodiscard]] std::optional<SfntAscender> ReadSfntAscenderFromFile(const char* path)
{
    SDL_IOStream* file = SDL_IOFromFile(path, "rb");
    if (file == nullptr)
        return std::nullopt;
    std::optional<SfntAscender> ascender = ReadSfntAscender(file);
    SDL_CloseIO(file);
    return ascender;
}

// The largest ascender (in font units) that FreeType rounds up to `ascentPx`
// pixels at `pointSize` (SDL_ttf renders at 72 dpi, so points are pixels).
[[nodiscard]] std::int16_t AscenderUnitsForPixels(const SfntAscender& ascender, float pointSize, int ascentPx)
{
    const double units = std::floor(static_cast<double>(ascentPx) * ascender.unitsPerEm / pointSize);
    return static_cast<std::int16_t>(std::clamp(units, static_cast<double>(std::numeric_limits<std::int16_t>::min()),
                                                static_cast<double>(std::numeric_limits<std::int16_t>::max())));
}

[[nodiscard]] TTF_Font* OpenFontWithAscender(const char* path, float pointSize, const SfntAscender& ascender)
{
    auto stream = std::make_unique<AscenderPatchedStream>();
    stream->file = SDL_IOFromFile(path, "rb");
    if (stream->file == nullptr)
        return nullptr;
    stream->ascender = ascender;
    const auto units = static_cast<std::uint16_t>(ascender.value);
    stream->replacement[0] = static_cast<std::uint8_t>(units >> 8);
    stream->replacement[1] = static_cast<std::uint8_t>(units & 0xFFu);

    SDL_IOStreamInterface patchedStream;
    SDL_INIT_INTERFACE(&patchedStream);
    patchedStream.size = PatchedStreamSize;
    patchedStream.seek = PatchedStreamSeek;
    patchedStream.read = PatchedStreamRead;
    patchedStream.write = PatchedStreamWrite;
    patchedStream.flush = PatchedStreamFlush;
    patchedStream.close = PatchedStreamClose;
    SDL_IOStream* io = SDL_OpenIO(&patchedStream, stream.get());
    if (io == nullptr)
    {
        SDL_CloseIO(stream->file);
        return nullptr;
    }

    stream.release(); // owned by `io` now; PatchedStreamClose frees it
    return TTF_OpenFontIO(io, true, pointSize);
}
} // namespace

std::optional<SfntAscender> ReadSfntAscender(SDL_IOStream* io)
{
    const std::optional<SfntTables> tables = FindSfntTables(io);
    if (!tables || tables->head == kMissingTable || tables->hhea == kMissingTable)
        return std::nullopt;

    std::uint16_t unitsPerEm = 0;
    std::uint16_t hheaAscender = 0;
    if (!ReadU16At(io, tables->head + kHeadUnitsPerEmOffset, unitsPerEm) || unitsPerEm == 0 ||
        !ReadU16At(io, tables->hhea + kHheaAscenderOffset, hheaAscender))
    {
        return std::nullopt;
    }

    SfntAscender ascender;
    ascender.unitsPerEm = unitsPerEm;
    ascender.value = static_cast<std::int16_t>(hheaAscender);
    ascender.offsets[ascender.offsetCount++] = tables->hhea + kHheaAscenderOffset;

    std::uint16_t typoAscender = 0;
    if (UsesTypoMetrics(io, *tables) && ReadU16At(io, tables->os2 + kOs2TypoAscenderOffset, typoAscender))
    {
        ascender.value = static_cast<std::int16_t>(typoAscender);
        ascender.offsets[ascender.offsetCount++] = tables->os2 + kOs2TypoAscenderOffset;
    }
    return ascender;
}

TTF_Font* OpenFontWithAscent(const char* path, float pointSize, int ascentPx)
{
    std::optional<SfntAscender> ascender = ReadSfntAscenderFromFile(path);
    if (!ascender)
        return TTF_OpenFont(path, pointSize);

    const std::int16_t units = AscenderUnitsForPixels(*ascender, pointSize, ascentPx);
    if (units == ascender->value)
        return TTF_OpenFont(path, pointSize);

    ascender->value = units;
    return OpenFontWithAscender(path, pointSize, *ascender);
}

TTF_Font* OpenFontWithAscentAtMost(const char* path, float pointSize, int maxAscentPx)
{
    TTF_Font* font = TTF_OpenFont(path, pointSize);
    if (font == nullptr || TTF_GetFontAscent(font) <= maxAscentPx)
        return font;

    TTF_CloseFont(font);
    return OpenFontWithAscent(path, pointSize, maxAscentPx);
}
} // namespace Render::Text
