// MoveCommandData.h: interface for the CMoveCommandData class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <algorithm>
#include <cstdint>
#include <cwchar>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Data/GameData/MapData/Warp.h"
#include "GameLogic/Warps/ServerWarpList.h"

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
            int		iReqZen;
            int		iGateNum;
            // The minimum master level and resets (0 = none): of the data files, or of the server
            // with its list (WarpRequirementsExtended).
            int iReqMasterLevel;
            int iReqResets;
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

        // The warp list of the data files (Data/Maps/Warps.json), which is shown unless the server sends its own.
        std::vector<MOVEREQINFO> m_dataFileWarps;
        // The same list as the data files have it, with the names in every language, and the language shown.
        Data::Maps::WarpList m_dataFileList;
        std::string m_locale;
        // The list of the server while it applies.
        std::optional<std::vector<GameLogic::Warps::ServerWarp>> m_serverWarps;
        bool m_followsUiLanguage = false;

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

        // A name into a name field of MOVEREQINFO, cut to fit.
        template <size_t N> static void CopyName(wchar_t (&target)[N], std::wstring_view name)
        {
            // In parentheses: a file with <windows.h> and no NOMINMAX has min as a macro.
            const size_t length = (std::min)(name.size(), N - 1);
            std::wmemcpy(target, name.data(), length);
            target[length] = L'\0';
        }

        // A warp of the data files as the list holds it, with its name in the language of `locale`.
        static MOVEREQINFO MakeMoveReqInfo(const Data::Maps::Warp& warp, std::string_view locale);

        // Shows the warps of the data files (Data/Maps/Warps.json), named in the language of `locale`
        // (English where a warp has no name in it).
        void SetDataFileWarps(const Data::Maps::WarpList& list, std::string_view locale);

        // The warps of the data files and the language of their names, as SetDataFileWarps took them.
        const Data::Maps::WarpList& GetDataFileWarps() const
        {
            return m_dataFileList;
        }
        const std::string& GetWarpNameLocale() const
        {
            return m_locale;
        }

        // Names the warps in the language of `locale`, in the list of the data files and in the one of the
        // server (the warps it has from the data files; the others keep the server's name).
        void SetWarpNameLocale(std::string_view locale);

        // From now on the names follow the UI language (I18N::SetLocale), as the monster names do.
        void FollowUiLanguage();
        bool FollowsUiLanguage() const
        {
            return m_followsUiLanguage;
        }
        // The names no longer follow the UI language (a test puts back the state it found).
        void StopFollowingUiLanguage();

        int GetNumMoveMap();
        const MOVEINFODATA* GetMoveCommandDataByIndex(int iIndex);
        const std::list<MOVEINFODATA*>& GetMoveCommandDatalist();

        // Shows the warps of the server instead of the ones of the data files (see GameLogic/Warps/WarpListMerge.h).
        void ApplyServerWarpList(const std::vector<GameLogic::Warps::ServerWarp>& serverWarps);

        // The minimum master level and resets of the warps of the server's list, which the server sends
        // right after it. They replace the ones the list had: a warp without an entry needs none. Without
        // a list of the server they are ignored, as they belong to it.
        void ApplyServerWarpRequirements(const std::vector<GameLogic::Warps::ServerWarpRequirement>& requirements);

        // Shows the warps of the data files again, e.g. before connecting to another server.
        void RestoreDataFileWarpList();

        // Changes whenever the list changes, so that holders of a copy of the list know when to refresh it.
        uint32_t GetListVersion() const;

    protected:
        void Release();

    private:
        void SetList(const std::vector<MOVEREQINFO>& warps);
        void ShowCurrentList();
        static void OnLocaleChanged(void* context) noexcept;
    };
}
