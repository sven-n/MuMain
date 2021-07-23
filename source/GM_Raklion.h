// GM_Raklion.h: interface for the CGM_Raklion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GM_RAKLION_H__00399A9C_5F21_4784_B501_278DE4C74A81__INCLUDED_)
#define AFX_GM_RAKLION_H__00399A9C_5F21_4784_B501_278DE4C74A81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef CSK_ADD_MAP_ICECITY

class BMD;

#include "w_BaseMap.h"
#include "./Time/Timer.h"

namespace SEASON4A
{
	BoostSmartPointer( CGM_Raklion );
	class CGM_Raklion : public BaseMap  
	{
	public:
		enum RAKLION_STATE			// 라클리온 전체 상태 스케쥴
		{
			RAKLION_STATE_IDLE				= 0,	// 라클리온 보스전 대기 상태(항상 대기 상태이다)
			RAKLION_STATE_NOTIFY_1			= 1,	// 일정량의 거미알이 깨졌다.(클라이언트에 메시지 보냄)
			RAKLION_STATE_STANDBY			= 2,	// 보스 출현전 연출을 위한 딜레이
			RAKLION_STATE_NOTIFY_2			= 3,	// 서버 메시지 출력(세루판이 등장하였습니다.)
			RAKLION_STATE_READY				= 4,	// 보스전을 준비 - 보스 생성 및 최초 스킬 발동
			RAKLION_STATE_START_BATTLE		= 5,	// 보스전 진행 상태 - 보스전 시작 후 5분간은 열린 상태이다
			RAKLION_STATE_NOTIFY_3			= 6,	// 서버 메시지 출력(라클리온 보스존이 닫혔습니다.)
			RAKLION_STATE_CLOSE_DOOR		= 7,	// 보스전 진행 상태 - 보스존 닫힌 상태로 진행
			RAKLION_STATE_ALL_USER_DIE		= 8,	// 보스존의 유저가 모두 죽은 상태
			RAKLION_STATE_NOTIFY_4			= 9,	// 서버 메시지 출력(보스전 종료) - 5분간 대기
			RAKLION_STATE_END				= 10,	// 전투 종료 - 보스존 다시 열림
			RAKLION_STATE_DETAIL_STATE		= 11,	// 세루판과의 보스전 세부 상태를 보낼때 쓰는 값
			RAKLION_STATE_MAX				= 12,
		};

		enum RAKLION_BATTLE_OF_SELUPAN_PATTERN				// 세루판 상태 스케쥴
		{
			BATTLE_OF_SELUPAN_NONE					= 0,	// 세루판전 상태가 아님(보스전 시작 안함)
			BATTLE_OF_SELUPAN_STANDBY				= 1,	// 세루판전 준비 상태
			BATTLE_OF_SELUPAN_PATTERN_1				= 2,	// 세루판 패턴 1단계
			BATTLE_OF_SELUPAN_PATTERN_2				= 3,	// 세루판 패턴 2단계(광폭화 1단계)
			BATTLE_OF_SELUPAN_PATTERN_3				= 4,	// 세루판 패턴 3단계(광폭화 2단계)
			BATTLE_OF_SELUPAN_PATTERN_4				= 5,	// 세루판 패턴 4단계(광폭화 2단계)
			BATTLE_OF_SELUPAN_PATTERN_5				= 6,	// 세루판 패턴 5단계(광폭화 3단계)
			BATTLE_OF_SELUPAN_PATTERN_6				= 7,	// 세루판 패턴 6단계(광폭화 4단계)
			BATTLE_OF_SELUPAN_PATTERN_7				= 8,	// 세루판 패턴 7단계(광폭화 4단계)
			BATTLE_OF_SELUPAN_DIE					= 9,	// 세루판이 죽었다.
			BATTLE_OF_SELUPAN_MAX					= 10,	// 세루판전 최대 상태 수
		};

	public:
		static CGM_RaklionPtr Make();
		virtual ~CGM_Raklion();

	public:	// Object
		// 오브젝트 생성
		virtual bool CreateObject(OBJECT* o);
		// 오브젝트 프로세서
		virtual bool MoveObject(OBJECT* o);
		// 오브젝트 이펙트
		virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
		// 오브젝트 랜더
		virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
		// 맵 관련 오브젝트 이펙트
		virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
		
	public:	// Character
		// 몬스터 생성
		virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
		// 몬스터(NPC) 프로세서
		virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
		// 몬스터 스킬 블러 이펙트
		virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
		// 몬스터 이펙트 ( 일반 )	
		virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
		// 몬스터 이펙트 ( 스킬 )
		virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
		// 스킬 애니메이션 관련 함수
		virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
		
	public: // Sound
		// 몬스터 사운드
		virtual bool PlayMonsterSound(OBJECT* o);
		// 오브젝트 사운드
		virtual void PlayObjectSound(OBJECT* o);
		// 배경음악
		void PlayBGM();

	public:
		void Init();
		void Destroy();

	//-----------------------------------------------------------------------------------------------

	private:
		CGM_Raklion();
		
		// 몬스터 렌더링
		bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);

		void SetBossMonsterAction(CHARACTER* c, OBJECT* o);

	public:
		// 맵에 눈 내리게 하는 효과
		bool CreateSnow( PARTICLE* o );
		// 화면 전체에 노이즈 효과
		void RenderBaseSmoke();
		// 보스맵 현재 상태 세팅
		void SetState(BYTE byState, BYTE byDetailState);
		// 라클리온 보스맵에 들어갈 수 있는가?
		bool CanGoBossMap();
		// 라클리온 보스맵에 들어갈 수 있나 없나를 설정해주는 함수
		void SetCanGoBossMap();
		// 라클리온 보스맵 연출 설정
		void SetEffect();
		// 라클리온 보스맵 연출 업데이트
		void MoveEffect();
		// 라클리온 보스맵 연출 이펙트 발생 함수
		void CreateMapEffect();
	
	private:
		CTimer2 m_Timer;			// 보스맵 연출용 타이머
		BYTE m_byState;				// 보스맵 현재 상태
		BYTE m_byDetailState;		// 보스맵 현재 세부상태
		bool m_bCanGoBossMap;		// 보스맵에 들어갈 수 있는 상태인가? 들어갈 수 있으면 true, 못들어가면 false
		bool m_bVisualEffect;		// 보스맵 연출 보이는가 상태 변수
		bool m_bMusicBossMap;		// 보스맵 BGM 플레이 시키는가?
		bool m_bBossHeightMove;		// 보스 몬스터 높이값 변경시켰는가 예외처리 변수.
	};
}

// 라클리온(빙하)맵 인가?
extern bool IsIceCity();
	
#endif // CSK_ADD_MAP_ICECITY

#endif // !defined(AFX_GM_RAKLION_H__00399A9C_5F21_4784_B501_278DE4C74A81__INCLUDED_)
