///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"


///////////////////////////////////////////////////////////////////////////////
// 잔상 효과(게임상의 칼 휘두를때 나는 잔상)
///////////////////////////////////////////////////////////////////////////////

// CreateBlur
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
#define MAX_BLURS      100		// 
#define MAX_BLUR_TAILS 30
#define MAX_BLUR_LIFETIME 30

#define MAX_OBJECTBLURS		1000
#define MAX_OBJECTBLUR_TAILS 600
#define MAX_OBJECTBLUR_LIFETIME 30
//#define MAX_OBJECTBLUR_LIFETIME 11		// FIX
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
#define MAX_BLURS      100
#define MAX_BLUR_TAILS 30
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD

typedef struct
{
	bool      Live;
	int       Type;
	int       LifeTime;
	CHARACTER *Owner;
	int       Number;
	vec3_t    Light;
	vec3_t    p1[MAX_BLUR_TAILS];
	vec3_t    p2[MAX_BLUR_TAILS];
	int       SubType;
} BLUR;

BLUR Blur[MAX_BLURS];

void AddBlur(BLUR *b,vec3_t p1,vec3_t p2,vec3_t Light,int Type)
{
	b->Type = Type;
	VectorCopy(Light,b->Light);
	for(int i=b->Number-1;i>=0;i--)
	{
		VectorCopy(b->p1[i],b->p1[i+1]);
		VectorCopy(b->p2[i],b->p2[i+1]);
	}
	VectorCopy(p1,b->p1[0]);
	VectorCopy(p2,b->p2[0]);
	b->Number++;
	if(b->Number >= MAX_BLUR_TAILS-1)
    {
		b->Number = MAX_BLUR_TAILS-1;
    }
}


// SubType : 한캐릭터 안에서의 식별자로 사용
void CreateBlur(CHARACTER *Owner,vec3_t p1,vec3_t p2,vec3_t Light,int Type,bool Short,int SubType)
{
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live && b->Owner==Owner)
		{
			if(SubType > 0 && b->SubType != SubType)
				continue;
            //  보간을 한다.
			AddBlur(b,p1,p2,Light,Type);
            return;
		}
	}
#ifdef _VS2008PORTING
	for(int i=0;i<MAX_BLURS;i++)
#else // _VS2008PORTING
	for(i=0;i<MAX_BLURS;i++)
#endif // _VS2008PORTING
	{
		BLUR *b = &Blur[i];
		if(!b->Live)
		{
			b->Live = true;
			b->Owner = Owner;
            b->Number= 0;
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			b->LifeTime = Short? 15 : MAX_BLUR_LIFETIME;
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
            b->LifeTime = Short? 15 : MAX_BLUR_TAILS;
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			b->SubType = SubType;
			AddBlur(b,p1,p2,Light,Type);
            return;
		}
	}
}

void MoveBlurs()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_BLURS, PROFILING_MOVE_BLURS );
#endif // DO_PROFILING
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live)
		{
			b->LifeTime--;
			b->Number--;

	        for(int i=b->Number-1;i>=0;i--)
	        {
		        VectorCopy(b->p1[i],b->p1[i+1]);
		        VectorCopy(b->p2[i],b->p2[i+1]);
	        }
			if(b->LifeTime <= 0)
			{
				b->Number--;
				if(b->Number <= 0)
				{
        			b->Live = false;
				}
			}
		}
	}
	MoveObjectBlurs();
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_BLURS );
#endif // DO_PROFILING
}

