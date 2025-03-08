#ifndef _GMCRYWOLF1ST_H_
#define _GMCRYWOLF1ST_H_

namespace M34CryWolf1st {		//. 크라이울프 점령지
    bool IsCyrWolf1st();

    //. 오브젝트
    bool CreateCryWolf1stObject(OBJECT* o);
    bool MoveCryWolf1stObject(OBJECT* o);
    bool RenderCryWolf1stObjectVisual(OBJECT* o, BMD* b);
    bool RenderCryWolf1stObjectMesh(OBJECT* o, BMD* b, int ExtraMon);

    //. 몬스터
    CHARACTER* CreateCryWolf1stMonster(int iType, int PosX, int PosY, int Key);

    bool MoveCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    void MoveCryWolf1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    bool RenderCryWolf1stMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);
    bool RenderCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    bool AttackEffectCryWolf1stMonster(CHARACTER* c, OBJECT* o, BMD* b);
    bool SetCurrentActionCrywolfMonster(CHARACTER* c, OBJECT* o);
    bool CreateMist(PARTICLE* pParticleObj);
    void RenderBaseSmoke(void);

    //. 크라이울프 레이드 관련 MVP
    void ChangeBackGroundMusic(int World);
    void RenderNoticesCryWolf();
    void CryWolfMVPInit();
    int IsCryWolf1stMVPStart();
    bool IsCryWolf1stMVPStatePeace();
    void CheckCryWolf1stMVPAltarfInfo(int StatueHP, BYTE AltarState1, BYTE AltarState2, BYTE AltarState3, BYTE AltarState4, BYTE AltarState5);
    void CheckCryWolf1stMVP(BYTE btOccupationState, BYTE btCrywolfState);
    bool Render_Mvp_Interface();
    void DisableCryWolfNPC();
    void EnableCryWolfNPC();
    void DoTankerFireFixStartPosition(int SourceX, int SourceY, int PositionX, int PositionY);

    void Check_AltarState(int Num, int State);
    void Set_Message_Box(int Str, int Num, int Key, int ObjNum = -1);
    void Sub_Interface();
    void MoveMvp_Interface();
    void Set_Hp(int State);
    void Set_Val_Hp(int State);
    void Set_BossMonster(int Val_Hp, int Dl_Num);
    void SetTime(BYTE byHour, BYTE byMinute);
    bool Get_State();
    bool Get_State_Only_Elf();
    bool Get_AltarState_State(int Num);

    void Set_MyRank(BYTE MyRank, int GettingExp);
    void Set_WorldRank(BYTE Rank, CLASS_TYPE Class, int Score, wchar_t* szHeroName);
}

using namespace M34CryWolf1st;

extern BYTE m_AltarState[];
extern bool	View_Bal;
extern char Suc_Or_Fail, Message_Box;
extern int m_StatueHP, Val_Hp, Dark_elf_Num;
extern bool View_End_Result;

#endif // _GMCRYWOLF1ST_H_