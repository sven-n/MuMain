#ifndef _COMGEM_H_
#define _COMGEM_H_

//////////////////////////////////////////////////////////////////////////
// 
//  내  용 : 축석, 영석 조합 NPC
//  
//  날  짜 : 2005년 2월 14일
//  
//  작성자 : 안 성 민
//  
//////////////////////////////////////////////////////////////////////////
#include "UIManager.h"

typedef std::pair<int, BYTE> INTBYTEPAIR;

namespace COMGEM
{
	//작업목록
	//UI가 번경됨에 따라 필요없는 함수와 변수를 정리한다.
	//인벤토리에서 필효한 아이템을 찾아내는 함수를 구성한다.

	//Data
	extern	BOOL	m_bType;	//0 : 조합, 1 : 해체
	extern	char	m_cGemType; //0 : 축석,	1 : 영석,	-1 : 미선택
	extern	char	m_cComType;	//조합타입, 10개, 20개, 30개
	extern	BYTE	m_cCount;	//인벤창에 올라 있는 보석의 갯수
	extern	int		m_iValue;	//현재 비용
	extern	BYTE	m_cPercent;	//현재 확률
	extern	char	m_cState;	//창의 모드
	extern	char	m_cErr;		//발생 에러 타입
	extern	CUIUnmixgemList	m_UnmixTarList;

	void	Init();
	void	Exit();
	void	GetBack();
	void	RenderUnMixList();	//해체할 보석 리스트 그리는 함수
	void	MoveUnMixList();	//해채할 보석 리스트 처리
	void	CalcGen();			//금액 계산 함수
	int		CalcItemValue(const ITEM*);	//한 개 아이템의 가격조사
	char	CalcCompiledCount(const ITEM*); //몇 개가 누적되어 있는지 조사
	int		CalcEmptyInv();				//현재 내 인벤에 빈 공간이 몇 개 있는지 체크
	char	CheckOneItem(const ITEM*);	//한 개의 아이템을 검사해서 어떤 아이템인지 리턴
	bool	CheckMyInvValid();			//내 인벤 검사해서 오류코드 세팅
	bool	CheckInv();					//최종 검사

	//보석 해체용 New Interface
	bool	FindWantedList();			//내 인벤토리에서 선택된 보석의 아이템 인덱스를 찾아서 저장한다.
	void	ResetWantedList();			//창을 리셋한다.
	void	SelectFromList(int iIndex, int iLevel);
	int		GetUnMixGemLevel();			//해체할 보석의 수를 리턴

	//서버와 통신
	void	SendReqMix();
	void	SendReqUnMix();
	void	ProcessCSAction();

#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	void	SetJewelList( void );
	int		GetJewelRequireCount( int i );
	int		GetJewelIndex( int _nJewel, int _nType );
	int		Check_Jewel( int _nJewel, int _nType = 0, bool _bModel=false );		// 보석 조합/해제가 가능한 아이템인지 체크
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX

	//인라인들
	inline	void	SetMode(BOOL mode)	{ m_bType = mode; }		//조합-해체 전환
	inline	void	SetGem(char gem)	{ m_cGemType = gem; }	//조합 보석 선택
	inline	void	SetComType(char type)	{ m_cComType = type; }	//조합 갯수 선택
	inline	void	SetState(char state)	{ m_cState = state; }	//상태 설정
	inline	void	SetError(char err)	{ m_cErr = err; }
	inline	char	GetError()	{ return m_cErr; }
	inline	bool	isComMode() { return (m_bType == ATTACH); }

#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 보석조합 묶음 보석인지 검사한다. (2010.10.28)
	__inline int	Check_Jewel_Unit( int _nJewel, bool _bModel = false )	{ return Check_Jewel(_nJewel, 1, _bModel); }	// 보석 조합에 쓰이는 단품 보석인가
	__inline int	Check_Jewel_Com( int _nJewel, bool _bModel = false )	{ return Check_Jewel(_nJewel, 2, _bModel); }	// 보석 조합에 쓰이는 묶음 보석인가
	__inline bool	isCompiledGem(const ITEM* p)							{ return ( Check_Jewel_Com(p->Type) != NOGEM ? true: false); }
#else // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	inline	bool	isCompiledGem(const ITEM* p) { return ( CheckOneItem(p) >= COMCELE); }
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	inline	bool	isAble() { return (m_cState == STATE_READY); }

#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
}

#endif //_COMGEM_H_