#pragma once

#include <string>

namespace mu::ui::window
{
    // Hover tooltip attachable to any native widget or bare rect; owns no position of its own --
    // Render() takes the anchor rect each frame and draws only if the mouse is within it.
    // For anything with an RmlUi presentation, use base.rcss's `.tooltip` convention instead.
    class CTooltip
    {
    public:
        ~CTooltip();

        void SetText(std::wstring text);
        // Slot overload: stores a pointer to an I18N string variable so the cached text refreshes on locale change.
        void SetText(const wchar_t* const* textSlot);
        void SetTextColor(unsigned int color);
        void SetFont(HFONT font);
        // true = anchor above the rect passed to Render() instead of the default (below).
        void SetAnchorAbove(bool above);

        bool HasText() const { return !m_text.empty(); }

        // Renders if the mouse is within [x, y, width, height]; offsetX/offsetY nudge the draw position. No-op if empty.
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
