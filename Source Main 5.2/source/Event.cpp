// Event.cpp: implementation of the CEvent class.
//
//////////////////////////////////////////////////////////////////////

// common
#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "UIManager.h"
#include "CKanturuDirection.h"
#include "CDirection.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "LoadData.h"
#include "Event.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////
// 크리스마스 이벤트
///////////////////////////////////////////////////////////////////////////////////////////////////////

CXmasEvent::CXmasEvent(void)
{
	m_iEffectID = SHRT_MAX - 60;
}

CXmasEvent::~CXmasEvent(void)
{

}

void CXmasEvent::LoadXmasEvent()
{
	gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_SSANTA, "Data\\Skill\\", "xmassanta");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_SSANTA, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_SNOWMAN, "Data\\Skill\\", "xmassnowman");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_SNOWMAN, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_DEER, "Data\\Skill\\", "xmassaum");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_DEER, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_CHANGE_GIRL, "Data\\Skill\\", "santa");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHANGE_GIRL, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_EARRING, "Data\\Skill\\", "ring");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_EARRING, "Skill\\");
	
	gLoadData.AccessModel(MODEL_XMAS_EVENT_ICEHEART, "Data\\Skill\\", "xmaseicehart");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_ICEHEART, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_BOX, "Data\\Skill\\", "xmasebox");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_BOX, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_CANDY, "Data\\Skill\\", "xmasecandy");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_CANDY, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_TREE, "Data\\Skill\\", "xmasetree");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_TREE, "Skill\\");

	gLoadData.AccessModel(MODEL_XMAS_EVENT_SOCKS, "Data\\Skill\\", "xmaseyangbal");
	gLoadData.OpenTexture(MODEL_XMAS_EVENT_SOCKS, "Skill\\");
}



