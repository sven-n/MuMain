// ursedTemple.h: interface for the CursedTemple class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_URSEDTEMPLE_H__82D1D8BF_A93D_49BE_B384_159381C2495D__INCLUDED_)
#define AFX_URSEDTEMPLE_H__82D1D8BF_A93D_49BE_B384_159381C2495D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class BMD;
namespace SEASON3A
{
	class CursedTemple
	{
	public:
		virtual ~CursedTemple();

	private:
		CursedTemple();
		void Initialize();
		void Destroy();

	public:
		void Process();
		void Draw();

	public:
		static CursedTemple* GetInstance();
		bool GetInterfaceState( int type, int subtype = -1 );
		void SetInterfaceState( bool state, int subtype = -1 );

	public:
		bool IsHolyItemPickState();
		bool IsCursedTemple();
		bool IsPartyMember( DWORD selectcharacterindex );
		void ReceiveCursedTempleInfo( BYTE* ReceiveBuffer );
		void ReceiveCursedTempleState( const eCursedTempleState state );

	public:
		bool CreateObject( OBJECT* o );
		CHARACTER* CreateCharacters(int iType, int iPosX, int iPosY, int iKey);

	public:
		bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
		bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
		void PlayBGM();
		void ResetCursedTemple();

	public:
		bool MoveObject( OBJECT* o ); 
		void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
		bool MoveMonsterVisual(OBJECT* o, BMD* b);
		void MoveMonsterSoundVisual(OBJECT* o, BMD* b);
		
	public:
		bool RenderObject_AfterCharacter( OBJECT* o, BMD* b );
		bool RenderObjectVisual( OBJECT* o, BMD* b );
		bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
		bool RenderObjectMesh( OBJECT* o, BMD* b, bool ExtraMon = 0 );
#ifdef PBG_FIX_CURSEDTEMPLE_SYSTEMMSG
		void UpdateTempleSystemMsg(int _Value);
#endif //PBG_FIX_CURSEDTEMPLE_SYSTEMMSG
#ifdef YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR
		void SetGaugebarEnabled(bool bFlag);
		void SetGaugebarCloseTimer();
		bool IsGaugebarEnabled();
#endif	// YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR
		
	private:
		bool			m_IsTalkEnterNpc;
		bool			m_InterfaceState;
		WORD			m_HolyItemPlayerIndex;
		eCursedTempleState	m_CursedTempleState;
		list<int>			m_TerrainWaterIndex;
		WORD				m_AlliedPoint;
		WORD				m_IllusionPoint;
		bool				m_ShowAlliedPointEffect;
		bool				m_ShowIllusionPointEffect;
#ifdef YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR
		bool				m_bGaugebarEnabled;
		float				m_fGaugebarCloseTimer;
#endif	// YDG_FIX_CURSEDTEMPLE_GAUGEBAR_ERROR
	};

	inline
	bool CursedTemple::IsCursedTemple()
	{
		extern int World;
		return ( World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6) ? true : false;
	}
};

#define g_TimeController SEASON3A::TimeController::GetInstance()
#define g_CursedTemple SEASON3A::CursedTemple::GetInstance()

#endif // !defined(AFX_URSEDTEMPLE_H__82D1D8BF_A93D_49BE_B384_159381C2495D__INCLUDED_)
