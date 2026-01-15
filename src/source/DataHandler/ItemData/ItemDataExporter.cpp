#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataExporter.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "GameData/ItemData/ItemFieldDefs.h"
#include "Utilities/StringUtils.h"
#include <string>

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

// X-Macro helpers for CSV generation
#define CSV_HEADER_SIMPLE(name, type, arraySize, width) "," #name
#define CSV_HEADER_ARRAY(nameWithIndex, baseName, index, type, width) "," #nameWithIndex

// Complete CSV header with Index and Name
#define CSV_FULL_HEADER "Index,Name" ITEM_FIELDS_SIMPLE(CSV_HEADER_SIMPLE) ITEM_FIELDS_ARRAYS(CSV_HEADER_ARRAY)

// Helper to print field values (handles bool conversion)
#define PRINT_FIELD_Bool(item, name) fprintf(csvFp, ",%d", (item).name ? 1 : 0)
#define PRINT_FIELD_Byte(item, name) fprintf(csvFp, ",%d", (item).name)
#define PRINT_FIELD_Word(item, name) fprintf(csvFp, ",%d", (item).name)
#define PRINT_FIELD_Int(item, name) fprintf(csvFp, ",%d", (item).name)

#define CSV_PRINT_SIMPLE(name, type, arraySize, width) PRINT_FIELD_##type(item, name);
#define CSV_PRINT_ARRAY(nameWithIndex, baseName, index, type, width) fprintf(csvFp, ",%d", item.baseName[index]);

bool ItemDataExporter::ExportToCsv(wchar_t* fileName)
{
    FILE* csvFp = _wfopen(fileName, L"w");
    if (!csvFp)
    {
        return false;
    }

    // Write BOM for UTF-8
    fprintf(csvFp, "\xEF\xBB\xBF");

    // Write CSV header using X-macros
    fprintf(csvFp, CSV_FULL_HEADER "\n");
    
    for (int i = 0; i < MAX_ITEM; i++)
    {
        // Only export items with names
        if (ItemAttribute[i].Name[0] != 0)
        {
            // Convert wide char name to UTF-8
            std::string utf8Name = StringUtils::WideToNarrow(ItemAttribute[i].Name);

            // Escape quotes in name
            std::string escapedName;
            for (const char* p = utf8Name.c_str(); *p != '\0'; ++p)
            {
                if (*p == '"')
                {
                    escapedName += "\"\"";
                }
                else
                {
                    escapedName += *p;
                }
            }

            // Print index and name
            fprintf(csvFp, "%d,\"%s\"", i, escapedName.c_str());

            // Print all fields using X-macros
            ITEM_ATTRIBUTE& item = ItemAttribute[i];
            ITEM_FIELDS_SIMPLE(CSV_PRINT_SIMPLE)
            ITEM_FIELDS_ARRAYS(CSV_PRINT_ARRAY)

            fprintf(csvFp, "\n");
        }
    }

    fclose(csvFp);
    return true;
}

#endif // _EDITOR
