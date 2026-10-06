#pragma once

#include "Render/Renderer/Overlay2DRecorder.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <string>
#include <vector>

namespace Rml
{
class Element;
class ElementDocument;
class ElementText;
} // namespace Rml

namespace UI::Character
{
// CNameWindow's world labels in RmlUi: one background-context document (world_labels.rml) behind
// every other document, holding a pool of absolutely positioned elements filled in draw order from
// what the native label code draws while this layer records (Render::Renderer::Overlay2DRecordScope).
// Each frame only the properties that changed are written and the unused tail is hidden, so the cost
// follows the labels drawn, not the pool.
class WorldLabelLayer final : public Render::Renderer::IOverlay2DRecorder
{
public:
    ~WorldLabelLayer() override;

    // Loads the document (once; re-entrant) and registers for theme reloads. False when RmlUi is not
    // available, in which case the caller draws natively.
    bool Create();
    void Release();
    bool IsAvailable() const
    {
        return m_view.Document() != nullptr;
    }

    void BeginFrame();
    void EndFrame();
    void Hide();

    void RecordText(const Render::Renderer::RecordedText& text) override;
    void RecordQuad(const Render::Renderer::RecordedQuad& quad) override;
    void RecordBitmap(const Render::Renderer::RecordedBitmap& bitmap) override;

private:
    enum class SlotKind
    {
        Hidden,
        Text,
        Quad,
        Bitmap,
    };

    // One pooled label: a box (text background or solid quad), its text, or an image. The cached
    // values mirror what was last written to the elements.
    struct Slot
    {
        Rml::Element* box = nullptr;
        Rml::Element* text = nullptr;
        Rml::ElementText* textNode = nullptr;
        Rml::Element* image = nullptr;
        SlotKind kind = SlotKind::Hidden;
        float x = -1.0f, y = -1.0f, width = -1.0f, height = -1.0f;
        std::uint32_t boxColor = 1;
        bool additive = false;           // the box adds its colour (an additive-fill decorator)
        std::uint32_t additiveColor = 0; // ABGR of that fill

        float textOffset = -1.0f, textPixelSize = -1.0f, lineHeight = -1.0f;
        bool bold = false;
        std::uint32_t textColor = 1;
        std::string utf8;
        const wchar_t* imageFile = nullptr; // the recorded file name the src was built from
        float sourceX = -1.0f, sourceY = -1.0f, sourceWidth = -1.0f, sourceHeight = -1.0f;
        float imageAlpha = -1.0f;
    };

    void OnBuilt();
    Slot& NextSlot(SlotKind kind);
    void SetKind(Slot& slot, SlotKind kind);
    struct Rect
    {
        float x;
        float y;
        float width;
        float height;
    };
    void SetBox(Slot& slot, const Rect& rect, std::uint32_t abgr);
    void SetFill(Slot& slot, const Rect& rect, std::uint32_t abgr, Render::Renderer::RecordedBlend blend);
    void SetAdditive(Slot& slot, bool additive, std::uint32_t abgr);

    std::vector<Slot> m_slots;
    std::size_t m_used = 0;
    std::size_t m_shown = 0; // slots used by the last finished frame
    // In the background context only: the labels draw under every window.
    UI::RmlBridge::ThemedView<> m_view{{{"Data/Interface/RmlUi/world_labels.rml",
                                         [] { return RmlUiRuntime::Instance().GetBackgroundContext(); }}},
                                       {.stacking = UI::RmlBridge::ThemedStacking::Back,
                                        .afterBuild = [this] { OnBuilt(); }}};
};
} // namespace UI::Character
