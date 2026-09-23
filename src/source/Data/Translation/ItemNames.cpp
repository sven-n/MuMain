// ItemNames.cpp: localized item names for ItemAttribute.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ItemNames.h"
#include "Core/Globals/_define.h"
#include "Core/Platform/SecureCrt.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "I18N/All.h"
#include "I18N/Items.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

namespace
{
    bool g_bObserverRegistered = false;

    // The item index doubles as the legacy id in Items.<locale>.resx, so a
    // single Lookup per slot is all the mapping this needs.
    void ApplyLocalizedNames() noexcept
    {
        if (ItemAttribute == nullptr)
        {
            return;
        }

        for (int i = 0; i < MAX_ITEM; ++i)
        {
            const wchar_t* pszName = I18N::Items::Lookup(i);
            if (pszName == nullptr || pszName[0] == L'\0')
            {
                continue;
            }

            wcsncpy_s(ItemAttribute[i].Name, pszName, _TRUNCATE);
        }
    }

    void OnLocaleChanged(void* /*pContext*/) noexcept
    {
        ApplyLocalizedNames();
    }
}

namespace Data::Items::Names
{
    void Initialize()
    {
        ApplyLocalizedNames();

        if (g_bObserverRegistered)
        {
            return;
        }

        I18N::RegisterLocaleObserver(&OnLocaleChanged, nullptr);
        g_bObserverRegistered = true;
    }

    void Shutdown()
    {
        if (!g_bObserverRegistered)
        {
            return;
        }

        I18N::UnregisterLocaleObserver(&OnLocaleChanged, nullptr);
        g_bObserverRegistered = false;
    }
}