void CXmasEvent::LoadXmasEventEffect()
{
	LoadBitmap("Effect\\snowseff01.jpg", BITMAP_SNOW_EFFECT_1, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Effect\\snowseff02.jpg", BITMAP_SNOW_EFFECT_2, GL_LINEAR, GL_CLAMP);
}


void CXmasEvent::LoadXmasEventItem()
{
	gLoadData.AccessModel(MODEL_POTION+51, "Data\\Item\\", "MagicBox", 2);
	gLoadData.OpenTexture(MODEL_POTION+51, "Item\\");
}

void CXmasEvent::LoadXmasEventSound()
{
	LoadWaveFile(SOUND_XMAS_JUMP_SNOWMAN, "Data\\Sound\\xmasjumpsnowman.wav", 1);		
	LoadWaveFile(SOUND_XMAS_JUMP_DEER, "Data\\Sound\\xmasjumpsasum.wav", 1);		
	LoadWaveFile(SOUND_XMAS_JUMP_SANTA, "Data\\Sound\\xmasjumpsanta.wav", 1);		
	LoadWaveFile(SOUND_XMAS_TURN, "Data\\Sound\\xmasturn.wav", 1);
}

void CXmasEvent::CreateXmasEventEffect(CHARACTER* pCha, OBJECT *pObj, int iType)
{
	if(pCha->m_iTempKey >= 0)
	{
		DeleteCharacter(pCha->m_iTempKey);
	}

	GenID();

	CHARACTER* c = CreateCharacter(m_iEffectID, MODEL_PLAYER, pCha->PositionX, pCha->PositionY, pCha->Rot);

	pCha->m_iTempKey = m_iEffectID;
	c->Object.Scale = 0.30f;
	c->Object.SubType = iType + MODEL_XMAS_EVENT_CHA_SSANTA;
	//c->Object.SubType = MODEL_XMAS_EVENT_CHA_DEER;
	
	switch(c->Object.SubType)
	{
	case MODEL_XMAS_EVENT_CHA_SSANTA:
		::strcpy(c->ID, GlobalText[2245]);	// 2245
		break;
	case MODEL_XMAS_EVENT_CHA_DEER:
		::strcpy(c->ID, GlobalText[2246]);	// 2246
		break;
	case MODEL_XMAS_EVENT_CHA_SNOWMAN:
		::strcpy(c->ID, GlobalText[2247]);	// 2247
		break;
	}

	c->Object.m_bRenderShadow = false;
	c->Object.Owner = pObj;

	c->Object.m_dwTime = timeGetTime();

	OBJECT* o = &c->Object;
	
	VectorCopy(pObj->Position, o->Position);
	VectorCopy(pObj->Angle, o->Angle);
	o->PriorAction = pObj->PriorAction;
	o->PriorAnimationFrame = pObj->PriorAnimationFrame;
	o->CurrentAction = pObj->CurrentAction;
	o->AnimationFrame = pObj->AnimationFrame;

	vec3_t vPos, vLight;
	Vector(0.6f, 0.6f, 0.6f, vLight);

	VectorCopy(o->Position, vPos);
	vPos[0] += 50.f;
	vPos[1] += 50.f;
	vPos[2] += 50.f;
	CreateParticle(BITMAP_EXPLOTION_MONO,vPos,o->Angle,vLight, 0, 0.6f);
	int i;
	for(i=0; i<2; ++i)
	{
		CreateEffect(MODEL_ICE_SMALL,vPos,o->Angle,vLight, 0);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateEffect(MODEL_HALLOWEEN_CANDY_STAR, vPos, o->Angle, vLight, 1);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
	}

	VectorCopy(o->Position, vPos);
	vPos[0] -= 50.f;
	vPos[1] -= 50.f;
	vPos[2] += 50.f;
	CreateParticle(BITMAP_EXPLOTION_MONO,vPos,o->Angle,vLight, 0, 0.6f);
	for(i=0; i<2; ++i)
	{
		CreateEffect(MODEL_ICE_SMALL,vPos,o->Angle,vLight, 0);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateEffect(MODEL_HALLOWEEN_CANDY_STAR, vPos, o->Angle, vLight, 1);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
	}

	VectorCopy(o->Position, vPos);
	vPos[0] += 50.f;
	vPos[1] -= 50.f;
	vPos[2] += 50.f;
	CreateParticle(BITMAP_EXPLOTION_MONO,vPos,o->Angle,vLight, 0, 0.6f);
	for(i=0; i<2; ++i)
	{
		CreateEffect(MODEL_ICE_SMALL,vPos,o->Angle,vLight, 0);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 3, 1.0f);
		CreateEffect(MODEL_HALLOWEEN_CANDY_STAR, vPos, o->Angle, vLight, 1);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
		CreateParticle(BITMAP_SNOW_EFFECT_1,vPos,o->Angle,vLight, 0, 0.5f);
	}
	
	if(o->CurrentAction == PLAYER_SANTA_2)
	{
		vec3_t vPos, vAngle, vLight;
		VectorCopy(o->Position, vPos);
		vPos[2] += 230.f;
		Vector(1.f, 1.f, 1.f, vLight);
		Vector(0.f, 0.f, 40.f, vAngle);
		CreateEffect(MODEL_XMAS_EVENT_ICEHEART, vPos, vAngle, vLight, 0, o);
		CreateParticle(BITMAP_DS_EFFECT, vPos, o->Angle, vLight, 0, 3.f, o);
		Vector(1.f, 0.f, 0.f, vLight);
		CreateParticle(BITMAP_LIGHT, vPos, o->Angle, vLight, 10, 3.f, o);
	}
}

void CXmasEvent::GenID()
{
	if(m_iEffectID >= SHRT_MAX - 10)
	{
		m_iEffectID = SHRT_MAX - 60;
	}

	m_iEffectID++;
}

CNewYearsDayEvent::CNewYearsDayEvent()
{

}

CNewYearsDayEvent::~CNewYearsDayEvent()
{

}

void CNewYearsDayEvent::LoadModel()
{
	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, "Data\\Monster\\", "sulbeksulki");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_CANDY, "Data\\Monster\\", "sulcandy");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_CANDY, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_MONEY, "Data\\Monster\\", "sulgold");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_MONEY, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, "Data\\Monster\\", "sulgreengochu");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, "Data\\Monster\\", "sulredgochu");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_PIG, "Data\\Monster\\", "sulpeg");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_PIG, "Monster\\");

	gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_YUT, "Data\\Monster\\", "sulyutnulre");
	gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_YUT, "Monster\\");
}

void CNewYearsDayEvent::LoadSound()
{
	LoadWaveFile(SOUND_NEWYEARSDAY_DIE, "Data\\Sound\\newyeardie.wav", 1);
}

CHARACTER* CNewYearsDayEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
	CHARACTER* pCharacter = NULL;

	switch(iType)
	{
	case 365:
		{
			OpenMonsterModel(122);
			pCharacter = CreateCharacter(iKey, MODEL_MONSTER01+122, iPosX, iPosY);
			strcpy(pCharacter->ID, "복주머니");
			pCharacter->Object.Scale = 1.5f;
		}
		break;
	}

	return pCharacter;
}

