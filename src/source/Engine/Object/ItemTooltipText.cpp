#include "stdafx.h"

#include "Engine/Object/ItemTooltipText.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemReferenceNames.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/Translation/ItemReference.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemNameParts.h"
#include "GameLogic/Skills/SkillManager.h"

#include <algorithm>
#include <array>
#include <cwchar>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Items::TooltipText
{
namespace
{
using Data::Items::TooltipValue;

// What a compiled text depends on: the text (its characters stay where they are until the item data changes, so where
// they are stands for what they say), the line's bindings and item, and the language of its level values.
struct Key
{
    const char* text = nullptr;
    size_t size = 0;
    const void* bound = nullptr;
    const void* definition = nullptr;
    std::array<char, 16> language{};
    bool englishNames = false;

    bool operator==(const Key& other) const = default;
};

struct KeyHash
{
    size_t operator()(const Key& key) const
    {
        size_t hash = std::hash<const void*>()(key.text) ^ (key.size * 0x9E3779B97F4A7C15ull);
        hash ^= std::hash<const void*>()(key.bound) + 0x9E3779B9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<const void*>()(key.definition) + 0x9E3779B9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::string_view>()(std::string_view(key.language.data())) + (hash << 6) + (hash >> 2);
        hash ^= static_cast<size_t>(key.englishNames);
        return hash;
    }
};

// The compiled texts, for the item data of `cacheVersion` and the item names of `cacheNameVersion` (the texts that
// name items show the names). The tooltip draws on the main thread only.
std::unordered_map<Key, Compiled, KeyHash> cache;
int cacheVersion = -1;
int cacheNameVersion = -1;

std::wstring Wide(std::string_view text)
{
    return Core::Text::FromUtf8(std::string(text));
}

// Resolves a placeholder's name into the piece, as TooltipPlaceholderValue looked it up by name every frame: the line's
// binding first, then a game data value, the item's level value, a built-in value and a table. False when it names
// nothing.
bool Resolve(std::string_view name, std::span<const std::pair<std::string, std::string>> bound,
             const Data::Items::ItemDefinition* definition, std::string_view language, Piece& piece)
{
    for (const auto& [placeholder, value] : bound)
    {
        if (placeholder == name)
        {
            name = value;
            break;
        }
    }
    if (const std::optional<Data::Items::GameDataValue> data = Data::Items::ReadGameDataValue(name))
    {
        switch (data->kind)
        {
        case Data::Items::GameDataValueKind::SkillMana:
            piece.kind = Piece::Kind::SkillMana;
            piece.number = data->number;
            return true;
        case Data::Items::GameDataValueKind::SkillName:
        case Data::Items::GameDataValueKind::MonsterName:
            // Looked up when written, as main did: the skill list and the monster list (which the monster editor
            // changes without the item data changing) decide then whether the name is there, and a monster's name
            // follows "Translate item names".
            piece.kind = data->kind == Data::Items::GameDataValueKind::SkillName ? Piece::Kind::SkillName
                                                                                 : Piece::Kind::MonsterName;
            piece.number = data->number;
            return true;
        }
        return false;
    }
    if (const Data::Items::ItemLevelValues* levelValues =
            definition != nullptr ? definition->FindLevelValues(name) : nullptr)
    {
        piece.kind = Piece::Kind::LevelValue;
        for (const Data::Items::ItemLevelValue& value : levelValues->values)
        {
            const Data::LocalizedString* text =
                value.textName.empty() ? &value.text : g_ItemDatabase.GetTooltipFile().FindText(value.textName);
            piece.levelTexts.push_back(text != nullptr ? Wide(text->Get(language)) : std::wstring());
        }
        return true;
    }
    if (const std::optional<TooltipValue> value = Data::Items::FindBuiltInPlaceholder(name))
    {
        piece.kind = Piece::Kind::BuiltIn;
        piece.value = *value;
        return true;
    }
    const std::string_view tableName = Data::Items::GetPlaceholderTable(name);
    if (const Data::Items::LevelBonusTable* table =
            tableName.empty() ? nullptr : g_ItemDatabase.GetStatProfiles().FindTable(tableName))
    {
        piece.kind = Piece::Kind::Table;
        piece.table = table;
        return true;
    }
    return false;
}

// The names the item references among the placeholders show, one per span (none for another placeholder): in the item
// name language, so the names follow "Translate item names" while the rest of the text stays in the UI language, or in
// English for an English text shown for a language without a translation of its own, as the UI texts do. A reference
// that names nothing shows as written; the rest of the text goes on.
std::vector<std::optional<std::string>>
ItemNamesOf(std::string_view text, std::span<const Data::Items::PlaceholderSpan> spans, std::string_view nameLocale)
{
    std::vector<std::optional<std::string>> names(spans.size());
    for (size_t index = 0; index < spans.size(); ++index)
    {
        const std::optional<Data::Translation::ItemReference> reference =
            Data::Translation::ParseItemReferenceName(spans[index].name);
        if (!reference)
        {
            continue;
        }
        std::string name = Data::Items::ResolveReferencedName(g_ItemDatabase, *reference, nameLocale).text;
        names[index] = name.empty()
                           ? std::string(text.substr(spans[index].begin, spans[index].end - spans[index].begin))
                           : std::move(name);
    }
    return names;
}

// The text from the span `first` on, with the items' names put in: what the rest of the line shows from a placeholder
// without a value on.
std::string NamedRest(std::string_view text, std::span<const Data::Items::PlaceholderSpan> spans,
                      const std::vector<std::optional<std::string>>& names, size_t first)
{
    std::string rest;
    size_t position = spans[first].begin;
    for (size_t index = first; index < spans.size(); ++index)
    {
        if (names[index])
        {
            rest.append(text.substr(position, spans[index].begin - position));
            rest.append(*names[index]);
            position = spans[index].end;
        }
    }
    rest.append(text.substr(position));
    return rest;
}

Compiled CompileText(std::string_view text, std::span<const std::pair<std::string, std::string>> bound,
                     const Data::Items::ItemDefinition* definition, std::string_view language, bool englishNames)
{
    const std::string_view nameLocale =
        englishNames ? Data::LocalizedString::NeutralLocale : std::string_view(g_ItemDatabase.GetDisplayLocale());
    Compiled compiled;
    const Data::Items::PlaceholderSpans found = Data::Items::FindPlaceholders(text);
    const std::span<const Data::Items::PlaceholderSpan> spans = found.spans;
    // Put in now: the names change only with the name version, which compiles the texts again.
    const std::vector<std::optional<std::string>> names = ItemNamesOf(text, spans, nameLocale);
    // The text since the last value, with the items' names put in.
    std::string literal;
    size_t position = 0;
    for (size_t index = 0; index < spans.size(); ++index)
    {
        const Data::Items::PlaceholderSpan& span = spans[index];
        literal.append(text.substr(position, span.begin - position));
        position = span.end;
        if (names[index])
        {
            literal.append(*names[index]);
            continue;
        }
        Piece piece;
        if (!Resolve(span.name, bound, definition, language, piece))
        {
            // As FillPlaceholders: a placeholder without a value stays as written, and so does the rest.
            Piece last;
            last.literal = Wide(literal + NamedRest(text, spans, names, index));
            compiled.pieces.push_back(std::move(last));
            return compiled;
        }
        piece.literal = Wide(literal);
        literal.clear();
        piece.rest = Wide(NamedRest(text, spans, names, index));
        compiled.pieces.push_back(std::move(piece));
    }
    Piece last;
    last.literal = Wide(literal + std::string(text.substr(std::min(position, text.size()))));
    compiled.pieces.push_back(std::move(last));
    return compiled;
}

// Appends to `out` (of `capacity` characters with its 0), keeping `length` up to date; stops at the end.
void Append(const wchar_t* text, size_t textLength, wchar_t* out, size_t capacity, size_t& length)
{
    const size_t room = capacity > length + 1 ? capacity - length - 1 : 0;
    const size_t count = std::min(textLength, room);
    std::copy_n(text, count, out + length);
    length += count;
    out[length] = L'\0';
}

void AppendNumber(int number, wchar_t* out, size_t capacity, size_t& length)
{
    wchar_t digits[16];
    const int written = std::swprintf(digits, std::size(digits), L"%d", number);
    if (written > 0)
        Append(digits, static_cast<size_t>(written), out, capacity, length);
}
} // namespace

const Compiled& Compile(std::string_view text, std::span<const std::pair<std::string, std::string>> bound,
                        const Data::Items::ItemDefinition* definition, std::string_view language,
                        std::string_view textLanguage)
{
    if (cacheVersion != g_ItemDatabase.GetVersion() || cacheNameVersion != g_ItemDatabase.GetNameVersion())
    {
        cache.clear();
        cacheVersion = g_ItemDatabase.GetVersion();
        cacheNameVersion = g_ItemDatabase.GetNameVersion();
    }
    Key key{text.data(), text.size(), bound.data(), definition};
    std::copy_n(language.data(), std::min(language.size(), key.language.size() - 1), key.language.data());
    key.englishNames = (textLanguage.empty() ? language : textLanguage) == Data::LocalizedString::NeutralLocale;
    // No text is compared: the item data, the stat file and the tooltip file change only by raising the version.
    const auto found = cache.find(key);
    if (found == cache.end())
    {
        return cache.emplace(key, CompileText(text, bound, definition, language, key.englishNames)).first->second;
    }
    return found->second;
}

size_t Write(const Compiled& text, int level, const ItemTooltipValues& values, wchar_t* out, size_t capacity)
{
    if (capacity == 0)
    {
        return 0;
    }
    size_t length = 0;
    out[0] = L'\0';
    for (const Piece& piece : text.pieces)
    {
        Append(piece.literal.c_str(), piece.literal.size(), out, capacity, length);
        switch (piece.kind)
        {
        case Piece::Kind::None:
            break;
        case Piece::Kind::BuiltIn:
            if (piece.value == TooltipValue::Level)
            {
                AppendNumber(level, out, capacity, length);
            }
            else if (const wchar_t* valueText = TooltipValues::GetItemTooltipText(values, piece.value))
            {
                Append(valueText, std::wcslen(valueText), out, capacity, length);
            }
            else if (const std::optional<int> value = TooltipValues::GetItemTooltipValue(values, piece.value))
            {
                AppendNumber(*value, out, capacity, length);
            }
            break;
        case Piece::Kind::Table:
            AppendNumber(piece.table->GetBonus(level), out, capacity, length);
            break;
        case Piece::Kind::SkillMana:
        {
            int mana = 0;
            gSkillManager.GetSkillInformation(piece.number, 1, nullptr, &mana, nullptr);
            AppendNumber(mana, out, capacity, length);
            break;
        }
        case Piece::Kind::SkillName:
        case Piece::Kind::MonsterName:
        {
            const wchar_t* name = piece.kind == Piece::Kind::SkillName
                                      ? (SkillAttribute != nullptr ? SkillAttribute[piece.number].Name : nullptr)
                                      : GameLogic::Items::NameParts::MonsterName(piece.number);
            if (name == nullptr)
            {
                // Not in the skill list or the monster list: the rest of the line as written.
                Append(piece.rest.c_str(), piece.rest.size(), out, capacity, length);
                return length;
            }
            Append(name, std::wcslen(name), out, capacity, length);
            break;
        }
        case Piece::Kind::LevelValue:
            if (level < 0 || level >= static_cast<int>(piece.levelTexts.size()))
            {
                // No value at this level: the rest of the line as written.
                Append(piece.rest.c_str(), piece.rest.size(), out, capacity, length);
                return length;
            }
            Append(piece.levelTexts[level].c_str(), piece.levelTexts[level].size(), out, capacity, length);
            break;
        }
    }
    return length;
}

void Clear()
{
    cache.clear();
    cacheVersion = -1;
}
} // namespace Engine::Items::TooltipText
