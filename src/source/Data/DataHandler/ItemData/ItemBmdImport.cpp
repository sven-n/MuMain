#include "stdafx.h"

#include "ItemBmdImport.h"
#include "ItemDataHandler.h"
#include "ItemDataLoader.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <optional>

namespace Data::Items
{
namespace
{
constexpr size_t LegacyNameFieldSize = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY::Name);
constexpr size_t NameFieldSize = sizeof(ITEM_ATTRIBUTE_FILE::Name);

// Where a field lives in a file record and in ITEM_ATTRIBUTE, and its size in each (a ByteInFile field is one byte in
// the file and two in ITEM_ATTRIBUTE).
struct FieldLayout
{
    std::string name;
    size_t fileOffset = 0;
    size_t attributeOffset = 0;
    size_t fileSize = 0;
    size_t attributeSize = 0;
};

template <typename TFileFormat> std::vector<FieldLayout> BuildFieldLayouts()
{
    std::vector<FieldLayout> fields;
#define ADD_ITEM_FIELD(name, type, arraySize, width, i18nName)                                                         \
    fields.push_back({#name, offsetof(TFileFormat, name), offsetof(ITEM_ATTRIBUTE, name), sizeof(TFileFormat::name),   \
                      sizeof(ITEM_ATTRIBUTE::name)});
    ITEM_FIELDS_SIMPLE(ADD_ITEM_FIELD)
#undef ADD_ITEM_FIELD

    for (size_t i = 0; i < MAX_CLASS; ++i)
    {
        fields.push_back({"RequireClass[" + std::to_string(i) + "]", offsetof(TFileFormat, RequireClass) + i,
                          offsetof(ITEM_ATTRIBUTE, RequireClass) + i, 1, 1});
    }
    for (size_t i = 0; i < MAX_RESISTANCE + 1; ++i)
    {
        fields.push_back({"Resistance[" + std::to_string(i) + "]", offsetof(TFileFormat, Resistance) + i,
                          offsetof(ITEM_ATTRIBUTE, Resistance) + i, 1, 1});
    }
    return fields;
}

// One loaded language file.
struct LanguageFile
{
    const ItemBmdLanguage* language = nullptr;
    ItemDataLoader::RawItemFile raw;
    std::vector<FieldLayout> fields;
    size_t nameFieldSize = 0;
};

// One item as one language file stores it.
struct LanguageRecord
{
    const LanguageFile* file = nullptr;
    const BYTE* bytes = nullptr;
    std::string name;
    // Position of the name's terminating null byte in the record.
    size_t nameEnd = 0;
    bool reencoded = false;

    bool NameRanOn() const
    {
        return nameEnd >= file->nameFieldSize;
    }

