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

// Returns one UTF-8 name per item type, or nothing when the loaded language
// is English already or the English file is missing. Then the loaded names
// are used for logs.
std::vector<std::string> LoadEnglishNames(const std::wstring& loadedLanguage)
{
    if (_wcsicmp(loadedLanguage.c_str(), EnglishLanguage) == 0)
    {
        return {};
    }

    const std::wstring englishFile = CItemDataHandler::GetItemFilePath(EnglishLanguage);
    auto englishAttributes = std::make_unique<ITEM_ATTRIBUTE[]>(MAX_ITEM);
    if (!ItemDataLoader::Load(englishFile.c_str(), englishAttributes.get(), ItemDataLoader::Reporting::Quiet))
    {
        MU_LOG_WARN(mu::log::Get("data"), "English item names not available ({}); logs use the loaded names.",
                    mu_wchar_to_utf8(englishFile.c_str()));
        return {};
    }

    std::vector<std::string> englishNames(MAX_ITEM);
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        englishNames[itemType] = mu_wchar_to_utf8(englishAttributes[itemType].Name);
    }
    return englishNames;
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

bool CItemDataHandler::Load(const std::wstring& language)
{
    const std::wstring fileName = GetItemFilePath(language);
    const auto loadStart = std::chrono::steady_clock::now();
    if (!ItemDataLoader::Load(fileName.c_str(), ItemAttribute))
    {
        return false;
    }
    const double loadMilliseconds = MillisecondsSince(loadStart);

    const auto englishNamesStart = std::chrono::steady_clock::now();
    std::vector<std::string> englishNames = LoadEnglishNames(language);
    const double englishNamesMilliseconds = MillisecondsSince(englishNamesStart);

    const auto buildStart = std::chrono::steady_clock::now();
    g_ItemDatabase.Build(std::span<const ITEM_ATTRIBUTE>(ItemAttribute, MAX_ITEM), englishNames);
    const double buildMilliseconds = MillisecondsSince(buildStart);

    MU_LOG_INFO(mu::log::Get("data"),
                "Loaded {} items from {} in {:.1f} ms (English names {:.1f} ms, item database build {:.2f} ms)",
                g_ItemDatabase.GetExistingItemCount(), mu_wchar_to_utf8(fileName.c_str()), loadMilliseconds,
                englishNamesMilliseconds, buildMilliseconds);

#ifdef _EDITOR
    m_englishNames = std::move(englishNames);
#endif
    return true;
}

#ifdef _EDITOR
void CItemDataHandler::RebuildItemDatabase()
{
    g_ItemDatabase.Build(std::span<const ITEM_ATTRIBUTE>(ItemAttribute, MAX_ITEM), m_englishNames);
}

bool CItemDataHandler::Save(const wchar_t* fileName, std::string* outChangeLog)
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
