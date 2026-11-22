#pragma once

#include "Data/GameData/MapData/Warp.h"

namespace Data::Maps
{
// The warp list in memory (Data/Maps/Warps.json), in the file's order. Built
// at start.
class WarpDatabase
{
public:
    static WarpDatabase& GetInstance();

    // Replaces the whole list.
    void Build(const WarpList& list);
    const WarpList& GetList() const
    {
        return m_list;
    }
    // Changes with every Build: what was taken from the list is old.
    int GetVersion() const
    {
        return m_version;
    }

private:
    WarpDatabase() = default;

    WarpList m_list;
    int m_version = 0;
};
} // namespace Data::Maps

#define g_WarpDatabase Data::Maps::WarpDatabase::GetInstance()
