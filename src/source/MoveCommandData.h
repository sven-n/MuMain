// MoveCommandData.h: interface for the CMoveCommandData class.
//////////////////////////////////////////////////////////////////////
#pragma once

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

    protected:
        bool Create(const std::wstring& filename);
        void Release();
    };
}
