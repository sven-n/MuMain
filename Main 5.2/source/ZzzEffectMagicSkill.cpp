///////////////////////////////////////////////////////////////////////////////
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
#include "SkillManager.h"


void RenderCircle(int Type,vec3_t ObjectPosition,float ScaleBottom,float ScaleTop,float Height,float Rotation,float LightTop,float TextureV)
{
	BindTexture(Type);

	vec3_t Light[4];
	Vector(1.f,1.f,1.f,Light[0]);
	Vector(1.f,1.f,1.f,Light[1]);
	Vector(LightTop,LightTop,LightTop,Light[2]);
	Vector(LightTop,LightTop,LightTop,Light[3]);

	float Num    = 12.f;
	for(float x=0.f;x<Num;x+=1.f)
	{
     	float UV[4][2];
		TEXCOORD(UV[0],(x    )*(1.f/Num),1.f);
		TEXCOORD(UV[1],(x+1.f)*(1.f/Num),1.f);
		TEXCOORD(UV[2],(x+1.f)*(1.f/Num),0.f);
		TEXCOORD(UV[3],(x    )*(1.f/Num),0.f);

		vec3_t Angle;
		float Matrix1[3][4];
		float Matrix2[3][4];
		Angle[0] = 0.f;
		Angle[1] = 0.f;
		Angle[2] = (x    )*30.f+Rotation;
		AngleIMatrix(Angle,Matrix1);
		Angle[2] = (x+1.f)*30.f+Rotation;
		AngleIMatrix(Angle,Matrix2);
		
     	vec3_t p,Position[4];
		Vector(0.f,ScaleBottom,0.f,p);
		VectorRotate(p,Matrix1,Position[0]);
		VectorAdd(ObjectPosition,Position[0],Position[0]);
		Vector(0.f,ScaleBottom,0.f,p);
		VectorRotate(p,Matrix2,Position[1]);
		VectorAdd(ObjectPosition,Position[1],Position[1]);
		Vector(0.f,ScaleTop,Height,p);
		VectorRotate(p,Matrix2,Position[2]);
		VectorAdd(ObjectPosition,Position[2],Position[2]);
		Vector(0.f,ScaleTop,Height,p);
		VectorRotate(p,Matrix1,Position[3]);
		VectorAdd(ObjectPosition,Position[3],Position[3]);

		glBegin(GL_QUADS);
		for(int i=0;i<4;i++)
		{
			glTexCoord2f(UV[i][0],UV[i][1]+TextureV);
			glColor3fv(Light[i]);
			glVertex3fv(Position[i]);
		}
		glEnd();
	}
}

void RenderCircle2D(int Type,vec3_t ScreenPosition,float ScaleBottom,float ScaleTop,float Height,float Rotation,float TextureV,float TextureVScale)
{
	vec3_t ObjectPosition;
	VectorCopy(ScreenPosition,ObjectPosition);
    ObjectPosition[1] = WindowHeight-ObjectPosition[1];

	BindTexture(Type);

	float Num    = 12.f;
	for(float x=0.f;x<Num;x+=1.f)
	{
     	float UV[4][2];
		TEXCOORD(UV[0],(x    )*(1.f/Num),1.f);
		TEXCOORD(UV[1],(x+1.f)*(1.f/Num),1.f);
		TEXCOORD(UV[2],(x+1.f)*(1.f/Num),TextureVScale);
		TEXCOORD(UV[3],(x    )*(1.f/Num),TextureVScale);

		vec3_t Angle;
		float Matrix1[3][4];
		float Matrix2[3][4];
		Angle[0] = 90.f;
		Angle[1] = 0.f;
		Angle[2] = (x    )*30.f+Rotation;
		AngleIMatrix(Angle,Matrix1);
		Angle[2] = (x+1.f)*30.f+Rotation;
		AngleIMatrix(Angle,Matrix2);
		
		vec3_t Light[4];
     	float Luminosity;
		vec3_t p,Position[4];
		Vector(0.f,ScaleBottom,0.f,p);
		VectorRotate(p,Matrix1,Position[0]);
		Luminosity = 0.5f+0.5f*(-Position[0][0]-Position[0][2])/sqrtf(Position[0][0]*Position[0][0]+Position[0][2]*Position[0][2]);
		Vector(Luminosity,Luminosity,Luminosity,Light[0]);
		VectorCopy(Light[0],Light[3]);
		Vector(0.f,ScaleBottom,0.f,p);
		VectorRotate(p,Matrix2,Position[1]);
		Luminosity = 0.5f+0.5f*(-Position[1][0]-Position[1][2])/sqrtf(Position[1][0]*Position[1][0]+Position[1][2]*Position[1][2]);
		Vector(Luminosity,Luminosity,Luminosity,Light[1]);
		VectorCopy(Light[1],Light[2]);
		Vector(0.f,ScaleTop,Height,p);
		VectorRotate(p,Matrix2,Position[2]);
		Vector(0.f,ScaleTop,Height,p);
		VectorRotate(p,Matrix1,Position[3]);
		
		glBegin(GL_QUADS);
		for(int i=0;i<4;i++)
		{
			glTexCoord2f(UV[i][0],UV[i][1]+TextureV);
			glColor3fv(Light[i]);
     		VectorAdd(ObjectPosition,Position[i],Position[i]);
			glVertex2f(Position[i][0],Position[i][1]);
		}
		glEnd();
	}
}