void RenderBlurs()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_BLURS, PROFILING_RENDER_BLURS );
#endif // DO_PROFILING
    int Type;
    //DisableCullFace();
	for(int i=0;i<MAX_BLURS;i++)
	{
		BLUR *b = &Blur[i];
		if(b->Live)
		{
            Type = b->Type;

			int nTexture = BITMAP_BLUR+Type;
			if(Type == 3)
			{
				nTexture = BITMAP_BLUR2;
			}
			else if(Type == 4)
			{
				nTexture = BITMAP_BLUR;
			}
#ifdef PBG_ADD_PKFIELD
			else if(Type == 5)
			{
				nTexture = BITMAP_BLUR+3;
			}
#endif //PBG_ADD_PKFIELD
#if defined PBG_ADD_PKFIELD || defined LDS_ADD_EMPIRE_GUARDIAN
			if(b->Owner->Level == 0 
				&& (Type<=3 || ( Type >= 5 && Type <= 10) ))
#else //PBG_ADD_PKFIELD || LDS_ADD_EMPIRE_GUARDIAN
			if(b->Owner->Level == 0 && Type<=3 )
#endif //PBG_ADD_PKFIELD
			{
              	EnableAlphaBlend();
			}
			else
			{
              	EnableAlphaBlendMinus();
			}

            if( Type>2 ) Type = Type - 3;
			if(b->Number >= 2)
			{
				BindTexture(nTexture);
				for(int j=0;j<b->Number-1;j++)
				{
      				glBegin(GL_TRIANGLE_FAN);
					float Light;
					float TexU;
					if(b->Owner->Level == 0)
						Light = (b->Number-j)/(float)b->Number;
					else
						Light = 1.f;
					glColor3f(b->Light[0]*Light,b->Light[1]*Light,b->Light[2]*Light);
					TexU = (j)/(float)b->Number;
					glTexCoord2f(TexU,1.f);glVertex3fv(b->p1[j]);
					glTexCoord2f(TexU,0.f);glVertex3fv(b->p2[j]);
					if(b->Owner->Level == 0)
      					Light = (b->Number-(j+1))/(float)b->Number;
					else
						Light = 1.f;
					glColor3f(b->Light[0]*Light,b->Light[1]*Light,b->Light[2]*Light);
					TexU = (j+1)/(float)b->Number;
					glTexCoord2f(TexU,0.f);glVertex3fv(b->p2[j+1]);
					glTexCoord2f(TexU,1.f);glVertex3fv(b->p1[j+1]);
    				glEnd();
				}
			}
		}
	}
	RenderObjectBlurs();

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_BLURS );
#endif // DO_PROFILING
}

typedef struct
{
	bool      Live;
	int       Type;
	int       LifeTime;
	OBJECT *  Owner;
	int       Number;
	vec3_t    Light;
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	int		  _iLimitLifeTime;
	vec3_t    p1[MAX_OBJECTBLUR_TAILS];
	vec3_t    p2[MAX_OBJECTBLUR_TAILS];
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	vec3_t    p1[MAX_BLUR_TAILS];
	vec3_t    p2[MAX_BLUR_TAILS];
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	int       SubType;
} OBJECT_BLUR;

#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
OBJECT_BLUR ObjectBlur[MAX_OBJECTBLURS];
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
OBJECT_BLUR ObjectBlur[MAX_BLURS];
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD

void ClearAllObjectBlurs()
{
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i = 0; i < MAX_OBJECTBLURS; ++i)
	{
		ObjectBlur[i].Live = false;
	}
#else //LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i = 0; i < MAX_BLURS; ++i)
	{
		ObjectBlur[i].Live = false;
	}
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
}

void AddObjectBlur(OBJECT_BLUR *b,vec3_t p1,vec3_t p2,vec3_t Light,int Type)
{
	b->Type = Type;
	VectorCopy(Light,b->Light);

	// 이미 블루어데이터가 있으면, 그 뒤로 하나씩 밀어준다.
	for(int i=b->Number-1;i>=0;i--)
	{
		VectorCopy(b->p1[i],b->p1[i+1]);
		VectorCopy(b->p2[i],b->p2[i+1]);
	}
	VectorCopy(p1,b->p1[0]);
	VectorCopy(p2,b->p2[0]);
	b->Number++;
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	if(b->Number >= MAX_OBJECTBLUR_TAILS-1)
    {
		b->Number = MAX_OBJECTBLUR_TAILS-1;
    }
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	if(b->Number >= MAX_BLUR_TAILS-1)
    {
		b->Number = MAX_BLUR_TAILS-1;
    }
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
}