bool CNewYearsDayEvent::MoveMonsterVisual(CHARACTER* c,OBJECT* o, BMD* b)
{
	if(c == NULL)	return false;
	if(o == NULL)	return false;
	if(b == NULL)	return false;
	switch(o->Type)
	{
	case MODEL_MONSTER01+122:
		{
			vec3_t vRelativePos, vWorldPos;
			Vector(0.f, 0.f, 0.f, vRelativePos);
			b->TransformPosition(o->BoneTransform[81], vRelativePos, vWorldPos, true);

			vec3_t vLight;
			Vector(1.f, 1.f, 1.f, vLight);

			if(o->CurrentAction >= MONSTER01_STOP1 && o->CurrentAction <= MONSTER01_ATTACK2)
			{
				if(rand()%3 == 0)
				{
					Vector(1.0f, 0.4f, 0.0f, vLight);
					CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, vLight, 0, 0.28f);

					if(rand()%2 == 0)
					{
						Vector(0.8f, 0.6f, 0.1f, vLight);
					}
					else
					{
						float fLumi;
						fLumi = (float)(rand()%100) * 0.01f;
						vLight[0] = fLumi;
						fLumi = (float)(rand()%100) * 0.01f;
						vLight[1] = fLumi;
						fLumi = (float)(rand()%100) * 0.01f;
						vLight[2] = fLumi;
					}
					CreateParticle(BITMAP_SHINY, vWorldPos, o->Angle, vLight, 4, 0.8f);
				}
				
			}
			else if(o->CurrentAction == MONSTER01_SHOCK || o->CurrentAction == MONSTER01_DIE)
			{
				if(rand()%3 == 0)
				{
					Vector(1.0f, 0.4f, 0.0f, vLight);
					float fScale;
					if(o->CurrentAction == MONSTER01_SHOCK)	fScale = 0.4f;
					else if(o->CurrentAction == MONSTER01_DIE) fScale = 0.5f;
						
					CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, vLight, 0, fScale);

					for(int i=0; i<5; ++i)
					{
						if(rand()%2 == 0)
						{
							Vector(0.8f, 0.6f, 0.1f, vLight);
						}
						else
						{
							float fLumi;
							fLumi = (float)(rand()%100) * 0.01f;
							vLight[0] = fLumi;
							fLumi = (float)(rand()%100) * 0.01f;
							vLight[1] = fLumi;
							fLumi = (float)(rand()%100) * 0.01f;
							vLight[2] = fLumi;
						}
						CreateParticle(BITMAP_SHINY, vWorldPos, o->Angle, vLight, 4, 0.8f);
					}
				}
				if(rand()%2 == 0)
				{
					Vector(0.3f, 0.3f, 0.8f, vLight);
					CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, vLight, 1, 1.f);
				}
			}

			if(o->CurrentAction == MONSTER01_DIE)
			{
				if(o->AnimationFrame <= 0.3f)
				{
					o->m_iAnimation = rand()%6 + MODEL_NEWYEARSDAY_EVENT_BEKSULKI;
					CreateParticle(BITMAP_EXPLOTION, vWorldPos, o->Angle, vLight, 0, 0.5f);
					if(rand()%4 == 0) o->m_iAnimation = MODEL_NEWYEARSDAY_EVENT_PIG;

					PlayBuffer(SOUND_NEWYEARSDAY_DIE);
				}
				
				if(o->AnimationFrame >= 4.5f)
					CreateEffect(MODEL_NEWYEARSDAY_EVENT_MONEY, vWorldPos, o->Angle, vLight);

				if(o->m_iAnimation != 0 && rand()%3 == 0)
				{
					if(o->AnimationFrame >= 4.5f)
						CreateEffect(o->m_iAnimation, vWorldPos, o->Angle, vLight);
				}
			}
		}
		break;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// 행운의 파란가방 이벤트
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND

const DWORD CBlueLuckyBagEvent::m_dwBlueLuckyBagCheckTime = 600000;

CBlueLuckyBagEvent::CBlueLuckyBagEvent()
{
	m_bBlueLuckyBag = false;
	m_dwBlueLuckyBagTime = 0;
}

CBlueLuckyBagEvent::~CBlueLuckyBagEvent()
{

}

void CBlueLuckyBagEvent::StartBlueLuckyBag()
{
	m_bBlueLuckyBag = true;
	m_dwBlueLuckyBagTime = timeGetTime();
}

void CBlueLuckyBagEvent::CheckTime()
{
	if(m_bBlueLuckyBag == true)
	{
		if(timeGetTime() - m_dwBlueLuckyBagTime > m_dwBlueLuckyBagCheckTime)
		{
			m_bBlueLuckyBag = false;
			m_dwBlueLuckyBagTime = 0;
		}
	}
}

