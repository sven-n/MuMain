#include "stdafx.h"
#include "RmlTooltip.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltipPlacement.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>

#include <algorithm>
#include <optional>
#include <string>

namespace UI::RmlBridge::Tooltip
{
    namespace
    {
        // Bool-per-color flags, matching every other themed document's own data-class-* binding
        // convention (RmlUi's DataModelConstructor has no enum-to-class mapping) -- White is the
        // implicit default when no flag is set, same convention MainFrameWindow's
        // SkillTooltipLineEntry already uses.
        struct TooltipLineEntry
        {
            Rml::String text;
            bool colorBlue = false;
            bool colorGray = false;
            bool colorRed = false;
            bool colorYellow = false;
            bool colorGreen = false;
            bool colorPurple = false;
            bool colorRedPurple = false;
            bool colorViolet = false;
            bool colorOrange = false;
            bool highlightDarkRed = false;
            bool highlightDarkBlue = false;
            bool highlightDarkYellow = false;
            bool highlightGreenBlue = false;
            bool bold = false;
            bool isHalfSpacer = false;
            bool isFullSpacer = false;
            // The native row box and the space to the next row (NativeMetrics()).
            float heightPx = 0.0f;
            float gapPx = 0.0f;

            bool operator==(const TooltipLineEntry&) const = default;
        };

        struct TooltipRmlModel
        {
            std::vector<TooltipLineEntry> lines;
            float posX = 0.0f;
            float posY = 0.0f;
            bool centerText = false;
            // What the native text renderer would use under the caller's transform -- the legacy
            // theme's tooltip.rml binds these to match RenderTipTextList(); others may ignore them.
            float textPx = 0.0f;
            float borderPx = 0.0f;
            float paddingPx = 0.0f;
            float fixedWidthPx = 0.0f;
            bool buttonHint = false;
        };

        // RenderTipTextList() (ZzzInventory.cpp) layout: each row is one text height tall and the
        // next row starts 1.1 heights below it (a half spacer: half a height), the box is the
        // widest line plus 4 units (2 per side) with no vertical padding, framed by a 1-unit border.
        constexpr float kNativeRowAdvance = 1.1f;
        constexpr float kNativeHalfSpacerFraction = 0.5f;
        constexpr float kNativePaddingUnits = 2.0f;
        constexpr float kNativeBorderUnits = 1.0f;
        // A button's hover text (CNewUIButton::Render()): a box 6 units wider than the text, no frame.
        constexpr float kButtonHintPaddingUnits = 3.0f;

