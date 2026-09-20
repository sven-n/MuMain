#include "stdafx.h"
#include "RmlTooltip.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>

#include <algorithm>
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
        };

        struct TooltipRmlModel
        {
            std::vector<TooltipLineEntry> lines;
            float posX = 0.0f;
            float posY = 0.0f;
        };

        RmlModelBinder<TooltipRmlModel> s_RmlBinder;
        Rml::ElementDocument* s_pRmlDoc = nullptr;
        Owner s_CurrentOwner = nullptr;

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

        void BuildRmlUi()
        {
            const bool modelCreated = s_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "tooltip",
                [](Rml::DataModelConstructor& c, TooltipRmlModel& model)
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
                    c.RegisterArray<std::vector<TooltipLineEntry>>();

                    c.Bind("lines", &model.lines);
                    c.Bind("pos_x", &model.posX);
                    c.Bind("pos_y", &model.posY);
                });

            if (modelCreated)
                s_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                    "Data/Interface/RmlUi/tooltip.rml");
        }
    }

    void Show(const Config& config, Owner owner)
    {
        if (!RmlUiRuntime::Instance().IsCreated() || config.lines.empty())
            return;

        if (!s_pRmlDoc)
            BuildRmlUi();
        if (!s_pRmlDoc)
            return;

        s_CurrentOwner = owner;

        // config.anchorX/Y are already real screen pixels -- see RmlTooltip.h's own comment for why
        // this document doesn't convert them itself (it used to, via the ambient
        // UI::Scaling::GetActiveTransform(), which broke the skill-hotkey tooltip: its anchor is
        // meaningful only relative to MainFrameWindow's own hand-rolled BottomHudCenterTransform,
        // not whatever transform happens to be ambient during MainFrameWindow::Update()).
        const float screenAnchorX = config.anchorX;
        const float screenAnchorY = config.anchorY;

        auto& model = s_RmlBinder.GetModel();
        model.lines.clear();
        model.lines.reserve(config.lines.size());
        for (const Line& line : config.lines)
            model.lines.push_back(ToLineEntry(line));

        // First pass: a reasonable guess so layout has something sane to measure. Growing upward
        // needs the real height to place the bottom edge at anchorY, which isn't known yet -- use
        // anchorY as a placeholder top for that case too, corrected below once measured.
        model.posX = screenAnchorX;
        model.posY = screenAnchorY;
        s_RmlBinder.MarkDirty("lines");
        s_RmlBinder.MarkDirty("pos_x");
        s_RmlBinder.MarkDirty("pos_y");

        s_pRmlDoc->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);

        // Force layout now so the .tt-line children data-for just created actually exist, with
        // their real resolved font (family/size/weight, including .bold) applied -- needed for the
        // width measurement below. Context::Update() only dispatches hover/click events on real
        // input-state transitions and otherwise just re-resolves data-model/layout state, so
        // calling it again here (and again below) with no new input in between doesn't double-fire
        // anything, it just makes each successive change visible to the next read/write in this
        // same function, all before the frame's own Update()/Render() pass runs.
        Rml::Context* context = RmlUiRuntime::Instance().GetContext();
        context->Update();

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
        Rml::Element* panel = s_pRmlDoc->GetElementById("tooltip_panel");
        float maxLineWidth = 0.0f;
        if (panel)
        {
            const int lineCount = panel->GetNumChildren();
            for (int i = 0; i < lineCount && i < static_cast<int>(config.lines.size()); ++i)
            {
                if (config.lines[static_cast<size_t>(i)].kind != Line::Kind::Text)
                    continue; // spacer lines render no text -- nothing to measure.
                if (Rml::Element* lineElement = panel->GetChild(i))
                {
                    const float lineWidth =
                        static_cast<float>(Rml::ElementUtilities::GetStringWidth(lineElement, config.lines[static_cast<size_t>(i)].text));
                    maxLineWidth = std::max(maxLineWidth, lineWidth);
                }
            }
            // +1px slack -- GetStringWidth() is a font-metrics estimate, not a guarantee against
            // sub-pixel rounding wrapping the very last character early.
            panel->SetProperty("width", std::to_string(static_cast<int>(maxLineWidth) + 1) + "px");
        }

        // Re-layout now that the panel has a real, explicit width, so the box below reflects the
        // final wrapped height at that width (relevant once any single line is long enough that a
        // future consumer's content wraps rather than fitting on one line).
        context->Update();

        // #tooltip_panel's own box, NOT s_pRmlDoc->GetBox() -- confirmed via runtime diagnostic
        // that the document's own outer body box reports the full viewport size here (1024x768,
        // not the actual small tooltip), not the shrink-wrapped panel size a plain element would
        // give. Since that always exceeded the viewport, the clamp below unconditionally forced
        // left/top back to 0 -- the real cause of every item/skill tooltip landing at the top-left
        // corner regardless of the (correctly computed) anchor.
        const Rml::Vector2f size = panel ? panel->GetBox().GetSize(Rml::BoxArea::Border) : Rml::Vector2f(0.0f, 0.0f);

        float left = config.centerHorizontally ? (screenAnchorX - size.x * 0.5f) : screenAnchorX;
        float top = (config.anchor == AnchorPoint::AboveLeft) ? (screenAnchorY - size.y) : screenAnchorY;

        // Clamp on all four sides -- every prior mechanism clamped at most horizontally; hovering
        // something near any screen edge must not clip the tooltip. Bounds are the real viewport
        // (this document has no parent transform of its own -- see the anchor conversion above),
        // not REFERENCE_WIDTH/HEIGHT.
        const Rml::Vector2i viewport = context->GetDimensions();
        if (left < 0.0f)
            left = 0.0f;
        if (left + size.x > static_cast<float>(viewport.x))
            left = static_cast<float>(viewport.x) - size.x;
        if (left < 0.0f)
            left = 0.0f; // wider than the viewport itself -- clamp to the left edge, not negative.

        if (top < 0.0f)
            top = 0.0f;
        if (top + size.y > static_cast<float>(viewport.y))
            top = static_cast<float>(viewport.y) - size.y;
        if (top < 0.0f)
            top = 0.0f;

        model.posX = left;
        model.posY = top;
        s_RmlBinder.MarkDirty("pos_x");
        s_RmlBinder.MarkDirty("pos_y");
    }

    void Hide(Owner owner)
    {
        if (!s_pRmlDoc)
            return;
        // See Owner's own comment (RmlTooltip.h) -- only actually hide if this caller (or an
        // ownerless caller) is the one the tooltip is currently showing for, so a not-hovered
        // caller's own per-frame Hide() can't clobber a different caller's legitimate Show() from
        // earlier the same frame.
        if (owner != nullptr && s_CurrentOwner != nullptr && owner != s_CurrentOwner)
            return;
        s_pRmlDoc->Hide();
        s_CurrentOwner = nullptr;
    }

    void ReloadRmlTheme()
    {
        if (!s_pRmlDoc)
            return;

        Rml::Context* context = RmlUiRuntime::Instance().GetContext();
        s_RmlBinder.Destroy(context);
        context->UnloadDocument(s_pRmlDoc);
        s_pRmlDoc = nullptr;
        s_CurrentOwner = nullptr;

        BuildRmlUi();
        // Left hidden -- whichever caller currently has the mouse hovered will call Show() again
        // on its own very next hover-detection tick, same as every other themed document's reload.
    }
}