// CreateObjectBlur : 검기 효과 함수, 식별자로 기존 CreateBlur는 Character를 넘겼으나, Object를 넘김.
// - SubType : 한캐릭터 안에서의 식별자로 사용
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
void CreateObjectBlur(OBJECT *Owner,vec3_t p1,vec3_t p2,vec3_t Light,int Type,bool Short,int SubType, int iLimitLifeTime)
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
void CreateObjectBlur(OBJECT *Owner,vec3_t p1,vec3_t p2,vec3_t Light,int Type,bool Short,int SubType)
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
{
	// 1. 전체 오브젝트 블루어 접근으로 같은계열 블루어가 있으면 AddObjectBlur 해준다. 즉 꼬리 붙이고 나면 바로 리턴.
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_OBJECTBLURS;i++)
#else //LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_BLURS;i++)
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live && b->Owner==Owner)
		{
			if(SubType > 0 && b->SubType != SubType)
				continue;


            // 1-1. 블루어테이블에 리스트형식으로 Tail 하나 붙여준다.
			AddObjectBlur(b,p1,p2,Light,Type);
            return;
		}
	}

	// 2. 같은 계열이 없으면 신규 블루어 추가.
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
#ifdef _VS2008PORTING
	for(int i=0;i<MAX_OBJECTBLURS;i++)
#else // _VS2008PORTING
	for(i=0;i<MAX_OBJECTBLURS;i++)
#endif // _VS2008PORTING
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
#ifdef _VS2008PORTING
	for(int i=0;i<MAX_BLURS;i++)
#else // _VS2008PORTING
	for(i=0;i<MAX_BLURS;i++)
#endif // _VS2008PORTING
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	{
		// 2-1. 빈자리 하나 찾아서 신규 생성.
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(!b->Live)
		{
			b->Live = true;
			b->Owner = Owner;
            b->Number= 0;
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			if( iLimitLifeTime > -1 )
			{
				b->LifeTime = iLimitLifeTime;
				b->_iLimitLifeTime = iLimitLifeTime;
			}
			else 
			{
				b->_iLimitLifeTime = Short? 15 : MAX_OBJECTBLUR_LIFETIME ;
				b->LifeTime = b->_iLimitLifeTime;				
			}
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			b->LifeTime = Short? 15 : MAX_BLUR_TAILS;
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			b->SubType = SubType;

			AddObjectBlur(b,p1,p2,Light,Type);
            return;
		}
	}
}

void MoveObjectBlurs()
{
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_OBJECTBLURS;i++)
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_BLURS;i++)
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	{
		OBJECT_BLUR *b = &ObjectBlur[i];

		// 1. 각 블루어의 하나씩 감산
		if(b->Live)
		{
			b->LifeTime--;
			b->Number--;

#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			if(b->LifeTime <= 0)
			{
				b->Number = 0;
				b->Live = false;
				continue;
			}
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD

			// 1-1. 블루어 내 각 블루어 위치값을 한칸씩 뒤로 이동.
	        for(int i=b->Number-1;i>=0;i--)
	        {
		        VectorCopy(b->p1[i],b->p1[i+1]);
		        VectorCopy(b->p2[i],b->p2[i+1]);
	        }

#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
			if(b->LifeTime <= 0)
			{
				b->Number--;
				if(b->Number <= 0)
				{
        			b->Live = false;
				}
			}
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
		}
	}
}

