#ifndef __ZZCHARACTER_H__
#define __ZZCHARACTER_H__

extern Script_Skill MonsterSkill[];
extern CHARACTER *CharactersClient;
extern CHARACTER CharacterView;
extern CHARACTER *Hero;

// 길드관계에 따른 텍스트색 얻기
DWORD GetGuildRelationShipTextColor( BYTE GuildRelationShip );
// 길드관계에 따른 텍스트배경색 얻기
DWORD GetGuildRelationShipBGColor( BYTE GuildRelationShip );

// 캐릭터명으로 포인터 얻기
CHARACTER* FindCharacterByID( char* szName );

void CreateMonsterServer(int Type,vec3_t Position,vec3_t Angle,int Level=0);

void CreateWeapon(int Type,int Level,CHARACTER *c,int Hand);

#ifdef CSK_REF_BACK_RENDERITEM
void RenderLinkObject(float x,float y,float z,CHARACTER *c,PART_t *f,int Type,int Level,int Option1,bool Link,bool Translate,int RenderType=0, bool bRightHandItem = true);
#else // CSK_REF_BACK_RENDERITEM
void RenderLinkObject(float x,float y,float z,CHARACTER *c,PART_t *f,int Type,int Level,int Option1,bool Link,bool Translate,int RenderType=0);
#endif // CSK_REF_BACK_RENDERITEM


void RenderCharacter(CHARACTER *c,OBJECT *o,int Select=0);
void RenderCharactersClient();
void RenderCharactersServer();
void RenderCharactersShadow();
void MoveCharacterClient(CHARACTER *cc);
void MoveCharactersClient();
void MoveCharactersServer();

void MoveEye(OBJECT *o, BMD *b, int Right, int Left, int Right2 = -1, int Left2 = -1, int Right3 = -1, int Left3 = -1);
void DeleteCloth( CHARACTER *c, OBJECT *o = NULL, PART_t *p2 = NULL);

bool CheckFullSet(CHARACTER *c);

void AttackRange(vec3_t Position,float Range,int Damage);

void MoveCharacterPosition(CHARACTER *c);
void ClientSendMonsterHit(CHARACTER *cc,int AttackPoint);
void ClientSendPositionPacket(CHARACTER *c);
void ClientSendPathPacket(unsigned short ID,unsigned char *PathX,unsigned char *PathY,unsigned char PathNum);
void ChangeCharacterExt(int Key,BYTE *Equipment, CHARACTER * pCharacter = NULL, OBJECT * pHelper = NULL);
void ClearCharacters(int Key=-1);
void DeleteCharacter(int Key);
void DeleteCharacter ( CHARACTER* c, OBJECT* o );
int FindCharacterIndex(int Key);
int FindCharacterIndexByMonsterIndex ( int Type );

void DeadCharacterBuff ( OBJECT* o );

//  블러드캐슬.
int  HangerBloodCastleQuestItem (int Key);
void SetAllAction ( int Action );

void ReleaseCharacters(void);
void CreateCharacterPointer(CHARACTER *c,int Type,unsigned char PositionX,unsigned char PositionY,float Rotation=0.f);
CHARACTER *CreateCharacter(int Key,int Type,unsigned char PositionX,unsigned char PositionY,float Rotation=0.f);
CHARACTER *CreateHero(int Key,int Class=0,int Skin=0,float x=0.f,float y=0.f,float Ratate=0.f);
CHARACTER *CreateMonster(int Type,int PositionX,int PositionY,int Key=0);
CHARACTER*  CreateHellGate ( BYTE* ID, int Key, int Index, int x, int y, int CreateFlag );
void OpenMonsters(char *FileName);
void SaveMonsters(char *FileName);

void SetAttackSpeed();
void SetPlayerShock(CHARACTER *c,int Hit);
void SetPlayerStop(CHARACTER *c);
void SetPlayerWalk(CHARACTER *c);

void SetPlayerAttack(CHARACTER *c);
void SetPlayerDie(CHARACTER *c);
void SetPlayerMagic(CHARACTER *c);
void SetPlayerTeleport(CHARACTER *c);
void SetPlayerHighBowAttack ( CHARACTER* c );
void SetCharacterClass(CHARACTER *c);
void SetCharacterScale(CHARACTER *c);
void SetChangeClass(CHARACTER *c);
int LevelConvert(BYTE Level);
float CharacterMoveSpeed(CHARACTER *c);
bool CheckMonsterSkill(CHARACTER* c, OBJECT* o);
bool CheckMonsterInRange( CHARACTER* c, float Range );
bool CharacterAnimation(CHARACTER* c, OBJECT* o);
bool AttackStage(CHARACTER* c, OBJECT* o);

