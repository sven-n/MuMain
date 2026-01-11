#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataExporter.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "GameData/ItemData/ItemFieldDefs.h"

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
            char utf8Name[MAX_ITEM_NAME * 3] = {0};
            WideCharToMultiByte(CP_UTF8, 0, ItemAttribute[i].Name, -1, utf8Name, sizeof(utf8Name), NULL, NULL);

            // Escape quotes in name
            char escapedName[MAX_ITEM_NAME * 6] = {0};
            int idx = 0;
            for (int j = 0; utf8Name[j] != 0 && idx < (MAX_ITEM_NAME * 6 - 2); j++)
            {
                if (utf8Name[j] == '"')
                {
                    escapedName[idx++] = '"';
                    escapedName[idx++] = '"';
                }
                else
                {
                    escapedName[idx++] = utf8Name[j];
                }
            }
            escapedName[idx] = '\0';

            // Print index and name
            fprintf(csvFp, "%d,\"%s\"", i, escapedName);

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
