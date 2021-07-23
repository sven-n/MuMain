//////////////////////////////////////////////////////////////////////////
//  
//  GMBattleCastle.h
//  
//  내  용 : 공성전 맵
//  
//  날  짜 : 2004년 10월 11일
//  
//  작성자 : 조 규 하
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __GMBATTLECASTLE_H__
#define __GMBATTLECASTLE_H__

//////////////////////////////////////////////////////////////////////////
//  Function.
//////////////////////////////////////////////////////////////////////////
namespace   battleCastle
{
    inline  bool    InBattleCastle ( int iMap=World ) { return ( iMap==WD_30BATTLECASTLE ); }
    //  (공성 2층) 특정 지역의 배경처리. ( 162, 190 ) ~ ( 189, 215 )
    inline  bool    InBattleCastle2 ( vec3_t Position ) { return ( Position[0]>=16100.f && Position[0]<=19000.f && Position[1]>=18900.f && Position[1]<=21700.f ); }
    //  (공성 불씨) 특정 지역의 배경처리. ( 47, 53 ) ~ ( 148, 239 )
    inline  bool    InBattleCastle3 ( vec3_t Position ) { return ( Position[0]>=4700.f && Position[0]<=14800.f && Position[1]>=5300.f && Position[1]<=23900.f ); }
    bool    IsBattleCastleStart ( void );
    void    SetBattleCastleStart ( bool bResult );

    void    Init ( void );
	bool    SettingBattleFormation ( CHARACTER* c, eBuffState state );
    bool    GetGuildMaster ( CHARACTER* c );
    void    SettingBattleKing ( CHARACTER* c );
	void    DeleteBattleFormation ( CHARACTER* c, eBuffState state );
    void    ChangeBattleFormation ( char* GuildName, bool bEffect=false );
    void    CreateBattleCastleCharacter_Visual ( CHARACTER* c, OBJECT* o );
    void    DeleteTmpCharacter ( void );
    void    CreateGuardStoneHealingVisual ( CHARACTER* c, float Range );
    void    CollisionHeroCharacter ( vec3_t Position, float Range, int AniType );
    void    CollisionTempCharacter ( vec3_t Position, float Range, int AniType );
    bool    CollisionEffectToObject ( OBJECT* eff, float Range, float RangeZ, bool bCollisionGround, bool bRealCollision=false );
    void    SetCastleGate_Attribute ( int x, int y, BYTE Operator, bool bAllClear=false );
    void    RenderAurora ( int Type, int RenderType, float x, float y, float sx, float sy, vec3_t Light );
    void    SetBuildTimeLocation ( OBJECT* o );
    void    RenderBuildTimes ( void );
	void    RenderBuffMagicIcon ( CHARACTER* c );
    void    InitEtcSetting ( void );
    void    StartFog ( vec3_t Color );
    void    EndFog ( void );
    void    RenderBaseSmoke ( void );
    bool    CreateFireSnuff ( PARTICLE* o );
    bool    MoveBattleCastleObjectSetting ( int& objCount, int object );
    bool    MoveBattleCastleObject ( OBJECT* o, int& object, int& visibleObject );
    bool    CreateBattleCastleObject ( OBJECT* o );
    bool    MoveBattleCastleVisual ( OBJECT* o );
    bool    RenderBattleCastleVisual ( OBJECT* o, BMD* b );
    bool    RenderBattleCastleObjectMesh ( OBJECT* o, BMD* b );
    void    MoveFlyBigStone ( OBJECT* o );
    CHARACTER* CreateBattleCastleMonster ( int Type, int PositionX, int PositionY, int Key );
    bool    SettingBattleCastleMonsterLinkBone ( CHARACTER* c, int Type );
    bool    StopBattleCastleMonster ( CHARACTER* c, OBJECT* o );
    bool    MoveBattleCastleMonster ( CHARACTER* c, OBJECT* o );
    bool    SetCurrentAction_BattleCastleMonster ( CHARACTER* c, OBJECT* o );
    bool    AttackEffect_BattleCastleMonster ( CHARACTER* c, OBJECT* o, BMD* b );
    bool    MoveBattleCastleMonsterVisual ( OBJECT* o, BMD* b );
    bool    RenderBattleCastleMonsterVisual ( CHARACTER* c, OBJECT* o, BMD* b );
    bool    RenderBattleCastleMonsterCloth ( CHARACTER* c, OBJECT* o, bool Translate, int Select );
    bool    RenderBattleCastleMonsterObjectMesh ( OBJECT* o, BMD* b );
    void    RenderMonsterHitEffect ( OBJECT* o );
}

using namespace battleCastle;

#endif// __GMBATTLECASTLE_H__