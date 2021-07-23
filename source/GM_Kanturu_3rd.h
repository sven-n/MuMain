#ifndef _GM_Kanturu_3rd_H_
#define _GM_Kanturu_3rd_H_

namespace M39Kanturu3rd {

	bool IsInKanturu3rd();

	//  오브젝트
	bool CreateKanturu3rdObject(OBJECT* o);
	bool MoveKanturu3rdObject(OBJECT* o);
	bool RenderKanturu3rdObjectVisual(OBJECT* o, BMD* b);
	bool RenderKanturu3rdObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	void RenderKanturu3rdAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);	
	
	//  몬스터
	CHARACTER* CreateKanturu3rdMonster(int iType, int PosX, int PosY, int Key);

	bool MoveKanturu3rdMonsterVisual(OBJECT* o, BMD* b);
	void MoveKanturu3rdBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);

	bool RenderKanturu3rdMonsterObjectMesh(OBJECT* o, BMD* b,bool ExtraMon);
	bool RenderKanturu3rdMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
	
	bool AttackEffectKanturu3rdMonster(CHARACTER* c, OBJECT* o, BMD* b);
	bool SetCurrentActionKanturu3rdMonster(CHARACTER* c, OBJECT* o);
	
	//  칸투르 상태 관련
	void Kanturu3rdInit();
	bool IsSuccessBattle();
	void CheckSuccessBattle(BYTE State, BYTE DetailState);
	void Kanturu3rdState(BYTE State, BYTE DetailState);
	void MayaSceneMayaAction(BYTE Skill);
	void MayaAction(OBJECT* o, BMD* b);

	void Kanturu3rdUserandMonsterCount(int Count1, int Count2);

	//  칸투르 인터페이스 관련
	void RenderKanturu3rdinterface();
	void RenderKanturu3rdResultInterface();
	void Kanturu3rdResult(BYTE Result);
	void Kanturu3rdSuccess();
	void Kanturu3rdFailed();
	
	//  칸투르 음악 관련
	void ChangeBackGroundMusic(int World);
}

extern int UserCount;
extern int MonsterCount;

#endif //_GM_Kanturu_3rd_H_
