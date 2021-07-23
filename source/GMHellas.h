//////////////////////////////////////////////////////////////////////////
//  
//  GMHellas.h
//  
//  내  용 : 24번째 맵 ( 마도시 헬라스--가칭 )
//
//  날  짜 : 2004/05/20
//
//  작성자 : 조 규 하
//
//////////////////////////////////////////////////////////////////////////
#ifndef __GMHELLAS_H__
#define __GMHELLAS_H__

//////////////////////////////////////////////////////////////////////////
//  Function.
//////////////////////////////////////////////////////////////////////////

// 칼리마 맵인가?
inline  bool    InHellas ( int iMap=World ) { return ((iMap>=WD_24HELLAS && iMap<=WD_24HELLAS_END) || (iMap==WD_24HELLAS_7)); }
inline	bool	InHiddenHellas(int iMap=World ) { return (iMap==WD_24HELLAS_7) ? true : false; }


//////////////////////////////////////////////////////////////////////////
//  물 지형 관련.
//////////////////////////////////////////////////////////////////////////
bool    CreateWaterTerrain ( int mapIndex );
bool    IsWaterTerrain ( void );
void    AddWaterWave ( int x, int y, int range, int height );
float   GetWaterTerrain ( float x, float y );
void    MoveWaterTerrain ( void );
bool    RenderWaterTerrain ( void );
void    DeleteWaterTerrain ( void );
void    RenderWaterTerrain ( int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation=0.f, float Alpha=1.f, float Height=0.f );


//////////////////////////////////////////////////////////////////////////
//  맵 관련.
//////////////////////////////////////////////////////////////////////////
void    SettingHellasColor ( void );


//////////////////////////////////////////////////////////////////////////
//  아이템 관련.
//////////////////////////////////////////////////////////////////////////
BYTE    GetHellasLevel ( int Class, int Level );
bool    GetUseLostMap ( bool bDrawAlert=false );
int     RenderHellasItemInfo ( ITEM* ip, int textNum );
bool	EnableKalima ( int Class, int Level, int ItemLevel);

//////////////////////////////////////////////////////////////////////////
//  오브젝트 관련.
//////////////////////////////////////////////////////////////////////////
void    AddObjectDescription ( char* Text, vec3_t position );
void    RenderObjectDescription ( void );

bool    CreateHellasObject ( OBJECT* o );                                       //  오브젝트 생성시 설정값 세팅.
bool    MoveHellasObjectSetting ( int& objCount, int object );                  //  맵의 오브젝트를 향한 효과를 위한 오브젝트 설정.
bool    MoveHellasObject ( OBJECT* o, int& object, int& visibleObject );        //  선택된 오브젝트에 향해 효과를 생성한다.
bool    MoveHellasAllObject ( OBJECT* o );                                      //  기타 오브젝트들의 연출 처리.
bool    MoveHellasVisual ( OBJECT* o );                                         //  각각 오브젝트들의 효과 처리.
bool    RenderHellasVisual ( OBJECT* o, BMD* b );                               //  각각 오브젝트들의 효과를 나타낸다.
bool    RenderHellasObjectMesh  ( OBJECT* o, BMD* b );                          //  각각 오브젝트들의 ( 메쉬 ) 효과를 나타낸다.
                                                                                //  해당 맵에 속하는 몬스터들의 (메쉬) 효과도 처리.

//////////////////////////////////////////////////////////////////////////
//  활동하는 맵 오브젝트.
//////////////////////////////////////////////////////////////////////////
int     CreateBigMon ( OBJECT* o );
void    MoveBigMon ( OBJECT* o );


//////////////////////////////////////////////////////////////////////////
//  몬스터. ( 맵에 속하는 ).
//////////////////////////////////////////////////////////////////////////
void    CreateMonsterSkill_ReduceDef ( OBJECT* o, int AttackTime, BYTE time, float Height );
void    CreateMonsterSkill_Poison ( OBJECT* o, int AttackTime, BYTE time );
void    CreateMonsterSkill_Summon ( OBJECT* o, int AttackTime, BYTE time );

void    SetActionDestroy_Def ( OBJECT* o );
void    RenderDestroy_Def ( OBJECT* o, BMD* b );
CHARACTER* CreateHellasMonster ( int Type, int PositionX, int PositionY, int Key );

bool    SettingHellasMonsterLinkBone ( CHARACTER* c, int Type );                //  헬라스 몬스터의 링크본을 설정한다.

bool    SetCurrentAction_HellasMonster ( CHARACTER* c, OBJECT* o );                           //  헬라스 몬스터의 행동을 설정한다.
bool    AttackEffect_HellasMonster ( CHARACTER* c, CHARACTER* tc, OBJECT* o, OBJECT* to, BMD* b ); //  헬라스 몬스터의 공격 효과.
void    MonsterMoveWaterSmoke ( OBJECT* o );
void    MonsterDieWaterSmoke ( OBJECT* o );
bool    MoveHellasMonsterVisual ( OBJECT* o, BMD* b );                          //  헬라스 몬스터 비주얼 처리.
bool    RenderHellasMonsterCloth ( CHARACTER* c, OBJECT* o, bool Translate=true, int Select=0 );
bool    RenderHellasMonsterObjectMesh ( OBJECT* o, BMD* b );                    //  해당 맵에 속하는 몬스터들의 (메쉬) 효과 처리.
bool    RenderHellasMonsterVisual ( CHARACTER* c, OBJECT* o, BMD* b );          //  해당 맵에 속하는 몬스터들의 효과 처리.

#endif// __GMHELLAS_H__