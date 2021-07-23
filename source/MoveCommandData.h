// MoveCommandData.h: interface for the CMoveCommandData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVECOMMANDDATA_H__CE4BD3EF_8287_4206_9913_909F28730A15__INCLUDED_)
#define AFX_MOVECOMMANDDATA_H__CE4BD3EF_8287_4206_9913_909F28730A15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace SEASON3B
{
	class CMoveCommandData  
	{
#ifdef _VS2008PORTING
	public:
#endif // _VS2008PORTING
#pragma pack(push, 1)
		typedef struct tagMOVEREQ
		{
			int		index;
			char	szMainMapName[32];		//. Main map name
			char	szSubMapName[32];		//. Substitute map name
			int		iReqLevel;				//. required level
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			int		m_iReqMaxLevel;			// 요구최대레벨
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			int		iReqZen;				//. required zen
			int		iGateNum;				//. Gate number
		} MOVEREQINFO;
#pragma pack(pop)

		typedef struct tagMOVEINFODATA
		{
			MOVEREQINFO _ReqInfo;
			bool		_bCanMove;
#ifdef ASG_ADD_GENS_SYSTEM
			bool		_bStrife;			// 분쟁 지역.
#endif	// ASG_ADD_GENS_SYSTEM
			bool		_bSelected;

			bool operator==(const int& iIndex) const {int iTempIndex=iIndex; return _ReqInfo.index==iTempIndex;};

		} MOVEINFODATA;

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	public:
		BYTE m_btKeyAddressBlock2[4];
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
		
	private:
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		int							MAX_MOVEREQSIZE;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		const int					MAX_MOVEREQSIZE;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		std::list<MOVEINFODATA*>	m_listMoveInfoData;
		
	protected:
		CMoveCommandData();
	public:
		virtual ~CMoveCommandData();

	public:
		static CMoveCommandData* GetInstance();
		static bool OpenMoveReqScript(const std::string& filename);
		
		int GetNumMoveMap();
		const MOVEINFODATA* GetMoveCommandDataByIndex(int iIndex);
		const std::list<MOVEINFODATA*>& GetMoveCommandDatalist();

	protected:
		bool Create(const std::string& filename);
		void Release();
		void BuxConvert(BYTE* pBuffer, int size);
	};	
}

#endif // !defined(AFX_MOVECOMMANDDATA_H__CE4BD3EF_8287_4206_9913_909F28730A15__INCLUDED_)
