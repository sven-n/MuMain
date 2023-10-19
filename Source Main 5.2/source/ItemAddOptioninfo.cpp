// ItemAddOptioninfo.cpp: implementation of the ItemAddOptioninfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "UIManager.h"

#include "ItemAddOptioninfo.h"


#define ITEMADDOPTION_DATA_FILE L"Data\\Local\\ItemAddOption.bmd"

ItemAddOptioninfo* ItemAddOptioninfo::MakeInfo()
{
    auto* option = new ItemAddOptioninfo();
    return option;
}

ItemAddOptioninfo::ItemAddOptioninfo()
{
    bool Result = true;

    Result = OpenItemAddOptionInfoFile(ITEMADDOPTION_DATA_FILE);

    if (!Result)
    {
        wchar_t szMessage[256];
        ::swprintf(szMessage, L"%s file not found.\r\n", ITEMADDOPTION_DATA_FILE);
        g_ErrorReport.Write(szMessage);
        ::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
        ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
}

ItemAddOptioninfo::~ItemAddOptioninfo()
{
}

const bool ItemAddOptioninfo::OpenItemAddOptionInfoFile(const std::wstring& filename)
{
    FILE* fp = ::_wfopen(filename.c_str(), L"rb");
    if (fp != NULL)
    {
        int nSize = sizeof(ITEM_ADD_OPTION) * MAX_ITEM;

        ::fread(m_ItemAddOption, nSize, 1, fp);
        ::BuxConvert((BYTE*)m_ItemAddOption, nSize);
        ::fclose(fp);

        return true;
    }
    return false;
}

void ItemAddOptioninfo::GetItemAddOtioninfoText(std::vector<std::wstring>& outtextlist, int type)
{
    int optiontype = 0;
    int optionvalue = 0;

    for (int i = 0; i < 2; ++i)
    {
        std::wstring text;
        wchar_t TempText[100];

        if (i == 0)
        {
            optiontype = m_ItemAddOption[type].m_byOption1;
            optionvalue = m_ItemAddOption[type].m_byValue1;
        }
        else
        {
            optiontype = m_ItemAddOption[type].m_byOption2;
            optionvalue = m_ItemAddOption[type].m_byValue2;
        }

        switch (optiontype)
        {
        case 1: swprintf(TempText, GlobalText[2184], optionvalue);
            break;
        case 2: swprintf(TempText, GlobalText[2185], optionvalue);
            break;
        case 3: swprintf(TempText, GlobalText[2186], optionvalue);
            break;
        case 4: swprintf(TempText, GlobalText[2187], optionvalue);
            break;
        case 5: swprintf(TempText, GlobalText[2188], optionvalue);
            break;
        case 6: swprintf(TempText, GlobalText[2189], optionvalue);
            break;
        case 7: swprintf(TempText, GlobalText[2190]);
            break;
        case 8: swprintf(TempText, GlobalText[2191], optionvalue);
            break;
        }

        text = TempText;
        outtextlist.push_back(text);
    }
}