    // True when the name (or its terminating null byte) overwrote the field.
    bool IsDamaged(const FieldLayout& field) const
    {
        return NameRanOn() && field.fileOffset <= nameEnd && field.fileOffset + field.fileSize > file->nameFieldSize;
    }
};

// ------------------------------------------------------------ name decoding

// Windows-1252 differs from Latin-1 only in 0x80-0x9F.
constexpr std::array<char32_t, 32> Windows1252HighControls = {
    0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
    0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD, 0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178};

void AppendUtf8(std::string& target, char32_t codePoint)
{
    if (codePoint < 0x80)
    {
        target += static_cast<char>(codePoint);
    }
    else if (codePoint < 0x800)
    {
        target += static_cast<char>(0xC0 | (codePoint >> 6));
        target += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else
    {
        target += static_cast<char>(0xE0 | (codePoint >> 12));
        target += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        target += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}

std::string Windows1252ToUtf8(std::string_view text)
{
    constexpr unsigned char FirstHighControl = 0x80;
    constexpr unsigned char LastHighControl = 0x9F;

    std::string result;
    for (const char character : text)
    {
        const auto byte = static_cast<unsigned char>(character);
        const bool isHighControl = byte >= FirstHighControl && byte <= LastHighControl;
        AppendUtf8(result, isHighControl ? Windows1252HighControls[byte - FirstHighControl] : char32_t{byte});
    }
    return result;
}

// Some names were typed on Korean systems, where a right single quote is the
// byte pair A1 AF (CP949). It becomes a plain apostrophe, as in OpenMU.
std::string ReplaceKoreanQuotes(std::string_view text)
{
    constexpr std::string_view KoreanRightQuote = "\xA1\xAF";
    std::string result(text);
    for (size_t position = result.find(KoreanRightQuote); position != std::string::npos;
         position = result.find(KoreanRightQuote, position))
    {
        result.replace(position, KoreanRightQuote.size(), "'");
    }
    return result;
}

std::string DecodeName(std::string_view raw, bool& reencoded)
{
    reencoded = !Core::Text::IsValidUtf8(raw);
    return reencoded ? Windows1252ToUtf8(ReplaceKoreanQuotes(raw)) : std::string(raw);
}

// ------------------------------------------------------------ records

std::optional<LanguageRecord> ReadRecord(const LanguageFile& file, int itemType)
{
    LanguageRecord record;
    record.file = &file;
    record.bytes = file.raw.GetRecord(itemType);
    if (record.bytes[0] == 0)
    {
        return std::nullopt;
    }

    const auto* nameEnd = static_cast<const BYTE*>(std::memchr(record.bytes, 0, file.raw.recordSize));
    record.nameEnd = nameEnd != nullptr ? static_cast<size_t>(nameEnd - record.bytes) : file.raw.recordSize;
    record.name =
        DecodeName(std::string_view(reinterpret_cast<const char*>(record.bytes), record.nameEnd), record.reencoded);
    return record;
}

long long ReadFieldValue(const LanguageRecord& record, const FieldLayout& field)
{
    unsigned long long value = 0;
    std::memcpy(&value, record.bytes + field.fileOffset, field.fileSize);
    return static_cast<long long>(value);
}

// The values of an item that sets nothing (e.g. slot "not equippable").
const ITEM_ATTRIBUTE& GetDefaultAttribute()
{
    static const ITEM_ATTRIBUTE Defaults = []
    {
        ITEM_ATTRIBUTE attribute{};
        ToItemAttribute(ItemDefinition{}, attribute);
        return attribute;
    }();
    return Defaults;
}

long long ReadDefaultValue(const FieldLayout& field)
{
    unsigned long long value = 0;
    std::memcpy(&value, reinterpret_cast<const BYTE*>(&GetDefaultAttribute()) + field.attributeOffset,
                field.attributeSize);
    return static_cast<long long>(value);
}

void CopyField(const LanguageRecord* source, const FieldLayout& sourceField, const FieldLayout& targetField,
               ITEM_ATTRIBUTE& target)
{
    auto* targetBytes = reinterpret_cast<BYTE*>(&target) + targetField.attributeOffset;
    // The value, widened from the file's size to ITEM_ATTRIBUTE's (both little-endian, unsigned).
    unsigned long long value = 0;
    if (source != nullptr)
    {
        std::memcpy(&value, source->bytes + sourceField.fileOffset, sourceField.fileSize);
    }
    else
    {
        std::memcpy(&value, reinterpret_cast<const BYTE*>(&GetDefaultAttribute()) + targetField.attributeOffset,
                    targetField.attributeSize);
    }
    std::memcpy(targetBytes, &value, targetField.attributeSize);
}

// The first record that has an undamaged value for the field.
const LanguageRecord* FindUndamagedSource(const std::vector<LanguageRecord>& records, size_t fieldIndex)
{
    for (const LanguageRecord& record : records)
    {
        if (!record.IsDamaged(record.file->fields[fieldIndex]))
        {
            return &record;
        }
    }
    return nullptr;
}

ITEM_ATTRIBUTE MergeStats(const std::vector<LanguageRecord>& records, int itemType, std::vector<ItemBmdRepair>& repairs)
{
    const LanguageRecord& reference = records.front();
    ITEM_ATTRIBUTE merged{};
    for (size_t fieldIndex = 0; fieldIndex < reference.file->fields.size(); ++fieldIndex)
    {
        const FieldLayout& referenceField = reference.file->fields[fieldIndex];
        const LanguageRecord* source = FindUndamagedSource(records, fieldIndex);
        CopyField(source, source ? source->file->fields[fieldIndex] : referenceField, referenceField, merged);
        if (source == &reference)
        {
            continue;
        }

        ItemBmdRepair repair;
        repair.group = GetItemGroup(itemType);
        repair.number = GetItemNumber(itemType);
        repair.field = referenceField.name;
        repair.oldValue = ReadFieldValue(reference, referenceField);
        repair.newValue =
            source ? ReadFieldValue(*source, source->file->fields[fieldIndex]) : ReadDefaultValue(referenceField);
        repair.fromLocale = source ? source->file->language->locale : "";
        repairs.push_back(std::move(repair));
    }
    return merged;
}

std::vector<LanguageFile> LoadLanguageFiles(std::span<const ItemBmdLanguage> languages, ItemBmdImportResult& result)
{
    std::vector<LanguageFile> files;
    for (const ItemBmdLanguage& language : languages)
    {
        LanguageFile file;
        file.language = &language;
        const std::wstring path = CItemDataHandler::GetItemFilePath(language.folder);
        if (!ItemDataLoader::ReadRawFile(path.c_str(), file.raw, ItemDataLoader::Reporting::Quiet))
        {
            const auto severity = files.empty() ? ItemDataIssueSeverity::Error : ItemDataIssueSeverity::Warning;
            result.issues.push_back({severity, Core::Text::ToUtf8(path.c_str()), ItemDataIssue::NoItem,
                                     ItemDataIssue::NoItem, "", "could not be read, language skipped"});
            if (files.empty())
            {
                return {};
            }
            continue;
        }

        file.fields = file.raw.isLegacyFormat ? BuildFieldLayouts<ITEM_ATTRIBUTE_FILE_LEGACY>()
                                              : BuildFieldLayouts<ITEM_ATTRIBUTE_FILE>();
        file.nameFieldSize = file.raw.isLegacyFormat ? LegacyNameFieldSize : NameFieldSize;
        result.importedLocales.push_back(language.locale);
        files.push_back(std::move(file));
    }
    return files;
}

void ImportItem(const std::vector<LanguageFile>& files, int itemType, ItemBmdImportResult& result)
{
    std::vector<LanguageRecord> records;
    for (const LanguageFile& file : files)
    {
        if (auto record = ReadRecord(file, itemType))
        {
            records.push_back(std::move(*record));
        }
    }
    if (records.empty())
    {
        return;
    }

    ItemDefinition definition;
    CopyItemAttributeStats(MergeStats(records, itemType, result.repairs), definition);
    definition.group = GetItemGroup(itemType);
    definition.number = GetItemNumber(itemType);
    for (const LanguageRecord& record : records)
    {
        // Also a name equal to the English one: the item can have the same
        // name in that language. The editor's import tells it apart from a
        // missing name (CItemDataHandler::KeepMissingNamesMissing).
        definition.names.Set(record.file->language->locale, record.name);
        result.recoveredNameCount += record.NameRanOn() ? 1 : 0;
        result.reencodedNameCount += record.reencoded ? 1 : 0;
    }
    result.items.push_back(std::move(definition));
}
} // namespace

namespace
{
constexpr int MaxRequirement = 65535;

// The stat requirements the original client grew from a weight: in the definition and in the
// bmd record.
struct GrownRequirement
{
    ItemRequirement requirement;
    ItemRequirementStat stat;
    WORD ItemRequirements::* value;
    WORD ITEM_ATTRIBUTE::* weight;
};
constexpr GrownRequirement GrownRequirements[] = {
    {ItemRequirement::Strength, ItemRequirementStat::Strength, &ItemRequirements::strength,
     &ITEM_ATTRIBUTE::RequireStrength},
    {ItemRequirement::Dexterity, ItemRequirementStat::Dexterity, &ItemRequirements::dexterity,
     &ITEM_ATTRIBUTE::RequireDexterity},
    {ItemRequirement::Energy, ItemRequirementStat::Energy, &ItemRequirements::energy, &ITEM_ATTRIBUTE::RequireEnergy},
    {ItemRequirement::Vitality, ItemRequirementStat::Vitality, &ItemRequirements::vitality,
     &ITEM_ATTRIBUTE::RequireVitality},
};

// The original client's requirement growth, which its bmd files hold the weights for: its code, so
// it stays as it was also when the item stat file's formulas change.
//   add + weight * drop level * percent / 100, energy with a percent of its own; a scroll's energy
//   (group 15) from its level requirement where it has one.
constexpr long long OriginalRequirementAdd = 20;
constexpr long long OriginalRequirementPercent = 3;
constexpr long long OriginalEnergyRequirementPercent = 4;
constexpr long long PercentDivisor = 100;

// What the original client asked for with `weight`.
int GrowWeight(const ItemDefinition& definition, ItemRequirement requirement, int weight)
{
    if (weight == 0)
    {
        return 0;
    }
    const bool energy = requirement == ItemRequirement::Energy;
    const bool fromLevelRequirement = energy && definition.group == ITEM_GROUP_ETC && definition.requirements.level > 0;
    const long long basis = fromLevelRequirement ? definition.requirements.level : definition.level;
    const long long percent = energy ? OriginalEnergyRequirementPercent : OriginalRequirementPercent;
    return static_cast<int>(
        std::min<long long>(OriginalRequirementAdd + weight * basis * percent / PercentDivisor, MaxRequirement));
}
} // namespace

namespace
{
// The profile the item uses: the one it names, or the one its rules give.
const ItemStatProfile& FindItemProfile(const ItemStatProfiles& profiles, const ItemDefinition& definition)
{
    const std::string name =
        definition.statProfile.empty() ? DeriveStatProfile(profiles, definition) : definition.statProfile;
    const int profile = profiles.Find(name);
    return profiles.Get(profile == ItemStatProfiles::UnknownProfile ? ItemStatProfiles::DefaultProfile : profile);
}

// Whether the item keeps the requirement as a value where the original client grew a weight.
bool HoldsWeightInBmd(const ItemStatProfiles& profiles, const ItemStatProfile& profile, ItemRequirementStat stat)
{
    return profile.requirements.Get(stat).keepBase &&
           !profiles.Get(ItemStatProfiles::DefaultProfile).requirements.Get(stat).keepBase;
}
} // namespace

bool HasRequirementWeightsInBmd(const ItemStatProfiles& profiles, const ItemDefinition& definition)
{
    const ItemStatProfile& profile = FindItemProfile(profiles, definition);
    return std::ranges::any_of(GrownRequirements, [&](const GrownRequirement& grown)
                               { return HoldsWeightInBmd(profiles, profile, grown.stat); });
}

void ToRequirementValues(const ItemStatProfiles& profiles, ItemDefinition& definition)
{
    const ItemStatProfile& profile = FindItemProfile(profiles, definition);
    for (const GrownRequirement& grown : GrownRequirements)
    {
        if (HoldsWeightInBmd(profiles, profile, grown.stat))
        {
            WORD& value = definition.requirements.*grown.value;
            value = static_cast<WORD>(GrowWeight(definition, grown.requirement, value));
        }
    }
}

std::vector<ItemRequirement> ToBmdRequirementWeights(const ItemStatProfiles& profiles, const ItemDefinition& definition,
                                                     ITEM_ATTRIBUTE& attribute)
{
    std::vector<ItemRequirement> inexact;
    const ItemStatProfile& profile = FindItemProfile(profiles, definition);
    for (const GrownRequirement& grown : GrownRequirements)
    {
        if (!HoldsWeightInBmd(profiles, profile, grown.stat))
        {
            continue;
        }
        const int value = definition.requirements.*grown.value;
        // The weights grow the value without going down: the smallest that reaches it.
        int low = 0;
        int high = MaxRequirement;
        while (low < high)
        {
            const int middle = low + (high - low) / 2;
            if (GrowWeight(definition, grown.requirement, middle) < value)
            {
                low = middle + 1;
            }
            else
            {
                high = middle;
            }
        }
        int weight = low;
        const int reached = GrowWeight(definition, grown.requirement, weight);
        if (reached != value)
        {
            // Past the value, or none reaches it: the nearer of this weight and the one below.
            if (weight > 0 && value - GrowWeight(definition, grown.requirement, weight - 1) <= reached - value)
            {
                --weight;
            }
            inexact.push_back(grown.requirement);
        }
        attribute.*grown.weight = static_cast<WORD>(weight);
    }
    return inexact;
}

std::span<const ItemBmdLanguage> GetItemBmdLanguages()
{
    return GetLegacyLanguages();
}

ItemBmdImportResult ImportItemBmdFiles(std::span<const ItemBmdLanguage> languages)
{
    ItemBmdImportResult result;
    const std::vector<LanguageFile> files = LoadLanguageFiles(languages, result);
    if (files.empty())
    {
        return result;
    }

    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        ImportItem(files, itemType, result);
    }
    return result;
}
} // namespace Data::Items
