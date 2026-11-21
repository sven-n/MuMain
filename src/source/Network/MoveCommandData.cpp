
#include "stdafx.h"
#include "MoveCommandData.h"
#include "Core/Text/Utf8.h"
#include "GameLogic/Warps/WarpListMerge.h"
#include "I18N/All.h"

#include <algorithm>
#include <cwchar>

using namespace SEASON3B;
using namespace mu::ui::window;

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

CMoveCommandData::MOVEREQINFO CMoveCommandData::MakeMoveReqInfo(const Data::Maps::Warp& warp, std::string_view locale)
{
    MOVEREQINFO info{};
    info.index = warp.index;
    CopyName(info.szMainMapName, Core::Text::FromUtf8(warp.names.Get(locale)));
    CopyName(info.szSubMapName, Core::Text::FromUtf8(warp.command));
    info.iReqLevel = warp.level;
    info.iReqZen = warp.zen;
    info.iGateNum = warp.gate;
    info.iReqMasterLevel = warp.minMasterLevel;
    info.iReqResets = warp.minResets;
    return info;
}

void CMoveCommandData::SetDataFileWarps(const Data::Maps::WarpList& list, std::string_view locale)
{
    m_dataFileList = list;
    SetWarpNameLocale(locale);
}

void CMoveCommandData::SetWarpNameLocale(std::string_view locale)
{
    // All or nothing: out of memory, the names, their language and the shown list stay as they were
    // (OnLocaleChanged), so the next change of the UI language tries again.
    std::string newLocale(locale);
    std::vector<MOVEREQINFO> warps;
    warps.reserve(m_dataFileList.warps.size());
    for (const Data::Maps::Warp& warp : m_dataFileList.warps)
    {
        warps.push_back(MakeMoveReqInfo(warp, newLocale));
    }
    m_dataFileWarps.swap(warps);
    try
    {
        ShowCurrentList();
    }
    catch (...)
    {
        m_dataFileWarps.swap(warps);
        throw;
    }
    m_locale = std::move(newLocale);
}

void CMoveCommandData::FollowUiLanguage()
{
    if (!m_followsUiLanguage)
    {
        I18N::RegisterLocaleObserver(&CMoveCommandData::OnLocaleChanged, this);
        m_followsUiLanguage = true;
    }
    if (m_locale != I18N::GetCurrentLocale())
    {
        SetWarpNameLocale(I18N::GetCurrentLocale());
    }
}

void CMoveCommandData::StopFollowingUiLanguage()
{
    if (m_followsUiLanguage)
    {
        I18N::UnregisterLocaleObserver(&CMoveCommandData::OnLocaleChanged, this);
        m_followsUiLanguage = false;
    }
}

void CMoveCommandData::OnLocaleChanged(void* context) noexcept
{
    try
    {
        static_cast<CMoveCommandData*>(context)->SetWarpNameLocale(I18N::GetCurrentLocale());
    }
    catch (...)
    {
        // Out of memory: the names stay in the language they had (SetWarpNameLocale changes nothing then).
    }
}

void CMoveCommandData::ShowCurrentList()
{
    if (m_serverWarps)
    {
        SetList(GameLogic::Warps::MergeWarpList(m_dataFileWarps, *m_serverWarps));
    }
    else
    {
        SetList(m_dataFileWarps);
    }
}

void CMoveCommandData::Release()
{
    // The locale observer stays: the instance lives until the end, after the observer list.
    m_serverWarps.reset();
    m_listMoveInfoData.clear();
    m_entries.clear();
    m_dataFileWarps.clear();
}

void CMoveCommandData::SetList(const std::vector<MOVEREQINFO>& warps)
{
    // What allocates comes first, so running out of memory leaves the shown list as it was.
    std::list<MOVEINFODATA*> shown;
    for (const auto& info : warps)
    {
        auto& entry = m_entries[info.index];
        if (!entry)
        {
            // Value-initialised: the flags stay false until the move window sets them.
            entry = std::make_unique<MOVEINFODATA>();
        }
        shown.push_back(entry.get());
    }

    auto entry = shown.begin();
    for (const auto& info : warps)
    {
        (*entry++)->_ReqInfo = info;
    }
    m_listMoveInfoData = std::move(shown);
    ++m_listVersion;
}

void CMoveCommandData::ApplyServerWarpList(const std::vector<GameLogic::Warps::ServerWarp>& serverWarps)
{
    m_serverWarps = serverWarps;
    ShowCurrentList();
}

void CMoveCommandData::ApplyServerWarpRequirements(
    const std::vector<GameLogic::Warps::ServerWarpRequirement>& requirements)
{
    if (!m_serverWarps)
    {
        return;
    }

    for (GameLogic::Warps::ServerWarp& warp : *m_serverWarps)
    {
        // Of entries with the same index, the first one counts, as with the warps of the list.
        const auto requirement = std::find_if(requirements.begin(), requirements.end(),
                                              [&](const GameLogic::Warps::ServerWarpRequirement& entry)
                                              { return entry.index == warp.index; });
        warp.minMasterLevel = requirement != requirements.end() ? requirement->minMasterLevel : 0;
        warp.minResets = requirement != requirements.end() ? requirement->minResets : 0;
    }
    ShowCurrentList();
}

void CMoveCommandData::RestoreDataFileWarpList()
{
    m_serverWarps.reset();
    ShowCurrentList();
}

uint32_t CMoveCommandData::GetListVersion() const
{
    return m_listVersion;
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