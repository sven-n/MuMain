#pragma once

#include "ItemStatProfilesScope.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include "doctest.h"

#include <filesystem>
#include <utility>

// The shipped stat profiles with the scroll profile's energy as it was before the scrolls kept
// their requirements as values: from the level requirement (the "levelRequirement" basis), and
// from the drop level where an item has none. No shipped profile uses that basis any more, so
// its tests install this one for their scope. A scroll's energy is then a weight again: the test
// gives the item one.
class LevelBasisScrollProfile
{
public:
    explicit LevelBasisScrollProfile(const std::filesystem::path& clientDirectory)
    {
        Data::Items::ItemStatsLoadResult stats =
            Data::Items::LoadItemStatsFile(clientDirectory / Data::Items::GetItemStatsFile());
        REQUIRE_FALSE(Data::Items::HasFatal(stats.issues));
        m_file = std::move(stats.file);
        Data::Items::WrittenStatProfile* scroll = nullptr;
        for (Data::Items::WrittenStatProfile& profile : m_file.profiles)
        {
            if (profile.name == "scroll")
            {
                scroll = &profile;
            }
        }
        REQUIRE(scroll != nullptr);
        scroll->requirements.stats[static_cast<size_t>(Data::Items::ItemRequirementStat::Energy)] =
            Data::Items::RequirementFormula{false, 20, 3, 4, true};
        g_ItemDatabase.SetStatProfiles(Data::Items::ItemStatProfiles(m_file));
    }

    // The stat file with that scroll profile, for what reads the file itself (the item tools'
    // formula lines).
    const Data::Items::ItemStatFile& GetFile() const
    {
        return m_file;
    }

private:
    // Before the profiles are set, so they are put back after.
    ItemStatProfilesScope m_restore;
    Data::Items::ItemStatFile m_file;
};
