#include "stdafx.h"

#include "Engine/Object/ItemOptionLines.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/OptionTextsJson.h"
#include "I18N/All.h"

#include <array>
#include <cwchar>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Items::OptionLines
{
namespace
{
// The placeholders a line fills in.
enum class Placeholder : unsigned char
{
    Value,
    Divisor,
};

// A text split at its {value} and {divisor}: between two pieces goes the number of the placeholder between them. A
// placeholder of another name stays as written.
struct SplitText
{
    std::vector<std::wstring> pieces;
    std::vector<Placeholder> between;
};

// A text of the option texts in a language. The texts stay where they are until the item data changes.
struct Key
{
    const Data::LocalizedString* text = nullptr;
    std::string language;

    bool operator==(const Key& other) const = default;
};

struct KeyHash
{
    size_t operator()(const Key& key) const
    {
        return std::hash<const void*>()(key.text) ^ std::hash<std::string>()(key.language);
    }
};

std::unordered_map<Key, SplitText, KeyHash> cache;
int cacheVersion = -1;

SplitText Split(std::string_view text)
{
    SplitText split;
    std::string piece;
    size_t position = 0;
    for (const Data::Items::PlaceholderSpan& span : Data::Items::FindPlaceholders(text).spans)
    {
        if (span.name != Data::Items::OptionValuePlaceholder && span.name != Data::Items::OptionDivisorPlaceholder)
        {
            continue;
        }
        piece.append(text.substr(position, span.begin - position));
        split.pieces.push_back(Core::Text::FromUtf8(piece));
        split.between.push_back(span.name == Data::Items::OptionValuePlaceholder ? Placeholder::Value
                                                                                 : Placeholder::Divisor);
        piece.clear();
        position = span.end;
    }
    piece.append(text.substr(position));
    split.pieces.push_back(Core::Text::FromUtf8(piece));
    return split;
}

const SplitText& Find(const Data::LocalizedString& text, std::string_view language)
{
    if (cacheVersion != Data::Items::ItemDatabase::GetInstance().GetVersion())
    {
        cache.clear();
        cacheVersion = Data::Items::ItemDatabase::GetInstance().GetVersion();
    }
    Key key{&text, std::string(language)};
    const auto found = cache.find(key);
    if (found != cache.end())
    {
        return found->second;
    }
    return cache.emplace(std::move(key), Split(text.Get(language))).first->second;
}

void Append(const wchar_t* text, size_t length, wchar_t* out, size_t capacity, size_t& written)
{
    const size_t room = capacity - 1 - written;
    const size_t copied = length < room ? length : room;
    std::wmemcpy(out + written, text, copied);
    written += copied;
    out[written] = L'\0';
}
} // namespace

bool Write(std::string_view name, const LineValues& values, wchar_t* out, size_t capacity)
{
    const Data::LocalizedString* text = Data::Items::ItemDatabase::GetInstance().GetOptions().GetTexts().Find(name);
    if (text == nullptr || capacity == 0)
    {
        return false;
    }
    const SplitText& split = Find(*text, I18N::GetCurrentLocale());
    std::array<wchar_t, 16> number{};
    size_t written = 0;
    out[0] = L'\0';
    for (size_t piece = 0; piece < split.pieces.size(); ++piece)
    {
        if (piece > 0)
        {
            const int shown = split.between[piece - 1] == Placeholder::Value ? values.value : values.divisor;
            const int numberLength = std::swprintf(number.data(), number.size(), L"%d", shown);
            Append(number.data(), static_cast<size_t>(numberLength > 0 ? numberLength : 0), out, capacity, written);
        }
        Append(split.pieces[piece].c_str(), split.pieces[piece].size(), out, capacity, written);
    }
    return true;
}
} // namespace Engine::Items::OptionLines