void RenderObjectBlurs()
{
    int Type;
    //DisableCullFace();
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_OBJECTBLURS;i++)
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_BLURS;i++)
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live)
		{
            Type = b->Type;
			int nTexture = BITMAP_BLUR+Type;
			if(Type == 3)
			{
				nTexture = BITMAP_BLUR2;
			}
			else if(Type == 4)
			{
				nTexture = BITMAP_BLUR;
			}
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
			else if (Type == 5)
			{
				nTexture = BITMAP_LAVA;
			}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

            EnableAlphaBlend();

            if( Type>2 ) Type = Type - 3;
			if(b->Number >= 2)
			{
				BindTexture(nTexture);
				for(int j=0;j<b->Number-1;j++)
				{
					//pjh
					float Data = 300.f;
					if(b->SubType == 113 || b->SubType == 114)
					{
					if(abs(b->p1[j][0] - b->p1[j+1][0]) > Data||abs(b->p1[j][1] - b->p1[j+1][1]) > Data||abs(b->p1[j][2] - b->p1[j+1][2]) > Data||
						abs(b->p1[j][0] - b->p2[j+1][0]) > Data||abs(b->p1[j][1] - b->p2[j+1][1]) > Data||abs(b->p2[j][2] - b->p2[j+1][2]) > Data)
						continue;
					}

      				glBegin(GL_TRIANGLE_FAN);
					float Light;
					float TexU;
					Light = (b->Number-j)/(float)b->Number;
					glColor3f(b->Light[0]*Light,b->Light[1]*Light,b->Light[2]*Light);
					TexU = (j)/(float)b->Number;
					glTexCoord2f(TexU,1.f);glVertex3fv(b->p1[j]);
					glTexCoord2f(TexU,0.f);glVertex3fv(b->p2[j]);
      				Light = (b->Number-(j+1))/(float)b->Number;
					glColor3f(b->Light[0]*Light,b->Light[1]*Light,b->Light[2]*Light);
					TexU = (j+1)/(float)b->Number;
					glTexCoord2f(TexU,0.f);glVertex3fv(b->p2[j+1]);
					glTexCoord2f(TexU,1.f);glVertex3fv(b->p1[j+1]);
    				glEnd();
				}
			}
		}
	}
}

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
void RemoveObjectBlurs(OBJECT *Owner,int SubType)
{
#ifdef LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_OBJECTBLURS;i++)
#else // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	for(int i=0;i<MAX_BLURS;i++)
#endif // LDS_MOD_EFFECTBLURSPARK_FORCEOFSWORD
	{
		OBJECT_BLUR *b = &ObjectBlur[i];
		if(b->Live && b->Owner==Owner)
		{
			ObjectBlur[i].Live = false;
		}
	}
}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

///////////////////////////////////////////////////////////////////////////////
// 스파크 효과(기사 스킬 사용시)
///////////////////////////////////////////////////////////////////////////////

void CreateSpark(int Type,CHARACTER *tc,vec3_t Position,vec3_t Angle)
{
	OBJECT *to = &tc->Object;
	vec3_t Light;
	Vector(1.f,1.f,1.f,Light);
   	CreateParticle(BITMAP_SPARK+1,Position,to->Angle,Light);
	vec3_t p,p2;
	float Matrix[3][4];
	Vector(0.f,50.f,0.f,p);
	AngleMatrix(Angle,Matrix);
	VectorRotate(p,Matrix,p2);
	VectorAdd(p2,Position,p2);
	for(int i=0;i<20;i++)
	{
		vec3_t a;
		Vector((float)(rand()%360),0.f,(float)(rand()%360),a);
		VectorAdd(a,Angle,a);
   		//CreateJoint(BITMAP_JOINT_SPARK,p2,p2,a);
	    CreateParticle(BITMAP_SPARK,Position,to->Angle,Light);
	}
}

///////////////////////////////////////////////////////////////////////////////
// 피 효과(케릭터나 몬스터 죽을때 나는 효과)
///////////////////////////////////////////////////////////////////////////////

