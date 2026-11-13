// MoveCommandData.h: interface for the CMoveCommandData class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace GameLogic::Warps
{
struct ServerWarp;
}

namespace SEASON3B
{
    class CMoveCommandData
    {
    public:

#pragma pack(push, 1)
        typedef struct tagMOVEREQ
        {
            int		index;
            wchar_t	szMainMapName[32];
            wchar_t	szSubMapName[32];
            int		iReqLevel;
            int		m_iReqMaxLevel;
            int		iReqZen;
            int		iGateNum;
        } MOVEREQINFO;
#pragma pack(pop)

        typedef struct tagMOVEINFODATA
        {
            MOVEREQINFO _ReqInfo;
            bool		_bCanMove;
            bool		_bStrife;
            bool		_bSelected;

            bool operator==(const int& iIndex) const { int iTempIndex = iIndex; return _ReqInfo.index == iTempIndex; };
        } MOVEINFODATA;
    private:
        std::list<MOVEINFODATA*>	m_listMoveInfoData;

        // The warp list of the data files (movereq_<lang>.bmd), which is shown unless the server sends its own.
        std::vector<MOVEREQINFO> m_dataFileWarps;

        // One entry per warp index, kept until the end: copies of the list hold pointers to them, which have
        // to stay valid when the list changes.
        std::map<int, std::unique_ptr<MOVEINFODATA>> m_entries;

        uint32_t m_listVersion = 0;

    protected:
        CMoveCommandData();
    public:
        virtual ~CMoveCommandData();

    public:
        static CMoveCommandData* GetInstance();
        static bool OpenMoveReqScript(const std::wstring& filename);

        int GetNumMoveMap();
        const MOVEINFODATA* GetMoveCommandDataByIndex(int iIndex);
        const std::list<MOVEINFODATA*>& GetMoveCommandDatalist();

        // Shows the warps of the server instead of the ones of the data files (see GameLogic/Warps/WarpListMerge.h).
        void ApplyServerWarpList(const std::vector<GameLogic::Warps::ServerWarp>& serverWarps);

        // Shows the warps of the data files again, e.g. before connecting to another server.
        void RestoreDataFileWarpList();

        // Changes whenever the list changes, so that holders of a copy of the list know when to refresh it.
        uint32_t GetListVersion() const;

    protected:
        bool Create(const std::wstring& filename);
        void Release();

    private:
        void SetList(const std::vector<MOVEREQINFO>& warps);
    };
}
