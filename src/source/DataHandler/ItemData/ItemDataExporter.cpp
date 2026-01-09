#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataExporter.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataExporter::ExportToCsv(wchar_t* fileName)
{
    FILE* csvFp = _wfopen(fileName, L"w");
    if (!csvFp)
    {
        return false;
    }

    // Write BOM for UTF-8
    fprintf(csvFp, "\xEF\xBB\xBF");

    // Write CSV header
    fprintf(csvFp, "Index,Name,TwoHand,Level,ItemSlot,SkillIndex,Width,Height,DamageMin,DamageMax,SuccessfulBlocking,Defense,MagicDefense,WeaponSpeed,WalkSpeed,Durability,MagicDur,MagicPower,RequireStrength,RequireDexterity,RequireEnergy,RequireVitality,RequireCharisma,RequireLevel,Value,Zen,AttType,RequireClass0,RequireClass1,RequireClass2,RequireClass3,RequireClass4,RequireClass5,RequireClass6,Resistance0,Resistance1,Resistance2,Resistance3,Resistance4,Resistance5,Resistance6,Resistance7\n");

    int itemCount = 0;
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

            fprintf(csvFp, "%d,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                i, escapedName,
                ItemAttribute[i].TwoHand ? 1 : 0,
                ItemAttribute[i].Level,
                ItemAttribute[i].m_byItemSlot,
                ItemAttribute[i].m_wSkillIndex,
                ItemAttribute[i].Width,
                ItemAttribute[i].Height,
                ItemAttribute[i].DamageMin,
                ItemAttribute[i].DamageMax,
                ItemAttribute[i].SuccessfulBlocking,
                ItemAttribute[i].Defense,
                ItemAttribute[i].MagicDefense,
                ItemAttribute[i].WeaponSpeed,
                ItemAttribute[i].WalkSpeed,
                ItemAttribute[i].Durability,
                ItemAttribute[i].MagicDur,
                ItemAttribute[i].MagicPower,
                ItemAttribute[i].RequireStrength,
                ItemAttribute[i].RequireDexterity,
                ItemAttribute[i].RequireEnergy,
                ItemAttribute[i].RequireVitality,
                ItemAttribute[i].RequireCharisma,
                ItemAttribute[i].RequireLevel,
                ItemAttribute[i].Value,
                ItemAttribute[i].iZen,
                ItemAttribute[i].AttType);

            for (int c = 0; c < MAX_CLASS; c++)
            {
                fprintf(csvFp, ",%d", ItemAttribute[i].RequireClass[c]);
            }

            for (int r = 0; r < MAX_RESISTANCE + 1; r++)
            {
                fprintf(csvFp, ",%d", ItemAttribute[i].Resistance[r]);
            }

            fprintf(csvFp, "\n");
            itemCount++;
        }
    }

    fclose(csvFp);
    return true;
}

#endif // _EDITOR
