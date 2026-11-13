
#include "stdafx.h"
#include "MoveCommandData.h"
#include "Data/DataHandler/DataFileIO.h"
#include "Core/Content/ContentStore.h"
#include "GameLogic/Warps/WarpListMerge.h"

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
    const Content::Stream fp = Content::OpenIO(filename);
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
        MOVEREQINFO info{};

        // cppcheck-suppress dangerousTypeCast
        BuxConvert((BYTE*)&moveReqInfo, sizeof moveReqInfo);
        info.index = moveReqInfo.index;
        info.iGateNum = moveReqInfo.iGateNum;
        info.iReqLevel = moveReqInfo.iReqLevel;
        info.iReqZen = moveReqInfo.iReqZen;
        info.m_iReqMaxLevel = moveReqInfo.m_iReqMaxLevel;
        CMultiLanguage::ConvertFromUtf8(info.szMainMapName, moveReqInfo.szMainMapName,
                                        sizeof moveReqInfo.szMainMapName);
        CMultiLanguage::ConvertFromUtf8(info.szSubMapName, moveReqInfo.szSubMapName, sizeof moveReqInfo.szSubMapName);

        m_dataFileWarps.push_back(info);
    }

    SetList(m_dataFileWarps);
    return true;
}

void CMoveCommandData::Release()
{
    m_listMoveInfoData.clear();
    m_entries.clear();
    m_dataFileWarps.clear();
}

void CMoveCommandData::SetList(const std::vector<MOVEREQINFO>& warps)
{
    m_listMoveInfoData.clear();
    for (const auto& info : warps)
    {
        auto& entry = m_entries[info.index];
        if (!entry)
        {
            // Value-initialised: the flags stay false until the move window sets them.
            entry = std::make_unique<MOVEINFODATA>();
        }

        entry->_ReqInfo = info;
        m_listMoveInfoData.push_back(entry.get());
    }

    ++m_listVersion;
}

void CMoveCommandData::ApplyServerWarpList(const std::vector<GameLogic::Warps::ServerWarp>& serverWarps)
{
    SetList(GameLogic::Warps::MergeWarpList(m_dataFileWarps, serverWarps));
}

void CMoveCommandData::RestoreDataFileWarpList()
{
    SetList(m_dataFileWarps);
}

uint32_t CMoveCommandData::GetListVersion() const
{
    return m_listVersion;
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