void CreateMagicShiny(CHARACTER *c,int Hand)
{
	OBJECT *o = &c->Object;
	BMD *b = &Models[o->Type];
	vec3_t p,Position;
	Vector(0.f,0.f,0.f,p);
	//for(int i=0;i<1;i++)
	{
		b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
		//VectorCopy(o->Position,Position);
		//Position[2] += 140.f;
		vec3_t Light;
		//Vector(o->Alpha,o->Alpha,o->Alpha,Light);
		Vector(1.f,0.5f,0.2f,Light);
		CreateParticle(BITMAP_SHINY+1,Position,o->Angle,Light,Hand+0,0.f,o);
		CreateParticle(BITMAP_SHINY+1,Position,o->Angle,Light,Hand+2,0.f,o);
	}
}

void CreateTeleportBegin(OBJECT *o)
{
    SetAttackSpeed();
	SetAction(o,PLAYER_SKILL_TELEPORT);
	o->AlphaTarget = 0.f;
	o->Teleport = TELEPORT_BEGIN;
	CreateEffect(BITMAP_SPARK+1,o->Position,o->Angle,o->Light);
    PlayBuffer(SOUND_MAGIC);
}

void CreateTeleportEnd(OBJECT *o)
{
    SetAttackSpeed();
    SetAction(o,PLAYER_SKILL_TELEPORT);
	o->AnimationFrame = 5.f;
    o->Teleport = TELEPORT_END;
	o->AlphaTarget = 1.f;
	CreateEffect(BITMAP_SPARK+1,o->Position,o->Angle,o->Light);
    PlayBuffer(SOUND_MAGIC);
}

extern int CurrentSkill;

