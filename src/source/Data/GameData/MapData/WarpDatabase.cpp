#include "stdafx.h"

#include "WarpDatabase.h"

#include "Data/GameData/MapData/WarpsJson.h"

namespace Data::Maps
{
WarpDatabase& WarpDatabase::GetInstance()
{
    static WarpDatabase database;
    return database;
}

void WarpDatabase::Build(const WarpList& list)
{
    m_list = list;
    ResolveItemTags(m_list);
    ++m_version;
}
} // namespace Data::Maps