        // Row heights come from the native text renderer itself (MeasureText's logical height,
        // as RenderTipTextList() uses), not from RmlUi's font metrics, which round differently.
        void ApplyNativeMetrics(TooltipRmlModel& model, const Config& config)
        {
            const UI::Scaling::Transform transform = config.transform.value_or(UI::Scaling::GetActiveTransform());
            model.centerText = (config.textAlign == Config::TextAlign::Center);
            model.fixedWidthPx = config.fixedWidth * transform.scaleX;
            model.textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
            model.buttonHint = (config.box == Config::Box::ButtonHint);
            model.borderPx = model.buttonHint ? 0.0f : kNativeBorderUnits * transform.scaleX;
            model.paddingPx = (model.buttonHint ? kButtonHintPaddingUnits : kNativePaddingUnits) * transform.scaleX;

            // The native line height follows the active transform: measure under the chosen one.
            float normalHeight = 0.0f;
            float boldHeight = 0.0f;
            {
                const UI::Scaling::ScopedActiveTransform measureScope(transform);
                normalHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal));
                boldHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold));
            }

            for (TooltipLineEntry& line : model.lines)
            {
                const float rowHeight = (line.bold ? boldHeight : normalHeight) * transform.scaleY;
                const float advance = rowHeight * kNativeRowAdvance;
                if (line.isHalfSpacer || line.isFullSpacer)
                {
                    line.heightPx = line.isHalfSpacer ? advance * kNativeHalfSpacerFraction : advance;
                    line.gapPx = 0.0f;
                }
                else
                {
                    line.heightPx = rowHeight;
                    line.gapPx = advance - rowHeight;
                }
            }
        }

        Owner s_CurrentOwner = nullptr;
        // The showing tooltip's refreshEachFrame, and whether Show() ran since ExpireUnrefreshed().
        bool s_RefreshEachFrame = false;
        bool s_Refreshed = false;

        // #tooltip_panel as last measured: its border box and the frame edges the anchor sits inside.
        struct Measurement
        {
            Rml::Vector2f size;
            Rml::Vector2f frameTopLeft;
            float frameBottom = 0.0f;
            // The context's dp ratio the panel was laid out at.
            float dpRatio = 0.0f;
        };
        // Valid while the document holds the model's current content: hiding only changes its
        // visibility, which leaves the layout as it was. Cleared when the document is rebuilt.
        std::optional<Measurement> s_Measured;
        // The width last set on #tooltip_panel; empty once the document is rebuilt.
        std::string s_PanelWidth;

        void BindModel(Rml::DataModelConstructor& c, TooltipRmlModel& model)
        {
            auto line = c.RegisterStruct<TooltipLineEntry>();
            line.RegisterMember("text", &TooltipLineEntry::text);
            line.RegisterMember("color_blue", &TooltipLineEntry::colorBlue);
            line.RegisterMember("color_gray", &TooltipLineEntry::colorGray);
            line.RegisterMember("color_red", &TooltipLineEntry::colorRed);
            line.RegisterMember("color_yellow", &TooltipLineEntry::colorYellow);
            line.RegisterMember("color_green", &TooltipLineEntry::colorGreen);
            line.RegisterMember("color_purple", &TooltipLineEntry::colorPurple);
            line.RegisterMember("color_redpurple", &TooltipLineEntry::colorRedPurple);
            line.RegisterMember("color_violet", &TooltipLineEntry::colorViolet);
            line.RegisterMember("color_orange", &TooltipLineEntry::colorOrange);
            line.RegisterMember("highlight_darkred", &TooltipLineEntry::highlightDarkRed);
            line.RegisterMember("highlight_darkblue", &TooltipLineEntry::highlightDarkBlue);
            line.RegisterMember("highlight_darkyellow", &TooltipLineEntry::highlightDarkYellow);
            line.RegisterMember("highlight_greenblue", &TooltipLineEntry::highlightGreenBlue);
            line.RegisterMember("bold", &TooltipLineEntry::bold);
            line.RegisterMember("is_half_spacer", &TooltipLineEntry::isHalfSpacer);
            line.RegisterMember("is_full_spacer", &TooltipLineEntry::isFullSpacer);
            line.RegisterMember("height_px", &TooltipLineEntry::heightPx);
            line.RegisterMember("gap_px", &TooltipLineEntry::gapPx);
            c.RegisterArray<std::vector<TooltipLineEntry>>();

            c.Bind("lines", &model.lines);
            c.Bind("pos_x", &model.posX);
            c.Bind("pos_y", &model.posY);
            c.Bind("center_text", &model.centerText);
            c.Bind("text_px", &model.textPx);
            c.Bind("border_px", &model.borderPx);
            c.Bind("padding_px", &model.paddingPx);
            c.Bind("fixed_width_px", &model.fixedWidthPx);
            c.Bind("button_hint", &model.buttonHint);
        }

        // A theme switch leaves it hidden and ownerless: whichever caller has the hover shows it
        // again on its next hover check.
        void OnReloaded();

        UI::RmlBridge::ThemedView<TooltipRmlModel> s_View{
            "tooltip", BindModel, {{"Data/Interface/RmlUi/tooltip.rml"}},
            {.afterBuild = [] { s_Measured.reset(); s_PanelWidth.clear(); }, .afterReload = [] { OnReloaded(); }}};

        void OnReloaded()
        {
            s_View.Hide();
            s_CurrentOwner = nullptr;
            s_Measured.reset();
        }

        TooltipLineEntry ToLineEntry(const Line& line)
        {
            TooltipLineEntry entry;
            entry.text = line.text;
            entry.bold = line.bold;
            entry.isHalfSpacer = (line.kind == Line::Kind::HalfSpacer);
            entry.isFullSpacer = (line.kind == Line::Kind::FullSpacer);

            switch (line.color)
            {
            case LineColor::Blue: entry.colorBlue = true; break;
            case LineColor::Gray: entry.colorGray = true; break;
            case LineColor::Red: entry.colorRed = true; break;
            case LineColor::Yellow: entry.colorYellow = true; break;
            case LineColor::Green: entry.colorGreen = true; break;
            case LineColor::Purple: entry.colorPurple = true; break;
            case LineColor::RedPurple: entry.colorRedPurple = true; break;
            case LineColor::Violet: entry.colorViolet = true; break;
            case LineColor::Orange: entry.colorOrange = true; break;
            case LineColor::DarkRedHighlight: entry.highlightDarkRed = true; break;
            case LineColor::DarkBlueHighlight: entry.highlightDarkBlue = true; break;
            case LineColor::DarkYellowHighlight: entry.highlightDarkYellow = true; break;
            case LineColor::GreenBlueHighlight: entry.highlightGreenBlue = true; break;
            case LineColor::White: default: break;
            }
            return entry;
        }

        // Copies `next` into the model, marking only the fields whose values changed. True when any did.
        bool UpdateContent(TooltipRmlModel& model, TooltipRmlModel&& next)
        {
            bool changed = false;
            const auto assign = [&changed](auto& field, auto&& value, const char* name) {
                if (field == value)
                    return;
                field = std::move(value);
                s_View.Binder().MarkDirty(name);
                changed = true;
            };
            assign(model.lines, std::move(next.lines), "lines");
            assign(model.centerText, next.centerText, "center_text");
            assign(model.textPx, next.textPx, "text_px");
            assign(model.borderPx, next.borderPx, "border_px");
            assign(model.paddingPx, next.paddingPx, "padding_px");
            assign(model.fixedWidthPx, next.fixedWidthPx, "fixed_width_px");
            assign(model.buttonHint, next.buttonHint, "button_hint");
            return changed;
        }

        // Lays the panel out for the model's content and measures it. The context update creates the
        // data-for rows with their resolved fonts; the width they need is then set explicitly and only
        // this document is laid out again.
        Measurement Measure(Rml::Context& context, const Config& config)
        {
            context.Update();

            // #tooltip_panel must not rely on shrink-to-fit width: this build's box-width computation
            // for an absolutely-positioned block with multiple block children undersizes it (the same
            // family of bug engine-findings.md documents for a single pre-line text node -- confirmed
            // in practice by main_frame.rcss's #skill_tooltip needing an explicit, if fixed, width for
            // exactly this reason). A tooltip's width genuinely varies with content (an item tooltip's
            // longest line is nothing like a skill tooltip's), so a fixed width isn't an option here --
            // measure the real per-line text width via RmlUi's own font engine (the same one that will
            // actually draw it, so this can't drift from the real render the way a native GDI
            // measurement transplanted onto RmlUi's own font metrics could) and set an explicit `width`
            // from that instead of trusting auto-sizing.
            Measurement measured;
            measured.dpRatio = context.GetDensityIndependentPixelRatio();
            Rml::Element* panel = s_View.Document()->GetElementById("tooltip_panel");
            if (!panel)
                return measured;

            float maxLineWidth = 0.0f;
            const int lineCount = panel->GetNumChildren();
            for (int i = 0; i < lineCount && i < static_cast<int>(config.lines.size()); ++i)
            {
                const Line& line = config.lines[static_cast<size_t>(i)];
                if (line.kind != Line::Kind::Text)
                    continue; // spacer lines render no text -- nothing to measure.
                if (Rml::Element* lineElement = panel->GetChild(i))
                    maxLineWidth = std::max(maxLineWidth, static_cast<float>(Rml::ElementUtilities::GetStringWidth(lineElement, line.text)));
            }
            // +1px slack -- GetStringWidth() is a font-metrics estimate, not a guarantee against
            // sub-pixel rounding wrapping the very last character early.
            const std::string width = std::to_string(static_cast<int>(maxLineWidth) + 1) + "px";
            if (width != s_PanelWidth)
            {
                panel->SetProperty("width", width);
                s_PanelWidth = width;
            }

            // Lay out again at that width, so the box below has the final wrapped height.
            s_View.Document()->UpdateDocument();

            // #tooltip_panel's own box, not the document's: the document body spans the viewport.
            const Rml::Box& box = panel->GetBox();
            measured.size = box.GetSize(Rml::BoxArea::Border);
            measured.frameTopLeft.x = box.GetEdge(Rml::BoxArea::Border, Rml::BoxEdge::Left);
            measured.frameTopLeft.y = box.GetEdge(Rml::BoxArea::Border, Rml::BoxEdge::Top);
            measured.frameBottom = box.GetEdge(Rml::BoxArea::Border, Rml::BoxEdge::Bottom);
            return measured;
        }

        // Top-left of the panel for the anchor, kept fully inside the viewport.
        Rml::Vector2f Place(const Config& config, const Measurement& measured, Rml::Vector2i viewport)
        {
            // The anchor places the panel's inner (padding) box; a theme's frame lies outside it, as
            // RenderTipTextList() draws its 1-unit frame around the box it anchored.
            const Rml::Vector2f size = measured.size;
            float left = config.centerHorizontally ? (config.anchorX - size.x * 0.5f) : (config.anchorX - measured.frameTopLeft.x);
            const bool above = (config.anchor == AnchorPoint::AboveLeft);
            // The anchors as the panel's border box edges: above meets its bottom, below its top.
            const auto edge = [&](float anchorY, bool growsUp)
            { return growsUp ? anchorY + measured.frameBottom : anchorY - measured.frameTopLeft.y; };
            std::optional<float> flipEdge;
            if (config.flipAnchorY)
                flipEdge = edge(*config.flipAnchorY, !above);
            const float top = TooltipPlacement::Top(edge(config.anchorY, above), above,
                                                    flipEdge ? &*flipEdge : nullptr, size.y,
                                                    static_cast<float>(viewport.y));

            // Clamped on all four sides to the real viewport (this document has no parent transform),
            // not REFERENCE_WIDTH/HEIGHT. Wider or taller than the viewport sticks to its left/top.
            left = std::max(0.0f, std::min(left, static_cast<float>(viewport.x) - size.x));
            return {left, top};
        }
    }

    void Show(const Config& config, Owner owner)
    {
        if (!RmlUiRuntime::Instance().IsCreated() || config.lines.empty())
            return;

        if (!s_View.Ensure())
            return;

        s_CurrentOwner = owner;
        s_RefreshEachFrame = config.refreshEachFrame;
        s_Refreshed = true;

        TooltipRmlModel next;
        next.lines.reserve(config.lines.size());
        for (const Line& line : config.lines)
            next.lines.push_back(ToLineEntry(line));
        ApplyNativeMetrics(next, config);

        auto& model = s_View.Binder().GetModel();
        Rml::ElementDocument* document = s_View.Document();
        Rml::Context& context = *document->GetContext();
        const bool contentChanged = UpdateContent(model, std::move(next));
        if (!document->IsVisible())
            document->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
        if (contentChanged || !s_Measured || s_Measured->dpRatio != context.GetDensityIndependentPixelRatio())
            s_Measured = Measure(context, config);

        const Rml::Vector2f position = Place(config, *s_Measured, context.GetDimensions());
        if (model.posX != position.x)
        {
            model.posX = position.x;
            s_View.Binder().MarkDirty("pos_x");
        }
        if (model.posY != position.y)
        {
            model.posY = position.y;
            s_View.Binder().MarkDirty("pos_y");
        }
    }

    void Hide(Owner owner)
    {
        if (!s_View.Document())
            return;
        // See Owner's own comment (RmlTooltip.h) -- only actually hide if this caller (or an
        // ownerless caller) is the one the tooltip is currently showing for, so a not-hovered
        // caller's own per-frame Hide() can't clobber a different caller's legitimate Show() from
        // earlier the same frame. Bug fixed here: the old condition also required
        // `s_CurrentOwner != nullptr` before protecting anything, so a NON-null-owner Hide() (e.g.
        // MainFrameWindow's own `Hide(g_pSkillList)`, called every frame no skill hotkey is
        // hovered) always fell through to hiding unconditionally whenever the current owner
        // happened to be nullptr (any ownerless caller, e.g. MyInventory's Set/Socket tooltip) --
        // it's the CALLER's own owner that must be nullptr (or match) to win, not the current
        // owner's.
        if (owner != nullptr && owner != s_CurrentOwner)
            return;
        s_View.Document()->Hide();
        s_CurrentOwner = nullptr;
    }

    void ExpireUnrefreshed()
    {
        Rml::ElementDocument* document = s_View.Document();
        if (document && document->IsVisible() && s_RefreshEachFrame && !s_Refreshed)
            Hide(s_CurrentOwner);
        s_Refreshed = false;
    }
}
