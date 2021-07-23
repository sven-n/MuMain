// GM3rdChangeUp.h: interface for the CGM3rdChangeUp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_)
#define AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class BMD;

namespace SEASON3A
{
	class CGM3rdChangeUp  
	{
	protected:
		bool m_nDarkElfAppearance;	// 다크엘프 등장.

	protected:
		CGM3rdChangeUp();

	public:
		virtual ~CGM3rdChangeUp();

		static CGM3rdChangeUp& Instance();

		bool IsBalgasBarrackMap();
		bool IsBalgasRefugeMap();

		// 오브젝트 관련
		bool CreateBalgasBarrackObject(OBJECT* pObject);
		bool CreateBalgasRefugeObject(OBJECT* pObject);
		bool MoveObject(OBJECT* pObject);
		bool RenderObjectVisual(OBJECT* pObject, BMD* pModel);
		bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);	// 오브젝트 렌더(몬스터 포함)
		void RenderAfterObjectMesh(OBJECT* o, BMD* b);				// 반투명 오브젝트 나중 랜더.

		// 맵이펙트 관련
		bool CreateFireSnuff( PARTICLE* o );
		void PlayEffectSound(OBJECT* o);
		void PlayBGM();

		// 몬스터 관련
		CHARACTER* CreateBalgasBarrackMonster(int iType, int PosX, int PosY, int Key);	// 몬스터 생성
		bool SetCurrentActionBalgasBarrackMonster(CHARACTER* c, OBJECT* o);		// 몬스터 현재 액션 세팅
		bool AttackEffectBalgasBarrackMonster(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 공격 이펙트
		bool MoveBalgasBarrackMonsterVisual(CHARACTER* c,OBJECT* o, BMD* b);	// 몬스터 효과 업데이트
		void MoveBalgasBarrackBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);		// 몬스터 무기의 잔상 처리
		bool RenderMonsterObjectMesh(OBJECT* o, BMD* b,int ExtraMon);	// 몬스터 오브젝트 렌더링
		bool RenderBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);	// 몬스터 효과 렌더링
	};
}

#endif // !defined(AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_)
