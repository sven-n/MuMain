#pragma once

#ifdef _EDITOR

namespace MuEditor::Monsters
{
// Monster Translations: the monster names in every UI language, side by side.
class MuMonsterTranslationsUI
{
public:
    static MuMonsterTranslationsUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterTranslationsUI() = default;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterTranslationsUI MuEditor::Monsters::MuMonsterTranslationsUI::GetInstance()

#endif // _EDITOR
