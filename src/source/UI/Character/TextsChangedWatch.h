#pragma once

namespace UI::Character
{
// Notes a language switch or a refresh of the item texts ("Translate item names", the item editor's names and
// grammatical forms, a reload of the item data), for a window that keeps formatted copies of its texts: the
// character window's labels and hints. Main thread only, like the observers it registers.
class TextsChangedWatch
{
public:
    TextsChangedWatch();
    ~TextsChangedWatch();
    TextsChangedWatch(const TextsChangedWatch&) = delete;
    TextsChangedWatch& operator=(const TextsChangedWatch&) = delete;

    // Whether the texts changed since the last call.
    bool TakeChanged();

private:
    static void OnTextsChanged(void* context) noexcept;

    bool m_Changed = false;
};
} // namespace UI::Character
