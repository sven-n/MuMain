#include "stdafx.h"

#include "UI/Character/TextsChangedWatch.h"

#include "Data/Translation/ItemTextExpansion.h"
#include "I18N/All.h"

namespace UI::Character
{
TextsChangedWatch::TextsChangedWatch()
{
    I18N::RegisterLocaleObserver(&TextsChangedWatch::OnTextsChanged, this);
    Data::Translation::ItemTexts::RegisterTextsChangedObserver(&TextsChangedWatch::OnTextsChanged, this);
}

TextsChangedWatch::~TextsChangedWatch()
{
    I18N::UnregisterLocaleObserver(&TextsChangedWatch::OnTextsChanged, this);
    Data::Translation::ItemTexts::UnregisterTextsChangedObserver(&TextsChangedWatch::OnTextsChanged, this);
}

bool TextsChangedWatch::TakeChanged()
{
    const bool changed = m_Changed;
    m_Changed = false;
    return changed;
}

void TextsChangedWatch::OnTextsChanged(void* context) noexcept
{
    static_cast<TextsChangedWatch*>(context)->m_Changed = true;
}
} // namespace UI::Character
