#include "stdafx.h"
#include "ItemDataHandler.h"
#include "ItemDataLoader.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <chrono>
#include <memory>

#ifdef _EDITOR
#include "ItemDataSaver.h"
#include "ItemDataExportS6E3.h"
#include "ItemDataExportAsCSV.h"
#endif

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

namespace
{
// Logs always use English item names, so they are loaded from this language
// folder when another language is selected.
constexpr const wchar_t* EnglishLanguage = L"Eng";

double MillisecondsSince(std::chrono::steady_clock::time_point start)
{
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count();
}
} // namespace

CItemDataHandler::CItemDataHandler()
{
}

CItemDataHandler& CItemDataHandler::GetInstance()
{
    static CItemDataHandler instance;
    return instance;
}

ITEM_ATTRIBUTE* CItemDataHandler::GetItemAttributes()
{
    return ItemAttribute;
}

ITEM_ATTRIBUTE* CItemDataHandler::GetItemAttribute(int index)
{
    if (index >= 0 && index < MAX_ITEM)
        return &ItemAttribute[index];
    return nullptr;
}

int CItemDataHandler::GetItemCount() const
{
    return MAX_ITEM;
}

std::wstring CItemDataHandler::GetItemFilePath(const std::wstring& language)
{
    return L"Data\\Local\\" + language + L"\\Item_" + language + L".bmd";
}

bool CItemDataHandler::Load(const wchar_t* fileName)
{
    const auto loadStart = std::chrono::steady_clock::now();
    if (!ItemDataLoader::Load(fileName, ItemAttribute))
    {
        return false;
    }

    LoadEnglishNames();

    const auto buildStart = std::chrono::steady_clock::now();
    RebuildItemDatabase();

    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} items from {} in {:.1f} ms (item database build {:.2f} ms)",
                g_ItemDatabase.GetExistingItemCount(), mu_wchar_to_utf8(fileName), MillisecondsSince(loadStart),
                MillisecondsSince(buildStart));
    return true;
}

void CItemDataHandler::LoadEnglishNames()
{
    m_englishNames.clear();
    if (_wcsicmp(g_strSelectedML.c_str(), EnglishLanguage) == 0)
    {
        return;
    }

    const std::wstring englishFile = GetItemFilePath(EnglishLanguage);
    auto englishAttributes = std::make_unique<ITEM_ATTRIBUTE[]>(MAX_ITEM);
    if (!ItemDataLoader::Load(englishFile.c_str(), englishAttributes.get()))
    {
        MU_LOG_WARN(mu::log::Get("data"), "English item names not available ({}); logs use the loaded names.",
                    mu_wchar_to_utf8(englishFile.c_str()));
        return;
    }

    m_englishNames.resize(MAX_ITEM);
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        m_englishNames[itemType] = mu_wchar_to_utf8(englishAttributes[itemType].Name);
    }
}

void CItemDataHandler::RebuildItemDatabase()
{
    g_ItemDatabase.Build(std::span<const ITEM_ATTRIBUTE>(ItemAttribute, MAX_ITEM), m_englishNames);
}

#ifdef _EDITOR
bool CItemDataHandler::Save(wchar_t* fileName, std::string* outChangeLog)
{
    if (!ItemDataSaver::Save(fileName, outChangeLog))
    {
        return false;
    }

    // The editor changes ItemAttribute directly; keep the database in sync.
    RebuildItemDatabase();
    return true;
}

bool CItemDataHandler::ExportAsS6E3(wchar_t* fileName)
{
    return ItemDataExportS6E3::SaveLegacy(fileName);
}

bool CItemDataHandler::ExportToCsv(wchar_t* fileName)
{
    return ItemDataExportAsCSV::ExportToCsv(fileName);
}
#endif