void CreateBlood(OBJECT *o)
{
	int BoneHead = Models[o->Type].BoneHead;
	if(BoneHead!=-1)
	{
		if(o->Type==MODEL_MONSTER01+15)
		{
			o->Live = false;
			for(int i=0;i<10;i++)
				CreateEffect(MODEL_ICE_SMALL,o->Position,o->Angle,o->Light);
		}
		else if(o->Type!=MODEL_MONSTER01+7 && o->Type!=MODEL_MONSTER01+14 && o->Type!=MODEL_MONSTER01+18)
		{
			vec3_t p,Position;
			for(int i=0;i<2;i++)
			{
				Vector((float)(rand()%100-50),(float)(rand()%100-50),0.f,p);
				Models[o->Type].TransformPosition(o->BoneTransform[BoneHead],p,Position,true);
				CreatePointer(BITMAP_BLOOD,Position,(float)(rand()%360),o->Light,(float)(rand()%4+8)*0.1f);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// 충격 효과(캐릭터나 몬스터가 충격을 받을때 나는 효과)
///////////////////////////////////////////////////////////////////////////////

/*void AttackRange(vec3_t Position,float Range,int Damage)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live)
		{
			vec3_t p;
			VectorSubtract(o->Position,Position,p);
			float Distance = p[0]*p[0]+p[1]*p[1];
			if(Distance <= Range*Range)
			{
				SetPlayerShock(c);
			}
		}
	}
}*/

/*int SearchTargetCharacter(CHARACTER *c)
{
	int Dir[] = {-1,-1, 0,-1,  1,-1,  1,0,  1,1,  0,1,  -1,1,  -1,0};
	int Angle = ((BYTE)((c->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8);
	int x = c->PositionX+Dir[Angle*2  ];
	int y = c->PositionY+Dir[Angle*2+1];
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *tc = &CharactersClient[i];
		OBJECT *to = &tc->Object;
		if(to->Live && to->Visible)
		{
			if(x==tc->PositionX && y==tc->PositionY)
				return i;
		}
	}
	return -1;
}*/

///////////////////////////////////////////////////////////////////////////////
// 마검사 등에 달린 망토 효과
///////////////////////////////////////////////////////////////////////////////

#define FLAG_WIDTH  7
#define FLAG_HEIGHT 10
#define FLAG_SCALE  10.f

typedef struct {
    int link_num;
	int link[4];
	float link_length[4];
	vec3_t p;
	vec3_t v;
	vec3_t f;
	vec3_t uf;
	vec3_t normal;
	float  light;
	bool collision;
} physics_vertex;

typedef struct {
	int vlist[4];
	vec3_t normal;
} physics_face;

typedef struct {
	physics_vertex vtx[8];
} physics_boundbox;

physics_vertex flag_vertex[FLAG_HEIGHT*FLAG_WIDTH];
physics_face   flag_face[(FLAG_HEIGHT-1)*(FLAG_WIDTH-1)];

vec3_t Gravity;
float Damping = 0.04f;
float Ks = 5.f; //hook's spring
float Kd = 0.1f; //spring dumping
float Kr = 0.8f;
float DeltaTime = 0.1f;

/*void integrate_physics(int num)
{
    physics_boundbox *b = &object_boundbox[num];
	
	Gravity[0] = 0.f;
	Gravity[1] = 0.5f;
	Gravity[2] = 0.f;
	
	int i,j;
	for(i=0;i<8;i++)
	{
		physics_vertex *v = &b->vtx[i];

		v->f[0]=0.f;
		v->f[1]=0.f;
		v->f[2]=0.f;

		VectorAdd(v->f,Gravity,v->f);
		VectorMA(v->f,-Damping,v->v,v->f);
		VectorAdd(v->f,v->uf,v->f);//user force

		for(j=0;j<v->link_num;j++)
		{
        	physics_vertex *v1 = &b->vtx[i];
			physics_vertex *v2 = &b->vtx[v->link[j]];

			vec3_t DeltaP;
			VectorSubtract(v1->p,v2->p,DeltaP);
			float dist = VectorLength(DeltaP);
			float Hterm = (dist - v->link_length[j]) * Ks;	

			vec3_t DeltaV;
			VectorSubtract(v1->v,v2->v,DeltaV);
			float Dterm = (DotProduct(DeltaV,DeltaP) * Kd) / dist;

			vec3_t SpringForce;
			VectorScale(DeltaP,1.f/dist,SpringForce);	
			VectorScale(SpringForce,-(Hterm+Dterm),SpringForce);
			VectorAdd(v1->f,SpringForce,v1->f);	
			VectorSubtract(v2->f,SpringForce,v2->f);	
		}
	}
	for(i=0;i<8;i++)
	{
    	physics_vertex *v = &b->vtx[i];
		VectorMA(v->v,DeltaTime,v->f,v->v);
		VectorMA(v->p,DeltaTime,v->v,v->p);
	}
	//for(j=0;j<100;j++)
	for(i=0;i<8;i++)
	{
    	physics_vertex *v = &b->vtx[i];

		float height = terrain_height(v->p[0],v->p[2]);
		if(v->p[1] >= height)
		{
			vec3_t normal;
            terrain_normal(v->p[0],v->p[1],normal);
			normal[0]=0.f;
			normal[1]=1.f;
			normal[2]=0.f;
			if(v->collision==0)
			{
				v->collision=1;
				float VdotN = -DotProduct(normal,v->v)*2.f;
				vec3_t Vn;
				VectorScale(normal,VdotN,Vn);
				ddd2=v->v[1];
				ddd3=Vn[1];
				VectorAdd(v->v,Vn,v->v);
				VectorScale(v->v,1.5f,v->v);
				//v->p[1]=height;
			}
		}
		else v->collision=0;
	}
}*/

void CreateFlag()
{
#ifndef _VS2008PORTING				// #ifndef
	int i,j;
#endif // _VS2008PORTING
#ifdef _VS2008PORTING
	for(int i=0;i<FLAG_HEIGHT;i++)
#else // _VS2008PORTING
	for(i=0;i<FLAG_HEIGHT;i++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for(int j=0;j<FLAG_WIDTH;j++)
#else // _VS2008PORTING
		for(j=0;j<FLAG_WIDTH;j++)
#endif // _VS2008PORTING
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];
			v->p[0] = -(float)i*FLAG_SCALE;
			v->p[1] = 0.f;
			v->p[2] = (float)j*FLAG_SCALE;
			if(i==0)
			{
				if(j==0)
				{
					v->p[0] -= 5.f;
					v->p[2] += 5.f;
				}
				if(j==6)
				{
					v->p[0] -= 5.f;
					v->p[2] -= 5.f;
				}
				v->p[1] += 10.f;
			}
			v->link_num = 0;
			if(j-1 >= 0            ) {v->link[v->link_num] = (i  )*FLAG_WIDTH+(j-1);v->link_num++;}
			if(j+1 <= FLAG_WIDTH -1) {v->link[v->link_num] = (i  )*FLAG_WIDTH+(j+1);v->link_num++;}
			if(i-1 >= 0            ) {v->link[v->link_num] = (i-1)*FLAG_WIDTH+(j  );v->link_num++;}
			if(i+1 <= FLAG_HEIGHT-1) {v->link[v->link_num] = (i+1)*FLAG_WIDTH+(j  );v->link_num++;}
		}
	}
#ifdef _VS2008PORTING
	for(int i=0;i<FLAG_HEIGHT-1;i++)
#else // _VS2008PORTING
	for(i=0;i<FLAG_HEIGHT-1;i++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for(int j=0;j<FLAG_WIDTH-1;j++)
#else // _VS2008PORTING
		for(j=0;j<FLAG_WIDTH-1;j++)
#endif // _VS2008PORTING
		{
			physics_face *f = &flag_face[i*(FLAG_WIDTH-1)+j];
			int index = i*FLAG_WIDTH+j;
			f->vlist[0] = index;
			f->vlist[1] = index+FLAG_WIDTH;
			f->vlist[2] = index+FLAG_WIDTH+1;
			f->vlist[3] = index+1;
		}
	}
}

bool InitFlag = false;
int  wind_frame=0;


void AnimationFlag()
{
	if(!InitFlag)
	{
		InitFlag = true;
		CreateFlag();
	}
	wind_frame++;

	Gravity[0] = -0.2f;
	Gravity[1] = 0.f;
	Gravity[2] = 0.f;

#ifdef _VS2008PORTING				
	for(int i=2;i<FLAG_HEIGHT;i++)
#else // _VS2008PORTING
	int i,j,k;
	for(i=2;i<FLAG_HEIGHT;i++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for (int j = 0; j < FLAG_WIDTH; ++j)
#else // _VS2008PORTING
		for ( j = 0; j < FLAG_WIDTH; ++j)
#endif // _VS2008PORTING
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];

			v->f[0]=0.f;
			v->f[1]=0.f;
			v->f[2]=0.f;
			
			VectorAdd(v->f,Gravity,v->f);
			VectorMA(v->f,-Damping,v->v,v->f);

			//v->f[2] += (float)sin(j*0.4f+wind_frame*0.03f)*(j*0.08f);
			if(i<5)
			{
				if(wind_frame%128<64)
				{
					v->f[1] += 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
					v->f[0] -= 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
				}
			}
			if(i==9)
			{
				if(wind_frame%128<64)
				{
					v->f[1] -= 0.8f-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.8f;
				}
			}
			
#ifdef _VS2008PORTING
			for(int k=0;k<v->link_num;k++)
#else // _VS2008PORTING
			for(k=0;k<v->link_num;k++)
#endif // _VS2008PORTING
			{
				physics_vertex *v1 = &flag_vertex[index];
				physics_vertex *v2 = &flag_vertex[v->link[k]];
				
				vec3_t DeltaP;
				VectorSubtract(v1->p,v2->p,DeltaP);
				float dist = VectorLength(DeltaP);
				float Hterm = (dist - FLAG_SCALE) * Ks;	
				
				vec3_t DeltaV;
				VectorSubtract(v1->v,v2->v,DeltaV);
				float Dterm = (DotProduct(DeltaV,DeltaP) * Kd) / dist;
				
				vec3_t SpringForce;
				VectorScale(DeltaP,1.f/dist,SpringForce);	
				VectorScale(SpringForce,-(Hterm+Dterm),SpringForce);
				VectorAdd(v1->f,SpringForce,v1->f);	
				if ( (index+1)/FLAG_WIDTH > (v->link[k]+1)/FLAG_WIDTH)
				{	// 이중 계산은 밑의 줄것만
					VectorSubtract(v2->f,SpringForce,v2->f);
				}
			}
		}
	}
#ifdef _VS2008PORTING
	for(int i=2;i<FLAG_HEIGHT;i++)
#else // _VS2008PORTING
	for(i=2;i<FLAG_HEIGHT;i++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for(int j=0;j<FLAG_WIDTH;j++)
#else // _VS2008PORTING
		for(j=0;j<FLAG_WIDTH;j++)
#endif // _VS2008PORTING
		{
			int index = i*FLAG_WIDTH+j;
			physics_vertex *v = &flag_vertex[index];
			VectorMA(v->v,DeltaTime,v->f,v->v);
			VectorMA(v->p,DeltaTime,v->v,v->p);
		}
	}

}

