#ifndef ZZZ_OBJECT_H
#define ZZZ_OBJECT_H

extern OBJECT_BLOCK ObjectBlock[256];

#ifdef MR0
extern AUTOOBJ       Butterfles[];
extern AUTOOBJ       Boids[];
extern AUTOOBJ       Fishs[];
#else
extern OBJECT       Butterfles[];
extern OBJECT       Boids[];
extern OBJECT       Fishs[];
#endif //MR0

extern int World;
extern float EarthQuake;
extern bool EnableShadow;
extern float AmbientShadowAngle;
extern int RainTarget;
extern int RainCurrent;

void CopyShadowAngle(OBJECT *o,BMD *b);
void CreateShadowAngle();

OBJECT *CollisionDetectObjects(OBJECT *PickObject);
void ClearWorld();

void RenderObject(OBJECT *o,bool Translate=false,int Select=0, int ExtraMon=0);
void RenderObjects();

#ifdef LDK_ADD_14_15_GRADE_ITEM_MODEL
// 15 레벨 효과 랜더링 하는 함수
void NextGradeObjectRender(CHARACTER *c);
#endif //LDK_ADD_14_15_GRADE_ITEM_MODEL

// 잔상 효과 내기 위해서 만든 함수
void RenderObject_AfterImage(OBJECT* o, bool Translate=false, int Select=0, int ExtraMon=0);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
void RenderCharacter_AfterImage(CHARACTER* pCha, PART_t *pPart, bool Translate=false, int Select=0, float AniInterval1 = 1.4f, float AniInterval2 = 0.7f);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
// 캐릭터 다음에 알파오브젝트들 렌더링 하기 위한 함수
void RenderObject_AfterCharacter(OBJECT *o,bool Translate=false,int Select=0, int ExtraMon=0);
void Draw_RenderObject_AfterCharacter(OBJECT *o,bool Translate=false,int Select=0, int ExtraMon=0);
void RenderObjects_AfterCharacter();

void MoveObjects();
void DeleteObjectTile(int x,int y);
void DeleteObject(OBJECT *o,OBJECT_BLOCK *ob);
OBJECT *CreateObject(int Type,vec3_t Position,vec3_t Angle,float Scale=1.f);
bool SaveObjects(char *FileName, int iMapNumber);
int OpenObjects(char *FileName);
int OpenObjectsEnc(char *FileName);
void DeleteObjects();
void SaveTrapObjects(char *FileName);


///////////////////////////////////////////////////////////////////////////////
// npc
///////////////////////////////////////////////////////////////////////////////

extern OPERATE Operates[MAX_OPERATES];

void CreateOperate(OBJECT *Owner);

///////////////////////////////////////////////////////////////////////////////
// world item
///////////////////////////////////////////////////////////////////////////////

extern ITEM_t Items[MAX_ITEMS];
extern ITEM   PickItem;
extern ITEM   TargetItem;

void ItemObjectAttribute(OBJECT *o);
void CreateItem(ITEM_t *n,BYTE *Item,vec3_t Position,int CreateFlag);
void DeleteItem(int Key);
void PartObjectColor(int Type,float Alpha,float Bright,vec3_t Light,bool ExtraMon=false);
void PartObjectColor2(int Type,float Alpha,float Bright,vec3_t Light,bool ExtraMon=false);

#ifdef CSK_ADD_GOLDCORPS_EVENT
void RenderPartObjectBodyColor(BMD *b,OBJECT *o,int Type,float Alpha,int RenderType,float Bright,int Texture=-1, int iMonsterIndex=-1);
#else // CSK_ADD_GOLDCORPS_EVENT
void RenderPartObjectBodyColor(BMD *b,OBJECT *o,int Type,float Alpha,int RenderType,float Bright,int Texture=-1);
#endif // CSK_ADD_GOLDCORPS_EVENT
void RenderPartObjectBodyColor2(BMD *b,OBJECT *o,int Type,float Alpha,int RenderType,float Bright,int Texture=-1);
void RenderPartObjectBody(BMD *b,OBJECT *o,int Type,float Alpha,int RenderType);

void RenderPartObjectEffect(OBJECT *o,int Type,vec3_t Light,float Alpha=0.f,int Level=0,int Option1=0,int ExtOption=0,int Select=0,int RenderType=RENDER_TEXTURE);
void RenderPartObject(OBJECT *o,int Type,void *p,vec3_t Light,float Alpha=0.f,int Level=0,int Option1=0,int ExtOption=0,bool GlobalTransform=false,bool HideSkin=false,bool Translate=false,int Select=0,int RenderType=RENDER_TEXTURE);

void RenderPartObjectEdge(BMD *b,OBJECT *o,int Flag,bool Translate,float Scale);
void RenderPartObjectEdge2(BMD *b, OBJECT *o, int Flag,bool Translate,float Scale, OBB_t* OBB );

void RenderPartObjectEdgeLight(BMD* b, OBJECT* o, int Flag, bool Translate, float Scale );

void RenderItems();
void MoveItems();
int SelectItem();
int GetScreenWidth();
void ClearItems();

void RenderCloudLowLevel ( int index, int Type );
void SortInBlockByType();

void ClearActionObject ();
void SetActionObject ( int iWorld, int iType, int iLifeTime, int iVel=-1 );
void ActionObject ( OBJECT* o );


void BodyLight(OBJECT *o,BMD *b);

bool Calc_RenderObject(OBJECT *o,bool Translate,int Select, int ExtraMon);
bool Calc_ObjectAnimation ( OBJECT* o, bool Translate, int Select );
void Draw_RenderObject(OBJECT *o,bool Translate,int Select, int ExtraMon);

bool IsPartyMemberBuff( int partyindex, eBuffState buffstate );
bool isPartyMemberBuff( int partyindex );

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
void RenderBoundingBox(OBJECT* pObj);
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX

#endif //ZZZ_OBJECT_H
