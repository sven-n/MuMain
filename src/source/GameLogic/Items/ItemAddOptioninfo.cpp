
#include "stdafx.h"
#include "Core/Platform/Quit.h"
#include "Core/Content/ContentStore.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/UIManager.h"
#include "I18N/All.h"

#include "GameLogic/Items/ItemAddOptioninfo.h"


#define ITEMADDOPTION_DATA_FILE L"Data\\Local\\ItemAddOption.bmd"

ItemAddOptioninfo* ItemAddOptioninfo::MakeInfo()
{
    auto* option = new ItemAddOptioninfo();
    return option;
}

ItemAddOptioninfo::ItemAddOptioninfo()
{
    if (const wchar_t* problem = OpenItemAddOptionInfoFile(ITEMADDOPTION_DATA_FILE))
    {
        wchar_t szMessage[256];
        ::mu_swprintf(szMessage, L"%ls %ls\r\n", ITEMADDOPTION_DATA_FILE, problem);
        g_ErrorReport.Write(szMessage);
        Core::Platform::ShowFatalError(L"Error", szMessage);
    }
}

ItemAddOptioninfo::~ItemAddOptioninfo()
{
}

const wchar_t* ItemAddOptioninfo::OpenItemAddOptionInfoFile(const std::wstring& filename)
{
    const Content::Stream fp = Content::OpenIO(filename);
    if (fp != nullptr)
    {
        int nSize = sizeof(ITEM_ADD_OPTION) * MAX_ITEM;

        const bool complete = DataFileIO::ReadExact(fp.get(), m_ItemAddOption, nSize);
        if (!complete)
        {
            return L"file corrupted.";
        }
        ::BuxConvert(reinterpret_cast<BYTE*>(m_ItemAddOption), nSize);

        return nullptr;
    }
    return L"file not found.";
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
        case 1: mu_swprintf(TempText, I18N::Game::AttackSucessRateIncreaseD, optionvalue);
            break;
        case 2: mu_swprintf(TempText, I18N::Game::AdditionalDamageD, optionvalue);
            break;
        case 3: mu_swprintf(TempText, I18N::Game::DefenseSuccessRateIncreaseD, optionvalue);
            break;
        case 4: mu_swprintf(TempText, I18N::Game::DefensiveSkillD, optionvalue);
            break;
        case 5: mu_swprintf(TempText, I18N::Game::MaxHPIncreaseD, optionvalue);
            break;
        case 6: mu_swprintf(TempText, I18N::Game::MaxSDIncreaseD, optionvalue);
            break;
        case 7: mu_swprintf(TempText, I18N::Game::SDAutoRecovery);
            break;
        case 8: mu_swprintf(TempText, I18N::Game::SDRecoveryRateIncreaseD, optionvalue);
            break;
        }

        text = TempText;
        outtextlist.push_back(text);
    }
}