void RenderFlagFace(OBJECT *o,int x,int y,vec3_t Light,int Tex1,int Tex2)
{
#ifdef _VS2008PORTING
    int n=4;
#else // _VS2008PORTING
    int i,n=4;
#endif // _VS2008PORTING

    float su = (float)x/6.f;
    float sv = (float)y/9.f;
	float TexCoord[4][2];
	TexCoord[0][0] = su;
	TexCoord[0][1] = sv;
	TexCoord[1][0] = su;
	TexCoord[1][1] = sv+1.f/9.f;
	TexCoord[2][0] = su+1.f/6.f;
	TexCoord[2][1] = sv+1.f/9.f;
	TexCoord[3][0] = su+1.f/6.f;
	TexCoord[3][1] = sv;

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    float minz = 65536.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	physics_face *f = &flag_face[y*(FLAG_WIDTH-1)+x];
#ifdef _VS2008PORTING
	for(int i=0;i<n;i++)
#else // _VS2008PORTING
	for(i=0;i<n;i++)
#endif // _VS2008PORTING
	{
		int vlist = f->vlist[i];
     	physics_vertex *v = &flag_vertex[vlist];
		v->light = (-v->normal[0]+v->normal[1])*0.5f+0.5f;
	}

	BindTexture(Tex2);
	glBegin(GL_QUADS);
#ifdef _VS2008PORTING
	for(int i=0;i<n;i++)
#else // _VS2008PORTING
	for(i=0;i<n;i++)
#endif // _VS2008PORTING
	{
		int vlist = f->vlist[i];
     	physics_vertex *v = &flag_vertex[vlist];
		glTexCoord2f(TexCoord[i][0],TexCoord[i][1]);
		glColor3f(Light[0]*v->light,Light[1]*v->light,Light[2]*v->light);
		vec3_t p,Position;
		Vector(v->p[0]+9.f,v->p[1]-12.f,v->p[2]-35.f,p);
		Models[o->Type].TransformPosition(o->BoneTransform[19],p,Position,true);
		glVertex3f(Position[0],Position[1],Position[2]);
	}
	glEnd();

	BindTexture(Tex1);
	glBegin(GL_QUADS);
#ifdef _VS2008PORTING
	for(int i=n-1;i>=0;i--)
#else // _VS2008PORTING
	for(i=n-1;i>=0;i--)
#endif // _VS2008PORTING
	{
		int vlist = f->vlist[i];
     	physics_vertex *v = &flag_vertex[vlist];
		glTexCoord2f(TexCoord[i][0],TexCoord[i][1]);
		glColor3f(Light[0]*v->light,Light[1]*v->light,Light[2]*v->light);
		vec3_t p,Position;
		Vector(v->p[0]+9.f,v->p[1]-12.f,v->p[2]-35.f,p);
		Models[o->Type].TransformPosition(o->BoneTransform[19],p,Position,true);
		glVertex3f(Position[0],Position[1],Position[2]);
	}
	glEnd();
}

