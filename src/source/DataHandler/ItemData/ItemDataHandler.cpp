#include "stdafx.h"
#include "ItemDataHandler.h"
#include "ItemDataLoader.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"

#ifdef _EDITOR
#include "ItemDataSaver.h"
#include "ItemDataExporter.h"
#endif

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

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

bool CItemDataHandler::Load(wchar_t* fileName)
{
    return ItemDataLoader::Load(fileName);
}

#ifdef _EDITOR
bool CItemDataHandler::Save(wchar_t* fileName, std::string* outChangeLog)
{
    return ItemDataSaver::Save(fileName, outChangeLog);
}

bool CItemDataHandler::ExportToCsv(wchar_t* fileName)
{
    return ItemDataExporter::ExportToCsv(fileName);
}
#endif