void CreateArrow(CHARACTER *c,OBJECT *o,OBJECT *to,
				 WORD SkillIndex,WORD Skill,WORD SKKey)
{
    int SubType = 0;
	int Right   = c->Weapon[0].Type;
	int Left    = c->Weapon[1].Type;
	if(c == Hero)
	{
		Right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		Left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		if(Right != -1)
			Right += MODEL_ITEM;
		if(Left != -1)
			Left += MODEL_ITEM;
	}

    if( SKKey == AT_SKILL_PIERCING )
    {
        SubType = 2;
		PlayBuffer(SOUND_BOW01,o);
    }
    CurrentSkill = SKKey;
    if ( SKKey==AT_SKILL_PARALYZE )
    {
		PlayBuffer(SOUND_ICEARROW,o);
    }

    if ( Skill==2 )
	{
		SubType = 99;
	}

    if ( SKKey==AT_SKILL_DEEPIMPACT )
    {
        CreateEffect ( MODEL_ARROW_IMPACT, o->Position, o->Angle, o->Light, 0, to, o->PKKey, SkillIndex, Skill );
    }
    else
    {
        switch(Right)
        {
        case MODEL_BOW+8 :CreateEffect(MODEL_ARROW_STEEL  ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+9 :CreateEffect(MODEL_ARROW_STEEL  ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+10:CreateEffect(MODEL_ARROW_SAW    ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+11:CreateEffect(MODEL_ARROW_LASER  ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+12:CreateEffect(MODEL_ARROW_THUNDER,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+13:CreateEffect(MODEL_ARROW_WING   ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+14:CreateEffect(MODEL_ARROW_BOMB   ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+16:CreateEffect(MODEL_ARROW_DOUBLE ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+18:
			CreateEffect(MODEL_ARROW_BEST_CROSSBOW,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);
			break;
        case MODEL_BOW+19:CreateEffect(MODEL_ARROW_DRILL,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        }
        switch(Left)
        {
        case MODEL_BOW  :CreateEffect(MODEL_ARROW        ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+1:CreateEffect(MODEL_ARROW        ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+2:CreateEffect(MODEL_ARROW_V      ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+3:CreateEffect(MODEL_ARROW        ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+4:CreateEffect(MODEL_ARROW        ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+5:CreateEffect(MODEL_ARROW        ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+6:CreateEffect(MODEL_ARROW_NATURE ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+17:CreateEffect(MODEL_ARROW_HOLY  ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+20:CreateEffect(MODEL_LACEARROW   ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        case MODEL_BOW+21:CreateEffect(MODEL_ARROW_SPARK ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
		case MODEL_BOW+22:CreateEffect(MODEL_ARROW_RING   ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
		case MODEL_BOW+23:CreateEffect(MODEL_ARROW_DARKSTINGER   ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;	
		case MODEL_BOW+24:CreateEffect(MODEL_ARROW_GAMBLE ,o->Position,o->Angle,o->Light,SubType,o,o->PKKey,SkillIndex,Skill);break;
        }                                                                               
    }
}

void CreateArrows(CHARACTER *c,OBJECT *o,OBJECT *to,WORD SkillIndex,WORD Skill,WORD SKKey)
{
    if (   SKKey==AT_SKILL_PIERCING || SKKey==AT_SKILL_PARALYZE 
        || SKKey==AT_SKILL_DEEPIMPACT
       )
    {
        CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
	    CharacterMachine->PacketSerial++;
    }
    else
    {
	    if ( Skill==1 )
	    {
			if ( c->Weapon[0].Type==MODEL_BOW+18 
				|| c->Weapon[0].Type==MODEL_BOW+19 
				|| c->Weapon[1].Type == MODEL_BOW+22
				|| c->Weapon[1].Type == MODEL_BOW+23
				|| c->Weapon[1].Type == MODEL_BOW+24
				)
			{	
				o->Angle[2] += 5.f;//15.f;//7.5f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] += 10.f;//15.f;//7.5f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] -= 20.f;//45.f;//22.5f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] -= 10.f;//15.f;//7.5f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] += 30.f;//30.f;//15.f;
				CharacterMachine->PacketSerial++;
			}
			else
			{
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] += 15.f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] -= 30.f;
				CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
				o->Angle[2] += 15.f;
				CharacterMachine->PacketSerial++;
			}
	    }
		else
		{
			CreateArrow(c,o,to,SkillIndex,Skill,SKKey);
		}

		if ( c->Weapon[0].Type==MODEL_BOW+19 )
		{
			vec3_t p, Position, Light;
			BMD *b = &Models[o->Type];

			Vector ( 1.f, 1.f, 1.f, Light );
			Vector ( 0.f, 10.f, -130.f, p );
			b->TransformPosition ( o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true );

			vec3_t a;
			for ( int i=0; i<15; ++i )
			{
				Vector ( (float)(rand()%360), 0.f, 0.f, a );
				if ( (rand()%2)==0 )
				{
					CreateJoint(BITMAP_JOINT_SPARK,Position,Position,a,3);
				}
				CreateParticle(BITMAP_SPARK,Position,a,Light,0 );
			}
		}
    }
}
