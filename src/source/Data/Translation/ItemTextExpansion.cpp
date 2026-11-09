#include "stdafx.h"

#include "Data/Translation/ItemTextExpansion.h"

#include "Core/Text/Utf8.h"
#include "I18N/All.h"

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Data::Translation::ItemTexts
{
namespace
{
constexpr std::wstring_view WideOpening = L"{item:";

NameResolver& Resolver()
{
    static NameResolver resolver;
    return resolver;
}

// Every expansion handed to the UI texts. Kept for the program's lifetime: code may still hold a copy of an older
// pointer, and an unchanged expansion gets the same pointer again, so ExpandItemTexts sees that nothing changed.
std::unordered_set<std::wstring>& Expansions()
{
    static std::unordered_set<std::wstring> expansions;
    return expansions;
}

using TextsChangedObservers = std::vector<std::pair<TextsChangedObserver, void*>>;

TextsChangedObservers& Observers()
{
    static TextsChangedObservers observers;
    return observers;
}

const wchar_t* ExpandWide(const wchar_t* text, int textLocaleIndex) noexcept
{
    try
    {
        if (text == nullptr || !Resolver() || std::wstring_view(text).find(WideOpening) == std::wstring_view::npos)
        {
            return text;
        }
        const std::string written = Core::Text::ToUtf8(text);
        const std::string expanded = ExpandText(written, I18N::GetAvailableLocales()[textLocaleIndex]);
        if (expanded == written)
        {
            return text;
        }
        return Expansions().insert(Core::Text::FromUtf8(expanded)).first->c_str();
    }
    catch (...)
    {
        // Out of memory or a resolver that threw: the text as written is still readable.
        return text;
    }
}
} // namespace

void SetNameResolver(NameResolver resolver)
{
    Resolver() = std::move(resolver);
    I18N::SetItemTextExpander(Resolver() ? &ExpandWide : nullptr);
}

std::string ExpandText(std::string_view text, std::string_view textLocale)
{
    std::string expanded;
    expanded.reserve(text.size());
    std::size_t position = 0;
    for (const ItemReferenceMatch& match : FindItemReferences(text))
    {
        expanded.append(text.substr(position, match.position - position));
        const std::string_view written = text.substr(match.position, match.length);
        std::optional<std::string> name;
        if (match.reference && Resolver())
        {
            name = Resolver()(*match.reference, textLocale);
        }
        expanded.append(name ? std::string_view(*name) : written);
        position = match.position + match.length;
    }
    expanded.append(text.substr(position));
    return expanded;
}

void RegisterTextsChangedObserver(TextsChangedObserver observer, void* context)
{
    Observers().emplace_back(observer, context);
}

void UnregisterTextsChangedObserver(TextsChangedObserver observer, void* context)
{
    std::erase(Observers(), std::pair(observer, context));
}

bool Refresh()
{
    const bool changed = I18N::ExpandItemTexts();
    if (changed)
    {
        // A copy, so an observer may unregister itself.
        const TextsChangedObservers observers = Observers();
        for (const auto& [observer, context] : observers)
        {
            observer(context);
        }
    }
    return changed;
}
} // namespace Data::Translation::ItemTexts