void RenderFlag(OBJECT *o,vec3_t Light,int Tex1,int Tex2)
{
#ifdef _VS2008PORTING
    for(int y=0;y<FLAG_HEIGHT-1;y++)
#else // _VS2008PORTING
    int x,y;
    for(y=0;y<FLAG_HEIGHT-1;y++)
#endif // _DEBUG
	{
#ifdef _VS2008PORTING
		for(int x=0;x<FLAG_WIDTH-1;x++)
#else // _VS2008PORTING
		for(x=0;x<FLAG_WIDTH-1;x++)
#endif // _VS2008PORTING
        {
         	physics_face *f = &flag_face[y*(FLAG_WIDTH-1)+x];
            FaceNormalize(flag_vertex[f->vlist[0]].p,
				flag_vertex[f->vlist[1]].p,
				flag_vertex[f->vlist[2]].p,
				f->normal);
        }
	}
#ifdef _VS2008PORTING
    for(int y=0;y<FLAG_HEIGHT;y++)
#else // _VS2008PORTING
    for(y=0;y<FLAG_HEIGHT;y++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for(int x=0;x<FLAG_WIDTH;x++)
#else // _VS2008PORTING
		for(x=0;x<FLAG_WIDTH;x++)
#endif // _VS2008PORTING
        {
         	physics_vertex *v = &flag_vertex[y*FLAG_WIDTH+x];
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			float num = 0.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			if(y<FLAG_HEIGHT-1 && x<FLAG_WIDTH-1) 
			{
				VectorCopy(flag_face[y*(FLAG_WIDTH-1)+x].normal,v->normal);
			}
			else 
			{
				v->normal[0] = 0.f;
				v->normal[1] = 0.f;
				v->normal[2] = 0.f;
			}
        }
	}

#ifdef _VS2008PORTING
    for(int y=0;y<FLAG_HEIGHT-1;y++)
#else // _VS2008PORTING
    for(y=0;y<FLAG_HEIGHT-1;y++)
#endif // _VS2008PORTING
	{
#ifdef _VS2008PORTING
		for(int x=0;x<FLAG_WIDTH-1;x++)
#else // _VS2008PORTING
		for(x=0;x<FLAG_WIDTH-1;x++)
#endif // _VS2008PORTING
        {
			RenderFlagFace(o,x,y,Light,Tex1,Tex2);
        }
	}
}
