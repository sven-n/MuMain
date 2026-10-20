#pragma once

#ifdef _EDITOR

#include <map>
#include <string>
#include <string_view>

namespace MuEditor::Items
{
// How many model entries use each shared model, render style, item effect and
// glow color, for the model tool's lists that show the count beside each
// name. Counted in one pass over the models, again when they changed.
class ItemLookUseCounts
{
public:
    int SharedModel(std::string_view name);
    int RenderStyle(std::string_view name);
    int ItemEffect(std::string_view name);
    int GlowColor(std::string_view name);

private:
    using Counts = std::map<std::string, int, std::less<>>;

    void Update();
    static int Find(const Counts& counts, std::string_view name);

    Counts m_sharedModels;
    Counts m_renderStyles;
    Counts m_itemEffects;
    Counts m_glowColors;
    int m_version = -1;
};
} // namespace MuEditor::Items

#endif // _EDITOR
