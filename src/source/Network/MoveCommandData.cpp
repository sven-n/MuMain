
#include "stdafx.h"
#include "MoveCommandData.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Core/Utilities/FileHandle.h"

using namespace SEASON3B;
using namespace mu::ui::window;

#pragma pack(push, 1)
typedef struct
{
    int index;
    char szMainMapName[32];
    char szSubMapName[32];
    int iReqLevel;
    int m_iReqMaxLevel;
    int iReqZen;
    int iGateNum;
} MOVEREQINFO_FILE;
#pragma pack(pop)

CMoveCommandData::CMoveCommandData() {}

CMoveCommandData::~CMoveCommandData()
{
    Release();
}

CMoveCommandData* CMoveCommandData::GetInstance()
{
    static CMoveCommandData s_Instance;
    return &s_Instance;
}

bool CMoveCommandData::Create(const std::wstring& filename)
{
    Core::Utilities::FileHandle fp(_wfopen(filename.c_str(), L"rb"));
    if (!fp)
        return false;

    int count = 0;
    if (!DataFileIO::ReadExact(fp.get(), &count, sizeof(int)))
    {
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        MOVEREQINFO_FILE moveReqInfo{};
        if (!DataFileIO::ReadExact(fp.get(), &moveReqInfo, sizeof moveReqInfo))
        {
            return false;
        }
        // Value-initialised: the flags stay false until the move window sets them.
        auto* pMoveInfoData = new MOVEINFODATA{};

        // cppcheck-suppress dangerousTypeCast
        BuxConvert((BYTE*)&moveReqInfo, sizeof moveReqInfo);
        pMoveInfoData->_ReqInfo.index = moveReqInfo.index;
        pMoveInfoData->_ReqInfo.iGateNum = moveReqInfo.iGateNum;
        pMoveInfoData->_ReqInfo.iReqLevel = moveReqInfo.iReqLevel;
        pMoveInfoData->_ReqInfo.iReqZen = moveReqInfo.iReqZen;
        pMoveInfoData->_ReqInfo.m_iReqMaxLevel = moveReqInfo.m_iReqMaxLevel;
        CMultiLanguage::ConvertFromUtf8(pMoveInfoData->_ReqInfo.szMainMapName, moveReqInfo.szMainMapName,
                                        sizeof moveReqInfo.szMainMapName);
        CMultiLanguage::ConvertFromUtf8(pMoveInfoData->_ReqInfo.szSubMapName, moveReqInfo.szSubMapName,
                                        sizeof moveReqInfo.szSubMapName);

        m_listMoveInfoData.push_back(pMoveInfoData);
    }

    return true;
}

void CMoveCommandData::Release()
{
    auto li = m_listMoveInfoData.begin();
    for (; li != m_listMoveInfoData.end(); li++)
        delete (*li);
    m_listMoveInfoData.clear();
}

bool CMoveCommandData::OpenMoveReqScript(const std::wstring& filename)
{
    return CMoveCommandData::GetInstance()->Create(filename);
}

int CMoveCommandData::GetNumMoveMap()
{
    if (m_listMoveInfoData.size() > 0)
        return m_listMoveInfoData.size();

    return -1;
}

const CMoveCommandData::MOVEINFODATA* CMoveCommandData::GetMoveCommandDataByIndex(int iIndex)
{
    auto li = m_listMoveInfoData.begin();
    for (; li != m_listMoveInfoData.end(); li++)
    {
        if ((*li)->_ReqInfo.index == iIndex)
        {
            return (*li);
        }
    }
    return 0;
}

const std::list<CMoveCommandData::MOVEINFODATA*>& CMoveCommandData::GetMoveCommandDatalist()
{
    return m_listMoveInfoData;
}