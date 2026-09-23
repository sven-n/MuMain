// ItemNames.h: localized item names for ItemAttribute.
//////////////////////////////////////////////////////////////////////

#pragma once

namespace Data::Items::Names
{
    // Overwrites ItemAttribute[].Name with the active locale's names from
    // I18N::Items and keeps them in sync on locale changes. Call once after
    // the item data file has been loaded; items the resx has no entry for
    // keep the name that came from the data file.
    void Initialize();

    // Stops tracking locale changes. Leaves the names currently applied.
    void Shutdown();
}
