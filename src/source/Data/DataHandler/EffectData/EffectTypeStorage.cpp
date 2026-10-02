#include "stdafx.h"

#include "EffectTypeStorage.h"

#include "Data/DataHandler/TextFile.h"

#include <optional>
#include <string>

namespace Data::Effects
{
std::filesystem::path GetEffectDataDirectory()
{
    return std::filesystem::path("Data/Effects");
}

EffectTypesLoadResult LoadEffectTypeFiles(const std::filesystem::path& directory)
{
    EffectTypesLoadResult result;
    for (const EffectKind kind : EffectKinds)
    {
        const std::filesystem::path file = directory / GetEffectTypesFileName(kind);
        const std::string source = file.generic_string();
        const std::optional<std::string> text = ReadTextFile(file);
        if (!text)
        {
            result.issues.push_back({Items::ItemDataIssueSeverity::Error, source, Items::ItemDataIssue::NoItem,
                                     Items::ItemDataIssue::NoItem, "", "could not be read"});
            continue;
        }

        // A file that cannot be read would report every type as unnamed.
        std::vector<Items::ItemDataIssue> readIssues;
        std::vector<EffectTypeEntry>& types = result.types[ToIndex(kind)];
        ReadEffectTypesJson(*text, source, kind, types, readIssues);
        if (!Items::HasErrors(readIssues))
        {
            ValidateEffectTypes(kind, types, source, readIssues);
        }
        result.issues.insert(result.issues.end(), readIssues.begin(), readIssues.end());
    }
    return result;
}
} // namespace Data::Effects
