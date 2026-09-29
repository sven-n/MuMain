#pragma once

#include <cstdint>
#include <string>

// A scoped seam that turns the native 2D primitives' draws into records: while an
// Overlay2DRecordScope is active, CUIRenderTextSDLTtf::RenderText(), RenderColorQuadARGB() and
// RenderBitmap() hand the rectangles, colours and texts they already computed to the active recorder
// instead of drawing them. Lets a window whose drawing is spread over shared legacy code (the world
// labels CNameWindow draws through the chat balloons, shop titles and item names) move that drawing
// to RmlUi without a second copy of the positioning rules. Positions are physical window pixels with
// a top-left origin; colours keep each primitive's own packing (documented per field).
namespace Render::Renderer
{
struct RecordedText
{
    float boxX = 0.0f;
    float boxY = 0.0f;
    float boxWidth = 0.0f;
    float boxHeight = 0.0f;
    std::uint32_t backColor = 0; // ABGR (mu::sdlttf::PackColorDWORD); alpha 0 draws no box
    float textX = 0.0f;          // the text's left edge (alignment applied); its top is boxY
    float textPixelSize = 0.0f;  // font pixel size, shrink-to-box included
    float lineHeight = 0.0f;
    bool bold = false;
    std::uint32_t textColor = 0; // ABGR
    std::string utf8;
};

// How a recorded quad combines with what is behind it: the native blend state it was drawn under.
enum class RecordedBlend
{
    Alpha,    // EnableAlphaTest() and most modes: ordinary alpha blending
    Additive, // EnableAlphaBlend(): BlendMode::Glow (ONE, ONE), the colour is added
    Opaque,   // DisableAlphaBlend(): blending off, alpha ignored
};

struct RecordedQuad
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::uint32_t argb = 0;
    RecordedBlend blend = RecordedBlend::Alpha;
};

struct RecordedBitmap
{
    const wchar_t* fileName = nullptr; // CGlobalBitmap file name, e.g. L"Data\\Interface\\x.tga"
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float sourceX = 0.0f; // source rectangle in texture pixels
    float sourceY = 0.0f;
    float sourceWidth = 0.0f;
    float sourceHeight = 0.0f;
    float alpha = 1.0f;
};

class IOverlay2DRecorder
{
public:
    virtual ~IOverlay2DRecorder() = default;
    virtual void RecordText(const RecordedText& text) = 0;
    virtual void RecordQuad(const RecordedQuad& quad) = 0;
    virtual void RecordBitmap(const RecordedBitmap& bitmap) = 0;
};

// The recorder the primitives report to, or nullptr (draw normally).
IOverlay2DRecorder* ActiveOverlay2DRecorder();

class Overlay2DRecordScope
{
public:
    explicit Overlay2DRecordScope(IOverlay2DRecorder* recorder);
    ~Overlay2DRecordScope();
    Overlay2DRecordScope(const Overlay2DRecordScope&) = delete;
    Overlay2DRecordScope& operator=(const Overlay2DRecordScope&) = delete;

private:
    IOverlay2DRecorder* m_previous;
};
} // namespace Render::Renderer
