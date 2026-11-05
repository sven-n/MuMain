
#include "stdafx.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Core/Utilities/FileHandle.h"
#include "Core/Utilities/ReadScript.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "w_BuffScriptLoader.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Utilities/Log/MuLogger.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
void CutTokenString(wchar_t* pcCuttoken, std::list<std::wstring>& out)
{
    if (wcslen(pcCuttoken) == 0)
        return;

    int cutpos = 0;

    for (int i = 0; i < MAX_DESCRIPT_LENGTH; ++i)
    {
        if (pcCuttoken[i] == '/' || pcCuttoken[i] == 0)
        {
            wchar_t Temp[MAX_DESCRIPT_LENGTH] = {
                0,
            };
            wcsncpy(Temp, pcCuttoken + cutpos, i - cutpos);
            out.push_back(Temp);
            cutpos = i + 1;

            if (pcCuttoken[i] == 0)
                return;
        }
    }
}
} // namespace

BuffInfo::BuffInfo()
    : s_BuffIndex(0), s_BuffEffectType(0), s_ItemType(255), s_ItemIndex(255), s_BuffClassType(0), s_NoticeType(0),
      s_ClearType(0)
{
    memset(&s_BuffName, 0, sizeof(wchar_t) * MAX_BUFF_NAME_LENGTH);
    memset(&s_BuffDescript, 0, sizeof(wchar_t) * MAX_DESCRIPT_LENGTH);
}

BuffInfo::~BuffInfo() {}

BuffScriptLoaderPtr BuffScriptLoader::Make()
{
    BuffScriptLoaderPtr info(new BuffScriptLoader());
    return info;
}

BuffScriptLoader::BuffScriptLoader()
{
    std::wstring filename = L"data/local/" + g_strSelectedML + L"/BuffEffect_" + g_strSelectedML + L".bmd";

    if (!Load(filename))
    {
        assert(0);
    }
}

BuffScriptLoader::~BuffScriptLoader() {}

namespace
{
// Like DataFileIO::ReportUnusableFile, but logged through mu::log instead of the error report (g_ErrorReport): the
// buff list comes with every character, and the error report would bring the crash reporter, and with it the
// renderer and the audio, into every binary that links it, the unit tests without Winmain.cpp included.
void ReportUnusableBuffFile(const std::wstring& fileName, const wchar_t* issue)
{
    mu::log::Get("gameplay")->warn("{} - {}", mu_wchar_to_utf8(fileName.c_str()), mu_wchar_to_utf8(issue));
    wchar_t text[256]{};
    mu_swprintf(text, L"%ls - %ls", fileName.c_str(), issue);
    MessageBox(g_hWnd, text, NULL, MB_OK);
    SendMessage(g_hWnd, WM_DESTROY, 0, 0);
}
} // namespace

bool BuffScriptLoader::Load(const std::wstring& pchFileName)
{
    Core::Utilities::FileHandle fp(_wfopen(pchFileName.c_str(), L"rb"));

    if (fp)
    {
        DWORD structsize = sizeof(_BUFFINFO);

        DWORD listsize;
        if (!DataFileIO::ReadExact(fp.get(), &listsize, sizeof(DWORD)))
        {
            ReportUnusableBuffFile(pchFileName, L"File corrupted.");
            return true;
        }

        std::vector<BYTE> content(static_cast<std::size_t>(structsize) * listsize);
        BYTE* Buffer = content.data();
        DWORD dwCheckSum;
        const bool complete = DataFileIO::ReadExact(fp.get(), Buffer, structsize * listsize) &&
                              DataFileIO::ReadExact(fp.get(), &dwCheckSum, sizeof(DWORD));

        fp.reset();
        if (!complete || dwCheckSum != GenerateCheckSum2(Buffer, structsize * listsize, 0xE2F1))
        {
            ReportUnusableBuffFile(pchFileName, L"File corrupted.");
        }
        else
        {
            BYTE* pSeek = Buffer;

            for (DWORD i = 0; i < listsize; i++)
            {
                _BUFFINFO tempbuffinfo;

                BuxConvert(pSeek, structsize);
                memcpy(&tempbuffinfo, pSeek, structsize);

                BuffInfo buffinfo;
                buffinfo.s_BuffIndex = tempbuffinfo.s_BuffIndex;
                buffinfo.s_BuffEffectType = tempbuffinfo.s_BuffEffectType;
                buffinfo.s_ItemType = tempbuffinfo.s_ItemType;
                buffinfo.s_ItemIndex = tempbuffinfo.s_ItemIndex;

                CMultiLanguage::ConvertFromUtf8(buffinfo.s_BuffName, tempbuffinfo.s_BuffName, MAX_BUFF_NAME_LENGTH);
                CMultiLanguage::ConvertFromUtf8(buffinfo.s_BuffDescript, tempbuffinfo.s_BuffDescript,
                                                MAX_DESCRIPT_LENGTH);

                buffinfo.s_BuffClassType = tempbuffinfo.s_BuffClassType;
                buffinfo.s_NoticeType = tempbuffinfo.s_NoticeType;
                buffinfo.s_ClearType = tempbuffinfo.s_ClearType;

                CutTokenString(buffinfo.s_BuffDescript, buffinfo.s_BuffDescriptlist);
                m_Info.insert(std::make_pair(static_cast<eBuffState>(buffinfo.s_BuffIndex), buffinfo));

                pSeek += structsize;
            }
        }
    }
    else
    {
        ReportUnusableBuffFile(pchFileName, L"File not exist.");
    }

    return true;
}

const BuffInfo BuffScriptLoader::GetBuffinfo(eBuffState type) const
{
    if (type >= eBuff_Count)
        return BuffInfo();

    auto iter = m_Info.find(type);

    if (iter != m_Info.end())
    {
        return (*iter).second;
    }

    return BuffInfo();
}

eBuffClass BuffScriptLoader::IsBuffClass(eBuffState type) const
{
    if (type >= eBuff_Count)
        return eBuffClass_Count;

    auto iter = m_Info.find(type);

    if (iter != m_Info.end())
    {
        return static_cast<eBuffClass>((*iter).second.s_BuffClassType);
    }
    else
    {
        return eBuffClass_Count;
    }
}

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
int BuffScriptLoader::GetBuffIndex(int iItemCode)
{
    auto iter = m_Info.begin();

    int iterItemCode = 0;

    while (iter != m_Info.end())
    {
        iterItemCode = ITEMINDEX(iter->second.s_ItemType, iter->second.s_ItemIndex);

        if (iterItemCode == iItemCode)
            return iter->second.s_BuffIndex;

        iter++;
    }

    return -1;
}

int BuffScriptLoader::GetBuffType(int iItemCode)
{
    auto iter = m_Info.begin();

    int iterItemCode = 0;

    while (iter != m_Info.end())
    {
        iterItemCode = ITEMINDEX(iter->second.s_ItemType, iter->second.s_ItemIndex);

        if (iterItemCode == iItemCode)
            return iter->second.s_BuffEffectType;

        iter++;
    }

    return -1;
}
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