bool CBlueLuckyBagEvent::IsEnableBlueLuckyBag()
{
	return m_bBlueLuckyBag;
}
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND

C09SummerEvent::C09SummerEvent()
{

}

C09SummerEvent::~C09SummerEvent()
{

}

void C09SummerEvent::LoadModel()
{

}

void C09SummerEvent::LoadSound()
{
	LoadWaveFile( SOUND_UMBRELLA_MONSTER_WALK1,	"Data\\Sound\\UmbMon_Walk01.wav", 1 );
	LoadWaveFile( SOUND_UMBRELLA_MONSTER_WALK2,	"Data\\Sound\\UmbMon_Walk02.wav", 1 );
	LoadWaveFile( SOUND_UMBRELLA_MONSTER_DAMAGE, "Data\\Sound\\UmbMon_Damage01.wav", 1 );
	LoadWaveFile( SOUND_UMBRELLA_MONSTER_DEAD,	"Data\\Sound\\UmbMon_Dead.wav", 1 );
}

CHARACTER* C09SummerEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
	CHARACTER* pCharacter = NULL;

	if(iType == 463)
	{
		OpenMonsterModel(154);
		pCharacter = CreateCharacter(iKey, MODEL_MONSTER01+154, iPosX, iPosY);
		strcpy(pCharacter->ID, "우산");
		pCharacter->Object.Scale = 0.8f;
		pCharacter->Object.HiddenMesh = 2;
		pCharacter->Object.m_iAnimation = 0;

		OBJECT* pObject = &pCharacter->Object;
	}

	return pCharacter;
}

bool C09SummerEvent::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	if( o->Type != MODEL_MONSTER01+154 )
		return false;

	vec3_t vRelativePos, vWorldPos;
	Vector(0.f, 0.f, 0.f, vRelativePos);
	b->TransformPosition(o->BoneTransform[39], vRelativePos, vWorldPos, true);
	
	vec3_t vLight;
	float fLumi = (sinf(WorldTime*0.0015f) + 1.0f) * 0.4f + 0.2f;

	Vector(0.8f, 0.4f, 0.2f, vLight);
		
	CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, vLight, o );
	Vector(fLumi*0.8f, fLumi*0.4f, fLumi*0.2f, vLight);
	CreateSprite(BITMAP_LIGHT, vWorldPos, 3.0f, vLight, o );
	
	switch( o->CurrentAction )
	{
	case MONSTER01_WALK:
		{
			if( o->AnimationFrame >= 1.0f && o->AnimationFrame <= 1.3f )
			{
				if( o->m_iAnimation == 0 )
				{
					PlayBuffer(SOUND_UMBRELLA_MONSTER_WALK1);
					o->m_iAnimation = 1;
				}
				else
				{
					PlayBuffer(SOUND_UMBRELLA_MONSTER_WALK2);
					o->m_iAnimation = 0;
				}
			}
		}break;
	case MONSTER01_SHOCK:
		{
			if( o->AnimationFrame >= 0.5f && o->AnimationFrame <= 0.8f )
				PlayBuffer(SOUND_UMBRELLA_MONSTER_DAMAGE);
		}break;
	case MONSTER01_DIE:
		{
			if( o->AnimationFrame > 0.0f && o->AnimationFrame <= 0.3f )
			{
 				CreateEffect( MODEL_EFFECT_SKURA_ITEM, o->Position, o->Angle, o->Light, 1, o );
				vec3_t Position, Angle, Light;
				Position[0] = o->Position[0];
				Position[1] = o->Position[1];
				Position[2] = RequestTerrainHeight(Position[0],Position[1]);
				ZeroMemory( Angle, sizeof ( Angle));
				Light[0] = Light[1] = Light[2] = 1.0f;
				CreateEffect(BITMAP_FIRECRACKER0001,Position,Angle,Light,0);
				CreateEffect( MODEL_EFFECT_UMBRELLA_DIE, o->Position, o->Angle, o->Light, 0, o);
				for(int i=0 ; i<40 ; i++)
				{
					CreateEffect( MODEL_EFFECT_UMBRELLA_GOLD, o->Position, o->Angle, o->Light, 0, o);
				}
			}
			if( o->AnimationFrame >= 0.5f && o->AnimationFrame <= 0.8f )
			{
				PlayBuffer(SOUND_UMBRELLA_MONSTER_DEAD);
			}
		}break;
	}

	return true;
}