#ifdef PBG_ADD_NEWCHAR_MONK
void RenderGuild(OBJECT *o,int Type=-1, vec3_t vPos = NULL);
#else //PBG_ADD_NEWCHAR_MONK
void RenderGuild(OBJECT *o,int Type=-1);
#endif //PBG_ADD_NEWCHAR_MONK

void RenderLight(OBJECT *o,int Texture,float Scale,int Bone,float x,float y,float z);
void RenderProtectGuildMark(CHARACTER * c);

void MakeElfHelper(CHARACTER * c);

// 펜릴 타입 얻는 함수
int GetFenrirType(CHARACTER* c);

///////////////////////////////////////////////////////////////////////////////
// snake
///////////////////////////////////////////////////////////////////////////////

extern JOINT     Joints[];
extern int       EquipmentLevelSet;
extern bool      g_bAddDefense;

void CreateJoint(int Type,vec3_t Position,vec3_t TargetPosition,vec3_t Angle,int SubType=0,OBJECT *Target=NULL,float Scale=10.f,short PK=-1,
				 WORD SkillIndex=0,WORD SkillSerialNum=0, int iChaIndex=-1, const float* vColor = NULL, short int sTargetIndex = -1);


// 서버 클래스 
BYTE ChangeServerClassTypeToClientClassType(const BYTE byServerClassType);
// 모든 클래스 타입을 얻는다.
BYTE GetCharacterClass(const BYTE byClass);
// 2차 전직했는가?
bool IsSecondClass(const BYTE byClass);
// 3차 전직했는가?
bool IsThirdClass(const BYTE byClass);
// 몇차 전직인가?
BYTE GetStepClass(const BYTE byClass);
// 모든 클래스 타입의 이름을 얻는다.
const unicode::t_char* GetCharacterClassText(const BYTE byClass);
// 스킨 모델 인덱스를 얻는다.
BYTE GetSkinModelIndex(const BYTE byClass);
// 기본클래스 타입을 얻는다.
inline int GetBaseClass( int iClass)	{ return ( 0x7 & iClass); }
// 여성캐릭인가?
inline int IsFemale(int iClass) { return (GetBaseClass(iClass) == CLASS_ELF || GetBaseClass(iClass) == CLASS_SUMMONER); }

#ifdef ADD_SOCKET_ITEM
// 활과 석궁을 구별해주는 함수
//** 캐릭터에 장착된 무기를 검색할 때에만 호출하시오!!!
// -- return값 
// -- BOWTYPE_BOW : 활, BOWTYPE_CROSSBOW : 석궁, BOWTYPE_NONE : 활/석궁이 아님
//** *ITEM으로 인자로 받는 함수는 활인지 석궁인지 하나씩 구분을 해야하기 때문에,
//** 연속적인 인덱스가 아닌이상 이 함수는 위험하다.
//** 기본적으로 ITEM인자가 캐릭터에 장착되어있는 무기를 검색할때에만 호출하기 바란다.
//** 되도록이면 인자없는 오버로드된 함수를 사용하라.!!!
int GetEquipedBowType( CHARACTER *pChar );
int GetEquipedBowType( ITEM* pItem );
int GetEquipedBowType( );
#endif // ADD_SOCKET_ITEM

#ifdef KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
bool IsEquipedWing();							// 날개를 착용 유무 검사.
#endif // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
int GetEquipedBowType_Skill( );
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT

bool IsMasterLevel(const BYTE byClass);

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
BOOL FindHeroSkill(enum ActionSkillType eSkillType); 
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
void SetActionBloodAttack(CHARACTER *c, OBJECT* o);
#endif // YDG_ADD_SKILL_RIDING_ANIMATIONS

#ifdef CSK_REF_BACK_RENDERITEM
bool RenderCharacterBackItem(CHARACTER *c, OBJECT* o, bool bTranslate);
bool IsBackItem(CHARACTER *c, int iType);
#endif // CSK_REF_BACK_RENDERITEM

#endif