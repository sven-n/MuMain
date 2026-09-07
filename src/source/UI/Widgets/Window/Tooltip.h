#pragma once

#include <string>

namespace mu::ui::window
{
    // A hover tooltip attachable to any native mu::ui::window widget (or a bare rect) --
    // extracted from mu::ui::window::CButton's own ChangeToolTipText()/Render() logic --
    // the strongest existing implementation of this idea in the codebase. This is the "native, transitional" side of
    // that document's Tooltip row -- for anything with an RmlUi presentation, use base.rcss's
    // `.tooltip` convention instead, not this class.
    //
    // Owns no position of its own: Render() takes the anchor rect (the owning widget's own
    // position/size) each frame and only draws if the mouse is currently within it, matching
    // CButton's prior inline behavior exactly.
    class CTooltip
    {
    public:
        ~CTooltip();

        void SetText(std::wstring text);
        // Slot overload: stores a pointer to an I18N::<Group>::<Identifier> variable so the
        // cached text refreshes automatically on locale change -- same idiom as
        // CButton::ChangeText(const wchar_t* const*)/ChangeToolTipText(const wchar_t* const*).
        void SetText(const wchar_t* const* textSlot);
        void SetTextColor(unsigned int color);
        void SetFont(HFONT font);
        // true = anchor above the rect passed to Render() instead of the default (below).
        void SetAnchorAbove(bool above);

        bool HasText() const { return !m_text.empty(); }

        // Renders the tooltip if the mouse is currently within [x, y, width, height] -- the same
        // anchor rect the owning widget itself hit-tests against. offsetX/offsetY nudge the
        // final draw position (CButton's own MoveTextTipPos() feature). No-op if empty.
        void Render(int x, int y, int width, int height, int offsetX = 0, int offsetY = 0) const;

    private:
        void EnsureLocaleObserver();
        static void OnLocaleChanged(void* ctx) noexcept;

        std::wstring m_text;
        const wchar_t* const* m_textSlot = nullptr;
        bool m_localeObserverRegistered = false;
        HFONT m_font = nullptr;
        unsigned int m_textColor = 0xffffffff;
        bool m_anchorAbove = false;
    };
}
