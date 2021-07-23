///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BoneManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "GMCrywolf1st.h"

vec3_t g_vParticleWind = { 0.0f, 0.0f, 0.0f};
vec3_t g_vParticleWindVelo = { 0.0f, 0.0f, 0.0f};


///////////////////////////////////////////////////////////////////////////////
// 여러가지 파티클 효과 처리
///////////////////////////////////////////////////////////////////////////////

void HandPosition(PARTICLE *o)
{
	OBJECT *Owner = o->Target;
	BMD *b = &Models[Owner->Type];
	vec3_t p;
	switch(o->Type)
	{
	case BITMAP_FLARE_RED:
	case BITMAP_SHINY+2:
		Vector(0.f,-120.f,0.f,p);
		break;
	default:
		Vector(0.f,0.f,0.f,p);
		break;
	}
	VectorCopy(Owner->Position,b->BodyOrigin);
	b->TransformPosition(Owner->BoneTransform[Hero->Weapon[o->SubType%2].LinkBone],p,o->Position,true);
}

int CreateParticle(int Type,vec3_t Position,vec3_t Angle,vec3_t Light,int SubType,float Scale,OBJECT *Owner)
{
	for(int i=0;i<MAX_PARTICLES;i++)
	{
		PARTICLE *o = &Particles[i];
		if(!o->Live)
		{
			o->Live = true;
			o->Type = Type;
            o->TexType = Type;
			o->SubType = SubType;

			VectorCopy(Position,o->Position);
			VectorCopy(Position,o->StartPosition);
			VectorCopy(Light,o->Light);
   			o->Scale = Scale;
   			o->Gravity = 0.f;
   			o->LifeTime = 2;
			o->Frame = 0;
			o->Target = Owner;
  			o->Rotation = 0.f;
            o->bEnableMove = true;
			VectorCopy(Angle,o->Angle);
			Vector(0.f,0.f,0.f,o->Velocity);

			vec3_t p;
			float Matrix[3][4];
			switch(o->Type)
			{
//////////////////////////////////////////////////////////////////////////

			case BITMAP_EFFECT:
				if(o->SubType == 1)
				{
					o->TexType = BITMAP_EXT_LOG_IN+2;
					o->Scale = (float)(rand()%10+20)*0.01f;
					o->Gravity = (float)(rand()%10+20)*0.05f;
				}
				else if(o->SubType == 2)
				{
					o->Gravity = 0.0f;
					o->Scale = (float)(rand()%5+12)*0.1f;
				}
				else if(o->SubType == 3)
				{
					o->TexType = BITMAP_CLUD64;
					o->Scale = (float)(rand()%5+10)*0.01f;
					o->Gravity = (float)(rand()%10+20)*0.05f;
				}
				else if(o->SubType == 0)
				{
					o->Gravity = 0.0f;
					o->Scale = (float)(rand()%5+20)*0.1f;
				}

				// 라이프타임
				o->LifeTime = 20 + rand()%3;
				if(o->SubType == 2) 
				{
					o->LifeTime = 40 + rand()%3;
				}
				// 위치
				o->Position[0] += (float)(rand()%50 - 25);
				o->Position[1] += (float)(rand()%50 - 25);
				o->Position[2] += (float)(rand()%200 - 100) + 250.0f;

				if(o->SubType == 4)
				{
					o->Gravity = 0.0f;
#ifdef PJH_NEW_SERVER_SELECT_MAP
					o->Scale = (float)(rand()%5+20)*0.1f*1.3f;
					o->Position[0] = Position[0] + (float)(rand()%80 - 40)*1.3f;
#else //PJH_NEW_SERVER_SELECT_MAP
					o->Scale = (float)(rand()%5+20)*0.1f;
					
					o->Position[0] = Position[0] + (float)(rand()%80 - 40);
#endif //PJH_NEW_SERVER_SELECT_MAP
					o->Position[2] -= 100.f;
				}
				else if(o->SubType == 5)
				{
					o->TexType = BITMAP_EXT_LOG_IN+2;
#ifdef PJH_NEW_SERVER_SELECT_MAP
					o->Scale = (float)(rand()%10+20)*0.01f*1.3f;
					o->Gravity = (float)(rand()%10+20)*0.05f*1.3f;
#else //PJH_NEW_SERVER_SELECT_MAP
					o->Scale = (float)(rand()%10+20)*0.01f;
					o->Gravity = (float)(rand()%10+20)*0.05f;
#endif //PJH_NEW_SERVER_SELECT_MAP
					o->Position[2] -= 100.f;
				}
#ifdef YDG_ADD_DOPPELGANGER_PORTAL
				else if(o->SubType == 6)
				{
					o->Gravity = (float)(rand()%20+80)*0.1f;
					o->Scale = (float)(rand()%5+20)*0.1f;
					
					//o->Position[0] = Position[0] + (float)(rand()%80 - 40);
					o->Position[2] -= 200.f;
				}
				else if(o->SubType == 7)
				{
					o->TexType = BITMAP_EXT_LOG_IN+2;
					o->Scale = (float)(rand()%10+20)*0.01f;
					o->Gravity = (float)(rand()%20+80)*0.1f;

					o->Position[2] -= 100.f;
				}
#endif	// YDG_ADD_DOPPELGANGER_PORTAL
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_FLOWER01:
			case BITMAP_FLOWER01+1:
			case BITMAP_FLOWER01+2:
   				o->LifeTime = 15+rand()%10;
   				o->Scale = (float)(rand()%8+4)*0.03f;
      			Vector(1.f,1.f,1.f,o->Light);
				o->Velocity[0] = (float)(rand()%32-16)*0.1f;
				o->Velocity[1] = (float)(rand()%32-16)*0.1f;
				o->Velocity[2] = (float)(rand()%32-32)*0.1f;
				break;
            case BITMAP_FLARE_BLUE:
                if ( o->SubType==0 )
                {
   				    o->LifeTime = 30+rand()%10;
      			    Vector ( 1.f, 1.f, 1.f, o->Light );
                    Vector ( 0.f, 0.f, (rand()%100)/50.f, o->Velocity );
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime = 26+(rand()%2);
					o->Gravity = 0;
					o->Velocity[0] = 0;
					o->Scale    = Scale+(float)(rand()%6)*0.1f;
					o->Rotation = (float)(rand()%360);
                }
				break;
            case BITMAP_FLARE+1:
                if ( o->SubType==0 )
                {
   				    o->LifeTime = 110+rand()%10;
                    o->Gravity  = ((rand()%100)/100.f)*2.f+1.f;
					o->Velocity[0] = (float)(rand()%100-50);
					o->Scale    = Scale+(float)(rand()%2)*0.01f;
					o->Rotation = (float)(rand()%360);

					Vector ( 0.f, 0.f, 0.f, o->Angle );

					VectorCopy(o->Position,o->StartPosition);
				}
                break;
			case BITMAP_BUBBLE:
                switch ( o->SubType )
                {
                case 0:
                case 1:
   				    o->LifeTime = 30+rand()%10;
   				    o->Scale = (float)(rand()%6+4)*0.03f;
      			    Vector(1.f,1.f,1.f,o->Light);
                    break;
                case 2:
   				    o->LifeTime = 30+rand()%10;
   				    o->Scale = (float)(rand()%6+4)*0.03f;
      			    Vector(1.f,1.f,1.f,o->Light);
                    PlayBuffer ( SOUND_DEATH_BUBBLE );
                    break;
                case 3:
   				    o->LifeTime = 30+rand()%10;
   				    o->Scale = (float)(rand()%6+4)*0.03f+Scale;
                    o->Gravity = (float)(rand()%6+4)*0.03f;
      			    Vector(1.f,1.f,1.f,o->Light);
                    break;
				case 4:
   				    o->LifeTime = 30+rand()%10;
   				    o->Scale = (float)(rand()%6+4)*0.03f;
      			    Vector(1.f,1.f,1.f,o->Light);
					break;
				case 5:
   				    o->LifeTime = 30+rand()%10;
   				    o->Scale = (float)(rand()%6+4)*0.04f;
      			    Vector(1.f,1.f,1.f,o->Light);
                    break;
                }
				break;
			case BITMAP_LIGHTNING+1:
				switch(o->SubType)
				{
				case 0:
    				o->Scale = 0.15f;
					break;
				case 4:		//. Monster81(울프) 눈깔 라이트
					o->Scale = 0.25f;
					break;
				case 1:
     				o->LifeTime = 10;
    				o->Scale = 0.f;
					break;
				case 2:
     				o->LifeTime = 20;
    				o->Scale = 1.f;
					break;
                case 3:
     				o->LifeTime = 1;
    				o->Scale = Scale;
                    break;
				case 5:
					o->Scale = Scale;
					break;
				}
				
				if(o->SubType==4)
				{
					VectorCopy(Light, o->Light);
				}
				else if( o->SubType == 5)
				{
					VectorCopy(Light, o->Light);
				}
				else
				{
					Vector(Light[0]+0.5f,Light[1]+0.5f,Light[2]+0.5f,o->Light);
				}
				break;
			case BITMAP_LIGHTNING:
				o->LifeTime = 10;
				o->Scale = 1.8f;
				o->Angle[0] = 30.f;
				Vector((float)(rand()%4+6)*0.1f,(float)(rand()%4+6)*0.1f,(float)(rand()%4+6)*0.1f,o->Light);
				o->Position[2] += 260.f;
				//CreateEffect(BITMAP_LIGHTNING+1,o->Position,o->Angle,o->Light);
				//for(i=0;i<5;i++)
     			//	CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,3);
				break;
			case BITMAP_CHROME_ENERGY2:
 				o->LifeTime = 8;
				Vector(0.f,0.f,0.f,o->Velocity);
				//Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
         		o->Scale = Scale*(float)(rand()%64+128)*0.01f;
				o->Rotation = (float)(rand()%360);
				break;
			case BITMAP_FIRE_CURSEDLICH:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK2_MONO:
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0)
				{
					o->LifeTime = (rand()%12+8);
					Vector(0.f,0.f,0.f,o->Velocity);
          			o->Scale = (float)(rand()%30+20)*0.01f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (rand()%15+15)*0.01f;
				}
				else if (o->SubType == 1)
				{
					o->Position[0] += (rand()%10-5)*0.2f;
					o->Position[1] += (rand()%10-5)*0.2f;
					o->Position[2] += (rand()%10-5)*0.2f;
					o->LifeTime = (rand()%28+4);
					Vector(0.f,0.f,0.f,o->Velocity);
          			o->Scale = (float)(rand()%5+50)*0.016f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (rand()%5+10)*0.01f;
					VectorCopy(o->Target->Position, o->StartPosition);
				}
				else if (o->SubType == 2)
				{
					o->LifeTime = (rand()%12+8);
					Vector(0.f,0.f,0.f,o->Velocity);
          			o->Scale = (float)(rand()%30+20)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (rand()%15+15)*0.01f;
				}
				else if (o->SubType == 3)
				{
					o->LifeTime = (rand()%12+16);
					Vector(0.f,0.f,0.f,o->Velocity);
          			o->Scale = (float)(rand()%30+20)*0.02f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (rand()%15+15)*0.02f;
					VectorCopy(o->Target->Position, o->StartPosition);
				}
				else if (o->SubType == 4
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
						|| o->SubType == 9
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
      				o->LifeTime = rand()%5+12;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
				else if (o->SubType == 5)
				{
      				o->LifeTime = rand()%5+24;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+74)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if (o->SubType == 6)
				{
					o->LifeTime = rand()%5+24;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+100)*0.1f;
					VectorCopy(o->Angle, o->Velocity);
					float fRand = (float)(rand()%50) * 0.03f;
					Vector(2.5f+fRand, -5.0f-fRand, 0.f, o->Velocity);
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif // CSK_ADD_MAP_ICECITY	
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 7)
				{
      				o->LifeTime = rand()%5+12;
     				o->Scale = (float)(rand()%72+52)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 8)
				{
					o->LifeTime = 8;//rand()%5+5;
					o->Scale = (float)(rand()%42+12)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
					VectorCopy(o->Target->Position, o->StartPosition);
				}
#endif	// YDG_ADD_SKELETON_PET
				break;
			case BITMAP_LEAF_TOTEMGOLEM:
				o->LifeTime = rand()%10+40;
				Vector(0.f,0.f,0.f,o->Velocity);
          		o->Scale = (float)(rand()%20+10)*0.04f;
				o->Rotation = (float)(rand()%360);
				o->Gravity = (rand()%40+30)*-0.05f;
				o->Velocity[0] = rand()%10-5;
				o->Velocity[1] = rand()%10-5;
				o->Velocity[2] = rand()%5+10;
				o->Alpha = 1.0f;
				break;
			case BITMAP_FIRE:
			case BITMAP_FIRE+2:
            case BITMAP_FIRE+3:
#ifdef KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
				{
					switch( o->SubType )
					{
					case 0:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							o->Scale = (float)(rand()%64+128)*0.01f;
							o->Rotation = (float)(rand()%360);
						}break;
					case 1:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							o->Scale = (float)(rand()%4+10)*0.01f;
							o->Rotation = (float)(rand()%360);
						}break;
					case 17:
					case 5:
						{
							if(o->SubType == 17)
								o->LifeTime = 10;
							else
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							o->Scale = Scale*(float)(rand()%64+128)*0.01f;
							o->Rotation = (float)(rand()%360);
						}break;
					case 7:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
							o->Scale = (float)(rand()%64+128)*0.008f+Scale;
							o->Rotation = (float)(rand()%360);
						}break;
					case 9:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							int range = rand()%60-30;
							o->StartPosition[0] = (float)range;
							o->StartPosition[1] = (float)range;
							o->StartPosition[2] = 190.f - abs( range )*1.5f;
							o->Rotation = (float)(rand()%360);
						}break;
					case 10:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							o->Rotation = (float)(rand()%360);
						}break;
					case 11:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
							o->Scale = (float)(rand()%64+128)*0.008f+Scale;
							o->Rotation = (float)(rand()%360);
						}break;
					case 12:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
							o->Rotation = (float)(rand()%360);
							o->Scale = Scale*(float)(rand()%16+150)*0.012f;
						}break;
					case 13:
						{
							o->LifeTime = 20;
							Vector(0.f,0.f,(float)(rand()%128+128)*0.15f,o->Velocity);
							o->Rotation = (float)(rand()%360);
							o->Scale = (float)(rand()%16+150)*0.012f;
						}break;
					case 14:
						{
							o->LifeTime = 24;
							o->Scale = 1.5f;
							o->TexType = BITMAP_CLUD64;
							Vector(0.f,0.0f,0.f,o->Velocity);
							//	o->Gravity = (float)(rand()%60 - 30)/10.0f;
							o->Rotation = (float)(rand()%360);
						}break;
					case 15:
						{
							o->LifeTime = 10;
							o->Scale = (float)(rand()%64+128)*0.01f;
							o->TexType = BITMAP_CLUD64;
							Vector(0.f,0.0f,0.f,o->Velocity);
							o->Rotation = (float)(rand()%360);
						}break;
					case 16:
						{
							o->LifeTime = 4;
						}break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					case 18:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
							o->Rotation = (float)(rand()%360);
							o->TexType = BITMAP_GROUND_SMOKE;
							Vector(1.0f, 1.0f, 1.0f, o->Light);
						}
						break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					default:
						{
							o->LifeTime = 24;
							Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
							o->Rotation = (float)(rand()%360);
						}
					}
				}
				break;
#else // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
				o->LifeTime = 24;
				Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
				if ( o->SubType==0 )
				{
          			o->Scale = (float)(rand()%64+128)*0.01f;
				}
				else if(o->SubType==1)
				{
          			o->Scale = (float)(rand()%4+10)*0.01f;
				}
				else if ( o->SubType==5 )
				{
          			o->Scale = Scale*(float)(rand()%64+128)*0.01f;
				}
                else if ( o->SubType==7 )
                {
				    Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
                    o->Scale = (float)(rand()%64+128)*0.008f+Scale;
                }
                else if ( o->SubType==9 )
                {
                    int range = rand()%60-30;
                    o->StartPosition[0] = (float)range;
                    o->StartPosition[1] = (float)range;
                    o->StartPosition[2] = 190.f - abs( range )*1.5f;
                }
				else if ( o->SubType==10)
				{	// 지정한 크기 적용
				}
                else if ( o->SubType==11 )
                {
					o->LifeTime = 24;
				    Vector(0.f,-(float)(rand()%32-16)*0.1f,0.f,o->Velocity);
                    o->Scale = (float)(rand()%64+128)*0.008f+Scale;
                }
				else if ( o->SubType==12 )
				{
                    o->Rotation = (float)(rand()%360);
          			o->Scale = Scale*(float)(rand()%16+150)*0.012f;
				}
				else if ( o->SubType==13 )
				{
				    o->LifeTime = 20;
				    Vector(0.f,0.f,(float)(rand()%128+128)*0.15f,o->Velocity);
                    o->Rotation = (float)(rand()%360);
          			o->Scale = (float)(rand()%16+150)*0.012f;
				}
				else if(o->SubType == 14)
				{
					o->LifeTime = 24;
					o->Scale = 1.5f;
					o->TexType = BITMAP_CLUD64;
					Vector(0.f,0.0f,0.f,o->Velocity);
				//	o->Gravity = (float)(rand()%60 - 30)/10.0f;
				}
				else if(o->SubType == 15)
				{
					o->LifeTime = 10;
					o->Scale = (float)(rand()%64+128)*0.01f;
					o->TexType = BITMAP_CLUD64;
					Vector(0.f,0.0f,0.f,o->Velocity);
				}
				o->Rotation = (float)(rand()%360);
				break;
#endif // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
			case BITMAP_FIRE+1:
				if(o->SubType==1)
				{
					o->LifeTime = 3;
					Vector(0.f,-(float)(rand()%8+32)*0.3f,0.f,o->Velocity);
        			//o->Scale = 0.8f;
				}
				else if(o->SubType==2)
				{
					o->LifeTime = 5;
					Vector(0.f,-(float)(rand()%8+32)*0.1f,0.f,o->Velocity);
        			//o->Scale = 0.8f;
				}
				else if(o->SubType==3)
				{
					o->LifeTime = 7;
					Vector(0.f,-(float)(rand()%8+32)*0.1f,0.f,o->Velocity);
        			//o->Scale = 0.8f;
				}
				else if(o->SubType==4)
				{
	                o->LifeTime = 100;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+1.f;
					o->Velocity[0] = (float)(rand()%300-150);
					o->Scale    = Scale+(float)(rand()%6)*0.15f;
					o->Rotation = (float)(rand()%360);

					Vector ( 0.f, 0.f, 0.f, o->Angle );
					VectorCopy(o->Position,o->StartPosition);
				}
				else if(o->SubType==5)
				{
	                o->LifeTime = 6;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+o->Angle[0]*1.2f;//45.f;
					o->Scale    = Scale+(float)(rand()%6)*0.20f;
					o->Rotation = (float)(rand()%360);

					VectorCopy(o->Position,o->StartPosition);
				}
				else if(o->SubType==6)
				{
	                o->LifeTime = 6;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+5.f;
					o->Scale    = Scale+(float)(rand()%6)*0.10f;
					o->Rotation = (float)(rand()%360);

					VectorCopy(o->Position,o->StartPosition);
				}
				else if(o->SubType==7)
				{
					o->LifeTime = 10;
				}
				else if(o->SubType==8)
				{
					o->Position[0] += (float)(rand()%20 - 10);
					o->Position[1] += (float)(rand()%20 - 10);
					o->Position[2] += (float)(rand()%20 - 10);
					o->LifeTime = 32;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%5 + 5) / 5.0f;
				}
				else if(o->SubType==9)
				{
					o->TexType = BITMAP_LIGHT+2;
					o->LifeTime = 32;
					//o->Scale    = Scale+(float)(rand()%6)*0.10f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%5 + 5) / 10.0f;
				}
				else if(o->SubType==0)
				{
					o->LifeTime = 12;
					Vector(0.f,-(float)(rand()%8+32)*0.3f,0.f,o->Velocity);
        			o->Scale = 0.8f;
				}
    			o->Rotation = (float)(rand()%360);
				break;

//--------------------------------------------------------------------------------------------------------------

			case BITMAP_FLAME:
                switch (o->SubType)
                {
                case 0: //  불기둥에 사용된는 불.
				    o->LifeTime = 20;
				    Vector(0.f,0.f,(float)(rand()%128+128)*0.15f,o->Velocity);
        		    o->Scale = Scale*(float)(rand()%64+64)*0.01f;
                    break;
				case 6:	// 짧은 불기둥
					o->LifeTime = 10;
				    Vector(0.f,0.f,(float)(rand()%128+256)*0.12f,o->Velocity);
        		    o->Scale = Scale*(float)(rand()%64+64)*0.01f;
					break;
				case 8:
				case 7:
					{
						o->LifeTime = 33;
        				o->Scale = -(float)(rand()%20)/100.0f + o->Scale;
						o->Rotation = (float)(rand()%360) - 180.0f;
						o->StartPosition[0] = o->Position[0];
						o->Position[0] += ((float)(rand()%2)/2.0f - 0.0f) * o->Scale;
						o->Gravity = ((float)(rand()%100)/100.0f + 1.8f) * o->Scale;
						float fTemp = 0.0f;
						if(rand()%10 >= 3)
							fTemp = ((float)(rand()%20)/10.0f - 1.0f) * o->Scale;
						Vector(0.0f, fTemp, 0.0f, o->Velocity);
						o->Position[1] += o->Position[0] - o->StartPosition[0];
					}
					break;
                case 1: //  작은 불기둥.
				    o->LifeTime = 15;
        		    o->Scale += (float)(rand()%32+32)*0.01f;
				    Vector(0.f,(float)(rand()%4+4)*0.15f,0.f,o->Velocity);
                    break;
				case 5:
				    o->LifeTime = 4;
				    Vector(0.f,(float)(rand()%4+4)*0.15f,0.f,o->Velocity);
					break;
                case 2: //  불. 같은 속도로 이동.
                    {
                        float Luminosity;

						o->LifeTime = 10;
						o->Scale += (float)(rand()%32+32)*0.01f;

						float inter = Light[0]*(rand()%80)/100.0f;
                        Vector(0.f,inter,0.f,o->Velocity);
                        MovePosition(o->Position,o->Angle,o->Velocity);

                        inter = (Light[0]-inter)/15.0f;
				        Vector(0.f,inter,0.f,o->Velocity);

                        //  색.
                        Luminosity = (float)sinf(WorldTime*0.002f)*0.3f+0.7f;
			            Vector(Luminosity,Luminosity*0.5f,Luminosity*0.5f,o->Light);
                    }
                    break;

                case 3: //  제자리에서 회전만 한다.
                    {
                        float Luminosity;

                        o->LifeTime = 10;
        		        o->Scale += (float)(rand()%32+32)*0.01f;
				        Vector(0.f,0.f,0.f,o->Velocity);

                        //  색.
                        Luminosity = (float)sinf(WorldTime*0.002f)*0.3f+0.7f;
			            Vector(Luminosity,Luminosity*0.5f,Luminosity*0.5f,o->Light);
                    }
                    break;
                case 4:
                    {
                        float Luminosity;

				        o->LifeTime = 5;
        		        o->Scale += (float)(rand()%32+32)*0.01f;
                        o->Gravity = 10.f;
                        VectorCopy(o->Target->Position,o->StartPosition);

                        //  색.
                        Luminosity = (float)sinf(WorldTime*0.002f)*0.3f+0.7f;
			            Vector(Luminosity,Luminosity*0.5f,Luminosity*0.5f,o->Light);
                    }
                    break;
				case 9:
					{
						o->LifeTime = 40;
        				o->Scale = -(float)(rand()%20)/100.0f + o->Scale;
						o->Rotation = (float)(rand()%360) - 180.0f;
						o->StartPosition[0] = o->Position[0];
						o->Position[0] += ((float)(rand()%2)/2.0f - 0.0f) * o->Scale;
						o->Gravity = ((float)(rand()%100)/100.0f + 1.8f) * o->Scale + 2.0f;
						float fTemp = 0.0f;
						if(rand()%10 >= 3)
						{
							fTemp = ((float)(rand()%20)/10.0f - 1.0f) * o->Scale;
						}
						Vector(0.0f, fTemp*2.0f, 0.0f, o->Velocity);
						o->Position[1] += o->Position[0] - o->StartPosition[0];
					}
					break;
				case 10:	// 불 뿜어져 나오는 효과.(BITMAP_ADV_SMOKE 2번과 동일)
					o->LifeTime = 20+rand()%5;
					o->Rotation = 0.f;
					o->Scale = Scale*0.5f + (float)(rand()%10)*0.02f;
					o->Velocity[0] = (float)(rand()%10+5)*0.4f;
					o->Velocity[1] = (float)(rand()%10-5)*0.4f;
					o->Velocity[2] = (float)(rand()%10+5)*0.2f;
					break;
#ifdef CSK_RAKLION_BOSS
				case 11:
					o->LifeTime = 20;
					o->Rotation = (float)(rand()%360);
					o->TexType = BITMAP_FIRE_HIK2_MONO;
					Vector(0.f,0.f,(float)(rand()%128+128)*0.15f,o->Velocity);
        		    o->Scale = Scale*(float)(rand()%64+64)*0.01f;
					break;
#endif // CSK_RAKLION_BOSS
#ifdef LDS_ADD_EMPIRE_GUARDIAN
                case 12: //  짧은 불기둥
					o->LifeTime = 10;
					Vector(0.f,0.f,(float)(rand()%128+128)*0.15f,o->Velocity);
					o->Scale = Scale*(float)(rand()%64+64)*0.01f;
                    break;
#endif //LDS_ADD_EMPIRE_GUARDIAN
				}
				break;

//--------------------------------------------------------------------------------------------------------------

			case BITMAP_FIRE_RED:
				o->LifeTime = 20;
				Vector(0.f,0.f,(float)(rand()%100+100)*0.15f,o->Velocity);
        		o->Scale = Scale*(float)(rand()%64+64)*0.01f;
				break;

//--------------------------------------------------------------------------------------------------------------

			case BITMAP_RAIN_CIRCLE:
			case BITMAP_RAIN_CIRCLE+1:
   				o->LifeTime = 20;
        		o->Scale = (rand()%6+8)*0.1f;
				if(o->Type==BITMAP_RAIN_CIRCLE+1 || o->SubType==1 )
				{
					o->Position[0] += (rand()%10-5)*1.f;
					o->Position[1] += (rand()%10-5)*1.f;
					o->Position[2] += (rand()%10-5)*1.f;
				}
                if ( o->SubType==1 )
                {
                    o->Scale = Scale;
                    o->Gravity = rand()%100/1000.f;
                    o->Velocity[1] = 1.f;
                }
				break;

//--------------------------------------------------------------------------------------------------------------

			case BITMAP_ENERGY://thunder energy
        		o->Scale = Scale*(float)(rand()%8+6)*0.1f;
				o->Rotation = (float)(rand()%360);
				o->Gravity = 20.f;	//. 회전속도

				if ( o->SubType==1 )
				{
					o->LifeTime = 10;
					Vector ( 0.5f, 0.5f, 0.5f, o->Light );
				}
				else if(o->SubType == 2)	// 빨강	//^ 펜릴 이펙트 관련
				{
					o->TexType = BITMAP_MAGIC+1;
					o->LifeTime = 10;
					o->Scale = 0.3f + Scale*(float)(rand()%8+6)*0.1f;
					VectorCopy(o->Light, Light);
					o->Rotation = 0;
					o->Gravity = 0;
				}
				// ChainLighting
				else if(o->SubType == 3 || o->SubType == 4 || o->SubType == 5)
				{
					o->LifeTime = 25;
				}
				else if(o->SubType == 6)
				{
					o->LifeTime = 20;
				}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType == 7)
				{
					o->LifeTime = 5;
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				break;

//--------------------------------------------------------------------------------------------------------------
				
			case BITMAP_MAGIC:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 10;
						o->Scale = Scale;
					}
				}
				break;

//--------------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case BITMAP_FLARE:  //
                o->LifeTime = 60;
                if ( o->SubType==0 || o->SubType==3 || o->SubType==6 
					|| o->SubType == 10
					)
                {
                    o->Gravity  = ((rand()%100)/100.f)*4.f+1.f;
                }
                else if(o->SubType==2)
                {
                    o->Gravity  = ((rand()%100)/100.f)*5.f+5.f;
                }
                else if ( o->SubType==4 )
                {
                    o->LifeTime = 40;
                    o->Gravity  = ((rand()%100)/100.f)*5.f+1.f;
                }
				else if ( o->SubType==5 )
				{
                    o->LifeTime = 40;
                    o->Gravity  = ((rand()%100)/100.f)*5.f+1.f;
				}
                o->Velocity[0] = (float)(rand()%300-150);
                o->Scale    = Scale+(float)(rand()%6)*0.01f;
                o->Rotation = (float)(rand()%360);

                Vector ( 0.f, 0.f, 0.f, o->Angle );

                VectorCopy(o->Position,o->StartPosition);
				if(o->SubType==10)
				{
					float count = (o->Velocity[0]+o->LifeTime)*0.1f;
					o->StartPosition[0] = o->StartPosition[0] + sinf(count)*40.f;
					o->StartPosition[1] = o->StartPosition[1] - cosf(count)*40.f;
				}
				else if ( o->SubType == 11 )
				{
                    o->LifeTime = 26+(rand()%2);
					o->Gravity = 0;
					o->Velocity[0] = 0;
					o->Scale    = Scale+(float)(rand()%6)*0.1f;
					o->Rotation = (float)(rand()%360);
				}
				else
				if(o->SubType == 12)
				{
                    o->LifeTime = 80;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+1.f;
					Vector(0.2f,0.6f,1.f,o->Light);
				}
                break;

//--------------------------------------------------------------------------------------------------------------

			case BITMAP_LIGHT+2:
				if(o->SubType == 0)
				{
					o->LifeTime = 16;
     				o->Scale += (float)(rand()%32+48)*0.01f;
      				o->Angle[0] = (float)(rand()%360);
					o->Rotation = (float)((int)WorldTime%360);
				}
				else if(o->SubType == 1)
				{
					o->TexType = BITMAP_SMOKE;
					o->LifeTime = 20;
     				o->Scale += (float)(rand()%10+10)*0.01f;
      				o->Angle[0] = (float)(rand()%360);
					o->Rotation = (float)((int)WorldTime%360);
					o->Gravity = (float)(rand()%10+40)*0.04f;
					o->Position[1] += 10.f;
				}
				else if(o->SubType == 2)
				{
					o->LifeTime = 10;
     				o->Scale -= (float)(rand()%40+48)*0.008f;
      				o->Angle[0] = (float)(rand()%360);
					o->Rotation = (float)((int)WorldTime%360);
					o->Gravity = 0.0f;
				}
				else if (o->SubType == 3)
				{
					o->LifeTime = 20;
     				o->Scale += (float)(rand()%32+48)*0.008f;
      				o->Angle[0] = (float)(rand()%360);
					o->Rotation = (float)((int)WorldTime%360);
					VectorCopy(o->Target->Position, o->StartPosition);
				}
				else if(o->SubType == 4)
				{
					o->LifeTime = 16;
					o->Scale += (float)(rand()%32+48)*0.01f;
					o->Angle[0] = (float)(rand()%360);
					o->Rotation = (float)((int)WorldTime%360);
				}
				else if(o->SubType == 5)
				{
					o->TexType = BITMAP_ADV_SMOKE;

					BMD * pModel = &Models[o->Target->Type];
					int iNumBones = pModel->NumBones;

					vec3_t vRelative, p1;
					Vector(0.0f, 0.0f, 0.0f, vRelative);
					pModel->TransformPosition(o->Target->BoneTransform[rand()%iNumBones], vRelative, o->Position, false);
					VectorScale(o->Position, pModel->BodyScale, o->Position);
					VectorAdd(o->Position, o->Target->Position, o->Position);

					o->Velocity[0] = 0;
					o->Velocity[1] = 0;
					o->Velocity[2] = 0;

					o->LifeTime = 21;
					o->Scale    = (float)(rand()%4+8)*0.01f;
					o->Gravity = (float)(rand()%1+4)*0.1f;

					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%2+60)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->TurningForce);

					o->TurningForce[2] += 4.0f;
 					o->Rotation = (float)((int)WorldTime%360);
					o->Alpha = 1.0f;
				}
				else if(o->SubType == 6)
				{
					o->LifeTime = 21;
					o->Scale    = Scale + (float)(rand()%4+8)*0.01f;
					o->Gravity = (float)(rand()%1+1)*0.1f;
					
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					vec3_t p1;
					Vector(0.f,(float)(rand()%2+60)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->TurningForce);
					
					o->TurningForce[2] += 2.0f;
					o->Rotation = (float)((int)WorldTime%360);
					o->Alpha = 1.0f;

					o->Velocity[0] = 0;
					o->Velocity[1] = 0;
					o->Velocity[2] = 0;
				}
				// Drain Life (드레인 라이프)
				else if( o->SubType == 7 )
				{
					o->LifeTime = 18;
					o->Scale    = Scale + (float)(rand()%4+8)*0.01f;
					o->Gravity = (float)(rand()%1+1)*0.1f;
					
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					vec3_t p1;
					Vector(0.f,(float)(rand()%2+60)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->TurningForce);
					
					o->TurningForce[2] += 2.0f;
					o->Rotation = (float)((int)WorldTime%360);
					o->Alpha = 1.0f;
					
					o->Velocity[0] = 0;
					o->Velocity[1] = 0;
					o->Velocity[2] = 0;
				}
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_MAGIC+1:
				o->LifeTime = 10;
     			o->Scale += (float)(rand()%32+48)*0.001f;
      			o->Angle[0] = (float)(rand()%360);
				o->Rotation = (float)((int)WorldTime%360);
				break;
			case BITMAP_BLUE_BLUR:
				switch(SubType)
				{
				case 0:
      				o->LifeTime = 30;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 1:
					o->LifeTime = 30;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					o->Position[2] -= 45.00f;
					o->TexType  = BITMAP_POUNDING_BALL; 
					break;
				}
				break;
				
//////////////////////////////////////////////////////////////////////////

			case BITMAP_CLUD64:
				{
					if(o->SubType == 0 || o->SubType == 2)
					{
						if(rand()%4 != 0)
							o->TexType = BITMAP_SMOKE;
						
						o->LifeTime = 40;
     					o->Scale = (float)(rand()%20+250)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Position[0] += (float)(rand()%20-10);
    					o->Position[1] += (float)(rand()%20-10);
    					o->Position[2] -= 20.f;
						o->Gravity = (float)(rand()%10+5)*0.1f;
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 12;
						o->Scale += (float)(rand()%30)*0.01f;
						o->Rotation = (float)(rand()%360);
						o->Position[2] += 30.0f;
					}
					else if(o->SubType == 3)
					{
						if(rand()%2 != 0)
							o->TexType = BITMAP_SMOKE;
						o->LifeTime = 50;
     					o->Scale = Scale + (float)(rand()%10+10)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Position[0] += (float)(rand()%20-10);
    					o->Position[1] += (float)(rand()%20-10);
    					o->Position[2] -= 20.f;
						o->Gravity = (float)(rand()%10+5)*0.1f;
					}
					else if(o->SubType == 4)
					{
						if(rand()%2 != 0)
							o->TexType = BITMAP_SMOKE;
						o->LifeTime = 4;
     					o->Scale = Scale + (float)(rand()%10+10)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Position[0] += (float)(rand()%20-10);
    					o->Position[1] += (float)(rand()%20-10);
    					o->Position[2] -= 20.f;
						o->Gravity = (float)(rand()%10+5)*0.1f;
					}
					else if(o->SubType == 5)
					{
						if(rand()%2 != 0)
							o->TexType = BITMAP_SMOKE;
						o->LifeTime = 50;
						o->Scale = Scale + (float)(rand()%10+10)*0.01f;
						o->Rotation = (float)(rand()%360);
						o->Position[0] += (float)(rand()%20-10);
						o->Position[1] += (float)(rand()%20-10);
						o->Position[2] -= 20.f;
						o->Gravity = (float)(rand()%10+5)*0.1f;
					}
					else if(o->SubType == 6)	// ◎
					{
      					o->LifeTime = 25;
     					o->Scale = (float)(rand()%8+50)*0.01f*Scale;
      					o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%10+35)*0.1f;
						o->Alpha = 0;
						VectorCopy(Light, o->TurningForce);
						Vector(0, 0, 0, o->Light);
						VectorCopy(o->Target->Position, o->StartPosition);
					}
					else if( o->SubType == 7 )		// 썬더네이핀 공격 이팩트 (연기)
					{
						o->LifeTime = 40;
						o->Velocity[0] = (float)(rand()%20-10)*0.1f;
						o->Velocity[1] = (float)(rand()%20-10)*0.1f;
						//o->Velocity[2] = (float)(rand()%20-10)*0.2f;
						o->Position[2] += 80.0f;
						o->Gravity = 0;
						o->Alpha = 0.5f;
					}
					else if( o->SubType == 8 )		// 고스트 네이핀 공격 이팩트 (연기)
					{
						o->LifeTime = 20;
						o->Velocity[0] = (float)(rand()%20-10)*0.1f;
						o->Velocity[1] = (float)(rand()%20-10)*0.1f;
						//o->Velocity[2] = (float)(rand()%20-10)*0.2f;
						o->Position[2] += 80.0f;
						o->Gravity = 0;
						o->Alpha = 0.5f;
					}
#ifdef ADD_SOCKET_ITEM
					else if( o->SubType == 9 )		// 프로스트메이스 (아이스연기 내려오는 효과)
					{ 
						if(rand()%2 != 0)
							o->TexType = BITMAP_SMOKE;
						o->LifeTime = 40;
						o->Alpha = 0.5f;
						// 10단계 +-10% 크기변환
						float fIntervalScale = Scale*0.1f;
						o->Scale += ((float((rand()%20)-10)/2.0f)*((fIntervalScale/2.f)));
      					o->Rotation = (float)(rand()%360);
						o->Velocity[0] = (float)(rand()%20-10)*0.03f;
						o->Velocity[1] = (float)(rand()%20-10)*0.03f;
						o->Velocity[2] = -((1.2f)+((float)(rand()%20-10)*0.025f));
						o->Gravity = 2.f+((float)(rand()%20-10)*0.05f);
					}
#endif // ADD_SOCKET_ITEM
#ifdef ASG_ADD_SUMMON_RARGLE
					else if(o->SubType == 10)	// BITMAP_FIRE_CURSEDLICH o->SubType == 1과 비슷.
					{
						o->Position[0] += (rand()%10-5)*0.2f;
						o->Position[1] += (rand()%10-5)*0.2f;
						o->Position[2] += (rand()%10-5)*0.2f;
						o->LifeTime = (rand()%28+4);
						Vector(0.f,0.f,0.f,o->Velocity);
						o->Scale = (float)(rand()%5+70)*0.016f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (rand()%5+10)*0.01f;
						VectorCopy(o->Target->Position, o->StartPosition);
					}
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef LDK_ADD_PC4_GUARDIAN
					else if(o->SubType == 11)
					{
						if(rand()%4 != 0)
							o->TexType = BITMAP_SMOKE;
						
						o->LifeTime = 30;
     					o->Scale = Scale;
      					o->Rotation = (float)(rand()%360);
						o->Position[0] += (float)(rand()%20-10);
    					o->Position[1] += (float)(rand()%20-10);
    					o->Position[2] += (float)(rand()%20-10);
						o->Gravity = -1.5f;
					}
#endif //LDK_ADD_PC4_GUARDIAN

				}
				break;

#ifdef PBG_ADD_LITTLESANTA_NPC
			case BITMAP_LIGHT+3:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = rand()%10+140;
						o->Gravity = rand()%10+5.0f;
						o->Scale = (rand()%20+20.f)/100.f*Scale;
						o->Position[0] += (float)(rand()%100-50);
						o->Position[1] += (float)(rand()%100-50);
					}
#ifdef PJH_ADD_PANDA_PET
					else if(o->SubType == 1)
					{
						o->Scale = 0.1f;
						o->StartPosition[0] = o->Scale;
						o->LifeTime = rand()%10+20;
						o->Position[0] += (float)(rand()%50 - 25);
						o->Position[1] += (float)(rand()%50 - 25);
						o->Position[2] += (float)(rand()%50 - 25);

						o->Gravity = 1.0f;

						o->Angle[0] = (float)(rand()%360);
						o->Angle[1] = (float)(rand()%360);
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle, Matrix);
						Vector(0.5f,0.5f,0.5f,p);
						VectorScale(p,0.8f,p);
						VectorRotate(p, Matrix, o->Velocity);

						o->Rotation = rand()%20-10;
					}
#endif //PJH_ADD_PANDA_PET
				}
				break;
#endif // PBG_ADD_LITTLESANTA_NPC

			case BITMAP_TWINTAIL_WATER:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 50+rand()%10;
     					o->Scale = (float)(rand()%32+140)*0.01f;
    					o->Position[0] += (float)(rand()%120-60);
    					o->Position[1] += (float)(rand()%100-50);
    					o->Position[2] += (float)(rand()%100);
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 20+rand()%10;
     					o->Scale = (float)(rand()%32+50)*0.01f;
    					o->Position[0] += (float)(rand()%80-40);
    					o->Position[1] += (float)(rand()%80-40);
    					o->Position[2] += (float)(rand()%80);
					}
					else if (o->SubType == 2)
					{
						o->LifeTime = 60+rand()%10;
     					o->Scale = (float)(rand()%32+50)*0.02f;
    					o->Position[0] += (float)(rand()%40-20);
    					o->Position[1] += (float)(rand()%40-20);
    					o->Position[2] += (float)(rand()%40);     					
					}
					o->Angle[0] = (float)(rand()%360);
    				o->Angle[2] = (float)(rand()%360);
					o->Gravity = (float)(rand()%10+20)*0.1f;
				}
				break;
			case BITMAP_SMOKE:
				switch(SubType)
				{
				case 0://일반
#ifdef PBG_ADD_PKFIELD
				case 61:
#endif //PBG_ADD_PKFIELD
				case 4://일반
                case 9://
				case 23:	// 일반 (녹색)
      				o->LifeTime = 16;
#ifdef PBG_ADD_PKFIELD
					if(o->Type == MODEL_MONSTER01+159)
						o->Scale = (float)(rand()%3+28)*0.01f;
					else
						o->Scale = (float)(rand()%32+48)*0.01f;
#endif //PBG_ADD_PKFIELD
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 42:
					o->LifeTime = 16;
     				o->Scale = (float)(rand()%32+48)*0.006f;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 17://색 바꾸기 가능 일반
					VectorCopy ( o->Light, o->TurningForce );
      				o->LifeTime = 12;
     				o->Scale = Scale*(float)(rand()%32+8)*0.01f;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 36:
				case 37:
					if(o->SubType == 37)
					{
						VectorCopy(o->Light , o->TurningForce);
						o->Position[2] -= 15.0f * o->TurningForce[2];
						o->Position[1] -= 15.0f * o->TurningForce[1];
						Vector(1.0f, 1.0f, 1.0f, o->Light);
					}
					else
						Vector(1.0f, 0.3f, 0.0f, o->Light);
	      			o->LifeTime = 25;
                    o->Rotation = (float)(rand()%360);
      				o->Angle[0] = (float)(rand()%360);
                    Vector( (float)(rand()%50-25), (float)(rand()%50-25), -(float)(rand()%50), o->StartPosition );
                    break;
				case 38:
					o->TexType = BITMAP_WATERFALL_2;
					o->LifeTime = 35;
					o->Rotation = (float)(rand()%360);
					break;
				case 34:
				case 35:
					Vector(1.0f, 0.2f, 0.2f, o->Light);
					o->LifeTime = 30;
      				o->Rotation = (float)(rand()%360);
					if(SubType == 35)
					{
						Vector(0.0f, -5.0f, 0.0f, o->Velocity);
					}
					else
					{
						Vector(0.0f, 0.0f, 0.0f, o->Velocity);
					}
					break;
				case 33:
					{
      					o->LifeTime = rand()%150;
     					o->Scale = Scale + 0.1f;
     					o->Angle[0] = (float)(rand()%90-45);
    					o->Angle[2] = (float)(rand()%360);
     					Vector(0.f,-(float)(rand()%8),0.f,o->Velocity);
					}
					break;
				case 32:
					{
      					o->LifeTime = 10;
     					o->Scale = (float)(rand()%32+80)*0.01f;
     					o->Angle[0] = (float)(rand()%90-45);
    					o->Angle[2] = (float)(rand()%360);
     					Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					}
					break;
				case 10:	//. Fire
      				o->LifeTime = 16;
					break;
				case 12:	//. Fire ( Black ).
      				o->LifeTime = 20;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 13:	//. Fire.
      				o->LifeTime = 20;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 18:	//. Fire.   천천히 커진다.
      				o->LifeTime = 20;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 3:
     				o->LifeTime = 10;
     				o->Scale = (float)(rand()%32+80)*0.01f;
     				o->Angle[0] = (float)(rand()%90-45);
    				o->Angle[2] = (float)(rand()%360);
     				Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					break;
				case 11://초록
				case 14:
     				o->LifeTime = 50;
    				o->Position[0] += (float)(rand()%64-32);
    				o->Position[1] += (float)(rand()%64-32);
    				o->Position[2] += (float)(rand()%64+32);
     				o->Angle[0] = (float)(rand()%90-45);
    				o->Angle[2] = (float)(rand()%360);
					o->Rotation = (float)(rand()%360);
					VectorCopy ( o->Light, o->TurningForce );
     				Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					break;
                case 15:    //  스케일을 갖는 일반 연기.
      				o->LifeTime = 80;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
                    break;
#ifdef PJH_GIANT_MAMUD
				case 53:
#endif //#ifdef PJH_GIANT_MAMUD
#ifdef ASG_ADD_SUMMON_RARGLE
				case 56:	// 보라색.
#endif	// ASG_ADD_SUMMON_RARGLE
				case 1:		//초록
     				o->LifeTime = 50;
     				o->Scale = (float)(rand()%32+80)*0.01f;
    				o->Position[0] += (float)(rand()%64-32);
    				o->Position[1] += (float)(rand()%64-32);
    				o->Position[2] += (float)(rand()%64+32);
     				o->Angle[0] = (float)(rand()%90-45);
    				o->Angle[2] = (float)(rand()%360);
     				Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					break;
				case 2://검정
      				o->LifeTime = 50;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
                case 16://검정.
      				o->LifeTime = 50;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
                    break;
                case 5://회색.
      				o->LifeTime = 20;
     				o->Scale = (float)(rand()%64+98)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
                    break;
                case 6 :
     			    o->LifeTime = 30;
				    o->Gravity = (float)(rand()%1000);
     			    o->Scale = (float)(rand()%20+180)*0.01f;
    			    o->Position[0] += (float)(rand()%200-100)*Scale;
    			    o->Position[1] += (float)(rand()%200-100)*Scale;
    			    o->Position[2] += (float)(rand()%20+20);
                    o->Rotation = o->Position[2];
         			Vector(0.f,0.f,0.f,o->Velocity);
                    break;

                case    7 :
     			    o->LifeTime = 30;
				    Vector(0.f,(float)(rand()%4+6)*o->Scale,0.f,o->Velocity);

                    o->Gravity = (float)(rand()%200/200.0f);
                    o->Scale *= (float)(rand()%20+120)*0.01f;
                    o->Rotation = (float)(rand()%360);
                    break;

                    //  나태.
                case 8:
				    o->LifeTime = 24;
                    Vector(0.f,-(float)(rand()%8+32)*0.3f,0.f,o->Velocity);
        		    o->Scale *= 0.8f;
				    o->Rotation = (float)(rand()%360);
                    break;
				case 19://색 바꾸기,스케일 가능 일반
					VectorCopy ( o->Light, o->TurningForce );
      				o->LifeTime = 40;
     				o->Scale = Scale*(float)(rand()%32+8)*0.01f;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 20://색 바꾸기,스케일 가능 일반
      				o->LifeTime = 40;
     				o->Scale = Scale*(float)(rand()%32+8)*0.01f;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 21:	//. 검은연기 박스 Fixed : 2004/11/05
					o->LifeTime = 80;
					o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 22:	//. 붉은연기 박스 : 2004/11/26
					o->LifeTime = 60;
					o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 24:	// 일반 (녹색, 스케일 조정가능)
#ifdef ASG_ADD_SUMMON_RARGLE
				case 57:	// 보라색.
#endif	// ASG_ADD_SUMMON_RARGLE
      				o->LifeTime = 32;
     				o->Scale = Scale+(float)(rand()%32+48)*0.01f*Scale;
      				o->Angle[0] = (float)(rand()%360);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 25:
      				o->LifeTime = 25;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 26:
					o->LifeTime = 25;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 27:	
					o->LifeTime = 30;
      				o->Rotation = (float)(rand()%360);
					o->TexType = BITMAP_CHROME3;
					break;
				case 28:
					o->LifeTime = 12;
     				o->Scale = 2.3f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = 33.0f;
					o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+15;
					o->TexType = BITMAP_POUNDING_BALL;	
					break;
				case 29:
					o->LifeTime = 12;
     				o->Scale = 2.3f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = 33.0f;
					o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+15;
					break;
				case 30:
					o->LifeTime = 4;
     				o->Scale = 1.8f;
					o->Gravity = 15.0f;	
      				o->Rotation = (float)(rand()%360);
					o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+15;
					o->Position[0] += (float)sinf(WorldTime)*5.0f;
					o->Position[1] += (float)sinf(WorldTime)*5.0f;	
					break;
				case 31:
					o->LifeTime = 15;
     				o->Scale = Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 40:
					{
						o->LifeTime = 50;
     					o->Scale = (float)(rand()%32+80)*0.01f;
    					o->Position[0] += (float)(rand()%64-32);
    					o->Position[1] += (float)(rand()%64-32);
    					o->Position[2] += (float)(rand()%64+32);
     					o->Angle[0] = (float)(rand()%90-45);
    					o->Angle[2] = (float)(rand()%360);
     					Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					}
					break;
				case 41:
					{
						o->LifeTime = 50;
     					o->Scale = (float)(rand()%32+80)*0.01f;
    					o->Position[0] += (float)(rand()%64-32);
    					o->Position[1] += (float)(rand()%64-32);
    					o->Position[2] += (float)(rand()%64+32);
     					o->Angle[0] = (float)(rand()%90-45);
    					o->Angle[2] = (float)(rand()%360);
     					Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
						o->Gravity = (float)(rand()%100-50)*0.1f;
					}
					break;
				case 43: // 몬스터(마야) 입김 효과- (색 ,스케일 조정 가능)
					{
						o->TexType = BITMAP_CLUD64;
      					o->LifeTime = 40;
						o->Rotation = (float)(rand()%360);
      					o->Angle[0] = 90.0f;
						Vector(rand()%20/10.0f-1.0f, 2.0f, 15.0f, o->Velocity);
					}
					break;
				case 44: // 몬스터(마야) 몸체 효과- (색 ,스케일 조정 가능)
					{
      					o->LifeTime = 60;
						o->Scale += (float)(rand()%10)/2.0f;
						o->Position[0] += (float)(rand()%500)-250.0f;
						o->Position[1] += (float)(rand()%700)-350.0f;
						o->Position[2] += (float)(rand()%100)+150.0f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%50)-25.0f;
						Vector(0.1f, 0.2f, 0.3f, o->Light);
					}
					break;
				case 45: // 몬스터(마야) 석상 효과 - (색, 스케일 조정 가능)
					{
						o->TexType = BITMAP_LIGHT+2;
      					o->LifeTime = 15;
						o->Scale += (float)(rand()%10)/20.0f;
						o->Position[0] += (float)(rand()%40)-20.0f;
						o->Position[1] += (float)(rand()%40)-20.0f;
						o->Position[2] += (float)(rand()%40)-20.0f;
					}
					break;
				case 46: // 연기 - 위로 올라가는것 (색, 스케일 조정 가능)
					{
						//o->TexType = BITMAP_CLOUD;
      					o->LifeTime = 60 + (int)(o->Scale*10.0f);
						o->Scale += (float)(rand()%50)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%30+50)*0.05f;
					}
					break;
				case 47:
					{
						o->TexType = BITMAP_MAGIC+1;
						o->LifeTime = 5;
						o->Scale += (float)(rand()%50)*0.01f;
						o->Rotation = (float)(rand()%360);
					}
					break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case 59:
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case 48:
      				o->LifeTime = 40;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					break;
				case 49:
					{
						o->TexType = BITMAP_CLOUD;
      					o->LifeTime = rand()%50+100;
	      				o->Rotation = (float)(rand()%360);
     					o->Scale = Scale * 0.4f;
						if (rand()%2 == 0)
							o->Angle[0] = 1.0f;
						else
							o->Angle[0] = -1.0f;
					}
					break;
				case 50:
      				o->LifeTime = 20;
     				o->Scale = (float)(rand()%32+32)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%16+16)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
					break;
				case 51:
      				o->LifeTime = 25;
     				o->Scale = (float)(rand()%64+64)*0.01f;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%32+60)*0.1f;
					VectorCopy(Light, o->TurningForce);
					break;
#ifdef CSK_RAKLION_BOSS
				case 52:
					{
						o->LifeTime = 40;
						o->Scale = Scale + (float)(rand()%10+10)*0.05f;
						o->Rotation = (float)(rand()%360);
						
						vec3_t vAngle, vPos, vPos2;
						Vector ( 0.f, 10.f, 0.f, vPos );
						float fAngle = o->Angle[2] + (float)(rand()%100-50) + 150 ;
						Vector ( 0.f, 0.f, fAngle, vAngle );
						AngleMatrix ( vAngle, Matrix );
						VectorRotate ( vPos, Matrix, vPos2 );
						VectorCopy(vPos2, o->Velocity);	
					}
					break;
#endif // CSK_RAKLION_BOSS
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
				case 54: // 연기 - 위로 올라가는것 (색, 스케일 조정 가능)
					{
						//o->TexType = BITMAP_CLOUD;
      					o->LifeTime = (int)(o->Scale*8.0f);
						o->Scale += (float)(rand()%50)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%30+50)*0.05f;
					}
					break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				case 55:
					{
						o->LifeTime = 30;
						o->Scale = Scale+(float)(rand()%32+48)*0.01f*Scale;
						o->Angle[0] = (float)(rand()%360);
						o->Rotation = (float)((int)WorldTime%360);	
					}
					break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
				case 58:		// 임의색 퍼지는 연기
     				o->LifeTime = 50;
     				o->Scale = (float)(rand()%32+80)*0.01f;
    				o->Position[0] += (float)(rand()%64-32);
    				o->Position[1] += (float)(rand()%64-32);
    				o->Position[2] += (float)(rand()%64+32);
     				o->Angle[0] = (float)(rand()%90-45);
    				o->Angle[2] = (float)(rand()%360);
     				Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
					VectorCopy(o->Light, o->StartPosition);
					break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#if defined PBG_ADD_PKFIELD || defined LDS_ADD_EMPIRE_GUARDIAN
				case 60:
					{
						Vector(0.4f, 0.4f, 0.4f, o->Light);
						o->LifeTime = 60;
						o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%32+60)*0.1f;
					}
					break;
#endif //defined PBG_ADD_PKFIELD || defined LDS_ADD_EMPIRE_GUARDIAN
#ifdef PBG_ADD_PKFIELD
				case 62:
					{
						o->LifeTime = 50;
					//	Vector(0.9f,0.4f,0.1f,o->Light);
     				//	o->Scale = (float)(rand()%32+80)*0.01f;
						o->Scale = o->Scale;
    					o->Position[0] += (float)(rand()%64-32);
    					o->Position[1] += (float)(rand()%64-32);
     					o->Angle[0] = (float)(rand()%90-45);
    					o->Angle[2] = (float)(rand()%360);
     					Vector(0.f,-(float)(rand()%8),0.f,o->Velocity);
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_PKFIELD
				case 63:
					{
						o->TexType = BITMAP_CLUD64;
      					o->LifeTime = 10;
						o->Rotation = (float)(rand()%360);
      					o->Angle[0] = 90.0f;
						Vector(0.0f, 30.0f, 15.0f, o->Velocity);
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT
				case 64:	// 방향성 연기(독가스효과)
					{
						o->LifeTime = 30;
						o->Scale = o->Scale * 0.3f;
						
						o->Velocity[0] = 0.0f;
						o->Velocity[1] = 10.0f;
						o->Velocity[2] = 0.0f;
						
						// 위치점 정의
						const float ANG_REVISION = 20.0f;
						float fAng;
						fAng = (float)((rand() % (int)ANG_REVISION) + 10);
						
						o->Angle[0] = o->Angle[0] + fAng;
					}
					break;
				case 65:	// 아지랭이 같은 연기
					{
						//Vector(0.4f, 0.4f, 0.4f, o->Light);
						o->LifeTime = 45;
						o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%10+18)*0.1f;
					}
					break;
#endif // LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT
#ifdef YDG_ADD_DOPPELGANGER_NPC
				case 66:
					{
						//o->TexType = BITMAP_CLOUD;
      					o->LifeTime = (int)(o->Scale*30.0f);
						o->Scale += (float)(rand()%50)*0.01f;
      					o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%30+50)*0.01f;
						int iAngle = rand()%360;
						o->TurningForce[0] = sinf(iAngle/3.14f*180);
						o->TurningForce[1] = cosf(iAngle/3.14f*180);
					}
					break;
#endif	// YDG_ADD_DOPPELGANGER_NPC
#ifdef LDK_ADD_CS7_UNICORN_PET
				case 67:
					{
						o->LifeTime = 45;
						o->Alpha = 1.0f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%32+60)*0.1f;
					}
					break;
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef LDS_ADD_EFFECT_UNITEDMARKETPLACE
				case 68:
					{
						o->LifeTime = 45;
						o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = (float)(rand()%10+18)*0.1f;
						o->Alpha = 1.0f;

						vec3_t	v3BasisDir, v3Dir;
						float matAngle[3][4];
						memset( matAngle, 0, sizeof(float) * 3 * 4 );
						Vector( 0.0f, 0.0f, 1.0f, v3BasisDir );
						AngleMatrix( o->Angle, matAngle );
						VectorRotate( v3BasisDir, matAngle, v3Dir );

						o->Velocity[0] = v3Dir[0] * 0.6f;
						o->Velocity[1] = v3Dir[1] * 0.6f;
						o->Velocity[2] = v3Dir[2] * 0.6f;
					}
					break;
#endif // LDS_ADD_EFFECT_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
				case 69:	// 색상지정가능 연기박스.
					o->LifeTime = 60;
					o->Scale = o->Scale * (float)(rand()%64+64)*0.005f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%16+30)*0.1f;
					VectorCopy(Light, o->Angle);	// o->Angle을 연기 원래 색상 저장 공간으로 이용.
					break;
#endif	// ASG_ADD_MAP_KARUTAN
				}
				break;
			case BITMAP_SMOKE+1:
            case BITMAP_SMOKE+4:
				o->LifeTime = 32;
				o->Position[0] += (float)(rand()%16-8);
				o->Position[1] += (float)(rand()%16-8);

                if ( o->Type==BITMAP_SMOKE+4 )
                {
                    o->Scale = (float)(rand()%32+32)*0.01f;
                    o->Scale *= Scale;
                }
                else
                {

                    o->Scale = (float)(rand()%32+32)*0.01f*Scale;
                }

				if(SubType==0)
				{
					AngleMatrix(Angle,Matrix);
					Vector(0.f,3.f,0.f,p);
				}
				else
				{
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,15.f,0.f,p);
				}
				VectorRotate(p,Matrix,o->Velocity);
				o->Angle[0] = (float)(rand()%360);

				if(SubType == 1)
				{
					o->LifeTime = 40;
					Vector(0.f, 0.0f, 0.0f,	o->Velocity);
					o->Position[2] += (float)(rand()%16-8);
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if(SubType == 6)
				{
					o->LifeTime = 40;
					Vector(0.f, 0.0f, 0.0f,	o->Velocity);
					o->Position[2] += (float)(rand()%16-8);
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
				break;
			case BITMAP_SMOKE+2:
				o->LifeTime = 50;
				o->Scale = (float)(rand()%32+64)*0.01f;
				Vector(0.f,0.f,(float)(rand()%360),o->Angle);
     			AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64),(float)(rand()%16+16),p);
				VectorRotate(p,Matrix,o->StartPosition);
				Vector(0.f,0.f,0.f,o->Angle);
				//o->Rotation = (float)(rand()%360);
				//Vector(0.f,-(float)(rand()%8+40),0.f,o->Velocity);
				break;
			case BITMAP_SMOKE+3:
				switch(SubType)
				{
				case 0:
					VectorCopy ( o->Light, o->TurningForce );
      				o->LifeTime = 55;
     				//o->Scale = (float)(rand()%32+48)*0.01f;
      				o->Angle[0] = -2 + (float)(rand()%4);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 1:
					VectorCopy ( o->Light, o->TurningForce );
      				o->LifeTime = 30;
     				//o->Scale = (float)(rand()%32+48)*0.01f;
      				o->Angle[0] = -2 + (float)(rand()%4);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 2:
					VectorCopy ( o->Light, o->TurningForce );
      				o->LifeTime = 55;
     				//o->Scale = (float)(rand()%32+48)*0.01f;
      				o->Angle[0] = -2 + (float)(rand()%4);
                    o->Rotation = (float)((int)WorldTime%360);
					break;
				case 3:	// 검은 연기 박스 - 칸투르 몹 사라질때 검은 연기
					o->TexType = BITMAP_CLUD64;
					o->LifeTime = 60;
					o->Scale = o->Scale * (float)(rand()%64+64)*0.02f;
      				o->Rotation = (float)(rand()%360);
					o->Position[0] += (float)(rand()%250-125);
					o->Position[1] += (float)(rand()%250-125);
					o->Position[2] -= (float)(rand()%45);
					o->Gravity = (float)(rand()%100-50)*0.1f;
					Vector(1.0f, 1.0f, 1.0f, o->Light);
					break;
				case 4:	// 검은 연기 박스 - 칸투르 몹 사라질때 검은 연기
					o->TexType = BITMAP_WATERFALL_3;
					o->LifeTime = 30;
					o->Scale = o->Scale * (float)(rand()%64+64)*0.02f;
      				o->Rotation = (float)(rand()%360);
					o->Position[0] += (float)(rand()%250-125);
					o->Position[1] += (float)(rand()%250-125);
					o->Position[2] -= (float)(rand()%45);
					o->Gravity = (float)(rand()%100-50)*0.1f;
					Vector(1.0f, 1.0f, 1.0f, o->Light);
					break;
				}
				break;
			case BITMAP_SMOKELINE1:
			case BITMAP_SMOKELINE2:
			case BITMAP_SMOKELINE3:
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 5
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
#ifdef PBG_ADD_PKFIELD
					if(World == WD_63PK_FIELD)
					{
						o->LifeTime = 25;
					}
					else
#endif //PBG_ADD_PKFIELD
					{
						o->LifeTime = 35;
					}
     				o->Scale = (float)(rand()%96+96)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%16+12)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType == 5)
					{
						VectorCopy(o->Target->Position, o->StartPosition);
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				else if (o->SubType == 1)	// ◎
				{
      				o->LifeTime = 25;
     				o->Scale = (float)(rand()%50+55)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%4+35)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
					VectorCopy(o->Target->Position, o->StartPosition);
				}
#ifdef ADD_RAKLION_IRON_KNIGHT
#ifdef KJH_ADD_EG_MONSTER_KATO_EFFECT
				else if ( (o->SubType == 2) || (o->SubType == 3) )
#else // KJH_ADD_EG_MONSTER_KATO_EFFECT
				else if (o->SubType == 2)
#endif // KJH_ADD_EG_MONSTER_KATO_EFFECT
				{
      				o->LifeTime = 45;
     				o->Scale = (float)(rand()%96+96)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%16+12)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif	// ADD_RAKLION_IRON_KNIGHT
#ifdef LDK_ADD_CS7_UNICORN_PET
				else if( o->SubType == 4 )
				{
					o->Position[0] += (float)(rand()%20-10)*2.f;
					o->Position[1] += (float)(rand()%20-10)*2.f;
					o->Position[2] += (float)(rand()%20-10)*2.f;

					o->Scale = o->Scale + ((float)(rand()%20-10)*(o->Scale*0.03f)); // o->Scale의 30% (20단계)
					o->LifeTime = 45;
					o->Alpha = 1.0f;

					o->Gravity = 0.2f;
					o->Rotation = (float)(rand()%360);
				}
#endif //LDK_ADD_CS7_UNICORN_PET
				break;
			case BITMAP_LIGHTNING_MEGA1:
			case BITMAP_LIGHTNING_MEGA2:
			case BITMAP_LIGHTNING_MEGA3:
				{
					switch( o->SubType )
					{
					case 0:
						{
							o->LifeTime = 5;
							// 스케일은 인자값으로 받는다.
							//o->Scale = (float)(rand()%80+32)*0.01f*Scale;
							o->Rotation = (float)(rand()%360);
							o->Gravity = 0;
							o->Alpha = 1.0f;
							VectorCopy(Light, o->TurningForce);
							Vector(0, 0, 0, o->Light);
						}
						break;
					}	
				}
				break;
			case BITMAP_FIRE_HIK1:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK1_MONO:	
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 6
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
      				o->LifeTime = rand()%5+27;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
				else if (o->SubType == 1)
				{
      				o->LifeTime = rand()%5+54;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+74)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 2)
				{
					o->LifeTime = rand()%5+27;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+100)*0.1f;
					float fRand = (float)(rand()%50) * 0.03f;
					Vector(2.5f+fRand, -7.0f-fRand, 0.f, o->Velocity);
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 3)
				{
      				o->LifeTime = rand()%5+27;
     				o->Scale = (float)(rand()%72+52)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 4)
				{
					o->LifeTime = 8;//rand()%5+5;
					o->Scale = (float)(rand()%42+12)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
					VectorCopy(o->Target->Position, o->StartPosition);
				}
#endif	// YDG_ADD_SKELETON_PET
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==5)
				{
					o->LifeTime = rand()%5+27;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);	
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				if (o->SubType == 10)
				{
					o->LifeTime = rand()%5+47;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				break;

			//////////////////////////////////////////////////////////////////////////

			case BITMAP_FIRE_HIK3:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK3_MONO:	
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 6
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
      				o->LifeTime = rand()%5+17;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
				else if (o->SubType == 1)
				{
      				o->LifeTime = rand()%5+34;
     				o->Scale = (float)(rand()%72+72)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+74)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 2)
				{
					o->LifeTime = rand()%5+17;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+100)*0.1f;
					float fRand = (float)(rand()%50) * 0.03f;
					Vector(2.5f+fRand, -5.0f-fRand, 0.f, o->Velocity);
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);	
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 3)
				{
      				o->LifeTime = rand()%5+17;
     				o->Scale = (float)(rand()%72+52)*0.01f*Scale;
      				o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if (o->SubType == 4)
				{
					o->LifeTime = rand()%5+34;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%10+40)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 5)
				{
					o->LifeTime = 8;//rand()%5+5;
					o->Scale = (float)(rand()%42+12)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%14+44)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
					VectorCopy(o->Target->Position, o->StartPosition);
				}
#endif	// YDG_ADD_SKELETON_PET

				break;

			//////////////////////////////////////////////////////////////////////////

			case BITMAP_LIGHT+1:
				o->LifeTime = 20+rand()%8;
				if(o->SubType == 1)
				{
					o->LifeTime = 5;
					o->Angle[0] = -2 + (float)(rand()%4);
				}
				else if(o->SubType == 2)
				{
					o->LifeTime = 19;
					o->Position[1] += -30.0f + (float)(rand()%60);
				}
				else if(o->SubType == 3)
				{
					o->LifeTime = 7;
					o->TexType = BITMAP_SHINY+1;
				}
				else if(o->SubType == 4)
				{
					o->LifeTime = 4;
					o->Rotation = -360.0f + (float)(rand()%180);
					o->TexType = BITMAP_SHINY+1;
				}
				else if(o->SubType == 5)
				{
					o->LifeTime = 3;
				}
				break;
			case BITMAP_SPARK:
     			o->Scale = (float)(rand()%4+4)*0.1f;
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				if(o->SubType==0 || o->SubType==2 || o->SubType==3 || o->SubType==4 || o->SubType==6 || o->SubType==11 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType==13
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)//터지는거
#else
      			if(o->SubType==0 || o->SubType==2 || o->SubType==3 || o->SubType==4 || o->SubType==6)//터지는거
#endif //LDS_ADD_EMPIRE_GUARDIAN
				{
					o->LifeTime = rand()%16+24;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					o->Gravity = (float)(rand()%16+6);
					Vector(0.f,(float)(rand()%20+20)*0.1f,0.f,p);
					
					if(o->SubType==2)
					{
						o->Scale *= 2.f;
						VectorScale(p,3.f,p);
					}
					else if(o->SubType==4)
					{
						o->Scale *= 3.f;
						VectorScale(p,10.f,p);
					}
					else if(o->SubType==6)
					{
						o->LifeTime = rand()%4+4;
						o->Scale = Scale;
						o->Gravity = 0.0f;
                        o->TexType = BITMAP_SPARK+1;
						Vector(0.f,(float)(rand()%20+20)*0.2f,0.f,p);
					}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
					else if (o->SubType==11)
					{
						Vector(1.0f,0.3f,0.3f,o->Light);
					}
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					else if(o->SubType==13)
					{
						o->Scale *= 1.6f;
						VectorScale(p,1.5f,p);
						o->TexType = BITMAP_SPARK+1;
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

					VectorRotate(p,Matrix,o->Velocity);
				}
				else if(o->SubType == 7)
				{
					o->LifeTime = rand()%12+12;
				}
				else if(o->SubType == 8 
#ifdef PJH_FIX_CAOTIC
					|| o->SubType == 10
#endif //PJH_FIX_CAOTIC
					)
				{
#ifdef PJH_FIX_CAOTIC
					if(o->SubType == 10)
					{
						o->TexType = BITMAP_CLUD64;
					}
					else
#endif //PJH_FIX_CAOTIC
					o->TexType = BITMAP_SPARK+1;
					o->LifeTime = rand()%16+24;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					o->Gravity = (float)(rand()%16+6);
					Vector(0.f,(float)(rand()%60-30)*0.1f,0.f,p);
					o->Scale *= 1.5f;
					VectorScale(p,3.f,p);
					VectorRotate(p,Matrix,o->Velocity);
				}
				else if(o->SubType == 9)
				{
					o->Scale = Scale*1.2f + (float)(rand()%4+4)*0.1f;
					o->LifeTime = rand()%16+50;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					o->Gravity = (float)(rand()%2);
					Vector(0.f,(float)(rand()%10+10)*0.05f,0.f,p);
					VectorRotate(p,Matrix,o->Velocity);
				}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType == 12)
				{
					o->Scale = Scale + (float)(rand()%4+4)*0.1f;
					o->LifeTime = rand()%16+24;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					o->Gravity = (float)(rand()%16+6);
					Vector(0.f,(float)(rand()%20+20)*0.1f,0.f,p);
					VectorRotate(p,Matrix,o->Velocity);
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
      			else//그냥 떨어지는거
				{
					if (o->SubType==5)
					{
						VectorCopy(o->Target->Position, o->StartPosition);
						o->Scale = Scale;
						o->LifeTime = rand()%4+12;
						o->Gravity = 0;
                        o->TexType = BITMAP_SPARK+1;
					}
					else
					{
						o->LifeTime = rand()%8+8;
					}

                    if ( o->SubType==7 )
                    {
                        o->Scale += Scale;
                    }
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case BITMAP_SPARK+1:
				switch(o->SubType)
				{
				case 1:
					Vector((float)(rand()%360),(float)(rand()%360),(float)(rand()%360),o->Angle);
					AngleMatrix(o->Angle,Matrix);
        			Vector(0.f,-50.f,0.f,p);
					VectorRotate(p,Matrix,o->Velocity);
					break;
				case 2:
                    o->LifeTime = 15;
                    Vector(0.f,0.f,0.f,o->Angle);
                    VectorCopy(o->Position,o->StartPosition);
                    o->Velocity[0] = ( o->Target->Position[0] - o->Position[0] ) / 10.f;
                    o->Velocity[1] = ( o->Target->Position[1] - o->Position[1] ) / 10.f;
                    o->Velocity[2] = ( o->Target->Position[2] - o->Position[2] ) / 10.f;
					break;
				case 3:
                    o->LifeTime = 5;
					break;
				case 4:
					o->LifeTime = 10;
					o->Alpha = 0.1f;
					Vector(0.f,0.f,0.f,o->Angle);
					VectorCopy(o->Position,o->StartPosition);
					o->Velocity[0] = ( o->Target->Position[0] - o->Position[0] ) / 6.f;
					o->Velocity[1] = ( o->Target->Position[1] - o->Position[1] ) / 6.f;
					o->Velocity[2] = ( o->Target->Position[2] - o->Position[2] ) / 6.f;
					break;
                case 5:
                    o->LifeTime = 10;
                    Vector ( 0.f, 0.f, Angle[2], o->Angle );
					AngleMatrix ( o->Angle, Matrix );
        			Vector ( -(rand()%2+2.f), 0.f, 0.f, p );
					VectorRotate ( p, Matrix, o->Velocity );
                    break;
				case 6:
					if(rand()%50 == 0)
					{
						o->LifeTime = 50;
						o->Gravity = 1.0f + (float)(rand()%20)/5.f;
						o->Scale = (float)(rand()%5)/10.0f + 1.0f;
					}
					else
						o->LifeTime = 0;
					break;
				case 7:
					o->Alpha = 1.0f;
					o->Position[1] += -30.0f + (float)(rand()%60);
					o->Position[2] += -30.0f + (float)(rand()%60);
					o->Position[3] += -30.0f + (float)(rand()%60);
					o->LifeTime = 30;
					o->Gravity = 1.3f;
					o->Scale = (float)(rand()%5)/10.0f + 1.0f;
					break;
				case 8:
					o->LifeTime = 10;
					break;
				case 9:
					{
						o->LifeTime = 30;
						Vector(0.0f,(float)(rand()%360),0.0f,o->Angle);
						AngleMatrix(o->Angle,Matrix);
       					Vector(0.0f,0.0f,(float)(rand()%10) + 160.0f,p);
						VectorRotate(p,Matrix,o->Velocity);
						VectorAdd(o->Velocity, o->Position, o->Position);
						Vector(12.0f, 0.0f, -2.0f, o->Velocity);
					}
					break;
				case 10:
					o->Alpha = 1.0f;
					o->Velocity[0] = -4.0f + (float)(rand()%8);
					o->Velocity[1] = 0.0f;
					o->Velocity[2] = -4.0f + (float)(rand()%8);
					o->LifeTime = 20;
					o->Scale = (float)(rand()%5)/10.0f + 1.0f;
					break;
				case 13:
				case 11:
					o->Alpha = 1.0f;
					o->Velocity[0] = -4.0f + (float)(rand()%8);
					o->Velocity[1] = (float)(rand()%4);
					o->Velocity[2] = (float)(rand()%4);
					o->LifeTime = 30;
					o->Scale = (float)(rand()%5)/10.0f + 1.0f;
					break;
				case 12:
					o->TexType = BITMAP_SHINY;
					o->Alpha = 1.0f;

					vec3_t p1, p2;
                    Vector(0.f,-100.f,0.f,p1);
                    AngleMatrix(o->Angle,Matrix);
				    VectorRotate(p1,Matrix,p2);
				    VectorAdd(o->Position,p2,o->Position);

					o->Position[0] += -8.0f + (float)(rand()%16);
					o->Position[1] += -8.0f + (float)(rand()%16);
					o->Position[2] += 150.0f;

					o->Velocity[0] = -1.0f + (float)(rand()%20)/10.f;
					o->Velocity[1] = -1.0f + (float)(rand()%20)/10.f;
					o->Velocity[2] = -(float)(rand()%3) - 3.0f;
					o->LifeTime = 40;
					o->Scale = -(float)(rand()%15)/10.0f + Scale;
					break;
				case 14:
					if(rand()%2 == 0)
					{
						o->Alpha = 1.0f;
						o->Velocity[0] = -4.0f + (float)(rand()%4);
						o->Velocity[1] = (float)(rand()%4);
						o->Velocity[2] = (float)(rand()%4);
						o->LifeTime = 30;
						o->Scale = (float)(rand()%5)/10.0f + 0.4f;
					}
					break;
				case 15:	//^ 펜릴 이펙트 관련
					if(rand()%3 == 0)
					{
						o->Alpha = 1.0f;
						o->LifeTime = 35;
						o->Velocity[2] = -((float)(rand()%5)+5)*0.5f;
					}
					break;
				case 16:
				case 18:
					{
						o->Alpha = 1.0f;
						o->LifeTime = 100;
						if(o->SubType == 18)
						{
							o->LifeTime = 80;
						}
						o->Frame = 77;
						o->Scale = -(float)(rand()%3)/3.0f + Scale+0.01f;
						
						o->Velocity[0] = (float)(rand()%50)/10.0f - 2.5f;
						o->Velocity[1] = (float)(rand()%4)/10.0f - 0.2f;
						if(o->SubType == 18)
						{
							o->Velocity[1] = (float)(rand()%50)/10.0f - 2.5f;
						}
						o->Velocity[2] = (float)(rand()%50)/10.0f - 2.5f;

						vec3_t p1, p2;
						Vector(o->Velocity[0]*10.0f,o->Velocity[1]*10.0f,o->Velocity[2]*10.0f+100.0f,p1);
						AngleMatrix(o->Angle,Matrix);
						VectorRotate(p1,Matrix,p2);
						VectorAdd(o->Position,p2,o->Position);

						o->Gravity = 3.5f;
					}
					break;
				case 17:
					{
						o->LifeTime = 200;
						o->Scale += (float)(rand()%3)*0.1f;
						o->Gravity = (float)(rand()%5)*0.3f + 2.0f;
						o->Rotation =  (float)(rand()%20)*0.0001f + 0.002f;
						o->Alpha =  (float)(rand()%10)*0.1f + 0.5f;
						VectorCopy(o->Position, o->StartPosition);
					}
					break;
#ifdef CSK_LUCKY_SEAL
				case 19:
					{
						o->LifeTime = 50;
						o->Gravity = 1.0f + (float)(rand()%20) * 0.1;
						o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) - 5.f;
						o->Scale = (float)(rand()%10) * 0.08f + 0.8f;
						VectorCopy(Light, o->Light);
					}
					break;
#endif // CSK_LUCKY_SEAL
				case 20:
					{
						o->Scale = Scale + (rand()%10)*0.02f;
						o->LifeTime = rand()%10+60;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						o->Gravity = (float)(rand()%10+15);
						Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
						VectorScale(p,2.5f,p);
						VectorRotate(p, Matrix, o->Velocity);
					}
					break;

#ifdef PRUARIN_EVENT07_3COLORHARVEST
				case 21:					// 흩어지는 파티클(13번 참고)
					{	
						vec3_t vSpeed;
						vSpeed[0] = -8.0f + (float)(rand()%16);
						vSpeed[1] = -8.0f + (float)(rand()%16);
						vSpeed[2] = -8.0f + (float)(rand()%16);
						
						VectorNormalize( vSpeed );
						VectorScale( vSpeed, 12.0f, vSpeed );			
						VectorCopy( vSpeed, o->Velocity );
						
						o->LifeTime = rand()%10+20;			// (20~30)
						o->Scale = (float)(rand()%20)/20.0f+1.0f;	//(1~2 20단계)
					}	
					break;
				case 22:					// 떨어지는 파티클(20번 참고)
					{	
						vec3_t vSpeed;
						vSpeed[0] = -8.0f + (float)(rand()%16);
						vSpeed[1] = -8.0f + (float)(rand()%16);
						vSpeed[2] = -8.0f + (float)(rand()%16);

						VectorNormalize( vSpeed );
						VectorScale( vSpeed, 14.0f, vSpeed );				
						VectorCopy( vSpeed, o->Velocity );

						o->Scale = (float)(rand()%20)/20.0f+1.0f;	//(1~2 20단계)
						o->LifeTime = rand()%10+20;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle, Matrix);
						o->Gravity = 20.0f;//(float)(rand()%10+15);
						Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
						VectorScale(p,2.5f,p);
						VectorRotate(p, Matrix, o->Velocity);
					}
					break;
#endif // PRUARIN_EVENT07_3COLORHARVEST
				case 23:
					o->Alpha = 1.0f;
					o->Velocity[0] = (float)(-1+rand()%3)*1.5f;
					o->Velocity[1] = (float)(-1+rand()%3)*1.5f;
					o->Velocity[2] = (float)(-1+rand()%3)*1.5f;
					o->LifeTime = 20;
					o->Rotation = 0;
					o->Scale = (float)(rand()%5)/10.0f + 0.8f;
					break;
				case 24:
					o->Alpha = 1.0f;
					o->Velocity[0] = (float)(-1+rand()%3)*2.0f;
					o->Velocity[1] = (float)(-1+rand()%3)*2.0f;
					o->Velocity[2] = (float)(-1+rand()%3)*2.0f;
					o->LifeTime = 16;
					o->Rotation = 0;
					o->Scale = (float)(rand()%5)/10.0f + 0.4f;
					break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
				case 25:
					o->Alpha = 1.0f;
					o->Velocity[0] = -2.0f + (float)(rand()%6);
					o->Velocity[1] = (float)(rand()%4);
					o->Velocity[2] = (float)(rand()%4);
					o->LifeTime = 20;
					break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef CSK_ADD_MAP_ICECITY	
				case 26:
					o->LifeTime = 20;
                    Vector(0.f,0.f,0.f,o->Angle);
                    VectorCopy(o->Position,o->StartPosition);
                    o->Velocity[0] = ( o->Target->Position[0] - o->Position[0] ) / 20.f;
                    o->Velocity[1] = ( o->Target->Position[1] - o->Position[1] ) / 20.f;
                    o->Velocity[2] = ( o->Target->Position[2] - o->Position[2] ) / 20.f;
					//Vector(1.f, 0.f, 0.f, o->Light);
					break;
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_FIRECRACKER_ITEM
				case 27:					// 흩어지는 파티클(13번 참고)
					{	
						vec3_t vSpeed;
						vSpeed[0] = -8.0f + (float)(rand()%16);
						vSpeed[1] = -8.0f + (float)(rand()%16);
						vSpeed[2] = -8.0f + (float)(rand()%16);
						
						VectorNormalize( vSpeed );
						VectorScale( vSpeed, 12.0f, vSpeed );			
						VectorCopy( vSpeed, o->Velocity );
						
						o->LifeTime = rand()%10+10;
						o->Scale = (float)(rand()%20)/20.0f+1.0f;
					}	
					break;
				case 28:					// 떨어지는 파티클(20번 참고)
					{	
// 						vec3_t vSpeed;
// 						vSpeed[0] = -8.0f + (float)(rand()%16);
// 						vSpeed[1] = -8.0f + (float)(rand()%16);
// 						vSpeed[2] = -8.0f + (float)(rand()%16);
// 
// 						VectorNormalize( vSpeed );
// 						VectorScale( vSpeed, 14.0f, vSpeed );				
// 						VectorCopy( vSpeed, o->Velocity );

						o->Scale = (float)(rand()%20)/20.0f+1.0f;
						o->LifeTime = rand()%10+20;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle, Matrix);
						o->Gravity = 20.0f;
						Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
						VectorScale(p,2.5f,p);
						VectorRotate(p, Matrix, o->Velocity);
					}
					break;
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef PBG_ADD_PKFIELD
				case 29:
					{
						o->Scale = (float)(rand()%20)/20.0f+0.5f;
						o->LifeTime = rand()%10+10;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle, Matrix);
						o->Gravity = -20.0f;
						Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
						VectorScale(p,2.5f,p);
						VectorRotate(p, Matrix, o->Velocity);
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				case 30:
					{
						o->LifeTime = rand()%5+50;

						o->Scale = (float)(rand()%10)/10.0f+0.2f;

						o->Velocity[0] = -2.0f + (float)(rand()%5);
						o->Velocity[1] = -2.0f + (float)(rand()%5);
						o->Velocity[2] = -2.0f + (float)(rand()%5);

						o->Gravity = 0.5f;
					}
					break;
#endif //LDK_ADD_GAMBLERS_WEAPONS
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
				case 31:
					{
	     				o->Scale = (float)(rand()%4+4)*0.1f;
						o->LifeTime = rand()%16+24;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						o->Gravity = (float)(rand()%16+6);
						Vector(0.f,(float)(rand()%20+20)*0.1f,0.f,p);
						
						VectorScale(p,3.f,p);

						VectorRotate(p,Matrix,o->Velocity);
					}					
					break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
                }
				break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			case BITMAP_SPARK+2:
				if(o->SubType == 0)
				{
					o->LifeTime = 16;
					o->Scale = Scale + (float)(rand()%10 - 5)*0.03f;
					o->Position[0] += (float)(rand()%10 - 5);
					o->Position[1] += (float)(rand()%10 - 5);
					o->Position[2] += (float)(rand()%10 - 5);
				}
				else if(o->SubType == 1)
				{
					o->LifeTime = 12;
				}
#ifdef ASG_ADD_SKILL_BERSERKER
				else if (o->SubType == 2 || o->SubType == 3)
				{
					o->Scale = 0.4f;
					o->LifeTime = 16;
				}
#endif	// ASG_ADD_SKILL_BERSERKER
				break;

			case BITMAP_EXPLOTION_MONO:
			case BITMAP_EXPLOTION:
				o->LifeTime = 20;
				o->Scale = Scale;
				if ( o->SubType == 2)
				{
					o->LifeTime = 30;
				}

				//if ( o->SubType != 1)
				{
#ifdef PRUARIN_EVENT07_3COLORHARVEST
					if( o->SubType == 10 )
					{
						PlayBuffer( SOUND_MOONRABBIT_EXPLOSION );
					}
					else
					{
						if (SceneFlag != LOG_IN_SCENE)
							PlayBuffer(SOUND_EXPLOTION01);
					}
#else // PRUARIN_EVENT07_3COLORHARVEST
					PlayBuffer(SOUND_EXPLOTION01);
#endif // PRUARIN_EVENT07_3COLORHARVEST
				}
				/*else
				{
					PlayBuffer(SOUND_METEORITE01);
				}*/
 				break;
			case BITMAP_SUMMON_SAHAMUTT_EXPLOSION:
				o->LifeTime = 16;
				o->Scale = Scale;
				break;
			case BITMAP_SPOT_WATER:
				o->LifeTime = 32;
				o->Scale = (float)((rand()%170+1)*0.01f);
				break;
			case BITMAP_FLARE_RED:
				o->LifeTime = 18;
				HandPosition(o);
				break;

            case BITMAP_EXPLOTION+1:
				o->LifeTime = 12;
				o->Scale = Scale;
            	PlayBuffer(SOUND_EXPLOTION01);
                break;

			case BITMAP_SHINY:
				o->LifeTime = 18;
				o->Angle[0] = 45.f;
                if ( o->SubType==2 )
                {
   				    o->LifeTime = 25;
                    o->Rotation = (float)(rand()%360);

                    o->Velocity[0] = -0.1f;
                    o->Velocity[1] = -0.5f;
                    o->Velocity[2] = -1.f;
                }
				else if(o->SubType == 3)
				{
					o->Alpha = 1.0f;
					o->Velocity[0] = -4.0f + (float)(rand()%8);
					o->Velocity[1] = (float)(rand()%4);
					o->Velocity[2] = (float)(rand()%4);
					o->LifeTime = 30;
					o->Scale = (float)(rand()%10)/20.0f + 0.5f;
				}
				else if(o->SubType == 4)
				{
					o->Scale = Scale + (rand()%10)*0.02f;
					o->LifeTime = rand()%10+15;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					o->Gravity = (float)(rand()%10+10);
					Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
					VectorScale(p,2.0f,p);
					VectorRotate(p, Matrix, o->Velocity);
				}
				else if(o->SubType == 5)
				{
					o->LifeTime = rand()%10+10;
					o->Scale = Scale + (rand()%10)*0.02f;
					o->Position[0] += (float)(rand()%10 - 5);
					o->Position[1] += (float)(rand()%10 - 5);
					o->Gravity = -(float)(rand()%10)*0.3f;
				}
#ifdef YDG_ADD_FIRECRACKER_ITEM
				else if(o->SubType == 6)	// 오색별가루
				{
					o->Scale = (float)(rand()%5)/10.0f+0.5f;
					o->StartPosition[0] = o->Scale;
					o->LifeTime = rand()%10+60;
					o->Gravity = 20.0f;

					o->Angle[0] = (float)(rand()%360);
					o->Angle[1] = (float)(rand()%360);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					Vector(1,1,1,p);
					VectorScale(p,8.0f,p);
					VectorRotate(p, Matrix, o->Velocity);

					o->Rotation = rand()%20-10;
				}
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef LDK_ADD_RUDOLPH_PET
				else if(o->SubType == 7)
				{
					o->Scale = 0.1f;//(float)(rand()%5)/6.0f+0.5f;
					o->StartPosition[0] = o->Scale;
					o->LifeTime = rand()%10+20;
					o->Position[0] += (float)(rand()%50 - 25);
					o->Position[1] += (float)(rand()%50 - 25);
					o->Position[2] += (float)(rand()%50 - 25);

					o->Gravity = 1.0f;

					o->Angle[0] = (float)(rand()%360);
					o->Angle[1] = (float)(rand()%360);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					Vector(0.5f,0.5f,0.5f,p);
					VectorScale(p,0.8f,p);
					VectorRotate(p, Matrix, o->Velocity);

					o->Rotation = rand()%20-10;
				}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef YDG_ADD_SANTA_MONSTER
				else if(o->SubType == 8)
				{
					o->Scale = (float)(rand()%5)/10.0f+0.5f;
					o->StartPosition[0] = o->Scale;
					o->LifeTime = rand()%10+40;
					o->Gravity = 20.0f;
					Vector(0, 0, 1.0f, o->Velocity);

					o->Rotation = rand()%20-10;
				}
#endif	// YDG_ADD_SANTA_MONSTER
#ifdef YDG_ADD_FIRECRACKER_ITEM
				else if(o->SubType == 9)	// 오색 별가루 + 노란 별가루 왕창
				{
					o->Scale = (float)(rand()%5)/10.0f+0.5f;
					o->StartPosition[0] = o->Scale;
					o->LifeTime = rand()%10+60;
					o->Gravity = 20.0f;

					o->Angle[0] = (float)(rand()%360);
					o->Angle[1] = (float)(rand()%360);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					Vector(1,1,1,p);
					VectorScale(p,8.0f,p);
					VectorRotate(p, Matrix, o->Velocity);

					o->Rotation = rand()%20-10;
				}
#endif	// YDG_ADD_FIRECRACKER_ITEM
 				break;

#ifdef CSK_EVENT_CHERRYBLOSSOM
			case BITMAP_CHERRYBLOSSOM_EVENT_PETAL:
				{
					if( o->SubType == 0 )
					{
						o->Alpha = 1.0f;
						o->LifeTime = 30;
						o->Scale = (float)(rand()%10)/20.0f + 0.5f;
						o->Rotation = (float)(rand()%360);
						o->Gravity = -(float)(rand()%10+10);
						Vector( 0.0f, 0.0f, 0.0f, o->Angle );
						o->Velocity[2] = -4.0f + (float)(rand()%8);
						o->Velocity[1] = (float)(rand()%4);
						o->Velocity[0] = (float)(rand()%4);
					}
					else if( o->SubType == 1 )
					{
						o->Alpha = 1.0f;
						o->LifeTime = 70;
						o->Alpha = 1.0f;

						o->Velocity[0] = -2.0f + (float)(rand()%6);
						o->Velocity[1] = (float)(rand()%4);
						o->Velocity[2] = (float)(rand()%4);

						//o->Velocity[0] = -2.0f + (float)(rand()%2);
						//o->Velocity[1] = (float)(rand()%2);
						//o->Velocity[2] = (float)(rand()%2);
						o->Rotation = (float)(rand()%360);
					}					
				}
				break;
				
			case BITMAP_CHERRYBLOSSOM_EVENT_FLOWER:
				{
					if( o->SubType == 0 )
					{
						vec3_t vSpeed;
						vSpeed[0] = -8.0f + (float)(rand()%16);
						vSpeed[1] = -8.0f + (float)(rand()%16);
						vSpeed[2] = -8.0f + (float)(rand()%16);

						VectorNormalize( vSpeed );
						VectorScale( vSpeed, 14.0f, vSpeed );				
						VectorCopy( vSpeed, o->Velocity );
						
						o->Alpha = 1.0f;
						//o->Scale = (float)(rand()%20)/20.0f+1.0f;	//(1~2 20단계)
						o->LifeTime = rand()%30+20;
						o->Angle[2] = (float)(rand()%360);
						o->Rotation = (float)(rand()%360);
						AngleMatrix(o->Angle, Matrix);
						o->Gravity = 20.0f;//(float)(rand()%10+15);
						Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
						VectorScale(p,2.5f,p);
						VectorRotate(p, Matrix, o->Velocity);
					}
				}
				break;
#endif //CSK_EVENT_CHERRYBLOSSOM				
			case BITMAP_SHINY+1:
#ifdef CSK_EVENT_CHERRYBLOSSOM
				if ( o->SubType==5 )
				{
					o->LifeTime = 14;
					//Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
					//o->Scale = (float)(rand()%70+40)*0.009f;
					o->Rotation = (float)(rand()%360);
					o->Gravity = 0.0f;
					Vector(0.f,0.f,0.f,o->Velocity);
                    //Vector ( 0.5f, 0.5f, 0.5f, o->Light );
				}
				else
#endif //CSK_EVENT_CHERRYBLOSSOM
				{
					o->LifeTime = 36;
					o->Angle[0] = 45.f;
					if(o->SubType!=99)
					{
						
						HandPosition(o);
					}
				}
 				break;
			case BITMAP_SHINY+2:
				o->LifeTime = 18;
				HandPosition(o);
 				break;
			case BITMAP_SHINY+4:
				o->LifeTime = 20;
                if ( o->SubType==1 )
                {
    				o->LifeTime = 15;
                    o->Gravity  = 0.f;
                    o->Rotation = (float)(rand()%360);
                }
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==2)
				{
					o->LifeTime = 8;
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
 				break;
			case BITMAP_SHINY+6:
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				if(o->SubType == 0)
#endif //LDK_ADD_GAMBLERS_WEAPONS
				{
					o->Alpha = 1.0f;
					o->Position[1] += -5.0f + (float)(rand()%10);
					o->Position[2] += -5.0f + (float)(rand()%10);
					o->Position[3] += -5.0f + (float)(rand()%10);
					o->LifeTime = 30;
					o->Gravity = 1.3f;
					o->Scale += (float)(rand()%3)/10.0f;
				}
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				else if(o->SubType == 1)
				{
					o->Alpha = 1.0f;
					o->LifeTime = rand()%3+10;
					o->Scale = 0.3f;

					o->Position[0] += (float)(rand()%50 - 25);
					o->Position[1] += (float)(rand()%50 - 25);
					o->Position[2] += (float)(rand()%50 - 25);
					o->Rotation = rand()%20-10;
				}
#endif //LDK_ADD_GAMBLERS_WEAPONS
				break;
			case BITMAP_PIN_LIGHT:
				o->Alpha = 1.0f;

				{
					o->Position[1] += -5.0f + (float)(rand()%10);
					o->Position[2] += -5.0f + (float)(rand()%10);
					o->Position[3] += -5.0f + (float)(rand()%10);
				}
				o->LifeTime = 30;
				o->Gravity = 10.0f;
				o->Scale += (float)(rand()%5)/10.0f;
#ifdef YDG_ADD_DOPPELGANGER_NPC
				if(o->SubType == 2)
				{
					o->Gravity = -5.0f;
				}
				else if (o->SubType == 3)
				{
					o->Gravity = -5.0f;
					o->TexType = BITMAP_SHINY+6;
					o->Rotation = rand()%360;
				}
#endif	// YDG_ADD_DOPPELGANGER_NPC
				break;
#ifdef ASG_ADD_SKILL_BERSERKER
			case BITMAP_ORORA:	// 플레이어 손목 주변을 돌며 커지면서 사라지는 이펙트. 색상만 변경하여 생성 가능.
				o->Scale = 0.2f;
				switch (o->SubType)
				{
				case 0:
				case 1:	o->LifeTime = 100;	break;
				case 2:
				case 3:	o->LifeTime = 25;	break;
				}
				break;
#endif	// ASG_ADD_SKILL_BERSERKER
			case BITMAP_SNOW_EFFECT_1:
			case BITMAP_SNOW_EFFECT_2:
				o->Scale = Scale + (rand()%10)*0.02f;
				o->LifeTime = rand()%10+30;
				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				o->Gravity = (float)(rand()%10+20);
				Vector((float)(rand()%40-20)*0.1f,(float)(rand()%40-20)*0.1f,0.f,p);
				VectorScale(p,1.5f,p);
				VectorRotate(p, Matrix, o->Velocity);	
				break;
				
			case BITMAP_DS_EFFECT:
				o->Scale = Scale + (rand()%10)*0.02f;
				o->LifeTime = 100;
				break;
			case BITMAP_BLOOD:
			case BITMAP_BLOOD+1:
				o->LifeTime = 12;
     			o->Scale = (float)(rand()%4+8)*0.05f;
				o->Angle[0] = (float)(rand()%360);
				//o->Gravity = (float)(rand()%16+32)*0.1f;
				AngleMatrix(Angle,Matrix);
				Vector(0.f,-(float)(rand()%16+8),(float)(rand()%6-3),p);
				VectorRotate(p,Matrix,o->Velocity);
#ifndef FOR_ADULT
				if(o->Type == BITMAP_BLOOD+1)
				{
					Vector(0.1f,0.f,0.f,o->Light);
				}
#endif
				break;
			case BITMAP_FIRECRACKER:
				o->Angle[0] = o->Angle[1] = o->Angle[2] = 0.0f;
				o->Rotation = (float)(rand()%360);
				if ( SubType == -1)
				{
					o->LifeTime = 2;
        			o->Scale = 0.25f;
				}
				else
				{
					int iSize = 15+SubType;
					o->Velocity[0] = (float)(rand()%iSize-iSize/2)*0.15f;
					o->Velocity[1] = (float)(rand()%iSize-iSize/2)*0.15f;
					o->Velocity[2] = (float)(rand()%iSize-iSize/2)*0.15f+10.0f;
					o->LifeTime = 50;
        			o->Scale = 0.10f;
				}
				break;
            case BITMAP_SWORD_FORCE:
                o->LifeTime = 10;
                o->Gravity  = 0.1f;
                o->Rotation = o->Angle[2]+135.f;
                break;
            case BITMAP_CLOUD:
#ifndef PJH_NEW_SERVER_SELECT_MAP
				if (World == WD_77NEW_LOGIN_SCENE && o->SubType == 3)
				{
     			    o->LifeTime = 30;
				    o->Gravity = (float)(rand()%1000);

    			    o->Position[0] += (float)(rand()%500-250);//*Scale;
    			    o->Position[1] += (float)(rand()%500-250);//*Scale;
    			    o->Position[2] += (float)(rand()%20+20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (float)(rand()%40+280)*0.01f;
                    o->TurningForce[0] = (Scale+rand()%30/100.f) * 0.1f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else
#endif//PJH_NEW_SERVER_SELECT_MAP
                if ( o->SubType==6 )
                {
                    o->LifeTime = 25+rand()%5;
                    o->Rotation = (float)(rand()%360);
                    o->Scale = 0.2f;
                    o->Velocity[0] = (float)(rand()%10+5)*0.4f;
                    o->Velocity[1] = 0.f;
                    o->Velocity[2] = (float)(rand()%10+5)*0.2f;
                }
				else if( o->SubType==8)
				{
					o->LifeTime = 300;
				    o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.1f;

    			    o->Position[0] += (float)(rand()%500-250);
    			    o->Position[1] += (float)(rand()%500-250);
    			    o->Position[2] += (float)(rand()%20-20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (float)(rand()%20+180)*0.01f;
                    o->TurningForce[0] = Scale+rand()%30/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if( o->SubType==9)
				{
					o->LifeTime = 300;
				    o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.1f;

    			    o->Position[0] += (float)(rand()%500-250);
    			    o->Position[1] += (float)(rand()%500-250);
    			    o->Position[2] += (float)(rand()%20-20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (((float)(rand()%20+180)*0.01f) * o->Scale);
                    o->TurningForce[0] = rand()%10/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if(o->SubType == 10)
				{
					Vector(0.0f, 0.0f, 0.0f, o->Light);
					o->LifeTime = 500;
	   			    o->Position[0] += (float)(rand()%400-200);
    			    o->Position[1] += (float)(rand()%400-200);
    			    o->Position[2] += (float)(rand()%400-200);
					VectorCopy(o->Position, o->StartPosition);
     			    o->Scale = (float)(rand()%15+10)/15.0f+Scale;
					float fTemp = (float)(rand()%10+5)*0.12f;
                    o->Velocity[0] = fTemp;
                    o->Velocity[1] = fTemp;
                    o->Velocity[2] = 0.0f;
				}
				else if(o->SubType == 11)
				{
					o->LifeTime = 500;
	   			    o->Position[0] += (float)(rand()%400-200);
    			    o->Position[1] += (float)(rand()%400-200);
    			    o->Position[2] += (float)(rand()%400-200);
					VectorCopy(o->Position, o->StartPosition);
     			    o->Scale = (float)(rand()%15+15)/30.0f+Scale;
					float fTemp = (float)(rand()%10+5)*0.12f;
                    o->Velocity[0] = fTemp;
                    o->Velocity[1] = fTemp;
                    o->Velocity[2] = 0.0f;
				}
				else if(o->SubType == 12)
				{
     			    o->LifeTime = 30;
				    o->Gravity = (float)(rand()%1000);

    			    o->Position[0] += (float)(rand()%500-250);//*Scale;
    			    o->Position[1] += (float)(rand()%500-250);//*Scale;
    			    o->Position[2] += (float)(rand()%20+20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (float)(rand()%20+180)*0.01f;
                    o->TurningForce[0] = Scale+rand()%30/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if(o->SubType == 13)
				{
					o->LifeTime = 30;
					o->Rotation = rand()%360;
					o->Gravity = (float)(rand()%50-25)/10.0f;
				}
				else if(o->SubType == 14)
				{
					Vector(0.f, 0.f, 0.f, Light);
					o->LifeTime = 25+rand()%5;
                    o->Rotation = float(rand()%360);
                    o->Scale = Scale*0.2f;
                    o->Velocity[0] = (float)(rand()%10+5)*0.4f;
                    o->Velocity[1] = 0.f;
                    o->Velocity[2] = (float)(rand()%10+5)*0.2f;		
				}
				else if(o->SubType == 15)
				{
					o->LifeTime = 500;

	   			    o->Position[0] += (float)(rand()%400-200);
    			    o->Position[1] += (float)(rand()%400-200);
    			    o->Position[2] += (float)(rand()%400-200);
					VectorCopy(o->Position, o->StartPosition);

					o->Rotation = rand()%360;

					o->Scale = (float)(rand()%15+15)/30.0f+Scale;

					float fTemp = (float)(rand()%10+5)*0.08f;
                    o->Velocity[0] = fTemp;
                    o->Velocity[1] = fTemp;
                    o->Velocity[2] = 0.0f;

					VectorCopy(o->Light, o->TurningForce);
					o->Light[0] = 0.f;
					o->Light[1] = 0.f;
					o->Light[2] = 0.f;
				}
				else if(o->SubType == 16)
				{
					Vector(0.0f, 0.0f, 0.0f, o->Light);

					o->LifeTime = 500;

	   			    o->Position[0] += (float)(rand()%400-200);
    			    o->Position[1] += (float)(rand()%400-200);
    			    o->Position[2] += (float)(rand()%400-200);
					VectorCopy(o->Position, o->StartPosition);

					o->Rotation = rand()%360;

     			    o->Scale = (float)(rand()%15+10)/15.0f+Scale;

					float fTemp = (float)(rand()%10+5)*0.05f;
                    o->Velocity[0] = fTemp;
                    o->Velocity[1] = fTemp;
                    o->Velocity[2] = 0.0f;
				}
				else if(o->SubType == 17)
				{
					o->TexType = BITMAP_EVENT_CLOUD;
					o->LifeTime = 500;

	   			    o->Position[0] += (float)(rand()%900-450);
    			    o->Position[1] += (float)(rand()%900-450);
    			    o->Position[2] += (float)(rand()%20+50);
					VectorCopy(o->Position, o->StartPosition);

					o->Rotation = rand()%360;
     			    o->Scale = (float)(rand()%20+20)/80.0f+Scale/3.f;

					float fTemp = (float)(rand()%10+5)*0.006f;
                    o->Velocity[0] = fTemp;
                    o->Velocity[1] = fTemp;
                    o->Velocity[2] = 0.0f;

					VectorCopy(o->Light, o->TurningForce);
					o->Light[0] = 0.f;
					o->Light[1] = 0.f;
					o->Light[2] = 0.f;
				}			
				else if(o->SubType == 18)
				{
					o->TexType = BITMAP_CHROME+2;
					
     			    o->LifeTime = 160;
					o->Light[0] = 0.f;
					o->Light[1] = 0.f;
					o->Light[2] = 0.f;
				    o->Gravity = (float)(rand()%1000);

					VectorCopy(Position, o->Position);
					o->Position[2] += (float)(rand()%80);
					o->Rotation = rand()%360;

                     o->StartPosition[0] = (rand()%200-10)/10.0f;
                     o->StartPosition[1] = (rand()%200-10)/10.0f;

     			    o->Scale = (float)(rand()%70+5)*0.02f;
                    o->TurningForce[0] = float(rand()%40+10)/10000.f;
                    o->TurningForce[1] = rand()%120+80;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if(o->SubType == 19)
				{
					o->TexType = BITMAP_CHROME+2;
					
     			    o->LifeTime = 60;
					o->Light[0] = 0.f;
					o->Light[1] = 0.f;
					o->Light[2] = 0.f;
				    o->Gravity = (float)(rand()%1000);

    			    o->Position[0] += (float)(rand()%500-250);//*Scale;
    			    o->Position[1] += (float)(rand()%500-250);//*Scale;
    			    o->Position[2] += (float)(rand()%20+20);
					o->Rotation = rand()%360;

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (float)(rand()%90+220)*0.02f;
                    o->TurningForce[0] = Scale+rand()%20/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if( o->SubType==20)
				{
					o->LifeTime = 300;
				    o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.1f;

    			    o->Position[0] += (float)(rand()%500-250);
    			    o->Position[1] += (float)(rand()%500-250);
    			    o->Position[2] += (float)(rand()%20-20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (((float)(rand()%20+180)*0.01f) * o->Scale);
                    o->TurningForce[0] = rand()%10/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if( o->SubType==21)
				{
					o->LifeTime = 100;
					o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.6f;
					
					o->Position[0] += (float)(rand()%200-100);
					o->Position[1] += (float)(rand()%200-100);
					o->Position[2] += (float)(rand()%20-20);
					
                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];
					
					o->Scale = (((float)(rand()%20+180)*0.01f) * o->Scale);
                    o->TurningForce[0] = rand()%10/100.f;
					Vector(0.f,0.f,0.f,o->Velocity);
				}
				else if( o->SubType==22)
				{
					o->LifeTime = 80;
					o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.1f;
					
					o->Position[0] += (float)(rand()%20-10);
					o->Position[1] += (float)(rand()%20-10);
					o->Position[2] += (float)(rand()%20-20);
					
					o->Scale = (((float)(rand()%20+50)*0.003f) * o->Scale);
					Vector(0.f,0.f,0.f,o->TurningForce);
					
					float fAngle = o->Angle[2] + rand()%360;
					vec3_t vAngle, vDirection;
					Vector(0.f, 0.f, fAngle, vAngle);
					AngleMatrix(vAngle, Matrix);
					Vector(0.f, (rand()%10)*0.1f+0.2f, 0.f, vDirection);
					VectorRotate(vDirection, Matrix, o->Velocity);
					o->Velocity[2] = 0;
				}
				else if( o->SubType==23)
				{
					o->LifeTime = 80;
					o->Gravity = (float)(rand()%1000);
					o->Alpha = 0.1f;
					
					o->Position[0] += (float)(rand()%6-3);
					o->Position[1] += (float)(rand()%6-3);
					o->Position[2] += (float)(rand()%6-3);
					
					o->Scale = 0;//(((float)(rand()%20+20)*0.001f) * o->Scale);
					Vector(0.f,0.f,0.f,o->TurningForce);
					o->TurningForce[0] = (rand()%2 == 0 ? 1.0f : -1.0f);
					
					float fAngle = o->Angle[2] + 90 + (rand()%40) - 20;
					vec3_t vAngle, vDirection;
					Vector(0.f, 0.f, fAngle, vAngle);
					AngleMatrix(vAngle, Matrix);
					Vector(0.f, (rand()%10)*0.2f+1.0f, 0.f, vDirection);
					VectorRotate(vDirection, Matrix, o->Velocity);
					o->Velocity[2] = 0;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
                else
                {
     			    o->LifeTime = 30;
				    o->Gravity = (float)(rand()%1000);

    			    o->Position[0] += (float)(rand()%500-250);//*Scale;
    			    o->Position[1] += (float)(rand()%500-250);//*Scale;
    			    o->Position[2] += (float)(rand()%20+20);

                    o->StartPosition[1] = (rand()%100)/100.f;
                    o->StartPosition[2] = o->Position[2];

     			    o->Scale = (float)(rand()%20+180)*0.01f;
                    o->TurningForce[0] = Scale+rand()%30/100.f;
         		    Vector(0.f,0.f,0.f,o->Velocity);
                }
                break;
			case BITMAP_TORCH_FIRE:
				{
					o->LifeTime = rand()%5 + 60;
					o->Scale = (float)(rand()%30+30) * 0.02f + Scale / 3.f;

					o->Position[0] += (float)(rand()%4-2) * 0.25f;
    				o->Position[1] += (float)(rand()%4-2) * 0.25f;
					o->Position[2] += (float)(rand()%10-5);

					o->Gravity = (float)(rand()%2+10)*0.5f;
				}
				break;

			case BITMAP_GHOST_CLOUD1:
			case BITMAP_GHOST_CLOUD2:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 500;
	   					o->Position[0] += (float)(rand()%400-200);

						
    					o->Position[1] += (float)(rand()%400-200);
    					o->Position[2] += (float)(rand()%400-200);
						VectorCopy(o->Position, o->StartPosition);
     					o->Scale = (float)(rand()%15+15)/30.0f+Scale;
						float fTemp = (float)(rand()%10+5)*0.08f;
						o->Velocity[0] = fTemp;
						o->Velocity[1] = fTemp;
						o->Velocity[2] = 0.0f;

						//o->Rotation = rand()%360;

						VectorCopy(o->Light, o->TurningForce);
						o->Light[0] = 0.f;
						o->Light[1] = 0.f;
						o->Light[2] = 0.f;
					}
				}
				break;
            case BITMAP_LIGHT:
				if ( 0 == o->SubType || 8 == o->SubType)
				{
					o->LifeTime = rand()%10+10;
					o->Gravity  = rand()%10+10.f;
					o->Scale    = (rand()%50+50.f)/100.f*Scale;
				}
				else if (o->SubType == 9)
				{
					o->LifeTime = 40 * o->Scale;
				}
                else if ( o->SubType==7 )
                {
					o->LifeTime = rand()%10+30;
					o->Gravity  = rand()%10+10.f;
					o->Scale    = (rand()%15+30.f)/100.f*Scale;
                }
				else if ( 6 == o->SubType)
				{
					o->LifeTime = rand()%10+10;
					o->Gravity  = rand()%10+10.f;
					o->Scale    = (rand()%50+50.f)/100.f*Scale;
				}
				else if ( 1 == o->SubType)
				{
					o->LifeTime = 50;
				}
				else if ( 2 == o->SubType)
				{
					o->LifeTime = rand()%10+10;
					o->Gravity  = rand()%10+10.f;
					o->Scale    = (rand()%50+50.f)/100.f*Scale;
				}
                else if ( 3 == o->SubType )
                {
                    o->LifeTime = 10;
                }
                else if ( o->SubType==4 )
                {
                    o->LifeTime= 10;
                    o->Gravity = 0.f;
                    o->Rotation= Scale;
                    o->Scale   = 2.f;
                }
				else if ( o->SubType == 5)
				{
					o->LifeTime = 50;
				}
				else if ( o->SubType==6 )
				{
					o->LifeTime = 2;
				}
				else if(o->SubType == 10)
				{
					o->LifeTime = 100;
					o->Scale = Scale + (rand()%10)*0.02f;	
				}
				else if ( o->SubType == 11 )
				{
					o->LifeTime = rand()%10+130;
					o->Gravity  = rand()%2+2.f;
					o->Scale    = (rand()%20+20.f)/100.f*Scale;
				}
				else if ( o->SubType == 12 )
				{
					o->LifeTime = rand()%10+100;
					o->Gravity  = rand()%2+2.f;
					o->Scale    = (rand()%20+20.f)/100.f*Scale;
				}
				else if ( o->SubType == 13 )
				{
					o->LifeTime = rand()%10+100;
					o->Gravity  = rand()%2+2.f;
					o->Scale    = (rand()%20+20.f)/100.f*Scale;
				}
				else if( o->SubType == 14)
				{
					o->LifeTime = 50;
					o->Scale = Scale + (rand()%10)*0.02f;
				}
#ifdef YDG_ADD_MAP_DOPPELGANGER1
				else if ( o->SubType == 15 )
				{
					o->LifeTime = rand()%10+100;
					o->Gravity  = rand()%3+2.f;
					o->Scale    = (rand()%20+20.f)/100.f*Scale;
					VectorCopy(Position, o->StartPosition);
					o->StartPosition[2] = 0;
					VectorCopy(Light, o->TurningForce);
					o->Alpha = 0;
				}
#endif	// YDG_ADD_MAP_DOPPELGANGER1
                break;
            case BITMAP_POUNDING_BALL:  //  자이언트 오거의 통통볼 공격.
                if ( o->SubType==0 )
                {
				    o->LifeTime = 24;
				    Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
          		    o->Scale = (float)(rand()%128+128)*0.01f;
				    o->Rotation = (float)(rand()%360);

                    Vector ( 0.5f, 0.5f, 0.5f, o->Light );
                }
                else if ( o->SubType==1 )
                {
				    o->LifeTime = 15;
				    Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
          		    o->Scale = (float)(rand()%128+128)*0.01f;
				    o->Rotation = (float)(rand()%360);

                    Vector ( 0.5f, 0.5f, 0.5f, o->Light );
                }
				else if ( o->SubType==2 )
				{
				    o->LifeTime = 20;
				    //Vector(0.f,-(float)(rand()%16+32)*0.1f,0.f,o->Velocity);
          		    o->Scale = (float)(rand()%128+80)*0.009f;
				    o->Rotation = (float)(rand()%360);
					o->Gravity = 0.0f;
					Vector(0.f,0.f,0.f,o->Velocity);
                    Vector ( 0.5f, 0.5f, 0.5f, o->Light );
				}
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				else if ( o->SubType==3 )
				{
					o->LifeTime = rand()%5+47;
					o->Scale = (float)(rand()%72+72)*0.01f*Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (float)(rand()%24+64)*0.1f;
					o->Alpha = 0;
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
                break;
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
            case BITMAP_ADV_SMOKE:      //  블러드캐슬의 불연기.
                o->LifeTime = 20+rand()%5;
                o->Rotation = 0.f;
                if(o->SubType == 0)
                {
                    o->Scale = 0.5f+(float)(rand()%10)*0.02f;
                    o->Velocity[0] = (float)(rand()%10+5)*0.4f;
                    o->Velocity[1] = (float)(rand()%10-5)*0.4f;
                    o->Velocity[2] = (float)(rand()%10+5)*0.2f;
                }
				else if(o->SubType == 2)
				{
					o->Scale = Scale*0.5f + (float)(rand()%10)*0.02f;
					o->Velocity[0] = (float)(rand()%10+5)*0.4f;
                    o->Velocity[1] = (float)(rand()%10-5)*0.4f;
                    o->Velocity[2] = (float)(rand()%10+5)*0.2f;
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if(o->SubType == 3)
				{
					o->Scale = Scale*0.5f + (float)(rand()%10)*0.02f;
					o->Alpha = 0.5f;
					o->Velocity[0] = 0;
                    o->Velocity[1] = (float)(rand()%10-5)*0.4f;
                    o->Velocity[2] = 0;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
                else
                {
                    o->Scale = 1.f+(float)(rand()%10)*0.1f;
                    o->Velocity[0] = (float)(rand()%10+5)*0.2f;
                    o->Velocity[1] = (float)(rand()%10-5)*0.2f;
                    o->Velocity[2] = (float)(rand()%10+5)*0.1f;
                }
                break;
            case BITMAP_ADV_SMOKE+1:    //  블러드캐슬의 불연기.
                o->LifeTime = 25+rand()%5;
                o->Rotation = (float)(rand()%360);
                o->Scale = 0.5f;
                if ( o->SubType==1 )
                {
                    o->Scale *= Scale;
                }
                o->Velocity[0] = (float)(rand()%10+5)*0.4f;
                o->Velocity[1] = 0.f;
                o->Velocity[2] = (float)(rand()%10+5)*0.2f;

            case BITMAP_TRUE_FIRE : //  진짜불 효과.
            case BITMAP_TRUE_BLUE : //  진짜불 효과.
				if(o->SubType == 7)
				{
					o->Scale = Scale + (float)(rand()%30)/100.f;
					o->LifeTime = 15;
					o->Position[0] += (float)(rand()%10)/10.f - 0.5f;
					o->Position[1] += (float)(rand()%10)/10.f - 0.5f;
					o->Position[2] += (float)(rand()%10)/10.f - 0.5f;
				}
				else if(o->SubType == 5)
					o->LifeTime = 20;
				else if(o->SubType == 6)
					o->LifeTime = 32;
				else if(o->SubType == 8)
				{
					o->LifeTime = 15;
				}
				else if(o->SubType == 9)
				{
					o->LifeTime = 20;
				}
				else
				{
					o->LifeTime = 24;
				}

				if(o->SubType == 8 || o->SubType == 9)
				{
					o->Velocity[0] = (float)(rand()%4-2)*0.4f;
					o->Velocity[1] = 0.f;
					o->Velocity[2] = (float)(rand()%4+2)*0.2f;
				}
				else
				{
					o->Velocity[0] = (float)(rand()%10-5)*0.4f;
					o->Velocity[1] = 0.f;
					o->Velocity[2] = (float)(rand()%10+5)*0.2f;
				}

				VectorCopy ( o->Position, o->StartPosition );
                break;

            case BITMAP_HOLE:
                o->LifeTime = 30;
                o->Rotation = (float)(rand()%360);
                Vector ( 0.1f, 0.1f, 0.1f, o->Light );
                break;
            case BITMAP_WATERFALL_1:
                //  SubType: 0 (기본텍스쳐), 1(BITMAP_CLOUD+2)
                o->LifeTime = 30;
                o->Rotation = (float)(rand()%360);
                o->Velocity[2] = (float)(-(rand()%5+7));

                Vector ( 0.2f, 0.2f, 0.2f, o->Light );

                if ( o->SubType==0 )
                {
                    o->Scale = 1.6f;
                }
                else if ( o->SubType==1 )
                {
     			    o->Scale = (float)(rand()%20+80)*0.01f;
                    o->TexType  = BITMAP_CLOUD+2;
                }
                else if ( o->SubType==2 )
                {
     			    o->Scale = 1.6f + Scale;
                }
                break;

            case BITMAP_WATERFALL_5:
                if ( o->SubType==0 )
                {
                    o->LifeTime = 30;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 1.6f;
                    o->Velocity[2] = (float)(-(rand()%5+7));

                    Vector ( 0.2f, 0.2f, 0.2f, o->Light );
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime = 20;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 1.f;
                    o->Position[2] += 50.f;
                }
				else if ( o->SubType==2 )
                {
                    o->LifeTime = 20;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 0.5f;
                    o->Position[2] += 50.f;
                    o->Velocity[2] = (float)((rand()%5+10));
                }
                else if ( o->SubType==3 )
                {
                    o->LifeTime = 20;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 1.6f;
                    o->Velocity[2] = (float)(-(rand()%5+7));
                }
				else if ( o->SubType==4 )
                {
	                o->LifeTime = 6;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+o->Angle[0]*1.2f;//45.f;
					o->Scale    = Scale+(float)(rand()%6)*0.20f;
					o->Rotation = (float)(rand()%360);

					VectorCopy(o->Position,o->StartPosition);
                }
                else if ( o->SubType==5 )
                {
                    o->LifeTime = 30;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 1.6f*Scale;
                    o->Velocity[2] = (float)(-(rand()%5+10));

                    Vector ( 0.2f, 0.2f, 0.2f, o->Light );
                }
				else if ( o->SubType==6)
				{
					o->LifeTime = 30;
					o->Rotation = (float)(rand()%360);
					o->Scale    = Scale+0.6f;		// 물파티클 박스 Fixed : 2004/11/05
					o->Velocity[2] = -(rand()%5+12);
					
					Vector ( 0.2f, 0.2f, 0.2f, o->Light );
				}
#ifdef CSK_ADD_MAP_ICECITY	
				else if(o->SubType == 7)
				{
					o->TexType = rand()%2 == 0 ? BITMAP_WATERFALL_4 : BITMAP_WATERFALL_5;
					o->LifeTime = 30;
					o->Rotation = (float)(rand()%360);
					o->Scale    = (float)(rand()%50)*0.05f + Scale;
					o->Velocity[0] = -(rand()%2+1);
					o->Velocity[1] = -(rand()%2+3);
					o->Velocity[2] = (rand()%3+3);
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 8)
				{
					o->LifeTime = 30;
                    o->Velocity[2] = rand()%3+1;
					o->Scale = (rand()%5+5)*0.05f + Scale;
				}
#endif // CSK_RAKLION_BOSS
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if ( o->SubType==9 )
                {
                    o->LifeTime = 30;
                    o->Rotation = (float)(rand()%360);
                    o->Scale    = 0.6f + Scale;
                    o->Velocity[2] = (float)(-(rand()%5+7));
					
                    Vector ( 0.2f, 0.2f, 0.2f, o->Light );
                }
#endif //LDS_ADD_EMPIRE_GUARDIAN
                break;

            case BITMAP_PLUS:
                o->LifeTime = 20;
                o->Scale    = Scale;
                o->Position[0] += rand()%30-15.f;
                o->Position[1] += rand()%30-15.f;
                o->Position[2] += 240.f;
                break;

            case BITMAP_WATERFALL_2:
                o->LifeTime = 30;
                o->Rotation = (float)(rand()%360);

				if(o->SubType == 2)
				{
					o->LifeTime = 40;
					o->Scale = (rand()%6+6)*0.1f + Scale;
				}
#ifdef PBG_ADD_PKFIELD
				else if(o->SubType == 6)
				{
					o->LifeTime = rand()%50+20;
					o->Gravity  = (float)(rand()%20+10);
					o->Scale *= 0.2f;
					VectorCopy(o->Position, o->StartPosition);
				}
#endif //PBG_ADD_PKFIELD
				else
	                o->Scale    = (rand()%6+6)*0.1f;

                o->Velocity[2] = (float)(-(rand()%3+3));

				if(SceneFlag == CHARACTER_SCENE)
				{
					Vector ( 0.25f, 0.25f, 0.25f, o->Light );
				}
				else
				{
					Vector ( 0.4f, 0.4f, 0.4f, o->Light );
				}

                o->Position[0] += rand()%20-10.f;
                o->Position[1] += rand()%20-10.f;
                o->Position[2] += rand()%40-20.f;

				if(o->SubType == 1)
				{  
					Vector ( 0.0f, 0.4f, 0.4f, o->Light );
					o->LifeTime = 50;
					o->Velocity[2] = (float)(-(rand()%3+3));	
				}
				if(o->SubType == 3)
				{
					o->TexType = BITMAP_LIGHT+2;
					VectorCopy(Light, o->Light);
					o->LifeTime = 12;
					o->Rotation = (float)(rand()%360);
					o->Scale    = (rand()%3+3)*0.18f;
					o->Velocity[2] = 2.0f;

					o->Position[0] += rand()%20-10.f;
					o->Position[1] += rand()%20-10.f;
					o->Position[2] += rand()%40-20.f;
				}
				if(o->SubType == 4)
				{
					o->LifeTime = 70;
					o->Scale = (rand()%6+6)*0.1f + Scale;
				}
#ifdef CSK_ADD_MAP_ICECITY	
				if(o->SubType == 5)
				{
					o->LifeTime = 40;
					o->Rotation = (float)(rand()%360);
					o->Gravity = (rand()%2 + 2);
					o->Scale = (rand()%6+6)*0.1f + Scale;
					o->Velocity[0] = -(rand()%2 + 2);
					o->Velocity[1] = -(rand()%2 + 2);
					o->Velocity[2] = (rand()%2+1);
					o->Position[0] += rand()%60-30.f;
					o->Position[1] += rand()%60-30.f;
					o->Position[2] += rand()%10;
					break;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				if(o->SubType == 11)
				{
                    o->LifeTime = 30;
                    o->Velocity[2] = rand()%5+5;
					o->Scale = (rand()%10+10)*0.05f * Scale;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
                break;

			//////////////////////////////////////////////////////////////////////////

            case BITMAP_WATERFALL_3:
            case BITMAP_WATERFALL_4:
                if ( o->SubType==0 )
                {
                    o->LifeTime = 20;
                    o->Velocity[2] = (float)(rand()%5+2);
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime = 10;
                    o->Velocity[2] = (float)(rand()%2+2);
                }
				else if ( o->SubType==2 )
                {
	                o->LifeTime = 6;
                    o->Gravity  = ((rand()%100)/100.f)*4.f+o->Angle[0]*1.2f;//45.f;
					o->Scale    = Scale+(float)(rand()%6)*0.10f;
					o->Rotation = (float)(rand()%360);

					VectorCopy(o->Position,o->StartPosition);
					break;
                }
				else if ( o->SubType==3 )		// 먼지불빛 박스 Added : 2004/11/09
				{
					o->LifeTime = 60;
					o->Velocity[2] = -1.0f;
					o->Scale    = o->Scale*(rand()%10+15)*0.02f;
					o->Rotation = (float)(rand()%360);
					o->Position[0] += rand()%40-20.f;
					o->Position[2] += rand()%25-10.f;
					break;
				}
#ifdef ADD_SOCKET_ITEM
				// 소드브레이커 이팩트
				// * 정해진 위치에 waterfall을 생성하여 돌아가면서 작아짐, 페이드 아웃
				// * 주의 : 다른곳에서 사용하지 말것
				else if( o->SubType == 12 )
				{
					o->Rotation = (float)(rand()%360);
					o->LifeTime = (rand()%2-1)+5;
					// 10단계 +-30% 크기변환
					float fIntervalScale = Scale*0.3f;
					o->Scale += ((float((rand()%20)-10)/2.0f)*((fIntervalScale/2.f)));
					o->Position[0] += (float)(rand()%20)-10.f;
					o->Position[1] += (float)(rand()%20)-10.f;
					o->Position[2] += (float)(rand()%20)-10.f;

					break;
				}
#endif // ADD_SOCKET_ITEM
#ifdef ASG_ADD_MAP_KARUTAN
				else if (o->SubType == 16)
				{
					o->LifeTime = 30;
					o->Rotation = (float)(rand()%360);
					o->Scale    = 0.6f + Scale;
					o->Velocity[2] = (float)(rand()%3+5);
					break;
				}
#endif	// ASG_ADD_KARURAN

                o->Rotation = (float)(rand()%360);
                o->Scale    = (rand()%10+10)*0.02f;

				if (o->SubType == 4)
				{
                    o->LifeTime = 20;
                    o->Velocity[2] = rand()%5+2;
					o->Scale    = (rand()%10+10)*0.02f + Scale;
				}
				else if(o->SubType == 5 || o->SubType == 6)
				{
					o->LifeTime = 20;
					o->Gravity  = ((rand()%100)/100.f)*2.f;
					o->Scale    = (rand()%10+10)*0.02f + Scale;
					o->Rotation = (float)(rand()%360);
					o->Position[0] += rand()%40-20.f;
					o->Position[1] += rand()%40-20.f;
					o->Position[2] -= 50.f;
					break;
				}
				else if(o->SubType == 7)	//@@ CreateParticle
				{
					o->LifeTime = 60;
					o->Velocity[2] = (rand()%4)*0.1f + 0.8f;
					o->Scale    = o->Scale*(rand()%10+15)*0.02f;
					o->Rotation = (float)(rand()%360);
					o->Position[0] += rand()%18-9.f;
					o->Position[2] += rand()%18-9.f;
					break;
				}
				else if (o->SubType == 8)
				{
                    o->LifeTime = 30;
                    o->Velocity[2] = rand()%5+5;
					o->Scale    = (rand()%10+10)*0.05f;// + Scale;
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 9)
				{
					o->LifeTime = 30;
                    o->Velocity[2] = rand()%5+2;
					o->Scale    = (rand()%5+5)*0.05f + Scale;
				}
				else if(o->SubType == 10)
				{
					o->LifeTime = 20;
					o->Gravity  = ((rand()%200)/100.f) + 2.f;
					o->Scale    = (rand()%10+10)*0.02f + Scale;
					o->Rotation = (float)(rand()%360);
					o->Velocity[0] = -(rand()%2 + 1);
					o->Velocity[1] = -(rand()%2 + 2);
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 11)
				{
					o->LifeTime = 20;
					o->Scale = Scale + (rand()%10-5)*0.05f;
					o->Rotation = (float)(rand()%360);

					vec3_t vAngle, vPos, vPos2;
					Vector ( 0.f, 5.f, 0.f, vPos );
					float fAngle = o->Angle[2] + (float)(rand()%90-45) + 150 ;
					Vector ( 0.f, 0.f, fAngle, vAngle );
					AngleMatrix ( vAngle, Matrix );
					VectorRotate ( vPos, Matrix, vPos2 );
					VectorCopy(vPos2, o->Velocity);

					break;
				}
				else if(o->SubType == 13)
				{
					o->LifeTime = 30;
					o->Scale = Scale;
					o->Rotation = (float)(rand()%360);
					o->Gravity = 2.5f + (float)(rand()%10) * 0.1f;
					break;
				}
#endif // CSK_RAKLION_BOSS
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if (o->SubType == 14)
				{
                    o->LifeTime = 30;
                    o->Velocity[2] = rand()%5+5;
					o->Scale    = (rand()%10+10)*0.05f * Scale;
				}
#endif // LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD
				else if(o->SubType == 15)
				{
					o->LifeTime = 20;
					o->Gravity  = ((rand()%100)/100.f)*2.f;
					o->Scale    = (rand()%10+10)*0.02f + Scale;
					o->Rotation = (float)(rand()%360);
					o->Position[0] += rand()%40-20.f;
					o->Position[1] += rand()%40-20.f;
					o->Position[2] -= 50.f;
					break;
				}
#endif	// LDS_ADD_EG_2_MONSTER_GRANDWIZARD

                o->Position[0] += rand()%40-20.f;
                o->Position[1] += rand()%40-20.f;
                o->Position[2] += rand()%20-10.f;
                break;
			case BITMAP_SHOCK_WAVE:
				if(o->SubType == 3)
				{
					o->LifeTime = 7;
				}
				else if(o->SubType == 0)
				{
					o->LifeTime = 7;
					o->Scale = Scale;
					VectorCopy(Light, o->Light);
				}
#ifdef CSK_EVENT_CHERRYBLOSSOM
				if(o->SubType == 4)
				{
					o->Alpha = 1.0f;
					o->LifeTime = 7;
					o->Scale = Scale;
					o->Gravity = 6.f;
					VectorCopy(Light, o->Light);
				}
#endif //CSK_EVENT_CHERRYBLOSSOM
				break;
			case BITMAP_GM_AURORA:
				o->LifeTime = 20;
				break;
			case BITMAP_CURSEDTEMPLE_EFFECT_MASKER:
				{
					o->LifeTime = 30;
				}
				break;	
#ifdef LDS_RAKLION_ADDMONSTER_ICEWALKER
			case BITMAP_RAKLION_CLOUDS:
				{
					o->Alpha		= 1.f;
					o->LifeTime		= 32;
					o->Rotation		= (float)(rand() % 360);
				}
				break;
#endif // LDS_RAKLION_ADDMONSTER_ICEWALKER
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD
			case BITMAP_CHROME2:
				{
					o->LifeTime = rand()%5+5;
					o->Rotation = rand()%360;
// 					o->Gravity  = rand()%3+2.f;
// 					o->Scale    = (rand()%20+20.f)/100.f*Scale;
					VectorCopy(Position, o->StartPosition);
					o->Scale *= 1.0f + (rand()%10)*0.03f;
// 					o->StartPosition[2] = 0;
// 					VectorCopy(Light, o->TurningForce);
// 					o->Alpha = 0;
				}
				break;
#endif	// LDS_ADD_EG_2_MONSTER_GRANDWIZARD
#ifdef PBG_ADD_AURA_EFFECT
			case BITMAP_AG_ADDITION_EFFECT:
				{
					float _Scale;
					if(o->SubType==0)
					{
						o->LifeTime = 33+rand()%5;
						o->Rotation = (float)(rand()%90)+270;
						_Scale = (rand()%20+20.0f)/50.0f*0.5f;
					}
					else if(o->SubType==1)
					{
						o->LifeTime = 27+rand()%5;
						o->Rotation = (float)(rand()%90);
						_Scale = (rand()%20+20.0f)/50.0f*1.5f;
					}
					else if(o->SubType==2)
					{
						o->LifeTime = 38+rand()%5;
						o->Rotation = (float)(rand()%90)+135;
						_Scale = (rand()%20+20.0f)/50.0f*1.0f;
					}
					o->Scale = _Scale;
					o->Gravity = (float)(rand()%16+12)*0.1f;
					o->Alpha = 0;
					Vector(1.0f,0.0f,0.6f,Light);
					VectorCopy(Light, o->TurningForce);
					Vector(0, 0, 0, o->Light);
				}
				break;
#endif //PBG_ADD_AURA_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case BITMAP_SBUMB:
				{
					o->LifeTime = 4;
					o->Scale = Scale;
				}
				break;
			case BITMAP_DAMAGE1:
				{
					o->LifeTime = 5;
					o->Scale = Scale;
				}
				break;
			case BITMAP_SWORD_EFFECT_MONO:
				{
					o->LifeTime = 20;
					o->Scale = Scale;
				}
				break;
			case BITMAP_DAMAGE2:
				{
					o->LifeTime = 15;
					o->Scale = Scale*0.9f+(rand()%2+2)*0.1f;
					o->Position[2] += 80.0f+rand()%20;
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			}
			return i;
		}
	}
	return false;
}

void MoveParticles()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_PARTICLES, PROFILING_MOVE_PARTICLES );
#endif // DO_PROFILING
	// 바람
	for ( int i = 0; i < 2; ++i)
	{
		g_vParticleWindVelo[i] += ( rand() % 2001 - 1000) * ( 0.001f * 0.6f);
		g_vParticleWindVelo[i] = max( -.6f, min( g_vParticleWindVelo[i], .6f));
	}
#ifdef _VS2008PORTING
	for ( int i = 0; i < 2; ++i)
#else // _VS2008PORTING
	for ( i = 0; i < 2; ++i)
#endif // _VS2008PORTING
	{
		g_vParticleWind[i] += g_vParticleWindVelo[i];
		g_vParticleWind[i] = max( -1.7f, min( g_vParticleWind[i], 1.7f));
	}

    int count = 0;
#ifdef _VS2008PORTING
	for( int i=0;i<MAX_PARTICLES;i++)
#else // _VS2008PORTING
	for( i=0;i<MAX_PARTICLES;i++)
#endif // _VS2008PORTING
	{
		PARTICLE *o = &Particles[i];
	
		if(o->Live)
		{
            count++;
			o->LifeTime--;
			if(o->LifeTime <= 0) o->Live = false;
			vec3_t Position;
			vec3_t TargetPosition;
			vec3_t Light;
			float Luminosity;
			float Height;
			float Matrix[3][4];

            if ( o->bEnableMove )
			    MovePosition(o->Position,o->Angle,o->Velocity);

			switch(o->Type)
			{
//////////////////////////////////////////////////////////////////////////

			case BITMAP_EFFECT:
				if(o->LifeTime >= 10)
				{
					if( o->SubType == 2 )
					{
						o->Light[0] /= 1.03f;
						o->Light[1] /= 1.03f;
						o->Light[2] /= 1.03f;
					}
					else
					{
						o->Light[0] *= 1.16f;
						o->Light[1] *= 1.16f;
						o->Light[2] *= 1.16f;
					}
				}
				else
				{
					if( o->SubType == 2 )
					{
						o->Light[0] /= 1.03f;
						o->Light[1] /= 1.03f;
						o->Light[2] /= 1.03f;
					}
					else
					{
						o->Light[0] /= 1.16f;
						o->Light[1] /= 1.16f;
						o->Light[2] /= 1.16f;
					}
				}
				o->Position[2] += o->Gravity;
				break;

//////////////////////////////////////////////////////////////////////////
	
			case BITMAP_FLOWER01:
			case BITMAP_FLOWER01+1:
			case BITMAP_FLOWER01+2:
				if(o->Frame == 0)
				{
                    if ( o->SubType==0 )
                    {
					    o->Velocity[0] += (float)(rand()%32-16)*0.1f;
					    o->Velocity[1] += (float)(rand()%32-16)*0.1f;
					    o->Velocity[2] += (float)(rand()%32-16)*0.1f;
                    }
                    else if ( o->SubType==1 )
                    {
					    o->Velocity[0] += (float)(rand()%8-4)*0.05f;
					    o->Velocity[1] += (float)(rand()%8-4)*0.05f;
					    o->Velocity[2] -= (float)(rand()%8)*0.05f;
                    }
					VectorAdd(o->Position,o->Velocity,o->Position);
				}
				Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
				if(o->Position[2] < Height)
				{
					o->Position[2] = Height;
					Vector(0.f,0.f,0.f,o->Velocity);
					o->Frame = 1;
				}
				o->Rotation += rand()%16;
				break;
            case BITMAP_FLARE_BLUE:
                if ( o->SubType==0 )
                {
                    o->Scale = 0.2f;//sin( WorldTime*0.001f )*0.2f+0.35f;

                    o->Velocity[2] += 0.4f;
                    o->Velocity[2] = min ( 8.f, o->Velocity[2] );

                    if ( o->LifeTime<5 )
                    {
                        o->Light[0] /= 1.2f;
                        o->Light[1] /= 1.2f;
                        o->Light[2] /= 1.2f;
                    }
                }
                else if ( o->SubType==1 )
                {
                    o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
					o->Scale += 1.5f;
                }
				break;
            case BITMAP_FLARE+1:
                if ( o->SubType==0 )
                {
                    float count = (o->Velocity[0]+o->LifeTime)*0.05f;
					o->Position[0] = o->StartPosition[0] + sinf(count)*(105.f+o->Scale*-250);
					o->Position[1] = o->StartPosition[1] - cosf(count)*(105.f+o->Scale*-250);
                    o->Position[2] += o->Gravity;

					o->Scale -= 0.0008f;
                }
                break;
			case BITMAP_BLUE_BLUR:
				switch(o->SubType)
				{
				case 1:
				case 0:
     				Luminosity = (float)(o->LifeTime)/20.f;
    				Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,o->Light);
					if(o->SubType == 1)
					{
						o->Scale += 0.19f;
						o->Position[2] += 5.00f;
					}
					else
						o->Scale += 0.05f;

				break;
				}
				break;
			case BITMAP_LIGHT+2:
				if (o->SubType == 3)
				{
					o->Gravity += 0.2f;
					//o->Position[2] += o->Gravity;
     				o->Scale -= 0.03f;

					if (o->Scale < 0 || !o->Target->Live)
						o->Live = false;
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->StartPosition, o->Position);
				}
				else if (o->SubType == 5)
				{
					o->TurningForce[2] -= o->Gravity;

					o->Position[0] += o->TurningForce[0];
					o->Position[1] += o->TurningForce[1];
					o->Position[2] += o->TurningForce[2];
					
					o->Angle[0] += 0.5f * o->LifeTime;
					o->Angle[1] += 0.5f * o->LifeTime;
     				o->Scale += 0.04f;
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					if (o->LifeTime < 20)
					{
						o->TurningForce[0] *= 0.6f;
						o->TurningForce[1] *= 0.6f;
					}
					Vector(o->Alpha * 1.0f, o->Alpha * 0.0f, o->Alpha * 0.6f, o->Light);

					if (o->Alpha < 0 || !o->Target->Live)
						o->Live = false;
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->StartPosition, o->Position);
					o->Velocity[0] += 0.05f;
					o->Velocity[1] += 0.05f;

					o->Rotation += 1.0f;
				}
				else if(o->SubType == 6)
				{
					o->TurningForce[2] -= o->Gravity;
					
					o->Position[0] += o->TurningForce[0];
					o->Position[1] += o->TurningForce[1];
					o->Position[2] += o->TurningForce[2];
					
					o->Angle[0] += 0.5f * o->LifeTime;
					o->Angle[1] += 0.5f * o->LifeTime;
					o->Scale += 0.04f;
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					if (o->LifeTime < 20)
					{
						o->TurningForce[0] *= 0.6f;
						o->TurningForce[1] *= 0.6f;
					}
					Vector(o->Alpha * 1.0f, o->Alpha * 1.0f, o->Alpha * 1.0f, o->Light);
					
					if (o->Alpha < 0)
					{
						o->Live = false;
					}

					o->Velocity[0] += 0.05f;
					o->Velocity[1] += 0.05f;
					
					o->Rotation += 2.0f;	
				}			
				else if(o->SubType == 7)		// Drain Life (드레인 라이프)
				{
					o->TurningForce[2] -= o->Gravity;
					
					o->Position[0] += o->TurningForce[0];
					o->Position[1] += o->TurningForce[1];
					o->Position[2] += o->TurningForce[2];
					
					o->Angle[0] += 0.5f * o->LifeTime;
					o->Angle[1] += 0.5f * o->LifeTime;
					o->Scale += 0.04f;
					o->TexType = BITMAP_LIGHT;
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					if (o->LifeTime < 20)
					{
						o->TurningForce[0] *= 0.6f;
						o->TurningForce[1] *= 0.6f;
					}
					Vector(o->Alpha * o->Light[0], o->Alpha * o->Light[1], o->Alpha * o->Light[2], o->Light);
					
					if (o->Alpha < 0)
					{
						o->Live = false;
					}

					o->Velocity[0] += 0.05f;
					o->Velocity[1] += 0.05f;
					
					o->Rotation += 2.0f;	

					Luminosity = (float)(o->LifeTime)/8.f * 0.02f;
					o->Light[0] += Luminosity;
					o->Light[1] += Luminosity;
					o->Light[2] += Luminosity;
				}
				else
				{
					Luminosity = (float)(o->LifeTime)/8.f * 0.02f;
					o->Light[0] += Luminosity;
					o->Light[1] += Luminosity;
					o->Light[2] += Luminosity;
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale -= 0.1f;
				}
				break;
			case BITMAP_GM_AURORA:
				{
					Luminosity = (float)(o->LifeTime)/8.f * 0.04f;
					o->Light[0] -= Luminosity;
					o->Light[1] -= Luminosity;
					o->Light[2] -= Luminosity;
				}
				break;
			case BITMAP_MAGIC+1:
				{
					o->Light[0] *= 0.9f;
					o->Light[1] *= 0.9f;
					o->Light[2] *= 0.9f;
     				o->Scale += 0.1f;
				}
				break;
			case BITMAP_BUBBLE:
                switch ( o->SubType )
                {
                case 0:
			        o->Frame++;
			        o->Position[0] += (float)(rand()%20-10)*2.5f*o->Scale;
			        o->Position[1] += (float)(rand()%20-10)*2.5f*o->Scale;
			        o->Position[2] += (float)(rand()%20+10)*2.5f*o->Scale;
                    break;

                case 1:
			        o->Frame++;
			        o->Position[0] += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[1] += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[2] += (float)(rand()%20+10)*0.3f*o->Scale;
                    o->Scale += 0.002f;
                    break;

                case 2:
			        o->Position[0];// += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[1];// += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[2] += (float)(rand()%20+10)*0.3f;
                    o->Scale += 0.005f;
                    break;

                case 3:
			        o->Frame++;
			        o->Position[0] += (float)(rand()%20-10)*2.5f*o->Gravity;
			        o->Position[1] += (float)(rand()%20-10)*2.5f*o->Gravity;
			        o->Position[2] += (float)(rand()%20+10)*2.5f*o->Gravity;
                    break;
				case 4:
			        o->Position[0];// += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[1];// += (float)(rand()%30-15)*1.f*o->Scale;
			        o->Position[2] += (float)(rand()%20+10)*0.3f;
                    o->Scale += 0.005f;
					if (o->Position[2] > 350)
					{
						o->Position[2] = 350;
						o->LifeTime = 0;
					}
					break;
				case 5:
					o->Frame++;
			        o->Position[0] += (float)(rand()%20-10)*2.5f*o->Scale;
			        o->Position[1] += (float)(rand()%20-10)*2.5f*o->Scale;
			        o->Position[2] += (float)(rand()%20+10)*2.5f*o->Scale;
					if (o->Position[2] > 550)	// 기포 올라오는 높이 제한.
						o->Live = false;
					break;
                }
				break;

			

			case BITMAP_EXPLOTION_MONO:
			case BITMAP_EXPLOTION:
				o->Frame = (20-o->LifeTime)/2;
				if(o->SubType==2)
				{
					int Life = 0;
					if(o->LifeTime > 10)
						Life = o->LifeTime - 10;
					else
						Life = 1;
					o->Frame = (20-Life)/2;
					vec3_t  p,Angle;
					if(o->LifeTime == 20)
					{
						for (int j=0; j<18; j++ )
						{
							Vector ( 0.f, 200.f, 0.f, p );
							Vector ( 0.f, 0.f, j*20.f, Angle );
							AngleMatrix ( Angle, Matrix );
							VectorRotate ( p, Matrix, Position );
							VectorAdd ( Position, o->Position, Position );
							Vector(1.f,1.0f,1.0f,Light);
							CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 35 , 2.5f);

							Vector(1.f,1.f,1.f,Light);
							CreateParticle(BITMAP_EXPLOTION,Position,o->Angle, Light,1);
//							Vector(0.3f,0.3f,0.3f,Light);
//							CreateParticle ( BITMAP_SMOKE+1, Position, o->Angle, Light );
						}
					}
					else
					if(o->LifeTime == 10)
					{
						for (int j=0; j<18; j++ )
						{
							Vector ( 0.f, 400.f, 0.f, p );
							Vector ( 0.f, 0.f, j*20.f, Angle );
							AngleMatrix ( Angle, Matrix );
							VectorRotate ( p, Matrix, Position );
							VectorAdd ( Position, o->Position, Position );
							Vector(0.6f,0.6f,0.6f,Light);
							CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 35 , 2.5f);
							CreateParticle(BITMAP_EXPLOTION,Position,o->Angle, Light,1);
//							Vector(0.3f,0.3f,0.3f,Light);
//							CreateParticle ( BITMAP_SMOKE+1, Position, o->Angle, Light );

						}
					}
					else
					if(o->LifeTime == 1)
					{
						for (int j=0; j<18; j++ )
						{
							Vector ( 0.f, 600.f, 0.f, p );
							Vector ( 0.f, 0.f, j*20.f, Angle );
							AngleMatrix ( Angle, Matrix );
							VectorRotate ( p, Matrix, Position );
							VectorAdd ( Position, o->Position, Position );
							Vector(0.3f,0.3f,0.3f,Light);
							CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 35 , 2.5f);
							CreateParticle(BITMAP_EXPLOTION,Position,o->Angle, Light,1);
						}
					}
				}
                if ( o->SubType!=1 )
                {
				    Luminosity = (float)(o->LifeTime)/20.f;
				    Vector(Luminosity*0.5f,Luminosity*0.3f,Luminosity*0.1f,Light);
				    AddTerrainLight(o->Position[0],o->Position[1],Light,4,PrimaryTerrainLight);
                }
 				break;
			case BITMAP_SUMMON_SAHAMUTT_EXPLOSION:
				o->Frame = (16-o->LifeTime);
				break;
			case BITMAP_SPOT_WATER:
				o->Frame = (32-o->LifeTime)/4;
				break;
            case BITMAP_EXPLOTION+1:
				o->Frame = (12-o->LifeTime)/3;
                break;
			case BITMAP_LIGHTNING+1:
				o->Rotation = (float)((int)WorldTime%1000)*0.001f; 
				Luminosity = (float)(o->LifeTime)/10.f;
				Vector(Luminosity*0.5f,Luminosity*1.f,Luminosity*0.8f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
				if(o->SubType == 2)
				{
					o->Scale += 0.1f;
     				Vector(Luminosity*0.5f,Luminosity*1.f,Luminosity*0.8f,o->Light);
       				VectorCopy(o->Target->Position,o->Position);
					o->Position[2] += 80.f;
				}
				break;
			case BITMAP_LIGHTNING:
				o->Frame = rand()%4;
				Luminosity = (float)(o->LifeTime)/5.f;
				Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,o->Light);
				Vector(-Luminosity*0.6f,-Luminosity*0.6f,-Luminosity*0.6f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,6,PrimaryTerrainLight);
				Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,4,PrimaryTerrainLight);
				break;
			case BITMAP_CHROME_ENERGY2:
				o->Gravity = 0.0f;
				Luminosity = (float)(o->LifeTime)/24.f;
				o->Scale -= 0.04f;
				o->Rotation+=5;
				o->Frame = (23-o->LifeTime)/6;
				break;
			case BITMAP_FIRE_CURSEDLICH:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK2_MONO:
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0)
				{
					o->Scale -= (rand()%20+10)*0.001f;
					Luminosity -= 0.05f;
					o->Position[2] += o->Gravity*10.f;
				}
				else if (o->SubType == 1)
				{
					o->Scale -= (rand()%5+15)*0.0016f;
					o->Position[2] += o->Gravity*10.f;

					if (o->Scale < 0 || !o->Target->Live)
						o->Live = false;
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->StartPosition, o->Position);
				}
				else if (o->SubType == 2)
				{
					o->Scale -= (rand()%20+10)*0.004f;
					Luminosity -= 0.05f;
					o->Position[2] += o->Gravity*25.f;
				}
				else if (o->SubType == 3)
				{
					o->Scale -= 0.03f;//(rand()%20+10)*0.001f;
					Luminosity -= 0.05f;
					o->Position[2] += o->Gravity*10.f;
					
					if (o->Scale < 0)
						o->Live = false;

					if (o->Target->Live)
					{
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorCopy(o->Target->Position, o->StartPosition);
						VectorAdd(o->Position, o->StartPosition, o->Position);
					}
				}
				else if (o->SubType == 4
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 9
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+6)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType==9)
					{
						o->Rotation = 0.0f;
						o->Position[2] += o->Gravity*1.2f;
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				else if (o->SubType == 5)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%2+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 6)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+6)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 7)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+3)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 8)
				{
					if (o->LifeTime < 5)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					
					o->Scale += (rand()%3+8)*0.01f;
					if (o->LifeTime < 3)
					{
						o->Scale += (10 - o->LifeTime)*0.02f;
					}
					
					AngleMatrix(o->Angle, Matrix);
					vec3_t vMoveDir, vMoveVec;
					Vector(0.0f, -1.0f, 0.0f, vMoveDir);
					VectorRotate(vMoveDir, Matrix, vMoveVec);
					VectorScale(vMoveVec, 8.0f, vMoveVec);
					VectorAdd(o->Position, vMoveVec, o->Position);
					
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->Target->Position, o->Position);
					
					o->Rotation += 5.0f;
				}
#endif	// YDG_ADD_SKELETON_PET
				break;
			case BITMAP_LEAF_TOTEMGOLEM:
				{
					o->Velocity[2] += o->Gravity;

					Height = RequestTerrainHeight(o->Position[0],o->Position[1])+20;
					if (o->Position[2] <= Height)
					{
						Vector(0,0,0,o->Velocity);
						o->Position[2] = Height;
						o->Alpha -= 0.05f;
						Vector(o->Alpha,o->Alpha,o->Alpha,o->Light);
					}
					else
					{
						o->Scale += 0.01f;
					}
				}
				break;
			case BITMAP_FIRE:
            case BITMAP_FIRE+2:
            case BITMAP_FIRE+3:
#ifdef KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT	
                switch ( o->SubType )
                {
                case    0 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale -= 0.04f;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
				case	17:
                case    5 : //  운석 마법.
                case    6 : //  색반전.
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale -= 0.04f;
						o->Rotation+=5;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
                case    7 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Frame = (15-o->LifeTime)/6;
						o->Scale -= 0.04f;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
				case    8 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale *= 0.95f;
						o->Rotation+=5;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
					break;     
				case    9 : //  위치 고정.
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Position[0] = o->Target->Position[0] + o->StartPosition[0];
						o->Position[1] = o->Target->Position[1] + o->StartPosition[1];
						o->Position[2] = o->Target->Position[2] + o->StartPosition[2];
						o->Gravity += ((rand()%60)+60)/100;
						o->Scale -= o->Gravity/90.f;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
                case    11 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						if ( o->LifeTime>12 )
						{
							o->Frame = (24-o->LifeTime)/3;
						}
						o->Scale += 0.04f;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
				case	10:
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale *= 0.95f;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
					break;
                case    12 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Rotation += (float)(rand()%10+10.f);
						o->Scale -= 0.04f;
						Vector ( Luminosity, Luminosity, Luminosity, o->Light );
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
                case    13 :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Rotation += (float)(rand()%10+10.f);
						o->Scale -= 0.04f;
						Vector ( Luminosity, Luminosity, Luminosity, o->Light );
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
				case 14:
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Rotation += o->Gravity;
						o->Scale += 0.03f;
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
						if(o->Light[2] <= 0.05f || o->Scale <= 0.0f)
							o->Live = false;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
					break;
				case 15:
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale -= 0.04f;
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
						if(o->Light[2] <= 0.05f || o->Scale <= 0.0f)
							o->Live = false;
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
					break;
				case 16:			// 제국수호군 방패병 공격 이팩트
					{
						o->Frame = (3-o->LifeTime);
						o->Light[0] /= 1.7f;
						o->Light[1] /= 1.7f;
						o->Light[2] /= 1.7f;
						o->Scale *= 1.1f;
					}break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				case 18:
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale += o->Gravity;
						VectorScale(o->Velocity,0.98f,o->Velocity);
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
						VectorScale(o->Light, 0.95f, o->Light);
					}
					break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
                default :
					{
						o->Gravity += 0.004f;
						Luminosity = (float)(o->LifeTime)/24.f;
						o->Scale += o->Gravity;
						VectorScale(o->Velocity,0.98f,o->Velocity);
						o->Frame = (23-o->LifeTime)/6;
						o->Position[2] += o->Gravity*10.f;
					}
                    break;
				}
				break;
#else // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
				o->Gravity += 0.004f;
				Luminosity = (float)(o->LifeTime)/24.f;
                switch ( o->SubType )
                {
                case    0 :
					o->Scale -= 0.04f;
                    break;
                case    5 : //  운석 마법.
                case    6 : //  색반전.
					o->Scale -= 0.04f;
                    o->Rotation+=5;
                    break;
                case    7 :
				    o->Frame = (15-o->LifeTime)/6;
					o->Scale -= 0.04f;
                    break;
				case    8 :
					o->Scale *= 0.95f;
					o->Rotation+=5;
					break;     
				case    9 : //  위치 고정.
                    o->Position[0] = o->Target->Position[0] + o->StartPosition[0];
                    o->Position[1] = o->Target->Position[1] + o->StartPosition[1];
                    o->Position[2] = o->Target->Position[2] + o->StartPosition[2];
                    o->Gravity += ((rand()%60)+60)/100;
                    o->Scale -= o->Gravity/90.f;
                    break;
                case    11 :
					if ( o->LifeTime>12 )
					{
						o->Frame = (24-o->LifeTime)/3;
					}
					o->Scale += 0.04f;
                    break;
				case	10:
					o->Scale *= 0.95f;
					break;
                case    12 :
                    o->Rotation += (float)(rand()%10+10.f);
					o->Scale -= 0.04f;
                    Vector ( Luminosity, Luminosity, Luminosity, o->Light );
                    break;
                case    13 :
                    o->Rotation += (float)(rand()%10+10.f);
					o->Scale -= 0.04f;
                    Vector ( Luminosity, Luminosity, Luminosity, o->Light );
                    break;
				case 14:
					o->Rotation += o->Gravity;
					o->Scale += 0.03f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					if(o->Light[2] <= 0.05f || o->Scale <= 0.0f)
						o->Live = false;
					break;
				case 15:
					o->Scale -= 0.04f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					if(o->Light[2] <= 0.05f || o->Scale <= 0.0f)
						o->Live = false;
					break;
                default :
					o->Scale += o->Gravity;
      				VectorScale(o->Velocity,0.98f,o->Velocity);
                    break;
				}

//                if( o->SubType!=7 )
                {
				    o->Frame = (23-o->LifeTime)/6;
                }
				o->Position[2] += o->Gravity*10.f;
   				/*o->Position[0] -= o->Gravity;
				o->Gravity += (float)(rand()%16)*0.01f;
				Vector(o->Scale,o->Scale,o->Scale,o->Light);*/
				break;
#endif // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
			case BITMAP_FIRE+1:
				if(o->SubType==1)
				{
//					o->Gravity += 0.02f;
//					o->Scale += o->Gravity;
     				VectorScale(o->Velocity,1.05f,o->Velocity);
					o->Position[2] += o->Gravity*20.f;
					Luminosity = (float)(o->LifeTime)*0.2f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				}
				else if(o->SubType==2)
				{
//					o->Gravity += 0.02f;
//					o->Scale += o->Gravity;
//     				VectorScale(o->Velocity,1.05f,o->Velocity);
//					o->Position[2] += o->Gravity*20.f;
					Luminosity = (float)(o->LifeTime)*0.2f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				}
				else if(o->SubType==3)
				{
					o->Gravity += 0.02f;
					o->Scale += o->Gravity;
     				VectorScale(o->Velocity,1.05f,o->Velocity);
					o->Position[2] += o->Gravity*20.f;
					Luminosity = (float)(o->LifeTime)*0.2f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				}
				else if(o->SubType==4)
				{
//     				Luminosity = (float)(o->LifeTime)/5.f;
//					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
                    float count = (o->Velocity[0]+o->LifeTime)*0.1f;
                    o->Position[0] = o->StartPosition[0] + sinf(count)*120.f;
                    o->Position[1] = o->StartPosition[1] - cosf(count)*120.f;
                    o->Position[2] += o->Gravity;
                    o->Scale -= 0.002f;
				}
				else if(o->SubType==5)
				{
//					Luminosity = (float)(o->LifeTime)*0.2f;
//					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Position[0] += (float)(cos(o->Angle[2]) * 20.0f);
					o->Position[1] += (float)(sin(o->Angle[2]) * 20.0f);
                    o->Position[2] += o->Gravity;
					o->Rotation += 1;
                    //o->Scale += 0.003f;
				}
				else if(o->SubType==6)
				{
                    o->Position[2] += o->Gravity;
					o->Rotation += 4;
				}
				else if(o->SubType==7)
				{
     				VectorScale(o->Velocity,1.05f,o->Velocity);
					Luminosity = (float)(o->LifeTime)*0.2f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				}
				else if(o->SubType == 8 || o->SubType == 9)
				{
					o->Scale += 0.13f;
					o->Rotation += o->Gravity;
					o->Light[0] /= 1.13f;
					o->Light[1] /= 1.13f;
					o->Light[2] /= 1.13f;
				}
				else if(o->SubType==0)
				{
					o->Gravity += 0.02f;
					o->Scale += o->Gravity;
     				VectorScale(o->Velocity,1.05f,o->Velocity);
					o->Position[2] += o->Gravity*20.f;
					Luminosity = (float)(o->LifeTime)*0.2f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				}
				break;

//----------------------------------------------------------------------------------------------------

            case BITMAP_FLAME:
				if(o->LifeTime <= 0) o->Live = false;
                
                switch (o->SubType)
                {
                case 1:
					if(o->LifeTime==10)
					{
						o->Velocity[1] += 32*0.2f;
						o->Scale -= 0.15f;
					}
					o->Rotation = (float)(rand()%360);
                    o->Light[0] -= 0.05f;
                    o->Light[1] -= 0.05f;
                    o->Light[2] -= 0.05f;
                    break;
				case 5:
					if(o->LifeTime==10)
					{
						o->Velocity[1] += 32*0.2f;
						o->Scale *= 0.8f;
					}
					o->Rotation = (float)(rand()%360);
                    o->Light[0] -= 0.05f;
                    o->Light[1] -= 0.05f;
                    o->Light[2] -= 0.05f;
					break;
                case 2:
                    if ( o->LifeTime<10 )
                    {
//                        o->Light[0] /= (15-o->LifeTime);
//                        o->Light[1] /= (15-o->LifeTime);
//                        o->Light[2] /= (15-o->LifeTime);
                    }
                    o->Rotation = (float)(rand()%360);
                    o->Light[0] -= 0.05f;
                    o->Light[1] -= 0.05f;
                    o->Light[2] -= 0.05f;
                    break;

                case 3:
                    o->Rotation = (float)(rand()%360);
                    break;

                case 4:
                    o->Position[0] +=  o->Target->Position[0] - o->StartPosition[0];
                    o->Position[1] +=  o->Target->Position[1] - o->StartPosition[1];
                    o->Position[2] +=  o->Target->Position[2] - o->StartPosition[2] + o->Gravity;
                    VectorCopy(o->Target->Position, o->StartPosition);
                    o->Gravity += 0.1f;
					o->Rotation = (float)(rand()%360);
                    o->Light[0] -= 0.05f;
                    o->Light[1] -= 0.05f;
                    o->Light[2] -= 0.05f;
                    break;
				case 8:
					{
						if(o->StartPosition[0] < o->Position[0])
							o->Rotation += 2.0f;
						else
							o->Rotation -= 2.0f;
						
						o->Position[2] += o->Gravity/2.f;
						o->Scale -= o->Gravity/95.f;
						if(o->Scale <= 0.0f)
							o->Live = false;

						o->Light[0] /= 1.007f;
						o->Light[1] /= 1.007f;
						o->Light[2] /= 1.007f;
					}
					break;
				case 7:
					{
						if(o->StartPosition[0] < o->Position[0])
							o->Rotation += 2.0f;
						else
							o->Rotation -= 2.0f;
						
						o->Position[2] += o->Gravity;
						o->Scale -= o->Gravity/95.f;
						if(o->Scale <= 0.0f)
							o->Live = false;

						o->Light[0] /= 1.007f;
						o->Light[1] /= 1.007f;
						o->Light[2] /= 1.007f;
					}
					break;
				case 9:
					{
						if(o->StartPosition[0] < o->Position[0])
							o->Rotation += 0.5f;
						else
							o->Rotation -= 0.5f;
						
						o->Position[2] += o->Gravity*1.2f;
						o->Scale -= o->Gravity/98.f;
						if(o->Scale <= 0.0f)
							o->Live = false;

						o->Light[0] /= 1.008f;
						o->Light[1] /= 1.008f;
						o->Light[2] /= 1.008f;
					}
					break;
				case 10:	// 불 뿜어져 나오는 효과.(BITMAP_ADV_SMOKE 2번과 동일)
					VectorAdd(o->Position,o->Velocity,o->Position);
					o->Velocity[0] *= 0.95f;
					o->Velocity[1] *= 0.95f;
					o->Light[0] = (float)(o->LifeTime)/10.f;
					o->Light[1] = o->Light[0];
					o->Light[2] = o->Light[0];
					o->Velocity[2] += 0.3f;
                    o->Scale += 0.07f;
				break;
#ifdef CSK_RAKLION_BOSS
				case 11:
					o->Light[0] /= 1.008f;
					o->Light[1] /= 1.008f;
					o->Light[2] /= 1.008f;
					break;
#endif // CSK_RAKLION_BOSS
				}
                break;

//----------------------------------------------------------------------------------------------------

			case BITMAP_FIRE_RED:
				if(o->LifeTime <= 0) 
					o->Live = false;
				break;
			case BITMAP_RAIN_CIRCLE:
			case BITMAP_RAIN_CIRCLE+1:
				if(o->LifeTime <= 0) o->Live = false;
                if ( o->SubType==1 )
                {
        		    o->Scale += o->Gravity;
                }
                else
                {
        		    o->Scale += 0.03f;
                }
				if(M34CryWolf1st::IsCyrWolf1st()==true)
				{
					Vector(1.0f,1.0f,0.7f,o->Light);
				}
				else
				{
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
					if( o->SubType == 2 )		// 통합 로렌 시장 인 경우
					{
						Vector(0.03f,0.03f,0.03f,Light);
						VectorSubtract(o->Light,Light,o->Light);
					}
					else
					{
						Vector(0.05f,0.05f,0.05f,Light);
						VectorSubtract(o->Light,Light,o->Light);
					}
#else // LDS_ADD_MAP_UNITEDMARKETPLACE
					Vector(0.05f,0.05f,0.05f,Light);
					VectorSubtract(o->Light,Light,o->Light);
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case BITMAP_ENERGY:
				{
					o->Rotation += o->Gravity;
					
					if ( o->SubType==1 )
					{
						o->Light[1] = o->Light[2] = o->Light[0] = o->LifeTime/15.f;
						o->Scale += 0.1f;
					}
					else if(o->SubType == 2)	//^ 펜릴 이펙트 관련
					{
						o->Light[0] -= 0.01f;
						o->Light[1] -= 0.01f;
						o->Light[2] -= 0.01f;
					}
					// ChainLighting
					else if(o->SubType == 3 || o->SubType == 4 || o->SubType == 5)
					{
						// 플레이어 모델
						BMD* pModel = &Models[o->Target->Type];
						vec3_t vRelativePos, vPos;
						Vector(0.f, 0.f, 0.f, vRelativePos );
						
						if(o->SubType == 3)
						{
							// 플레이어 왼손
							pModel->TransformPosition( o->Target->BoneTransform[37], vRelativePos, vPos, true);
							VectorCopy(vPos, o->Position);
						}
						else if(o->SubType == 4)
						{
							// 플레이어 오른손
							pModel->TransformPosition( o->Target->BoneTransform[28], vRelativePos, vPos, true);
							VectorCopy(vPos, o->Position);
						}
						else if(o->SubType == 5)
						{
							// 몬스터 
							VectorCopy(o->Target->Position, o->Position);
							o->Position[2] += 80.f;
						}

						o->Light[0] /= 1.01f;
						o->Light[1] /= 1.01f;
						o->Light[2] /= 1.01f;
					}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					else if(o->SubType==7)
					{
						VectorScale(o->Light, 0.97f, o->Light);
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

			case BITMAP_MAGIC:
				if(o->SubType == 0)
				{
					o->Scale -= 0.05f;
					
					o->Light[0] -= 0.01f;
					o->Light[1] -= 0.01f;
					o->Light[2] -= 0.01f;
				}
				break;

//////////////////////////////////////////////////////////////////////////

            case BITMAP_FLARE:  //
                if ( o->SubType==0 || o->SubType==3 || o->SubType==6 
					|| o->SubType == 10
					)
                {
                    float count = (o->Velocity[0]+o->LifeTime)*0.1f;
					if(o->SubType == 10)
					{
						o->Position[0] = o->StartPosition[0];
						o->Position[1] = o->StartPosition[1];
					}
					else
					{
						o->Position[0] = o->StartPosition[0] + sinf(count)*40.f;
						o->Position[1] = o->StartPosition[1] - cosf(count)*40.f;
					}
                    o->Position[2] += o->Gravity;

                    o->Scale -= 0.002f;
                }
                else if(o->SubType==2)
                {
                    o->Position[0] = o->StartPosition[0];
                    o->Position[1] = o->StartPosition[1];
                    o->Position[2] += o->Gravity*((60-o->LifeTime)/10);
                    o->Scale -= 0.002f;
                }
				else if ( o->SubType==5 )
				{
                    float count = (o->Velocity[0]+o->LifeTime)*0.1f;
                    o->Position[0] = o->StartPosition[0] + sinf(count)*40.f;
                    o->Position[1] = o->StartPosition[1] - cosf(count)*40.f;
                    o->Position[2] -= o->Gravity;
                    o->Scale -= 0.002f;

					o->StartPosition[0] += 2.5f;
				}

                if ( o->SubType==4 )
                {
                    float count = (o->Velocity[0]+o->LifeTime)*0.1f;
                    o->Position[0] = o->StartPosition[0] + sinf(count)*40.f;
                    o->Position[1] = o->StartPosition[1] - cosf(count)*40.f;
                    o->Position[2] += o->Gravity;
                    o->Scale -= 0.004f;

                    if(o->LifeTime<=30)
                    {
                        o->Light[0] /= 1.1f;
                        o->Light[1] /= 1.1f;
                        o->Light[2] /= 1.1f;
                    }
                }
                else if(o->LifeTime<=20)
                {
                    o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
                }
				else if ( o->SubType == 11 )
				{
                    o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
					o->Scale += 1.5f;
				}
				else
				if(o->SubType == 12)
                {
                    float count = (o->Velocity[0]+o->LifeTime)*0.1f;

					{
						o->Position[0] = o->StartPosition[0] + sinf(count)*110.f;
						o->Position[1] = o->StartPosition[1] - cosf(count)*110.f;
					}
                    o->Position[2] += (o->Gravity + 0.1f);

                    o->Scale -= 0.002f;
                    if(o->LifeTime<=35)
                    {
                        o->Light[0] /= 1.1f;
                        o->Light[1] /= 1.1f;
                        o->Light[2] /= 1.1f;
                    }
				}
                //  정지시.
                if( o->SubType==0 )
                {
                    if((o->Target->CurrentAction>=PLAYER_WALK_MALE && o->Target->CurrentAction<=PLAYER_RUN_RIDE_WEAPON) || 
                       (o->Target->CurrentAction>=PLAYER_ATTACK_SKILL_SWORD1 && o->Target->CurrentAction<=PLAYER_ATTACK_SKILL_SWORD5 ) 
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
					   || (o->Target->CurrentAction==PLAYER_RAGE_UNI_RUN || o->Target->CurrentAction==PLAYER_RAGE_UNI_RUN_ONE_RIGHT)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
					   )
                    {
                        o->SubType  = 1;
                        o->LifeTime = min ( 20, o->LifeTime );
                        Vector( 0.f, 0.f, 0.f, o->Velocity );
                        VectorCopy ( o->Position, o->StartPosition );
                    }
                }
                break;
			case BITMAP_FLARE_RED:
				o->Scale = sinf(o->LifeTime*10.f*(Q_PI/180.f))*3.f;
				HandPosition(o);
				break;
			case BITMAP_CLUD64:
				{
					if(o->SubType == 0)
					{
						Luminosity = (float)(o->LifeTime)/10.f;
						Vector(Luminosity, Luminosity, Luminosity, o->Light);
						o->Gravity -= 0.01f;
						o->Position[2] -= o->Gravity;
						o->Scale -= 0.03f;
						o->Alpha -= 0.05f;
					}
					else if(o->SubType == 1 || o->SubType == 2)
					{
						o->Scale += 0.5f;

						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;

						if(o->Light[0] <= 0.05f)
							o->Live = false;
					}
					else if(o->SubType == 3)
					{
						o->Scale += 0.08f;

						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;

						if(o->Light[0] <= 0.05f)
							o->Live = false;
					}
					else if(o->SubType == 5)
					{	
						o->Scale += 0.08f;
						
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
						
						if(o->Light[0] <= 0.05f)
							o->Live = false;
					}
					else if(o->SubType == 6)	// ◎
					{
						if (o->LifeTime < 10)
						{
							o->Alpha -= 0.2f;
						}
						else if (o->Alpha < 1.0f)
						{
							o->Alpha += (rand()%2+3)*0.1f;
						}
						else
						{
							o->Alpha = 1.0f;
						}

						if (o->Alpha < 0.1f)
							o->Live = false;
     					Luminosity = o->Alpha;
    					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
						if (o->Scale > 0)
						{
							o->Scale -= (rand()%5+55)*0.001f;
						}
						else if (o->Scale < 0.1f)
						{
							o->Live = false;
						}
						o->Position[2] += o->Gravity;
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorCopy(o->Target->Position, o->StartPosition);
						VectorAdd(o->Position, o->StartPosition, o->Position);
					}
					else if( o->SubType == 7 )		// 썬더네이핀 공격 이팩트 (연기)
					{
						if (o->LifeTime < 10)
						{
							o->Alpha -= 0.1f;
							VectorScale( o->Light, 0.9f, o->Light );
						}
						else if (o->Alpha < 1.0f)
						{
							o->Alpha += (float)(rand()%20)*0.005f;
						}

						VectorAdd( o->Position, o->Velocity, o->Position );
						o->Scale += ((float)(rand()%10)*0.01f);
					}
					else if( o->SubType == 8 )		// 썬더네이핀 공격 이팩트 (연기)
					{
						if (o->LifeTime < 10)
						{
							o->Alpha -= 0.1f;
							VectorScale( o->Light, 0.9f, o->Light );
						}
						else if (o->Alpha < 1.0f)
						{
							o->Alpha += (float)(rand()%20)*0.005f;
						}

						VectorAdd( o->Position, o->Velocity, o->Position );
						o->Scale += ((float)(rand()%10)*0.01f);
					}
#ifdef ADD_SOCKET_ITEM
					else if( o->SubType == 9 )
					{
						o->Scale *= 0.98f+((float)(rand()%20-10)*0.002f);

						o->Light[0] *= 0.95f+((float)(rand()%20-10)*0.002f);
						o->Light[1] *= 0.95f+((float)(rand()%20-10)*0.002f);
						o->Light[2] *= 0.95f+((float)(rand()%20-10)*0.002f);
						o->Alpha *= 0.95f+((float)(rand()%20-10)*0.002f);
					}
#endif // ADD_SOCKET_ITEM
#ifdef ASG_ADD_SUMMON_RARGLE
					else if (o->SubType == 10)
					{
						o->Scale -= (rand()%5+15)*0.0016f;
						o->Position[2] += o->Gravity*10.f;
						
						if (o->Scale < 0 || !o->Target->Live)
							o->Live = false;
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorCopy(o->Target->Position, o->StartPosition);
						VectorAdd(o->Position, o->StartPosition, o->Position);
					}
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef LDK_ADD_PC4_GUARDIAN
					if(o->SubType == 11)
					{
						Luminosity = (float)(o->LifeTime)/10.f;
						Vector(Luminosity, Luminosity, Luminosity, o->Light);
						o->Gravity -= 0.01f;
						o->Position[2] -= o->Gravity;
						o->Scale -= 0.03f;
						o->Alpha -= 0.05f;
					}
#endif //LDK_ADD_PC4_GUARDIAN
				}
				break;

//////////////////////////////////////////////////////////////////////////

#ifdef PBG_ADD_LITTLESANTA_NPC
			case BITMAP_LIGHT+3:
				{
					if(o->SubType == 0)
					{
						float _Angle = (o->Velocity[0]+o->LifeTime)*0.1f;
						o->Position[0] = o->StartPosition[0] + sinf(_Angle)*35.f;
						o->Position[1] = o->StartPosition[1] - cosf(_Angle)*35.f;
						o->Position[2] += o->Gravity*0.1f;
						o->Scale -= 0.001f;
					}
#ifdef PJH_ADD_PANDA_PET
					else if(o->SubType == 1)
					{
						o->Scale = sinf(o->LifeTime*2.f*(Q_PI/180.f));
					}
#endif //PJH_ADD_PANDA_PET
				}
				break;
#endif // PBG_ADD_LITTLESANTA_NPC
	
			case BITMAP_TWINTAIL_WATER:
				{
					if (o->SubType == 0 || o->SubType == 1)
					{
						o->Scale -= 0.013f;
					}
					else if (o->SubType == 2)
					{
						o->Scale -= 0.026f;
					}

					o->Position[2] += o->Gravity;
					o->Alpha -= 0.05f;
					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;
				}
				break;
    		case BITMAP_SMOKE:
				switch(o->SubType)
				{
				case 0:
     				Luminosity = (float)(o->LifeTime)/8.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
				case 36:
                    if( o->LifeTime<23 )
                    {
						o->Gravity += (0.1f*o->Scale);
						o->Position[2] += o->Gravity;
						o->Position[1] += o->Gravity;
     					o->Scale -= 0.08f;
					}
					else
						VectorAdd ( o->Target->Position, o->StartPosition, o->Position );
                    o->Rotation += 0.01f;
					break;
				case 37:
					o->Position[2] -= o->TurningForce[2]*0.8f;
					o->Position[1] -= o->TurningForce[1]*0.8f;
     				o->Scale += 0.08f;
                    o->Rotation += 0.01f;
					o->Light[0] /= 1.03f;
					o->Light[1] /= 1.03f;
					o->Light[2] /= 1.03f;
					break;
				case 38:
					o->Scale += 0.09f;
					o->Light[0] /= 1.04f;
					o->Light[1] /= 1.04f;
					o->Light[2] /= 1.04f;
					break;
				case 33:
					Luminosity = (float)(o->LifeTime)/8.f;
    				Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity,o->Light);
     				VectorScale(o->Velocity,0.001f,o->Velocity);
     				o->Scale += (0.01f*2);
					o->Position[2] +=o->Scale;
					break;
				case 32:
					Luminosity = (float)(o->LifeTime)/8.f;
    				Vector(Luminosity*0.8f,Luminosity*0.8f,Luminosity,o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.1f;
					break;
				case 23:	//. 일반(녹색)
					Luminosity = (float)(o->LifeTime)/8.f;
					Vector(Luminosity*0.1f,Luminosity,Luminosity*0.6f,o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
				case 3:
					Luminosity = (float)(o->LifeTime)/8.f;
    				Vector(Luminosity*0.8f,Luminosity*0.8f,Luminosity,o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.1f;
					break;
				case 11:
				case 14:
					Luminosity = (float)(o->LifeTime)/50.f;
    				Vector(Luminosity*o->TurningForce[0],Luminosity*o->TurningForce[1],Luminosity*o->TurningForce[2],o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.05f;
					o->Position[2] -= 1.f;
					break;
				case 17:
     				Luminosity = (float)(o->LifeTime)/8.f;
					Luminosity = (Luminosity > 1.0f ? 1.0f - Luminosity : Luminosity);
    				Vector(Luminosity*o->TurningForce[0],Luminosity*o->TurningForce[1],Luminosity*o->TurningForce[2],o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
                case 15:
     				Luminosity = (float)(o->LifeTime)/40.f;
					Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*0.5f,o->Light);
					o->Gravity += (rand()%20)/500.f;
                    o->Position[0] += (rand()%100-20)/100.f;
                    o->Position[1] += (rand()%100-20)/100.f;
					o->Position[2] += o->Gravity;
     				o->Scale += (rand()%10)/1000.f;
                    o->Rotation = o->Scale;
                    break;
				case 1:
					Luminosity = (float)(o->LifeTime)/50.f;
    				Vector(Luminosity*0.5f,Luminosity*1.f,Luminosity*0.8f,o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.05f;
					break;
				case 2:
     				Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity -= 0.1f;
      				o->Position[0] -= o->Gravity*0.2f;
					o->Position[2] += o->Gravity;
					o->Scale -= 0.01f;
					break;
                case 16:
     				Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity -= 0.1f;
      				o->Position[0] -= o->Gravity*0.2f;
					o->Position[2] += o->Gravity;
					o->Scale -= 0.01f;
                    break;
				case 12:
     				Luminosity = (float)(o->LifeTime)/40.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Scale += 0.05f;
					break;
				case 13:
     				Luminosity = (float)(o->LifeTime)/20.f;
    				Vector(Luminosity*1.f,Luminosity*0.5f,Luminosity*0.1f,o->Light);
					o->Scale += 0.09f;
					break;
				case 18:
     				Luminosity = (float)(o->LifeTime)/20.f;
    				Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,o->Light);
					o->Scale += 0.09f;
					break;
				case 4:
     				Luminosity = (float)(o->LifeTime)/8.f;
					Vector(Luminosity*120.f/255.f,Luminosity*100.7f/255.f,Luminosity*80.f/255.f,o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
                case 5:
     				Luminosity = (float)(o->LifeTime)/10.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity -= 0.1f;
      				o->Position[0] -= o->Gravity*(0.2f*(rand()%2+1));
					o->Position[2] += o->Gravity;
					o->Scale -= 0.01f;
                    break;
				case 6:
                    Luminosity = 0.6f;
                    o->LifeTime = 10;
				    o->Position[2] = o->Rotation + sinf((WorldTime+o->Gravity)/5000.0f)*20.0f;
                    o->Scale = sinf(((int)(o->Gravity+WorldTime)%1800*0.1f*(Q_PI/180.f)))*0.5f+1.8f;

					Vector(Luminosity*0.6f,Luminosity*0.5f,Luminosity*0.4f,o->Light);
					break;
				case 7:
					Luminosity = 1.0f;
					
     			    o->Scale += 0.03f;
                    o->Gravity += 1.0f;
                    o->Velocity[1] -= 0.1f;
                    o->Position[2] -= o->Gravity;
     			    if(o->LifeTime<5)
                    {
                        Luminosity = (float)(o->LifeTime)/8.f;
         			    o->Scale -= 0.1f;
                    }
					Vector(Luminosity*0.725f,Luminosity*0.572f,Luminosity*0.333f,o->Light);
					break;
                case 8: //  나태.
				    o->Gravity += 0.02f;
                    if ( o->LifeTime>5 )
                    {
				        o->Scale += o->Gravity;
				        o->Position[2] += o->Gravity*20.f;

     			        VectorScale(o->Velocity,1.05f,o->Velocity);
					    Luminosity = (float)(o->LifeTime)/24.f;
					    Vector(Luminosity,Luminosity,Luminosity,o->Light);
     				    VectorScale(o->Velocity,0.4f,o->Velocity);
                    }
                    else
                    {
				        Vector(o->Light[0]/2.0f,o->Light[1]/2.0f,o->Light[2]/2.0f,o->Light);
                        Vector ( 0.f, 0.f, 0.f, o->Velocity );
                    }
                    break;
				case 9:
     				Luminosity = (float)(o->LifeTime)/10.f;
					Vector(Luminosity*250.f/255.f,Luminosity*156.7f/255.f,0.f,o->Light);
					o->Gravity += 0.5f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.1f;
					break;
				case 10:    //  블러드캐슬의 연기
					Luminosity = (float)(o->LifeTime)/16.f;
    				Vector(Luminosity*1.f,Luminosity*0.1f,Luminosity*0.1f,o->Light);
     				o->Scale += 0.05f;
					break;
				case 19:
     				Luminosity = (float)(o->LifeTime)/40.f;
					Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*0.5f,o->Light);
					o->Gravity += (rand()%20)/500.f;
                    o->Position[0] += (rand()%100-20)/100.f;
                    o->Position[1] += (rand()%100-20)/100.f;
					o->Position[2] += o->Gravity;
     				o->Scale += (rand()%10)/1000.f;
                    o->Rotation = o->Scale;
					break;
				case 20:
     				Luminosity = (float)(o->LifeTime)/40.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity += (rand()%20)/500.f;
                    o->Position[0] += (rand()%100-20)/100.f;
                    o->Position[1] += (rand()%100-20)/100.f;
					o->Position[2] += o->Gravity;
     				o->Scale += (rand()%10)/1000.f;
                    o->Rotation = o->Scale;
					break;
				case 21:	//. 검은연기 박스 Fixed : 2004/11/05
     				Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
					o->Gravity -= 0.1f;
      				o->Position[0] -= o->Gravity*0.2f;
					o->Position[2] += o->Gravity;
					o->Scale -= 0.01f;
					break;
				case 22:	//. 붉은연기 박스 : 2004/11/26
					Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity*0.9f,Luminosity*0.5f,Luminosity*0.5f,o->Light);
					o->Gravity -= 0.1f;
      				o->Position[0] += o->Gravity*sinf(WorldTime)*0.05f;
					o->Position[2] += o->Gravity;
					o->Scale += 0.04f;
					break;
				case 24:	//. 일반(녹색, 스케일조정가능)
					Luminosity = (float)(o->LifeTime)/32.f;
					Vector(Luminosity*0.2f,Luminosity*0.5f,Luminosity*0.35f,o->Light);
					o->Gravity += 0.1f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
				case 25:
     				Luminosity = (float)(o->LifeTime)/20.f;
    				Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,o->Light);
					o->Scale += 0.09f;
					break;
				case 35:
					o->Light[0] /= 1.04f;
					o->Light[1] /= 1.04f;
					o->Light[2] /= 1.04f;
					o->Scale += 0.06f;
					break;
				case 34:
					o->Light[0] /= 1.04f;
					o->Light[1] /= 1.04f;
					o->Light[2] /= 1.04f;
					o->Scale += 0.06f;
					break;
				case 26:
     				Luminosity = (float)(o->LifeTime)/20.f;
    				Vector(o->Light[0]*Luminosity*1.f,o->Light[1]*Luminosity*1.f,o->Light[2]*Luminosity*1.f,o->Light);
					o->Scale += 0.15f;
					break;
				case 27:
					Vector(o->Light[0]*0.92f,o->Light[1]*0.92f,o->Light[2]*0.92f,o->Light);
					o->Scale -= 0.05f;
					break;
				case 28:
					o->Position[2] += o->Gravity;
					if(o->LifeTime >= 9) o->Scale -= 0.4f;
					else o->Scale *= 1.2f;
					Vector(o->Light[0]*0.92f,o->Light[1]*0.92f,o->Light[2]*0.92f,o->Light);
					break;
				case 29:	
					o->Position[2] += o->Gravity;
					if(o->LifeTime >= 9) o->Scale -= 0.4f;
					else o->Scale *= 1.2f;
					Vector(1.0f,1.0f,1.0f,o->Light);
					break;
				case 30:	
					o->Scale += 0.15f;
					o->Position[2] += o->Gravity;
					Vector(1.0f,3.0f,1.0f,o->Light);
					break;
				case 31:	
					o->Scale -= 0.05f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					if(o->Scale <= 0.0f && o->Light[2] <= 0.0f)
						o->Live = false;
					break;	
				case 40:
					{
						Luminosity = (float)(o->LifeTime)/50.f;
    					Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*1.0f,o->Light);
     					VectorScale(o->Velocity,0.4f,o->Velocity);
     					o->Scale += 0.05f;
					}
					break;
				case 41:
					{
						Luminosity = (float)(o->LifeTime)/50.f;
    					Vector(Luminosity*0.5f,Luminosity*1.f,Luminosity*0.8f,o->Light);
     					VectorScale(o->Velocity,0.4f,o->Velocity);
     					o->Scale += 0.05f;
						o->Gravity += 0.2f;
						o->Position[2] += o->Gravity;
					}
					break;
				case 42:	// case 4:와 동일하지만 o->Light 세팅 부분만 틀림(버서커 형광색).
					Luminosity = (float)(o->LifeTime) / 8.f;
					Vector(Luminosity*127.f/255.f, Luminosity*255.f/255.f,
						Luminosity*200.f/255.f, o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.05f;
					break;
				case 43:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;
     					o->Scale += 0.08f;
					}
					break;
				case 44:
					{
						if(o->LifeTime >= 30)
						{
							o->Light[0] *= 1.07f;
							o->Light[1] *= 1.07f;
							o->Light[2] *= 1.07f;
						}
						else
						{
							o->Light[0]  /= 1.07f;
							o->Light[1]  /= 1.07f;
							o->Light[2]  /= 1.07f;
						}
						o->Scale += 0.02f;
						o->Position[2] += 0.8f;
						o->Rotation += o->Gravity/50.0f;
					}
					break;
				case 45:
					{
						o->Light[0]  /= 1.1f;
						o->Light[1]  /= 1.1f;
						o->Light[2]  /= 1.1f;
					}
					break;
				case 46:
					{
						o->Light[0]  /= 1.02f;
						o->Light[1]  /= 1.02f;
						o->Light[2]  /= 1.02f;

						if(o->Light[0] <= 0.05f)
							o->LifeTime = 0;
							
						o->Gravity -= 0.05f;
  						o->Position[2] += (o->Scale + o->Gravity)*1.5f;
						o->Scale += o->Scale/100.0f;
					}
					break;
				case 47:
					{
						o->Light[0]  /= 1.02f;
						o->Light[1]  /= 1.02f;
						o->Light[2]  /= 1.02f;
						
						if(o->Light[0] <= 0.05f)
							o->LifeTime = 0;

						o->Scale += 1.0f;
					}
					break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case 59:
     				Luminosity = (float)(o->LifeTime)/40.f;
    				Vector(Luminosity*0.9f,Luminosity*0.9f,Luminosity*0.9f,o->Light);
					o->Scale += 0.09f;
					break;
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case 48:
     				Luminosity = (float)(o->LifeTime)/40.f;
    				Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity*0.4f,o->Light);
					o->Scale += 0.09f;
					break;
				case 49:
					{
						if (o->LifeTime < 100)
						{
							o->Light[0]  *= 0.97f;
							o->Light[1]  *= 0.97f;
							o->Light[2]  *= 0.97f;
						}
						else
						{
							o->Light[0]  *= 1.03f;
							o->Light[1]  *= 1.03f;
							o->Light[2]  *= 1.03f;
						}

						if(o->Light[0] <= 0.01f)
							o->LifeTime = 0;
						
     					VectorScale(o->Velocity,0.001f,o->Velocity);
     					o->Scale += (rand()%15+4)*(0.001f);
						o->Position[2] += o->Scale*6.0f;
						o->Rotation += (rand()%10+10)*0.1f*o->Angle[0];
					}
					break;
				case 50:
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 0.5f)
					{
						o->Alpha += (rand()%3+2)*0.1f;
					}
					else
					{
						o->Alpha = 0.5f;
					}
					if (o->Alpha < 0.1f)
					{
						o->Live = false;
					}
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Scale += 0.02f;
					o->Position[2] += o->Gravity;
					break;
				case 51:
     				Luminosity = (float)(o->LifeTime)/20.f;
					VectorScale(o->TurningForce, Luminosity, o->Light);
					o->Scale += 0.09f;
					break;
#ifdef CSK_RAKLION_BOSS
				case 52:
					{
						VectorAdd(o->Position, o->Velocity, o->Position);
						o->Scale += 0.06f;
						o->Light[0] /= 1.07f;
						o->Light[1] /= 1.07f;
						o->Light[2] /= 1.07f;
					}
					break;
#endif // CSK_RAKLION_BOSS	
#ifdef PJH_GIANT_MAMUD
				case 53:
					Luminosity = (float)(o->LifeTime)/50.f;
    				Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.05f;
					break;
#endif //#ifdef PJH_GIANT_MAMUD
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
				case 54:
					{
						o->Light[0]  /= 1.02f;
						o->Light[1]  /= 1.02f;
						o->Light[2]  /= 1.02f;

						if(o->Light[0] <= 0.05f)
							o->LifeTime = 0;
							
						o->Gravity -= 0.05f;
  						o->Position[2] += (o->Scale + o->Gravity)*1.5f;
						o->Scale += o->Scale/100.0f;
					}
					break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				case 55:
					{
						o->Gravity += 0.1f;
						o->Position[2] += o->Gravity;
     					o->Scale += 0.05f;

						o->Light[0] /= 1.08f;
						o->Light[1] /= 1.08f;
						o->Light[2] /= 1.08f;
					}
					break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef ASG_ADD_SUMMON_RARGLE
				case 56:	// case 1:과 동일하나 색상만 다름.
					Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity*0.5f,Luminosity*0.1f,Luminosity*0.8f,o->Light);
					VectorScale(o->Velocity,0.4f,o->Velocity);
					o->Scale += 0.05f;
					break;
				case 57:	//case 24:와 동일하나 색상만 다름.
					Luminosity = (float)(o->LifeTime)/32.f;
					Vector(Luminosity*0.5f,Luminosity*0.1f,Luminosity*0.8f,o->Light);
					o->Gravity += 0.1f;
					o->Position[2] += o->Gravity;
					o->Scale += 0.05f;
					break;
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
				case 58:
					Luminosity = (float)(o->LifeTime)/50.f;
    				Vector(Luminosity*o->StartPosition[0],Luminosity*o->StartPosition[1],
						Luminosity*o->StartPosition[2],o->Light);
     				VectorScale(o->Velocity,0.4f,o->Velocity);
     				o->Scale += 0.05f;
					break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_PKFIELD
				case 60:
					{
						Luminosity = (float)(o->LifeTime)/50.f;
						Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity*0.4f,o->Light);
						o->Gravity -= 0.1f;
						o->Position[0] += o->Gravity*sinf(WorldTime)*0.05f;
						o->Position[2] += o->Gravity;
						o->Scale += 0.04f;
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_PKFIELD
				case 61:
					{
						Luminosity = (float)(o->LifeTime)/8.f;
						o->Gravity += 0.2f;
						o->Position[2] += o->Gravity;
     					o->Scale += 0.05f;
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_PKFIELD
				case 62:
					{
						Luminosity = (float)(o->LifeTime)/50.f;
    					//Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.1f,o->Light);
						Vector(Luminosity*0.9f,Luminosity*0.9f,Luminosity*0.9f,o->Light);
     					VectorScale(o->Velocity,0.001f,o->Velocity);
     					o->Scale += 0.03f;
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_PKFIELD
				case 63:
					{
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
     					o->Scale += 0.5f;
						o->Velocity[1] += 1.5f;
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT	
				case 64: // 방향성 연기 (독가스 효과)
					{
						Luminosity = (float)(o->LifeTime)/24.f;
						Vector(Luminosity*0.1f,Luminosity*0.7f,Luminosity*0.4f,o->Light);
						
						MovePosition(o->Position,o->Angle,o->Velocity);
						
						o->Rotation += 0.05f;//(rand()%100)/100.f;
						
						o->Gravity -= 0.1f;
						o->Position[0] += o->Gravity*sinf(WorldTime)*0.05f;
						o->Position[2] += o->Gravity;
						
						o->Scale += (float)(rand()%15+15)*0.01f;
						
						if(16 > o->LifeTime)
						{
							o->Alpha -= 0.1f;
						}
					}
					break;
				case 65:
					{
						Luminosity = (float)(o->LifeTime)/40.f;
						Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity*0.4f,o->Light);
						o->Gravity -= 0.05f;
						o->Position[0] += o->Gravity*sinf(WorldTime)*0.05f;
						o->Position[2] += o->Gravity;
					}
					break;
#endif // LDS_ADD_MAP_EMPIREGUARDIAN4_MAPEFFECT
#ifdef YDG_ADD_DOPPELGANGER_NPC
				case 66:
					{
						o->Light[0]  /= 1.02f;
						o->Light[1]  /= 1.02f;
						o->Light[2]  /= 1.02f;
						
						if(o->Light[0] <= 0.05f)
							o->LifeTime = 0;
						
						o->Gravity -= 0.05f;
						o->Position[0] += o->TurningForce[0] * (o->Scale + o->Gravity)*1.0f;
						o->Position[1] += o->TurningForce[1] * (o->Scale + o->Gravity)*1.0f;
						o->Scale += o->Scale/100.0f;
					}
					break;
#endif	// YDG_ADD_DOPPELGANGER_NPC
#ifdef LDK_ADD_CS7_UNICORN_PET
				case 67:
					{
						o->Alpha -= 0.01f;
						
						if(o->Alpha < 0.1f)
						{
							o->Live = false;
						}
						o->Light[0] *= o->Alpha;
						o->Light[1] *= o->Alpha;
						o->Light[2] *= o->Alpha;

						o->Scale += 0.05f;
						o->Rotation += 0.01f;
					}
					break;
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef LDS_ADD_EFFECT_UNITEDMARKETPLACE
				case 68:		// 방향성 연기 Angle 값 넣으면 조정.
					{
						Luminosity = (float)(o->LifeTime)/40.f;
						Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity*0.4f,o->Light);
						o->Velocity[0] += (0.03f * o->Velocity[0]);
						o->Velocity[1] += (0.03f * o->Velocity[1]);
						o->Velocity[2] += (0.03f * o->Velocity[2]);

						MovePosition(o->Position,o->Angle,o->Velocity);

						if(30 > o->LifeTime)
						{
							o->Alpha -= 0.02f;
						}
					}
					break;
#endif // LDS_ADD_EFFECT_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
				case 69:
					Luminosity = (float)(o->LifeTime)/50.f;
					Vector(Luminosity*o->Angle[0],Luminosity*o->Angle[1],Luminosity*o->Angle[2],o->Light);	// o->Angle을 연기 원래 색상 저장 공간으로 이용.
					o->Gravity -= 0.04f;
					o->Position[0] += o->Gravity*sinf(WorldTime)*0.05f;
					o->Position[2] += o->Gravity; // 상하방향
					o->Scale += 0.04f;
					break;
#endif	// ASG_ADD_MAP_KARUTAN
				}
				break;
			case BITMAP_SMOKE+2:
				Luminosity = 1.f;
				if(o->LifeTime < 10) Luminosity -= (float)(10-o->LifeTime)*0.1f;
				Vector(Luminosity,Luminosity,Luminosity,o->Light);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(o->StartPosition,Matrix,Position);
				VectorCopy(o->Target->Position,TargetPosition);
				VectorAdd(TargetPosition,Position,o->Position);
				o->Angle[1] += 5.f;
				o->Scale -= 0.01f;
				break;
    		case BITMAP_SMOKE+3:
				switch(o->SubType)
				{
				case 0:
					if (o->TurningForce[0] > 0.1f) o->TurningForce[0] -= 0.015f;
					if (o->TurningForce[1] > 0.1f) o->TurningForce[0] -= 0.005f;
					//o->TurningForce[1] -= 0.001f;
     				Luminosity = (float)(o->LifeTime)/55.f;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.03f;
					o->Rotation += o->Angle[0];
					break;
				case 1:
     				Luminosity = (float)(o->LifeTime)/55.f;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Gravity += 0.1f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.03f;
					o->Rotation += o->Angle[0];
					break;
				case 2:
					if (o->TurningForce[1] > 0.1f) o->TurningForce[1] -= 0.005f;
					if (o->TurningForce[2] > 0.1f) o->TurningForce[2] -= 0.015f;
     				Luminosity = (float)(o->LifeTime)/55.f;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Gravity += 0.2f;
					o->Position[2] += o->Gravity;
     				o->Scale += 0.03f;
					o->Rotation += o->Angle[0];
					break;
				case 3:
					if(o->Light[0] <= 0.05f)
						o->Live = false;
  					o->Light[0] /= 1.012f;
  					o->Light[1] /= 1.012f;
  					o->Light[2] /= 1.012f;
     				o->Scale += 0.01f;
                    o->Rotation += o->Gravity/2.0f;

					if(o->Gravity <= 0.0f)
						o->Gravity = -o->Gravity;
					o->Position[2] += o->Gravity;
					break;
				case 4:
					if(o->Light[0] <= 0.05f)
						o->Live = false;
  					o->Light[0] /= 1.012f;
  					o->Light[1] /= 1.012f;
  					o->Light[2] /= 1.012f;
     				o->Scale += 0.01f;
                    o->Rotation += o->Gravity/2.0f;

					if(o->Gravity <= 0.0f)
						o->Gravity = -o->Gravity;
					o->Position[2] += o->Gravity;					
					break;
				}
				break;
			case BITMAP_SMOKELINE1:
			case BITMAP_SMOKELINE2:
			case BITMAP_SMOKELINE3:
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 5
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 0.7f)
					{
						o->Alpha += (rand()%5+2)*0.1f;
					}
					else
					{
						o->Alpha = 0.7f;
					}

					if (o->Alpha < 0.1f)
					{
						o->Live = false;
					}
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Scale += (rand()%3+6)*0.01f;//0.07f;
					o->Position[2] += o->Gravity;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType == 5)
					{
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorCopy(o->Target->Position, o->StartPosition);
						VectorAdd(o->Position, o->StartPosition, o->Position);
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				else if (o->SubType == 1)	// ◎
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%5+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
					{
						o->Live = false;
					}
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%10+10)*0.001f;//0.07f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->StartPosition, o->Position);
				}
#ifdef ADD_RAKLION_IRON_KNIGHT
#ifdef KJH_ADD_EG_MONSTER_KATO_EFFECT
				else if ( (o->SubType == 2) || (o->SubType == 3) )
#else // KJH_ADD_EG_MONSTER_KATO_EFFECT
				else if (o->SubType == 2)
#endif // KJH_ADD_EG_MONSTER_KATO_EFFECT
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 0.7f)
					{
						o->Alpha += (rand()%5+2)*0.1f;
					}
					else
					{
						o->Alpha = 0.7f;
					}

					if (o->Alpha < 0.1f)
					{
						o->Live = false;
					}
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					o->Scale += (rand()%3+6)*0.01f;
					o->Position[2] += o->Gravity;
				}
#endif	// ADD_RAKLION_IRON_KNIGHT
#ifdef LDK_ADD_CS7_UNICORN_PET
				else if( o->SubType == 4 )
				{
					o->Position[2] += o->Gravity;
					o->Rotation += 0.8f;
					o->Scale += 0.01f;

					o->Alpha -= 0.01f;

					if(o->Alpha < 0.1f)
					{
						o->Live = false;
					}
					o->Light[0] *= o->Alpha;
					o->Light[1] *= o->Alpha;
					o->Light[2] *= o->Alpha;
				}
#endif //LDK_ADD_CS7_UNICORN_PET
				break;
			case BITMAP_LIGHTNING_MEGA1:
			case BITMAP_LIGHTNING_MEGA2:
			case BITMAP_LIGHTNING_MEGA3:
				{
					switch( o->SubType )
					{
					case 0:
						{
							o->Alpha -= 0.15f;
							
							if (o->Alpha < 0.1f)
							{
								o->Live = false;
							}
							Luminosity = o->Alpha;
							Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
						}
						break;
					}
				}
				break;
			case BITMAP_FIRE_HIK1:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK1_MONO:
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 6
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType==6)
					{
						o->Rotation = 0.0f;
						o->Position[2] += o->Gravity*1.2f;
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				else if (o->SubType == 1)
				{
					if (o->LifeTime < 20)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%2+4)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 2)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+7)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				else if (o->SubType == 10)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 3)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+2)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 4)
				{
					if (o->LifeTime < 5)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					
					o->Scale += (rand()%3+8)*0.01f;
					if (o->LifeTime < 3)
					{
						o->Scale += (10 - o->LifeTime)*0.02f;
					}
					
					AngleMatrix(o->Angle, Matrix);
					vec3_t vMoveDir, vMoveVec;
					Vector(0.0f, -1.0f, 0.0f, vMoveDir);
					VectorRotate(vMoveDir, Matrix, vMoveVec);
					VectorScale(vMoveVec, 8.0f, vMoveVec);
					VectorAdd(o->Position, vMoveVec, o->Position);
					
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->Target->Position, o->Position);
					
					o->Rotation += 5.0f;
				}
#endif	// YDG_ADD_SKELETON_PET
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==5)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				break;
			case BITMAP_FIRE_HIK3:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK3_MONO:
#endif // CSK_ADD_MAP_ICECITY
				if (o->SubType == 0
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 6
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+7)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType==6)
					{
						o->Rotation = 0.0f;
						o->Position[2] += o->Gravity*1.2f;
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				else if (o->SubType == 1)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%2+6)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#ifdef CSK_ADD_MAP_ICECITY
				else if(o->SubType == 2)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+7)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_NEW_DUEL_NPC
				else if (o->SubType == 3)
				{
					if (o->LifeTime < 10)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}

					if (o->Alpha < 0.1f)
						o->Live = false;
     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+7)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if (o->SubType == 4)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%4+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += (o->Gravity);//*((float)o->LifeTime/39.0f));
					o->Rotation += 3.0f;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef YDG_ADD_SKELETON_PET
				else if (o->SubType == 5)
				{
					if (o->LifeTime < 4)
					{
						o->Alpha -= 0.1f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					
					o->Scale += (rand()%3+8)*0.01f;
					if (o->LifeTime < 3)
					{
						o->Scale += (10 - o->LifeTime)*0.02f;
					}
					
					AngleMatrix(o->Angle, Matrix);
					vec3_t vMoveDir, vMoveVec;
					Vector(0.0f, -1.0f, 0.0f, vMoveDir);
					VectorRotate(vMoveDir, Matrix, vMoveVec);
					VectorScale(vMoveVec, 8.0f, vMoveVec);
					VectorAdd(o->Position, vMoveVec, o->Position);
					
					VectorSubtract(o->Position, o->StartPosition, o->Position);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorAdd(o->Position, o->Target->Position, o->Position);
					
					o->Rotation += 5.0f;
				}
#endif	// YDG_ADD_SKELETON_PET
				break;
			case BITMAP_LIGHT+1:
				if(o->SubType == 2)
				{
					o->Scale *= 0.92f;
				}
				else if(o->SubType == 3)
				{
					o->Scale *= 1.3f;
					o->Light[0] *= 0.9f;
					o->Light[1] *= 0.9f;
					o->Light[2] *= 0.9f;
				}
				else if(o->SubType == 5)
				{
					o->LifeTime = 5;
					o->Light[0] *= 0.7f;
					o->Light[1] *= 0.7f;
					o->Light[2] *= 0.7f;
				}
				else if(o->SubType == 4)
				{
					
				}
				else
				{
					o->Position[2] -= o->Gravity;
					o->Position[1] -= 1.f;
					o->Gravity += 1.f;
					o->Scale *= 0.95f;
				}
				break;
			case BITMAP_BLOOD:
			case BITMAP_BLOOD+1:
				o->Frame = (12-o->LifeTime)/3;
				//o->Position[2] += o->Gravity;
				//o->Gravity -= 1.5f;
				VectorScale(o->Velocity,0.95f,o->Velocity);
				VectorAdd(o->Position,o->Velocity,o->Position);
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_SPARK:
				Luminosity = (float)(o->LifeTime)/16.f;
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				if (o->LifeTime < 0)
				{
					o->Live = false;
				}
				if (o->SubType==11)
				{
					Vector(Luminosity,Luminosity*0.3f,Luminosity*0.3f,o->Light);
				}
				else
#endif //LDS_ADD_EMPIRE_GUARDIAN
				if(o->SubType != 8)
					Vector(Luminosity,Luminosity,Luminosity,o->Light);
				o->Position[2] += o->Gravity;
				if (o->SubType == 5)
				{
					o->Gravity -= 0.3f;
				}
				else if (o->SubType == 6)
				{
					o->Gravity -= 0.3f;
					Vector(0.9f,0.9f,0.9f,o->Light);
				}
				else if(o->SubType == 8
#ifdef PJH_FIX_CAOTIC
					|| o->SubType == 10
#endif //PJH_FIX_CAOTIC
					)
				{
					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;
				}
				if(o->SubType == 7)
					o->Gravity -= 0.6f;
				else if(o->SubType == 9)
					o->Gravity -= 0.4f;
				else
					o->Gravity -= 2.f;

				Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
				if(o->Position[2] < Height)
				{
					o->Position[2] = Height;
					o->Gravity = -o->Gravity*0.6f;
					o->LifeTime -= 4;
				}
				VectorAdd(o->Position,o->Velocity,o->Position);
				if (o->SubType == 5 || o->SubType == 6)
				{
					vec3_t p;
					VectorSubtract(o->StartPosition, o->Target->Position, p);
					VectorCopy(o->Target->Position, o->StartPosition);
					VectorSubtract(o->Position, p, o->Position);
				}
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_SPARK+1:
				switch(o->SubType)
				{
				case 0:
                    o->Scale -= 0.5f;
      				if(o->Scale < 0.2f) o->Live = false;
					break;
				case 1:
                    o->Scale -= 2.f;
  	    			if(o->Scale < 0.2f) o->Live = false;
					break;
				case 2:
				    if(o->LifeTime<=0) o->Live = false;
                    if(o->LifeTime>5 && o->Target->Live==0) o->LifeTime=5;
                    
                    o->Light[0] = o->LifeTime/5.f;
                    o->Light[1] = o->LifeTime/5.f;
                    o->Light[2] = o->LifeTime/5.f;

                    o->Scale += 0.05f;
                    VectorAdd(o->StartPosition,o->Velocity,o->Position);
                    VectorCopy(o->Position,o->StartPosition);
					break;
				case 3:
					o->Scale *= 0.8f;
					break;
				case 4:
					if(o->LifeTime<=0) o->Live = false;
					if(o->LifeTime>5 && o->Target->Live==0) o->LifeTime=5;

					o->Light[0] = o->LifeTime/5.f;
					o->Light[1] = o->LifeTime/5.f;
					o->Light[2] = o->LifeTime/5.f;

					o->Scale += 0.08f;

					VectorAdd(o->StartPosition,o->Velocity,o->Position);
					VectorCopy(o->Position,o->StartPosition);
					
					vec3_t p;
					VectorSubtract(o->Target->Position, o->Target->StartPosition, p);
					VectorAdd(o->Position, p, o->Position);
					break;
                case 5:
					o->Scale *= 0.9f;
                    o->Light[0] /= 1.03f;
                    o->Light[1] /= 1.03f;
                    o->Light[2] /= 1.03f;
                    break;
				case 6:
					{

						if(o->LifeTime<=0) o->Live = false;
						o->Position[2] -= o->Gravity;
						o->Position[1] -= 1.f;
						o->Gravity += 0.1f;
						float fLight = (float)(rand()%10)/100.0f + 0.7f;
						Vector(o->Light[0], fLight, o->Light[2], o->Light);
					}
					break;
				case 7:
					{
						o->Scale -= 0.02f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
						o->Position[2] -= o->Gravity;
						float fLight = (float)(rand()%50)/100.0f + 0.2f;
						Vector(0.0f, fLight, 0.0f, o->Light);
					}
					break;
				case 8:
					{
					   o->Scale -= 0.5f;
      					if(o->Scale < 0.2f) o->Live = false;
					}
					break;
				case 9:
					{
						o->Velocity[0] -= 1.2f;
						o->Velocity[2] -= 1.0f;
						o->Scale -= 0.08f;
						if(o->LifeTime <= 0)
							o->Live = false;
						if(o->Scale < 0.2f)
							o->Live = false;
					}
					break;
				case 10:
					{
						o->Light[0] /= 1.08f;
						o->Light[1] /= 1.08f;
						o->Light[2] /= 1.08f;
						o->Scale -= 0.03f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 13:
				case 11:
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						o->Scale -= 0.04f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 12:
					{
						float fLight = rand()%2;
						Vector(fLight-0.6f, fLight-0.6f, fLight-0.8f, o->Light);
						o->Scale -= 0.04f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 14:
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						o->Scale -= 0.01f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 15:	//^ 펜릴 이펙트 관련
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 16:
				case 18:
					{
						float Height = RequestTerrainHeight ( o->Position[0], o->Position[1] );

						if(Height+2.0f >= o->Position[2])
							o->Frame = 88;

						if(o->Frame == 77)
						{
							o->Gravity *= 1.03f; 
							o->Position[2] -= o->Gravity;
						}
						else if(o->Frame == 88)
						{
							o->Gravity /= 1.2f;
							if(o->Gravity <= 0.1f)
							{
								o->Gravity = 0.1f;
								o->Frame = 99;
							}
							o->Position[2] += o->Gravity;
						}
						else if(o->Frame == 99)
						{
							o->Gravity *= 1.2f; 
							o->Position[2] -= o->Gravity;
						}

						o->Alpha -= 0.007f;
						if(o->Alpha <= 0.0f)
							o->Live = false;
					}
					break;
				case 17:
					{
						if(o->Position[2] >= o->StartPosition[2]+350.0f)
						{
							o->Light[0] /= 1.05f;
							o->Light[1] /= 1.05f;
							o->Light[2] /= 1.05f;
						}

						if(o->Light[0] <= 0.05f)
							o->Live = false;

						o->Position[0] += sinf(WorldTime*o->Rotation)*o->Gravity*0.3f;
						//o->Position[1] += sinf(WorldTime*o->Rotation)*o->Gravity*0.3f;
						o->Position[2] += o->Gravity;

					}
					break;
#ifdef CSK_LUCKY_SEAL
				case 19:
					{
						if(o->LifeTime <= 0) 
							o->Live = false;

						o->Position[2] += o->Gravity;
						o->Gravity += 0.1f;

						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
					}
					break;
#endif // CSK_LUCKY_SEAL
				case 20:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;

						if(o->Light[0] <= 0.05f)
							o->Live = false;

						o->Position[2] += (o->Gravity * 0.5f);
						o->Gravity -= 1.5f;

						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height+3.f)
						{
							o->Position[2] = Height+3.f;
							o->Gravity = -o->Gravity*0.3f;
							o->LifeTime -= 2;
						}
					}
					break;
					
#ifdef PRUARIN_EVENT07_3COLORHARVEST
				case 21:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;
					}
					break;
				case 22:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;
						
						if(o->Light[0] <= 0.05f)
							o->Live = false;
						
						o->Position[2] += (o->Gravity * 0.5f);
						o->Gravity -= 1.5f;
						
						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height+3.f)
						{
							o->Position[2] = Height+3.f;
							o->Gravity = -o->Gravity*0.3f;
							o->LifeTime -= 2;
						}
					}
					break;
#endif // PRUARIN_EVENT07_3COLORHARVEST
				case 23:
					{
						if (o->LifeTime < 10 && o->Rotation == 0)
						{
							VectorScale(o->Velocity, -1, o->Velocity);
							o->Rotation = 1;
						}
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						o->Scale -= 0.02f;
						o->Alpha -= 0.0001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
				case 24:
					{
						if (o->LifeTime < 10 && o->Rotation == 0)
						{
							VectorScale(o->Velocity, -1, o->Velocity);
							o->Rotation = 1;
						}

						if( o->LifeTime <= 10 )
						{
							o->Alpha -= 0.05f;
							o->Light[0] *= o->Alpha;
							o->Light[1] *= o->Alpha;
							o->Light[2] *= o->Alpha;
						}

						o->Scale -= 0.02f;
					}
					break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
				case 25:
					{
						if( o->LifeTime == 19 )
						{
							o->Velocity[0] -= 1.f;
							o->Velocity[1] -= 1.f;
							o->Velocity[2] -= 1.f;
						}

						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						o->Scale = sinf(o->LifeTime*(Q_PI/40.f));
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef CSK_ADD_MAP_ICECITY	
				case 26:
					{
						if(o->LifeTime <= 0) 
							o->Live = false;
						
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;
						
						o->Scale -= 0.05f;
						VectorAdd(o->StartPosition,o->Velocity,o->Position);
						VectorCopy(o->Position,o->StartPosition);
					}
					break;
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_FIRECRACKER_ITEM
				case 27:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;

						o->Scale /= 1.03f;
					}
					break;
				case 28:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;
						
						o->Scale /= 1.01f;

						if(o->Light[0] <= 0.03f)
							o->Live = false;
						
						o->Position[2] += (o->Gravity * 0.5f);
						o->Gravity -= 1.5f;
						
						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height+3.f)
						{
							o->Position[2] = Height+3.f;
							o->Gravity = -o->Gravity*0.3f;
							o->LifeTime -= 2;
						}
					}
					break;
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef PBG_ADD_PKFIELD
				case 29:
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;
						
						o->Scale /= 1.01f;
						
						if(o->Light[0] <= 0.03f)
							o->Live = false;
						
						o->Position[2] += (o->Gravity * 0.5f);
						o->Gravity -= 1.5f;
						
						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height+3.f)
						{
							o->Position[2] = Height+3.f;
							o->Gravity = -o->Gravity*0.3f;
							o->LifeTime -= 2;
						}
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				case 30:
					{
						
						if( o->LifeTime <= 45 )
						{
							o->Velocity[0] += sinf(Q_PI / 180.0f * float(rand()%360));
 							o->Velocity[1] += cosf(Q_PI / 180.0f * float(rand()%360));
						}

						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;

						o->Scale -= 0.01f;

						o->LifeTime -= 1;
 						if(o->LifeTime <= 0) o->Live = false;
					}
					break;
#endif //LDK_ADD_GAMBLERS_WEAPONS
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
				case 31:
					{
						o->Position[2] += o->Gravity;
						o->Gravity -= 2.f;

						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height)
						{
							o->Position[2] = Height;
							o->Gravity = -o->Gravity*0.6f;
							o->LifeTime -= 4;
						}
						VectorAdd(o->Position,o->Velocity,o->Position);
					}
					break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
                }
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_SPARK+2:
				{
					if(o->SubType == 0)
					{
						o->Frame = (16 - o->LifeTime) / 4;
					}
					else if(o->SubType == 1)
					{	
						if(o->LifeTime%3 == 0)
						{
							vec3_t vPos;
							VectorCopy(o->Position, vPos);
							vPos[0] += (float)(rand()%100 - 50);
							vPos[1] += (float)(rand()%100 - 50);
							vPos[2] += (float)(rand()%100 - 50);
							CreateBomb(vPos, true);
						}
					}
#ifdef ASG_ADD_SKILL_BERSERKER
#ifdef CSK_FIX_JACK_PARTICLE
					else if((o->SubType == 2 || o->SubType == 3) && o->Target != NULL)
#else // CSK_FIX_JACK_PARTICLE
					if (o->SubType == 2 || o->SubType == 3)
#endif // CSK_FIX_JACK_PARTICLE	
					{
						o->Frame = (16 - o->LifeTime) / 4;
						
						// 플레이어 모델
						BMD* pModel = &Models[o->Target->Type];
						vec3_t vPos;
						
						switch (o->SubType)
						{
						case 2:
							// 플레이어 왼손
							pModel->TransformByObjectBone(vPos, o->Target, 37);
							break;
						case 3:
							// 플레이어 오른손
							pModel->TransformByObjectBone(vPos, o->Target, 28);
							break;
						}
						
						VectorCopy(vPos, o->Position);
					}
#endif	// ASG_ADD_SKILL_BERSERKER
				}
				break;


			case BITMAP_SMOKE+1:
            case BITMAP_SMOKE+4:
				Luminosity = (float)(o->LifeTime)/32.f;
				Vector(Luminosity,Luminosity,Luminosity,o->Light);
				o->Scale += 0.08f;
				//o->Angle[0] += 4.f;
				VectorAdd(o->Position,o->Velocity,o->Position);
				VectorScale(o->Velocity,0.9f,o->Velocity);
#ifdef LDS_ADD_EMPIRE_GUARDIAN
                if ( o->SubType == 6 )
				{
					o->Scale -= 0.05f;
					o->Position[2] += 4.0f;
				}
				else
#endif //LDS_ADD_EMPIRE_GUARDIAN
				if ( o->SubType!=5 )
				{
					o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + Bitmaps[o->Type].Height*o->Scale*0.5f;
				}
				break;

			case BITMAP_SHINY:
                if ( o->SubType==2 )
                {
					o->Rotation -= 12.f;

					o->Velocity[2] -= (float)(rand()%8)*0.05f;
					VectorAdd(o->Position,o->Velocity,o->Position);

                    o->Light[0] /= 1.05f;
                    o->Light[1] /= 1.05f;
                    o->Light[2] /= 1.05f;

                    CreateSprite( BITMAP_LIGHT, o->Position, o->Scale/1.5f, o->Light, NULL );
                }
				else if(o->SubType == 3)
				{
					o->Light[0] /= 1.05f;
					o->Light[1] /= 1.05f;
					o->Light[2] /= 1.05f;
					o->Scale -= 0.04f;
					o->Alpha -= 0.001f;
					if(o->Alpha <= 0.0f) o->Live = false;
				}
				else if(o->SubType == 4)
				{
					o->Rotation += 20.f;
					o->Position[2] += (o->Gravity * 0.5f);

					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;

					o->Gravity -= 1.5f;
					o->Scale -= 0.01f;

					VectorAdd(o->Position,o->Velocity,o->Position);
				}
				else if(o->SubType == 5)
				{
					o->Rotation += 10.f;
					o->Position[2] += o->Gravity;

					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;

					o->Scale -= 0.01f;
				}
#ifdef YDG_ADD_FIRECRACKER_ITEM
				else if(o->SubType == 6)
				{
					o->Light[0] /= 1.01f;
					o->Light[1] /= 1.01f;
					o->Light[2] /= 1.01f;
					
					if (o->Light[0] < 0.2f)
					{
						o->Live = false;
					}

					o->StartPosition[0] /= 1.01f;
					if (o->LifeTime < 60 && rand()%2 == 0)
					{
						o->Scale = 0;
					}
					else
					{
						o->Scale = o->StartPosition[0];
					}
					
					if (o->LifeTime < 60)
					{
						o->Velocity[0] /= 1.04f;
						o->Velocity[1] /= 1.04f;
						o->Velocity[2] /= 1.04f;
						
						o->Position[2] -= (o->Gravity * 0.1f);
						o->Gravity += 0.5f;
					}
				}
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef LDK_ADD_RUDOLPH_PET
				else if(o->SubType == 7)
                {
                    o->Scale = sinf(o->LifeTime*2.f*(Q_PI/180.f));
                }
#endif //LDK_ADD_RUDOLPH_PET
#ifdef YDG_ADD_SANTA_MONSTER
				else if(o->SubType == 8)
				{
					o->Light[0] /= 1.01f;
					o->Light[1] /= 1.01f;
					o->Light[2] /= 1.01f;
					
					if (o->Light[0] < 0.2f)
					{
						o->Live = false;
					}

					o->StartPosition[0] /= 1.01f;
					if (o->LifeTime < 60 && rand()%2 == 0)
					{
						o->Scale = 0;
					}
					else
					{
						o->Scale = o->StartPosition[0];
					}
					
					if (o->LifeTime < 60)
					{
						o->Velocity[0] /= 1.04f;
						o->Velocity[1] /= 1.04f;
						o->Velocity[2] /= 1.04f;
						
						o->Position[2] -= (o->Gravity * 0.1f);
						o->Gravity += 0.5f;
					}
				}
#endif	// YDG_ADD_SANTA_MONSTER
#ifdef YDG_ADD_FIRECRACKER_ITEM
				else if(o->SubType == 9)
				{
					o->Light[0] /= 1.01f;
					o->Light[1] /= 1.01f;
					o->Light[2] /= 1.01f;
					
					if (o->Light[0] < 0.2f)
					{
						o->Live = false;
					}

					o->StartPosition[0] /= 1.01f;
					if (o->LifeTime < 60 && rand()%2 == 0)
					{
						o->Scale = 0;
					}
					else
					{
						o->Scale = o->StartPosition[0];
					}
					
					if (o->LifeTime < 60)
					{
						o->Velocity[0] /= 1.04f;
						o->Velocity[1] /= 1.04f;
						o->Velocity[2] /= 1.04f;
						
						o->Position[2] -= (o->Gravity * 0.1f);
						o->Gravity += 0.5f;

						if (rand()%5==0)
						{
							Position[0] = o->Position[0] + rand()%40-20;
							Position[1] = o->Position[1] + rand()%40-20;
							Position[2] = o->Position[2];
							Vector(0.8f+(rand()%200)*0.001f, 0.5f+(rand()%200)*0.001f, 0.1f+(rand()%100)*0.001f, Light);
// 							Vector(1.0f,0.7f,0.2f, Light);
							CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 8 );
						}
					}
				}
#endif	// YDG_ADD_FIRECRACKER_ITEM
                else
                {
                    o->Scale = sinf(o->LifeTime*10.f*(Q_PI/180.f));
				    if(o->SubType == 1)
				    {
					    o->Scale *= 0.75f;
					    o->Rotation -= 12.f;
				    }
                }
				break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
			case BITMAP_CHERRYBLOSSOM_EVENT_PETAL:
				{
					if( o->SubType == 0 )
					{
						o->Rotation = (float)(rand()%360);
						o->Scale = sinf(o->LifeTime*(Q_PI/180.f));
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
					}
					else if( o->SubType == 1 )
					{
						if( o->LifeTime == 69 )
						{
							o->Velocity[0] -= 1.f;
							o->Velocity[1] -= 1.f;
							o->Velocity[2] -= 1.f;
						}

						o->Light[0] /= 1.01f;
						o->Light[1] /= 1.01f;
						o->Light[2] /= 1.01f;

						//o->Scale = sinf(o->LifeTime*(Q_PI/240.f));
						o->Alpha -= 0.001f;
						//o->Rotation = (float)(rand()%360);
						if(o->Alpha <= 0.0f) o->Live = false;
					}
				}
				break;
			case BITMAP_CHERRYBLOSSOM_EVENT_FLOWER:
				{
					if( o->SubType == 0 )
					{
						o->Scale = sinf(o->LifeTime*(Q_PI/180.f));
						o->Alpha -= 0.002f;
						if(o->Alpha <= 0.0f) o->Live = false;

						//o->Light[0] /= 1.045f;
						//o->Light[1] /= 1.045f;
						//o->Light[2] /= 1.045f;
						
						o->Position[2] += (o->Gravity * 0.5f);
						o->Gravity -= 1.5f;

						o->Rotation = (float)(rand()%360);
						
						Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
						if(o->Position[2] < Height+3.f)
						{
							o->Position[2] = Height+3.f;
							o->Gravity = -o->Gravity*0.3f;
							o->LifeTime -= 2;
						}
					}
				}
				break;
#endif //CSK_EVENT_CHERRYBLOSSOM
			case BITMAP_SHINY+1:
                if(o->SubType==99)
                {
                    VectorCopy(o->Target->Position,o->Position);
                    o->Position[2] += 50.f;
//                    o->Scale *= 0.75f;
                    o->Rotation -= 1.f;
                }
                else
                {
#ifdef CSK_EVENT_CHERRYBLOSSOM
					if(o->SubType==5)
					{
						VectorCopy( o->Target->Angle, o->Angle );
						o->Scale -= 0.06f;
						o->Position[2] += o->Gravity*10.f;
						//o->Light[0] = o->LifeTime/10.f;
						//o->Light[1] = o->Light[0];
						//o->Light[2] = o->Light[0];
					}
					else
#endif //CSK_EVENT_CHERRYBLOSSOM
					{
						o->Scale = sinf(o->LifeTime*5.f*(Q_PI/180.f))*5.f;
						//Luminosity = (float)(o->LifeTime)/36.f;
						//Vector(Luminosity*0.6f,Luminosity*0.8f,Luminosity,o->Light);
						if(o->SubType>=2) {
							o->Scale *= 0.75f;
							o->Rotation -= 12.f;
						}
						HandPosition(o);
					}
                }
				break;
			case BITMAP_SHINY+2:
   				o->Scale = sinf(o->LifeTime*10.f*(Q_PI/180.f))*3.f;
				HandPosition(o);
				break;
			case BITMAP_SHINY+4:
                if(o->SubType==0)
                {
   				    o->Scale = sinf(o->LifeTime*10.f*(Q_PI/180.f))*3.f+2.f;
                    Luminosity = (float)(o->LifeTime)/5.f;
				    Vector(Luminosity,Luminosity,Luminosity,o->Light);
                }
                else if(o->SubType==1)
                {
   				    o->Scale = sinf( o->Gravity*(Q_PI/180.f))*3.f+1.5f;
                    o->Gravity += (15-o->LifeTime)*6.f;
                    if ( o->Gravity>90.f )
                    {
                        o->Gravity = 90.f;
                    }
                    if ( o->LifeTime<6 )
                    {
                        o->Light[0] *= 0.5f;
                        o->Light[1] *= 0.5f;
                        o->Light[2] *= 0.5f;
                    }
                }
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==2)
				{
					o->Scale *= 1.1f;
					VectorScale(o->Light, 0.9f, o->Light);
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				break;
			case BITMAP_SHINY+6:
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				if(o->SubType == 0)
#endif //LDK_ADD_GAMBLERS_WEAPONS
				{
					o->Rotation += 5.f;
					o->Scale -= 0.02f;
					o->Alpha -= 0.001f;
					if (o->Alpha <= 0.0f)
						o->Live = false;
					o->Position[2] -= o->Gravity;
					float fLight = (float)(rand()%10)/100.0f - 0.05f;
					o->Light[0] += fLight;
					o->Light[1] += fLight;
					o->Light[2] += fLight;
				}
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				else if(o->SubType == 1)
				{
					o->Scale -= 0.01f;
					o->Rotation -= 5.f;
				}
#endif //LDK_ADD_GAMBLERS_WEAPONS

				break;
			case BITMAP_PIN_LIGHT:
				{
					o->Scale -= 0.02f;
					o->Alpha -= 0.001f;
					if (o->Alpha <= 0.0f)
						o->Live = false;
#ifdef PJH_SEASON4_FIX_MULTI_SHOT
					if(o->SubType == 1)
					{

						float Matrix[3][4];
						vec3_t p;
						Vector(0.f,-150.f,0.f,p);
						AngleMatrix(o->Angle,Matrix);
						VectorRotate(o->Position,Matrix,o->Position);
					}
					else
#endif //PJH_SEASON4_FIX_MULTI_SHOT
					{
						o->Position[2] -= o->Gravity;
					}
					float fLight = (float)(rand()%10)/100.0f - 0.05f;
					o->Light[0] += fLight;
					o->Light[1] += fLight;
					o->Light[2] += fLight;
				}
				break;
#ifdef ASG_ADD_SKILL_BERSERKER
			case BITMAP_ORORA:	// 플레이어 손목 주변을 돌며 커지면서 사라지는 이펙트.
				{
					float fScale, fLight;	// 각 증감값.
					// 플레이어 모델
					BMD* pModel = &Models[o->Target->Type];
					vec3_t vRelativePos, vPos;
					Vector(0.f, 0.f, 0.f, vRelativePos);

					switch (o->SubType)
					{
					case 0:
						fScale = 0.01f;
						fLight = 1.05f;
						o->Rotation += 5.f;
						// 플레이어 왼손
						pModel->TransformByObjectBone(vPos, o->Target, 37);
						break;
					case 1:
						fScale = 0.01f;
						fLight = 1.05f;
						o->Rotation -= 5.f;
						// 플레이어 오른손
						pModel->TransformByObjectBone(vPos, o->Target, 28);
						break;
					case 2:
						fScale = 0.04f;
//						fLight = 1.12f;
						fLight = 1.33f;
						o->Rotation += 20.f;
						// 플레이어 왼손
						pModel->TransformByObjectBone(vPos, o->Target, 37);
						break;
					case 3:
						fScale = 0.04f;
//						fLight = 1.12f;
						fLight = 1.33f;
						o->Rotation -= 20.f;
						// 플레이어 오른손
						pModel->TransformByObjectBone(vPos, o->Target, 28);
						break;
					}

					VectorCopy(vPos, o->Position);
					o->Scale += fScale;
					if (o->Scale >= 0.8f)	// 투명해지는 시점.
					{
						o->Light[0] /= fLight;
						o->Light[1] /= fLight;
						o->Light[2] /= fLight;
					}
				}
				break;
#endif	// ASG_ADD_SKILL_BERSERKER
			case BITMAP_SNOW_EFFECT_1:
			case BITMAP_SNOW_EFFECT_2:
				o->Rotation += 20.f;
				o->Position[2] += (o->Gravity * 0.5f);

				o->Light[0] /= 1.02f;
				o->Light[1] /= 1.02f;
				o->Light[2] /= 1.02f;

				o->Gravity -= 1.5f;
				if(o->LifeTime < 10)
					o->Scale += 0.01f;
				else
					o->Scale -= 0.01f;

				VectorAdd(o->Position,o->Velocity,o->Position);
				break;

			case BITMAP_DS_EFFECT:
				o->Rotation += 10.f;
				
				if(o->Target != NULL)
				{
					if(o->Target->CurrentAction != PLAYER_SANTA_2)
					{
						o->Live = false;
					}
				}
				break;
			case BITMAP_FIRECRACKER:
				o->Velocity[2] -= .5f;
				break;
            case BITMAP_SWORD_FORCE:
                o->Gravity += 0.01f;
                o->Scale += o->Gravity;
                Luminosity = (float)(o->LifeTime)/10.f;
				Vector(Luminosity,Luminosity,Luminosity,o->Light);
                break;
			case BITMAP_TORCH_FIRE:
				if(o->LifeTime <= 0)
				{
					o->Live = false;
				}
				else
				{
					o->Position[2] += o->Gravity;
					
					if(o->Position[2] >= 500.f)
					{
						o->Live = false;
					}
					if(o->LifeTime >= 20.f)
					{
						o->Scale -= 0.02f;
						if(o->Scale <= 0.02f)
						{
							o->Live = false;
						}
					}
				}
				break;
				
			case BITMAP_GHOST_CLOUD1:
			case BITMAP_GHOST_CLOUD2:
				{
					if(o->SubType == 0)
					{
						VectorAdd(o->Position,o->Velocity,o->Position);
						
						vec3_t vTemp;
						VectorSubtract(o->Position, o->StartPosition, vTemp);
						float fTemp = VectorLength(vTemp);
						
						float fSin = sinf(WorldTime*0.001f);
						if(fSin > 0.f)
							o->Rotation += 0.2f + (rand()%4*0.1f);
						else
							o->Rotation -= 0.2f + (rand()%4*0.1f);
						
						if(fTemp <= 20.f)
						{
							for(int i=0; i<3; ++i)
							{
								o->Light[i] = o->TurningForce[i] * 0.1f * (500 - o->LifeTime);
								if(o->Light[i] > o->TurningForce[i])
								{
									o->Light[i] = o->TurningForce[i];
								}
							}
						}
						else
						{
							o->Light[0] /= 1.02f;
							o->Light[1] /= 1.02f;
							o->Light[2] /= 1.02f;
						}
						
						if(fTemp > 500.0f)
						{
							o->Live = false;
						}
					}
				}
				break;
            case BITMAP_CLOUD:
                if ( o->SubType==6 )
                {
                    VectorAdd(o->Position,o->Velocity,o->Position);
                    o->Velocity[0] *= 0.95f;
                    o->Velocity[1] *= 0.95f;
                    o->Velocity[2] += 0.6f;
                    o->Position[0] += (float)(rand()%4-2);
                    o->Position[1] += (float)(rand()%4-2);
                    o->Position[2] += (float)(rand()%4-2)*0.8f;
                    o->Scale += 0.05f;
                    o->Light[0] = (float)(o->LifeTime)/50.f;
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                    o->Rotation += 1.f+rand()%2;
                }
				else if( o->SubType==8)
				{
					if(o->LifeTime <=0) {
						 o->Live = false;
					}
					else if(o->LifeTime > 150){
						if(o->Alpha < 1.0f)
							o->Alpha += 0.04;
						o->Position[2] += 1.f;
					}
					else {
						if(o->Alpha > 0.1f)
							o->Alpha -= 0.025f;
						o->Position[2] -= 0.5f;
					}
				}
				else if( o->SubType==9)
				{
					if(o->LifeTime <=0) {
						 o->Live = false;
					}
					else if(o->LifeTime > 150){
						if(o->Alpha < 1.0f)
							o->Alpha += 0.04;
						o->Position[2] += 1.f;
					}
					else {
						if(o->Alpha > 0.1f)
							o->Alpha -= 0.025f;
						o->Position[2] -= 0.5f;
					}
				}
				else if(o->SubType == 10)
				{
					VectorAdd(o->Position,o->Velocity,o->Position);
					o->TurningForce[0] = 1.5f;
					
					vec3_t vTemp;
					VectorSubtract(o->Position, o->StartPosition, vTemp);
					float fTemp = VectorLength(vTemp);

					if(fTemp <= 300.0f)
					{
						o->Light[0] += 0.01f;
						o->Light[1] += 0.01f;
						o->Light[2] += 0.01f;
						if(o->Light[0] >= 0.15f)
							Vector(0.15f, 0.15f, 0.15f, o->Light);
					}
					else if(fTemp > 300.0f && fTemp <= 500.0f)
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}
					else
                    {
                        o->Live = false;
                    }
				}
				else if(o->SubType == 11)	//@@ MovePaticle
				{
					VectorAdd(o->Position,o->Velocity,o->Position);
					o->TurningForce[0] = 1.5f;
					
					vec3_t vTemp;
					VectorSubtract(o->Position, o->StartPosition, vTemp);
					float fTemp = VectorLength(vTemp);

					o->Rotation += 0.5f+rand()%2;

					if(fTemp > 500.0f)
					{
						o->Live = false;
					}
				}
				else if(o->SubType == 13)
				{
					o->Rotation += o->Gravity*1.5f;
				}
				else if(o->SubType == 14)
				{
					VectorAdd(o->Position,o->Velocity,o->Position);
                    o->Velocity[0] *= 0.95f;
                    o->Velocity[1] *= 0.95f;
                    o->Velocity[2] += 0.6f;
                    o->Position[0] += (float)(rand()%4-2);
                    o->Position[1] += (float)(rand()%4-2);
                    o->Position[2] += (float)(rand()%4-2)*0.8f;
                    o->Scale += 0.05f;
                    o->Light[0] = (float)(o->LifeTime)/50.f;
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                    o->Rotation += 1.f+rand()%2;
				}
				else if(o->SubType == 15)
				{
					if(o->LifeTime <= 10)
					{
						o->Live = false;
					}
					
					VectorAdd(o->Position,o->Velocity,o->Position);
					
					vec3_t vTemp;
					VectorSubtract(o->Position, o->StartPosition, vTemp);
					float fTemp = VectorLength(vTemp);

					o->Rotation += 0.5f+rand()%2;

					if(fTemp <= 50.f)
					{
						for(int i=0; i<3; ++i)
						{
							o->Light[i] = o->TurningForce[i] * 0.1f * (500 - o->LifeTime);
							if(o->Light[i] > o->TurningForce[i])
							{
								o->Light[i] = o->TurningForce[i];
							}
						}
					}
					else
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;

						if(o->Light[0] < 0.001f)
							o->Live = false;
					}
		
					if(fTemp > 500.0f)
					{
						o->Live = false;
					}
				}
				else if(o->SubType == 16)
				{
					if(o->LifeTime <= 10)
					{
						o->Live = false;
					}

					VectorAdd(o->Position,o->Velocity,o->Position);
				
					vec3_t vTemp;
					VectorSubtract(o->Position, o->StartPosition, vTemp);
					float fTemp = VectorLength(vTemp);

					o->Rotation += 0.5f+rand()%2;

					if(fTemp <= 50.0f)
					{
						o->Light[0] += 0.01f;
						o->Light[1] += 0.01f;
						o->Light[2] += 0.01f;
						if(o->Light[0] >= 0.2f)
							Vector(0.2f, 0.2f, 0.2f, o->Light);
					}
					else
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;

						if(o->Light[0] < 0.001f)
							o->Live = false;
					}
					
					if(fTemp >= 100.f)
                    {
                        o->Live = false;
                    }
				}
				else if(o->SubType == 17)
				{
					if(o->LifeTime <= 10)
					{
						o->Live = false;
					}
					VectorAdd(o->Position,o->Velocity,o->Position);
					
					vec3_t vTemp;
					VectorSubtract(o->Position, o->StartPosition, vTemp);
					float fTemp = VectorLength(vTemp);

					o->Rotation += 0.08f+rand()%2*0.15f;

					if(o->LifeTime >= 400.f)
					{
						for(int i=0; i<3; ++i)
						{
							o->Light[i] = o->TurningForce[i] * 0.02f * (500 - o->LifeTime);
							if(o->Light[i] > o->TurningForce[i])
							{
								o->Light[i] = o->TurningForce[i];
							}
						}
					}
					else
					{
						o->Light[0] /= 1.02f;
						o->Light[1] /= 1.02f;
						o->Light[2] /= 1.02f;

						if(o->Light[0] < 0.015f)
							o->Live = false;
					}
		
					if(fTemp > 500.0f)
					{
						o->Live = false;
					}
				}
				else if(o->SubType == 18)
				{
                    Luminosity = 1.0f;
                    if(o->LifeTime > 90)
                    {
						o->Light[0] += 0.002f;
						o->Light[1] += 0.002f;
						o->Light[2] += 0.0017f;
                    }
					else if(o->LifeTime < 30)
					{
						o->Light[0] -= 0.004f;
						o->Light[1] -= 0.004f;
						o->Light[2] -= 0.0033f;
					}
					if (o->Scale > 0) o->Scale -= 0.001f;
					if (o->TurningForce[1] > 0) o->TurningForce[1] -= 0.5f;
					o->Position[0] = o->Target->Position[0] + o->StartPosition[0] + cosf((WorldTime+o->Rotation)*o->TurningForce[0])*o->TurningForce[1];
					o->Position[1] = o->Target->Position[1] + o->StartPosition[1] + sinf((WorldTime+o->Rotation)*o->TurningForce[0])*o->TurningForce[1];
					o->Position[2] += (rand()%10)/10.0f;

                    if(o->LifeTime <= 0)
                    {
                        o->Target->HiddenMesh = 0;
                        o->Live = false;
                    }
				}
				else if(o->SubType == 19)
				{
                    Luminosity = 1.0f;
                    if(o->LifeTime > 30)
                    {
						o->Light[0] += 0.01f;
						o->Light[1] += 0.01f;
						o->Light[2] += 0.01f;
						o->Scale += 0.01f;
                    }
					else
					{
						o->Light[0] -= 0.01f;
						o->Light[1] -= 0.01f;
						o->Light[2] -= 0.01f;
					}
					//o->Position[0] -= 4.0f+(rand()%100)*0.1f;
					o->Position[1] += 8.0f+(rand()%200)*0.1f;
				    //o->Position[2] = o->StartPosition[2] + sinf((WorldTime+o->Gravity)/5000.0f)*20.0f;

                    if(o->LifeTime <= 0)
                    {
                        o->Target->HiddenMesh = 0;
                        o->Live = false;
                    }
				}
				else if( o->SubType==20)
				{
					if(o->LifeTime <=0) {
						 o->Live = false;
					}
					else if(o->LifeTime > 150){
						if(o->Alpha < 1.0f)
							o->Alpha += 0.04;
						o->Position[2] += 1.f;
					}
					else {
						if(o->Alpha > 0.1f)
							o->Alpha -= 0.025f;
						o->Position[2] -= 0.5f;
					}
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if( o->SubType==21)
				{
					if(o->LifeTime <=0) {
						o->Live = false;
					}
					else if(o->LifeTime > 50){
						if(o->Alpha < 1.0f)
							o->Alpha += 0.04;
						o->Position[2] += 2.f;
					}
					else {
						if(o->Alpha > 0.1f)
							o->Alpha -= 0.005f;
						o->Position[2] -= 1.0f;
					}
				}
				else if( o->SubType==22)
				{
					if(o->LifeTime <=0) {
						o->Live = false;
					}
					else if(o->LifeTime > 40)
					{
						if(o->Alpha < 1.0f)
							o->Alpha += 0.1f;
						o->Position[2] += 0.5f;
					}
					else {
						if(o->Alpha > 0.0f)
							o->Alpha -= 0.025f;
						else
							o->Live = false;
						o->Position[2] -= 0.5f;
					}
					
					o->Scale += 0.001f;
				}
				else if( o->SubType==23)
				{
					if(o->LifeTime <=0) {
						o->Live = false;
					}
					else if(o->LifeTime > 40)
					{
						if(o->Alpha < 1.0f)
							o->Alpha += 0.2f;
						//o->Position[2] += 0.5f;
						
						o->Scale += 0.003f;
					}
					else
					{
						o->Velocity[0] *= 0.7f;
						o->Velocity[1] *= 0.7f;
						
						if(o->Alpha > 0.0f)
							o->Alpha -= (rand()%10*0.01f);
						else
							o->Live = false;
						o->Position[2] += 0.2f;
						
						//o->Scale += 0.0005f;
					}
					o->Rotation += o->TurningForce[0] * 5.0f;;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
                else
                {
                    Luminosity = 0.6f;
                    if( o->Target->Visible==true )
                    {
                        o->LifeTime = 50;
                    }
                    if(o->LifeTime <= 0)
                    {
                        o->Target->HiddenMesh = 0;
                        o->Live = false;
                    }

				    o->Position[2] = o->StartPosition[2] + sinf((WorldTime+o->Gravity)/5000.0f)*20.0f;
                }
                switch ( o->SubType )
                {
                case    1 :
                case    4 :
                    o->Rotation = (WorldTime*0.02f*o->TurningForce[0]) + o->StartPosition[1];
                    break;

                case    2 :
                case    5 :
                    o->Rotation = (WorldTime*(-0.02f)*o->TurningForce[0]) + o->StartPosition[1];
                    break;
                }
                break;

            case BITMAP_LIGHT:
				if ( 0 == o->SubType || 2 == o->SubType || o->SubType==7 || o->SubType==8 )
				{
					float fScale = ( 0 == o->SubType || o->SubType==8 ) ? 0.5f : 4.f;
					vec3_t vRandom;

                    if ( o->SubType==7 )
                    {
                        fScale = sinf ( WorldTime+o->LifeTime )*5.f+10.f;
					    o->Position[2] += fScale;
                    }
                    else
                    {
					    vRandom[0] = ( ( float)( rand() % 2001 - 1000)) * ( 0.001f * 0.4f);
					    vRandom[1] = ( ( float)( rand() % 2001 - 1000)) * ( 0.001f * 0.4f);
					    vRandom[2] = 0.f;
					    VectorAdd( vRandom, g_vParticleWind, vRandom);
					    VectorScale( vRandom, fScale, vRandom);
					    VectorAdd( o->Position, vRandom, o->Position);
					    o->Position[2] += 5.f * fScale;
                    }
					if ( 0 == o->SubType || o->SubType==8 )
					{
						o->Scale -= 0.05f;
					}
                    else if ( o->SubType==7 )
                    {
//                        o->Scale = sin ( o->Gravity+WorldTime*0.001f )*0.2f+0.5f;
                        o->Rotation += 30.f;
                    }
					else if ( 1 == o->SubType)
					{
						o->Scale *= 0.98f;
					}
					else if ( 5 == o->SubType)
					{
						o->Scale *= 0.95f;
					}
					else
					{
						o->Scale *= 0.95f;
					}
                    if ( o->SubType==7 )
                    {
                        if ( o->LifeTime<10 )
                        {
                            o->Light[0] /= 1.2f;
                            o->Light[1] /= 1.2f;
                            o->Light[2] /= 1.2f;
                        }
                    }
                    else
                    {
					    Vector(o->Scale,o->Scale,o->Scale,o->Light);
                    }
					if ( 1 == o->SubType)
					{
						Vector(0.3f*o->Scale,0.3f*o->Scale,0.3f*o->Scale,o->Light);
					}
					if ( 5 == o->SubType)
					{
						Vector(0.3f*o->Scale,0.3f*o->Scale,0.3f*o->Scale,o->Light);
					}
					if ( ( ( 0 == o->SubType || o->SubType==8 ) && o->Scale<=0.1f ) || ( o->SubType==7 && o->Scale<=0.1f ) || ( 2 == o->SubType && o->Scale<=0.3f ) )
					{
						o->LifeTime = -1;
						o->Live = 0;
					}
					if ( 8 == o->SubType)
					{
						Vector(1.0f*o->Scale,0.5f*o->Scale,0.3f*o->Scale,o->Light);
					}
				}
				else if ( o->SubType==6 )
				{
					float fScale = 0.5f;
					vec3_t vRandom;
					vRandom[0] = ( ( float)( rand() % 2001 - 1000)) * ( 0.001f * 0.4f);
					vRandom[1] = ( ( float)( rand() % 2001 - 1000)) * ( 0.001f * 0.4f);
					vRandom[2] = 0.f;
					VectorScale( vRandom, fScale, vRandom);
					VectorAdd( o->Position, vRandom, o->Position);
					o->Position[2] += 5.f * fScale;
					o->Scale *= 0.95f;

//                    Vector(o->Scale,o->Scale,o->Scale,o->Light);
                    o->Light[0] *= 0.95f;
                    o->Light[1] *= 0.95f;
                    o->Light[2] *= 0.95f;
					if ( o->Scale<=0.1f )
					{
						o->LifeTime = -1;
						o->Live = 0;
					}
				}
				else if ( 1 == o->SubType)
				{
					VectorScale( o->Light, 0.9f, o->Light);
					o->Scale *= 0.95f;
				}
				else if ( 5 == o->SubType)
				{
					VectorScale( o->Light, 0.9f, o->Light);
					o->Scale *= 0.95f;
				}
                else if ( 3 == o->SubType )
                {
                    VectorScale( o->Light, 0.9f, o->Light );
                    o->Scale *= 0.85f;
                    o->Gravity += 5.f;

                    VectorCopy ( o->Target->Position, o->Position );

                    o->Position[0] += 2.f;
                    o->Position[1] -= 2.f;
                    o->Position[2] += o->Gravity;
                }
                else if ( o->SubType==4 )   //  최대 생명력 증가 상태.
                {
                    if ( o->Target!=NULL && o->Target->Type==MODEL_PLAYER && o->Target->Kind==KIND_PLAYER )
                    {
	                    OBJECT *Owner = o->Target;
	                    BMD *b = &Models[Owner->Type];

                    	b->TransformPosition( Owner->BoneTransform[(int)o->Rotation], o->Angle, o->Position, false );

                        VectorAdd ( o->Position, Owner->Position, o->Position );
                    }
                    else
                    {
                        o->Live = false;
                    }

                    o->Gravity += (rand()%40+60)/100.f*9.5f;
                    o->Scale   -= (rand()%400+400)/10000.f;
                    o->Position[2] += o->Gravity;

                    o->Light[0] /= 1.35f;
                    o->Light[1] /= 1.35f;
                    o->Light[2] /= 1.35f;
                }
				else if (o->SubType == 9)
				{
					o->Scale -= 0.011f;
					if(!o->Live)
						o->LifeTime = 0;
					if(o->Scale <= 0.0f)
						o->LifeTime = 0;

					if(o->LifeTime >= 75)
					{
						o->Light[0] *= 1.05f;
						o->Light[1] *= 1.05f;
						o->Light[2] *= 1.05f;					
					}
					else
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}

				}
				else if(o->SubType == 10)
				{
					o->Rotation -= 10;
					if(o->Target != NULL)
					{
						if(o->Target->CurrentAction != PLAYER_SANTA_2)
						{
							o->Live = false;
						}
					}
				}
				else if ( o->SubType == 11 )
				{
                    o->Position[2] += o->Gravity;
					o->Scale -= 0.0005f;
				}
				else if ( o->SubType == 12 )
				{
					if (o->LifeTime > 80)
						o->Scale += 0.02f;
					else
						o->Scale -= 0.005f;
				}
				else if ( o->SubType == 13 )
				{
					o->Position[1] -= o->Gravity;// + (rand()%10)*0.1f;
                    o->Position[2] += o->Gravity;
					o->Scale -= 0.001f;
				}
				else if( o->SubType == 14 )
				{
					VectorScale( o->Light, 0.98f, o->Light);
					o->Scale *= 0.95f;
				}
#ifdef YDG_ADD_MAP_DOPPELGANGER1
				else if ( o->SubType == 15 )
				{
					o->StartPosition[2] += (rand()%10+5) * 0.01f;
					o->Position[0] = o->StartPosition[0] + sinf(o->StartPosition[2]) * o->Gravity * 2;
					o->Position[1] = o->StartPosition[1] + cosf(o->StartPosition[2]) * o->Gravity * 2;
                    o->Position[2] += o->Gravity;
					o->Scale -= 0.001f;
					if (o->LifeTime > 20)
					{
						o->Alpha += 0.1f;
						if (o->Alpha > 1.0f) o->Alpha = 1.0f;
					}
					else
					{
						o->Scale -= 0.01f;
						o->Alpha -= 0.1f;
						if (o->Alpha > 1.0f) o->Alpha = 1.0f;
					}
					o->Light[0] = o->TurningForce[0] * o->Alpha;
					o->Light[1] = o->TurningForce[1] * o->Alpha;
					o->Light[2] = o->TurningForce[2] * o->Alpha;
				}
#endif	// YDG_ADD_MAP_DOPPELGANGER1
                break;
			case BITMAP_POUNDING_BALL:  //  블러드 캐슬 자이언트 오거의 통통볼
				if ( o->SubType==0 && o->SubType==1 )
				{
					o->Gravity += 0.004f;
					o->Scale -= 0.02f;

					o->Frame = (23-o->LifeTime)/6;
					o->Position[2] += o->Gravity*10.f;

					if ( o->SubType==1 )
					{
						o->Light[0] = o->LifeTime/10.f;
						o->Light[1] = o->Light[0];
						o->Light[2] = o->Light[0];
					}
				}
				else if ( o->SubType==2 )
				{
					VectorCopy( o->Target->Angle, o->Angle );
					
					o->Scale -= 0.06f;
					o->Position[2] += o->Gravity*10.f;
                    o->Light[0] = o->LifeTime/10.f;
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
				}
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				else if (o->SubType == 3)
				{
					if (o->LifeTime < 15)
					{
						o->Alpha -= 0.2f;
					}
					else if (o->Alpha < 1.0f)
					{
						o->Alpha += (rand()%2+2)*0.1f;
					}
					else
					{
						o->Alpha = 1.0f;
					}
					
					if (o->Alpha < 0.1f)
						o->Live = false;
					Luminosity = o->Alpha;
					Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					if (o->Scale > 0)
					{
						o->Scale -= (rand()%3+5)*0.01f;
					}
					else
					{
						o->Live = false;
					}
					o->Position[2] += o->Gravity;
					o->Rotation += 3.0f;
				}
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				break;
            case BITMAP_ADV_SMOKE:      //  블러드 캐슬의 불 연기.
                VectorAdd(o->Position,o->Velocity,o->Position);
                o->Velocity[0] *= 0.95f;
                o->Velocity[1] *= 0.95f;
                o->Light[0] = (float)(o->LifeTime)/10.f;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                if(o->SubType == 0)
                {
                    o->Velocity[2] += 0.3f;
                    o->Scale += 0.07f;
                }
				else if(o->SubType == 2)
				{
					o->Velocity[2] += 0.3f;
                    o->Scale += 0.07f;
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if(o->SubType == 3)
				{
					o->Velocity[1] += 0.1f;
                    o->Scale += 0.05f;
					o->Alpha -= 0.2f;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN

                else
                {
                    o->Velocity[2] += 0.3f;
                    o->Scale += 0.09f;
                }
                break;
            case BITMAP_ADV_SMOKE+1:    //  블러드 캐슬의 불 연기.
                VectorAdd(o->Position,o->Velocity,o->Position);
                o->Velocity[0] *= 0.95f;
                o->Velocity[1] *= 0.95f;
                o->Velocity[2] += 0.6f;
                o->Position[0] += (float)(rand()%4-2);
                o->Position[1] += (float)(rand()%4-2);
                o->Position[2] += (float)(rand()%4-2)*0.8f;
                o->Scale += 0.05f;
				if(o->SubType == 2)
				{
					//~-1.f, ~1.f사이 
					o->Light[0] = (float)(o->LifeTime)/25.f*2 - 1.f;
					o->Light[1] = o->Light[0];
					o->Light[2] = o->Light[0];					
				}
				else
				{
					o->Light[0] = (float)(o->LifeTime)/25.f;
					o->Light[1] = o->Light[0];
					o->Light[2] = o->Light[0];
				}
                o->Rotation += 1.f+rand()%2;
                break;

            case BITMAP_TRUE_FIRE :
            case BITMAP_TRUE_BLUE :
                if ( ( o->SubType==1 || o->SubType==2 ) && o->Target!=NULL )
                {
                    if ( o->Target->CurrentAction==1 )
                        o->LifeTime-=2;
                }

				if(o->SubType>=3 && o->SubType<5 && o->Target!=NULL)	//. 불골렘에 붙는 불..
				{
					std::string name;
					switch(o->SubType)
					{
					case 3: name = "Monster82_LHand"; break;
					case 4: name = "Monster82_RHand"; break;
					}
					if(BoneManager::GetBonePosition(o->Target, name, TargetPosition)) {
						vec3_t Direction;
						VectorSubtract(o->Position, TargetPosition, Direction);
						
						float length = VectorLength(Direction);
						//. 스케일 보정
						o->Scale -= (length * 0.003f);
						//. 라이트 보정
						o->Light[0] -= (length * 0.08f);
						o->Light[1] -= (length * 0.08f);
						o->Light[2] -= (length * 0.08f);
					}
				}
				
				if(o->SubType == 8)
				{
					vec3_t vPos, vRelativePos;
					if(o->Target != NULL)
					{
						BMD* pModel = &Models[o->Target->Type];
						VectorCopy(o->Target->Position,pModel->BodyOrigin);
						Vector(6.f, 6.f, 0.f, vRelativePos);
						pModel->TransformPosition(o->Target->BoneTransform[20], vRelativePos, vPos, true);
					}

					o->Position[0] = vPos[0];
					o->Position[1] = vPos[1];
				}

				if(o->SubType == 7)
				{
					o->Scale -= 0.1f;
					o->Position[2] += 2.0f;
				}
				else
		          o->Scale -= 0.02f;

                if ( o->Scale<0 )
					o->Scale = 0.f;

                o->Velocity[0] *= 0.95f;
                o->Velocity[1] *= 0.95f;

				if (o->Type == 6)
					o->Position[2] += 2.0f;
				else
	                o->Position[2] += 1.f;

                o->Light[0] = (float)(o->LifeTime)/25.f;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                break;

            case BITMAP_HOLE:
                o->Rotation += 5.f;
                if ( o->LifeTime<20 )
                {
                    o->Scale -= 0.08f;
                    if ( o->LifeTime<10 )
                    {
                        o->Light[0] /= 1.3f;
                        o->Light[1] /= 1.3f;
                        o->Light[2] /= 1.3f;
                    }
                }
                else if ( o->LifeTime<30 )
                {
                    o->Light[0] *= 1.1f;
                    o->Light[1] *= 1.1f;
                    o->Light[2] *= 1.1f;
                }
                break;
            case BITMAP_WATERFALL_1:
                o->Scale -= 0.005f;
                if ( o->LifeTime<5 )
                {
                    o->Light[0] /= 1.2f;
                    o->Light[1] /= 1.2f;
                    o->Light[2] /= 1.2f;
                }
                else if ( o->LifeTime>20 )
                {
                    o->Light[0] *= 1.1f;
                    o->Light[1] *= 1.1f;
                    o->Light[2] *= 1.1f;
                }
                o->Velocity[2] += 0.1f;
                
                if ( o->SubType==1 )
                {
                    if ( o->Light[0]>0.5f )
                    {
                        Vector ( 0.5f, 0.5f, 0.5f, o->Light );
                    }
                    o->Rotation++;
                    o->Scale += 0.01f;
                    o->Velocity[2] -= 0.4f;
                }
                break;
                
            case BITMAP_WATERFALL_5:
                if ( o->SubType==0 )
                {
                    o->Scale -= 0.005f;
                    o->Velocity[2] += 0.1f;
                }
                else if ( o->SubType==1 )
                {
                    o->Scale += 0.1f;
                    o->Position[0] += rand()%10-5.f;
                    o->Position[1] += rand()%10-5.f;
                    //                    o->Position[2] += rand()%20-10.f
                }
                else if ( o->SubType==2 )
                {
					if (o->Scale < 1.0f)
						o->Scale += 0.1f;
                    o->Position[0] += rand()%10-5.f;
                    o->Position[1] += rand()%10-5.f;
					o->Velocity[2] -= 1.0f;
                }
                else if ( o->SubType==3 )
                {
                    o->Scale -= 0.005f;
                    o->Rotation += 4.f;
                    o->Velocity[2] += 0.1f;
                }
                else if ( o->SubType==4 )
                {
					o->Position[0] += (float)(cos(o->Angle[2]) * 20.0f);
					o->Position[1] += (float)(sin(o->Angle[2]) * 20.0f);
                    o->Position[2] += o->Gravity;
					o->Rotation += 1;
                }
                else if ( o->SubType==5 )
                {
                    o->Scale -= 0.005f;
                    o->Velocity[2] += 0.1f;
                }
#ifdef CSK_ADD_MAP_ICECITY	
				else if(o->SubType == 7)
				{
					o->Scale -= 0.005f;
					o->Rotation += 1;
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] += o->Velocity[2];
					
					o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 8)
				{
					o->Scale    += 0.05f;
					o->Velocity[2] -= 0.6f;

					o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
					break;
				}
#endif // CSK_RAKLION_BOSS	
#ifdef LDS_ADD_EMPIRE_GUARDIAN
                else if ( o->SubType==9 )
                {
                    o->Scale -= 0.005f;
                    o->Velocity[2] += 0.1f;
                }
#endif //LDS_ADD_EMPIRE_GUARDIAN
				
                if ( o->LifeTime<8 )
                {
                    o->Light[0] /= 1.2f;
                    o->Light[1] /= 1.2f;
                    o->Light[2] /= 1.2f;
                }
                else if ( o->LifeTime>20 )
                {
                    o->Light[0] *= 1.1f;
                    o->Light[1] *= 1.1f;
                    o->Light[2] *= 1.1f;
                }
                break;
                
            case BITMAP_PLUS:
                o->Scale -= 0.01f;
                o->Position[0] += rand()%2-1;
                o->Position[1] += rand()%2-1;
                o->Position[2] += 2.f;

                o->Light[0] = o->LifeTime/20.f;
                o->Light[1] = o->LifeTime/20.f;
                o->Light[2] = o->LifeTime/20.f;
                break;
                
            case BITMAP_WATERFALL_2:
#ifdef CSK_ADD_MAP_ICECITY	
				if(o->SubType == 5)
				{
					o->Scale += 0.03f;
					o->Velocity[2] += 0.1f;
					
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] += o->Gravity + o->Velocity[2];
					
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
#endif // CSK_ADD_MAP_ICECITY

                o->Scale += 0.03f;
                o->Velocity[0] = (rand()%20-10)*0.1f;
                o->Velocity[1] = (rand()%20-10)*0.1f;
                o->Velocity[2] += 0.1f;
                if ( o->LifeTime<10 )
                {
                    o->Light[0] /= 1.1f;
                    o->Light[1] /= 1.1f;
                    o->Light[2] /= 1.1f;
                }

				if(o->SubType ==1)
				{
                    o->Light[0] /= 1.05f;
                    o->Light[1] /= 1.05f;
                    o->Light[2] /= 1.05f;
				}
				if(o->SubType == 3)
				{
					o->Scale -= 0.038f;
					o->Velocity[2] -= 0.02f;
                    o->Light[0] /= 1.02f;
                    o->Light[1] /= 1.02f;
                    o->Light[2] /= 1.02f;
				}
				if(o->SubType == 4)
				{
					o->Rotation -= 1.1f;
				}
#ifdef PBG_ADD_PKFIELD
				if(o->SubType == 6)
				{
					o->Gravity += (rand()%5+1.5f*(o->LifeTime/5.0f));
					
					o->Velocity[0] += 1;

					o->Velocity[0] -= 100*(20.0f/o->LifeTime);
					o->Rotation += 50.f;
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(o->Velocity,Matrix,TargetPosition);
					VectorAdd(o->StartPosition,TargetPosition,o->Position);
					o->Scale += (1.0/o->LifeTime)*2.0f;
					
					o->Light[0]  *= 0.8f;
					o->Light[1]  *= 0.77f;
					o->Light[2]  *= 0.77f;

				//	o->Gravity += 1.f;
					VectorCopy(o->StartPosition,o->Position);
					o->StartPosition[2] = o->StartPosition[2]+rand()%5+10.0f;
 					o->Position[2] = o->StartPosition[2] + o->Gravity;
				}
#endif //PBG_ADD_PKFIELD
                break;
                
            case BITMAP_WATERFALL_3:
            case BITMAP_WATERFALL_4:
                if ( o->SubType==2 )
                {
					o->Position[0] += (float)(cos(o->Angle[2]) * 20.0f);
					o->Position[1] += (float)(sin(o->Angle[2]) * 20.0f);
                    o->Position[2] += o->Gravity;
					o->Rotation += 1;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
                }
				else if( o->SubType == 3)		//. 먼지불빛 박스 : 2004/11/09
				{
					o->Scale    += 0.005f;
					o->Velocity[2] -= 0.05f;
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					break;
				}
				else if(o->SubType == 5)
				{
                    o->Position[2] -= o->Gravity;
					o->Gravity -= 0.05f;
					//o->Scale    += 0.005f;
					//o->Velocity[2] -= 0.05f;

					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
				else if(o->SubType == 6)
				{
					o->Position[2] += o->Gravity;
					o->Gravity += 0.05f;

					o->Alpha -= 0.01f;

					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
				else if(o->SubType == 7)	//@@ MoveParticle
				{
					o->Velocity[2] += 0.01f;
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					break;
				}
				else if (o->SubType == 8)
				{
					o->Scale    += 0.05f;
					o->Velocity[2] -= 0.6f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
#ifdef CSK_ADD_MAP_ICECITY	
				else if(o->SubType == 10)
				{
					o->Position[0] += o->Velocity[0];
					o->Position[1] += o->Velocity[1];
					o->Position[2] -= o->Gravity;
					o->Gravity -= 0.05f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
#endif // CSK_ADD_MAP_ICECITY
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 11)
				{
					VectorAdd(o->Position, o->Velocity, o->Position);
					o->Scale += 0.01f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
#endif // CSK_RAKLION_BOSS	
#ifdef ADD_SOCKET_ITEM
				// 소드브레이커
				else if( o->SubType == 12 )
				{
					o->Rotation += ((int)WorldTime%360)*0.01f;
					o->Scale *= 0.92f;	
					Vector( o->Light[0]*=0.92f, o->Light[1]*=0.92f, o->Light[2]*=0.92f, o->Light );
					o->Alpha *= 0.8f;

					break;
				}
#endif // ADD_SOCKET_ITEM
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 13)
				{
					o->Position[2] -= o->Gravity;
					o->Scale += 0.001f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;

					break;
				}
#endif // CSK_RAKLION_BOSS	
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if (o->SubType == 14)
				{
					o->Scale    += 0.05f;
					o->Velocity[2] -= 0.6f;
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
				}
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD
				else if(o->SubType == 15)
				{
                    o->Position[2] -= o->Gravity;
					o->Gravity -= 0.05f;
					o->Scale    -= 0.05f;
					//o->Velocity[2] -= 0.05f;

					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
					break;
				}
#endif	// LDS_ADD_EG_2_MONSTER_GRANDWIZARD
#ifdef ASG_ADD_MAP_KARUTAN
				else if (o->SubType == 16)
				{
					o->Scale -= 0.005f;
					o->Velocity[2] -= 0.1f;
					if (o->LifeTime < 8)
					{
						o->Light[0] /= 1.2f;
						o->Light[1] /= 1.2f;
						o->Light[2] /= 1.2f;
					}
					else if (o->LifeTime > 20)
					{
						o->Light[0] *= 1.1f;
						o->Light[1] *= 1.1f;
						o->Light[2] *= 1.1f;
					}
					break;
				}
#endif	// ASG_ADD_MAP_KARUTAN
                o->Scale    += 0.005f;
                o->Velocity[2] -= 2.5f;
                o->Light[0] /= 1.1f;
                o->Light[1] /= 1.1f;
                o->Light[2] /= 1.1f;
                break;
				
			case BITMAP_SHOCK_WAVE:
				{
					if(o->SubType == 3)
					{
						o->Light[0] /= 1.8f;
						o->Light[1] /= 1.8f;
						o->Light[2] /= 1.8f;
						o->Scale += 0.3f;
					}
					else if(o->SubType == 0)
					{		
						o->Light[0] /= 1.5f;
						o->Light[1] /= 1.5f;
						o->Light[2] /= 1.5f;
						o->Scale += 0.8f;
					}		
#ifdef CSK_EVENT_CHERRYBLOSSOM
					if(o->SubType == 4)
					{
						o->Alpha -= 0.001f;
						if(o->Alpha <= 0.0f) o->Live = false;
						o->Light[0] /= 1.01f;
						o->Light[1] /= 1.03f;
						o->Light[2] /= 1.03f;	
						o->Scale += (o->Gravity * 0.015f);
						o->Gravity += 2.4f;
					}
#endif //CSK_EVENT_CHERRYBLOSSOM
				}
				break;
			case BITMAP_CURSEDTEMPLE_EFFECT_MASKER:
				{
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;

					o->Scale += 0.02f;

					if( o->SubType == 0 )
					{
						vec3_t Light;
						Vector(0.8f,0.3f,0.3f,Light);
						if( rand()%2 == 1)
							CreateParticle( BITMAP_CURSEDTEMPLE_EFFECT_MASKER, o->StartPosition, o->Angle, Light, 1, 1.3f );
					}
				}
				break;
#ifdef LDS_RAKLION_ADDMONSTER_ICEWALKER
			case BITMAP_RAKLION_CLOUDS:
				{
					// A. 다음 위치 지정
					// 1. 방향 벡터를 y = 1인 단위 벡터 로 설정한뒤
					// 2. 오브텍트가 바라보는 방향으로 
					// 3. Velocity 값을 지정한뒤
					// 4. 2,3 요소를 곱해서 방향_크기벡터를 만들고,
					// 5. 기존 위치 에서 변위위치를 더해준다.
					// B. 다음 알파 값 지정 
					// C. 다음 크기 지정
					
					// A. 다음 위치 지정
					const	float FORCESCALAR	= 13.0f;
					const	float SCALEFACTOR	= 0.065f;
					const	float LIGHTDIVIDEDFACTOR = 1.6f;
					const	float EPSILON_		= 0.01f;
					
					vec3_t	vForceVec_, vForceVec;
					float	fForceScalar;	fForceScalar = FORCESCALAR;
					
					float	MatRotation[3][4];
					vForceVec_[0] = 0.0f; vForceVec_[1] = -1.0f; vForceVec_[2] = 0.0f;
					
					AngleMatrix( o->Angle, MatRotation );
					VectorRotate( vForceVec_, MatRotation, vForceVec );
					vForceVec[0] = vForceVec[0] * fForceScalar;
					vForceVec[1] = vForceVec[1] * fForceScalar;
					vForceVec[2] = vForceVec[2] * fForceScalar;
					
					VectorAdd( o->Position, vForceVec, o->Position );				
					// B. 다음 크기 지정
					o->Scale = o->Scale + SCALEFACTOR; 
					
					// C. ALPHA 감산 효과
					o->Light[0] = o->Light[0] / LIGHTDIVIDEDFACTOR;
					o->Light[1] = o->Light[1] / LIGHTDIVIDEDFACTOR;
					o->Light[2] = o->Light[2] / LIGHTDIVIDEDFACTOR;
					//o->Alpha	= o->Alpha / LIGHTDIVIDEDFACTOR;
					
					if( o->Light[0] + o->Light[1] + o->Light[2] <= EPSILON_)
					{
						o->Live = false;
					}
					
					// ShinyStar 드문드문 하게 하나씩 반짝.
					if ( rand() % 10 == 0 )
					{
						// ShinyStar 효과 부분 
						const	float BASERANGE_SHINYSTAR	= 70.0f * o->Scale;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						const	float RESIZINGVALUE			= 0.8f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						float	fRand1,fRand2,fRand3;
						vec3_t	v3Pos_ShinyStar, v3OffsetPos_ShinyStar;
						
						fRand1 = (float)((rand() % 2000) - 1000) * 0.001f;
						fRand2 = (float)((rand() % 2000) - 1000) * 0.001f;
						fRand3 = (float)((rand() % 2000) - 1000) * 0.001f;
						
						v3OffsetPos_ShinyStar[0] = BASERANGE_SHINYSTAR * fRand1;
						v3OffsetPos_ShinyStar[1] = BASERANGE_SHINYSTAR * fRand2;
						v3OffsetPos_ShinyStar[2] = BASERANGE_SHINYSTAR * fRand3;
						
						VectorAdd( o->Position, v3OffsetPos_ShinyStar, v3Pos_ShinyStar );
						CreateParticle(BITMAP_SHINY + 6, v3Pos_ShinyStar, o->Angle, o->Light, 0, 0.5f);
					}
				}
				break;
#endif // LDS_RAKLION_ADDMONSTER_ICEWALKER
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD
			case BITMAP_CHROME2:
				{
// 					o->StartPosition[2] += (rand()%10+5) * 0.01f;
// 					o->Position[0] = o->StartPosition[0] + sinf(o->StartPosition[2]) * o->Gravity * 2;
// 					o->Position[1] = o->StartPosition[1] + cosf(o->StartPosition[2]) * o->Gravity * 2;
//                     o->Position[2] += o->Gravity;
					if (o->Scale > 0) o->Scale -= 0.1f;
					else o->Scale = 0;

					o->Rotation += 1.0f;
					o->Alpha -= 0.05f;
					if (o->Target != NULL)
					{
						o->Position[0] = o->Target->Position[0] - o->StartPosition[0];
						o->Position[1] = o->Target->Position[1] - o->StartPosition[1];
						o->Position[2] = o->Target->Position[2] - o->StartPosition[2];
					}
// 					if (o->LifeTime > 20)
// 					{
// 						o->Alpha += 0.1f;
// 						if (o->Alpha > 1.0f) o->Alpha = 1.0f;
// 					}
// 					else
// 					{
// 						o->Scale -= 0.01f;
// 						o->Alpha -= 0.1f;
// 						if (o->Alpha > 1.0f) o->Alpha = 1.0f;
// 					}
// 					o->Light[0] = o->TurningForce[0] * o->Alpha;
// 					o->Light[1] = o->TurningForce[1] * o->Alpha;
// 					o->Light[2] = o->TurningForce[2] * o->Alpha;
				}
				break;
#endif	// LDS_ADD_EG_2_MONSTER_GRANDWIZARD
#ifdef PBG_ADD_AURA_EFFECT
			case BITMAP_AG_ADDITION_EFFECT:
				{
					if(o->LifeTime < 10)
						o->Alpha -= 0.1f;
					else
						o->Alpha = 0.7f;

					if(o->Alpha < 0.1f)
						o->Live = false;

     				Luminosity = o->Alpha;
    				Vector(o->TurningForce[0]*Luminosity,o->TurningForce[1]*Luminosity,o->TurningForce[2]*Luminosity,o->Light);
					vec3_t Temp_Pos;
					
					BMD* b = &Models[o->Target->Type];
					b->TransformByObjectBone(Temp_Pos, o->Target, 18);

					if(o->SubType==0)
					{
						o->Scale *= 1.05f;
						Temp_Pos[2] -= 10.0f;
					}
					else if(o->SubType==1)
					{
						o->Scale *= 1.02f;
						Temp_Pos[2] += 10.0f;
					}
					else if(o->SubType==2)
					{
						o->Scale *= 1.03f;	
						Temp_Pos[2] += 25.0f;
					}					
					VectorCopy(Temp_Pos, o->Position);

					if(!o->Live && g_isCharacterBuff(o->Target, eBuff_AG_Addition))
					{
						float _Scale = (rand()%20+20.0f)/50.0f;
						if(o->SubType==0)
							CreateParticle(BITMAP_AG_ADDITION_EFFECT, Temp_Pos, o->Angle, o->Light, 0, 1.0f, o->Target);
						else if(o->SubType==1)
							CreateParticle(BITMAP_AG_ADDITION_EFFECT, Temp_Pos, o->Angle, o->Light, 1, 1.0f, o->Target);
						else if(o->SubType==2)
							CreateParticle(BITMAP_AG_ADDITION_EFFECT, Temp_Pos, o->Angle, o->Light, 2, 1.0f, o->Target);
					}
				}
				break;
#endif //PBG_ADD_AURA_EFFECT				
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case BITMAP_SBUMB:
				{
					o->Frame = 4-o->LifeTime;
				}
				break;
			case BITMAP_DAMAGE1:
				{
					o->Scale *= 1.2f;
					VectorScale(o->Light, 0.8f, o->Light);
				}
				break;
			case BITMAP_SWORD_EFFECT_MONO:
				{
					if(o->SubType == 1)
					{
						o->Scale *= 0.8f;
					}
					else if(o->SubType==0)
					{
						o->Scale *= 1.2f;
					}
					if(o->Scale > 6.0f)
					{
						o->SubType = 1;
						break;
					}
					else if(o->SubType == 1)
					{
						if(o->Scale < 1)
						{
							o->Live = false;
							break;
						}
					}
					
					VectorScale(o->Light, 0.94f, o->Light);
				}
				break;
			case BITMAP_DAMAGE2:
				{
					o->Scale *= 1.2f;
					VectorScale(o->Light, 0.55f, o->Light);
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
            }
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_PARTICLES );
#endif // DO_PROFILING
}


void RenderParticles ( BYTE byRenderOneMore )
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_PARTICLES, PROFILING_RENDER_PARTICLES );
#endif // DO_PROFILING
	for(int i=0;i<MAX_PARTICLES;i++)
	{
		PARTICLE *o = &Particles[i];
		if(o->Live)
		{
            if ( byRenderOneMore==1 )            //  수면 아래의 것들만 찍는다.
            {
                if ( o->Position[2]>350.f ) continue;
            }
            else if ( byRenderOneMore==2 )       //  수면위의 것들만 찍는다.
            {
                if ( o->Position[2]<=300.f ) continue;
            }

			BITMAP_t* pBitmap = Bitmaps.GetTexture(o->TexType);
			float Width  = pBitmap->Width *o->Scale;
			float Height = pBitmap->Height*o->Scale;
          	if(pBitmap->Components==3)
			{
				EnableAlphaBlend();
			}
			else
			{
				EnableAlphaTest(false);
			}
			
			if ( o->Type==BITMAP_LIGHT && o->SubType==6 )
			{
				EnableDepthTest();
			}
			if ( o->Type==BITMAP_EXPLOTION && o->SubType==5 )
			{
				DisableDepthTest ();
			}
			int Frame;
			switch(o->Type)
			{
            case BITMAP_WATERFALL_1:
                RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                break;

			case BITMAP_BUBBLE:
				Frame = o->Frame%9;
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,0.f,Frame%3*0.25f+0.005f,Frame/3*0.25f+0.005f,0.25f-0.01f,0.25f-0.01f);
				break;
			case BITMAP_SPOT_WATER:
				RenderSprite(o->TexType,o->Position,Width,Height*0.125,o->Light,o->Angle[0],0.f,o->Frame%8*0.125f,1.f,0.125f);
				break;

			case BITMAP_SPARK+2:
				if (o->SubType == 0
#ifdef ASG_ADD_SKILL_BERSERKER
					|| o->SubType == 2 || o->SubType == 3
#endif	// ASG_ADD_SKILL_BERSERKER
					)
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,0.f, o->Frame%2*0.5f, o->Frame/2*0.5f, 0.5f, 0.5f);
				}
				break;

			case BITMAP_EXPLOTION_MONO:
			case BITMAP_EXPLOTION:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,0.f,o->Frame%4*0.25f+0.005f,o->Frame/4*0.25f+0.005f,0.25f-0.01f,0.25f-0.01f);
				break;
            case BITMAP_EXPLOTION+1:
    			RenderSprite(o->TexType,o->Position,Width*0.25f,Height,o->Light,o->Angle[0],o->Frame%4*0.25f,0.f,0.25f,1.f);
                break;
			case BITMAP_SUMMON_SAHAMUTT_EXPLOSION:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,0.f,o->Frame%4*0.25f+0.005f,o->Frame/4*0.25f+0.005f,0.25f-0.01f,0.25f-0.01f);
				break;

//////////////////////////////////////////////////////////////////////////
	
			case BITMAP_CLUD64:
				{
					if(o->SubType == 0
						|| o->SubType == 5
#ifdef LDK_ADD_PC4_GUARDIAN
						|| o->SubType == 11
#endif //LDK_ADD_PC4_GUARDIAN
						)
					{
						EnableAlphaBlendMinus();
					}

					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
			case BITMAP_TORCH_FIRE:
				{
					vec3_t vPos;
					VectorCopy(o->Position, vPos);
					for(int i=0; i<3; ++i)
					{
						RenderSprite(o->Type,vPos,Width,Height,o->Light,o->Rotation);
						vPos[2] -= 10.f;
					}
				}
				break;
			case BITMAP_GHOST_CLOUD1:
			case BITMAP_GHOST_CLOUD2:
				{
					RenderSprite(o->Type,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;

#ifdef PBG_ADD_LITTLESANTA_NPC
			case BITMAP_LIGHT+3:
				{
					RenderSprite(o->Type,o->Position,Width,Height,o->Light,o->Rotation);	
				}
				break;
#endif // PBG_ADD_LITTLESANTA_NPC

			case BITMAP_TWINTAIL_WATER:
				{
					EnableAlphaBlend();
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
			case BITMAP_SMOKE:
				if(o->SubType==2||o->SubType==5||o->SubType==12||o->SubType==14||o->SubType==15||o->SubType==20||o->SubType==21||o->SubType==29)
					EnableAlphaBlendMinus();
				if(o->SubType == 37 || o->SubType == 38
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				|| o->SubType==59
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
					)
     				EnableAlphaBlendMinus();
				if(o->SubType==6 )
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light);
				}
				else
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
            case BITMAP_SMOKE+1:
            case BITMAP_SMOKE+4:
				EnableAlphaBlend3();
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                break;
            case BITMAP_ADV_SMOKE+1:
				if(o->SubType == 2)
				{
					//라이트값에 따라 더 밝게 만든다.
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
					EnableAlphaBlend3();
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);	
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				}
				else
				{
					EnableAlphaBlend3();
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);			
				}
                break;
			case BITMAP_SMOKE+3:
				if(o->SubType==3 || o->SubType==4)
				{
					EnableAlphaBlendMinus();
				}
				else
				{
					EnableAlphaBlend3();
				}
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;
			case BITMAP_LIGHTNING:
				RenderSprite(o->TexType,o->Position,Width*0.25f,Height,o->Light,o->Angle[0],o->Frame%4*0.25f,0.f,0.25f,1.f);
				break;
			case BITMAP_BLOOD+1:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,0.f,o->Frame%2*0.5f,o->Frame/2*0.5f,0.5f,0.5f);
				break;
			case BITMAP_CHROME_ENERGY2:
				RenderSprite(o->TexType,o->Position,Width*0.25f,Height,o->Light,o->Rotation,o->Frame%4*0.25f,0.f,0.25f,1.f);
				break;
			case BITMAP_FIRE_CURSEDLICH:
#ifdef CSK_ADD_MAP_ICECITY
			case BITMAP_FIRE_HIK2_MONO:
#endif // CSK_ADD_MAP_ICECITY
    			RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;
			case BITMAP_LEAF_TOTEMGOLEM:
    			RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;
			case BITMAP_FIRE:
			case BITMAP_FIRE+2:
			case BITMAP_FIRE+3:
                if (
					o->SubType==17 || o->SubType==5 || o->SubType==7 || o->SubType==8 || o->SubType==11 || o->SubType==12 || o->SubType==13 )
                {
				    RenderSprite(o->TexType,o->Position,Width*0.25f,Height,o->Light,o->Rotation,o->Frame%4*0.25f,0.f,0.25f,1.f);
                }
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType == 18)
				{	
					EnableAlphaBlend3();
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				else if( o->SubType == 14 || o->SubType == 15)
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                else
                {
    				RenderSprite(o->TexType,o->Position,Width*0.25f,Height,o->Light,o->Angle[0],o->Frame%4*0.25f,0.f,0.25f,1.f);
                }
                break;
			case BITMAP_FIRECRACKER:
				{
					int iCount = i % 8 + 22;
					vec3_t Position;
					vec3_t Light;
					int iTemp = o->LifeTime/4+o->SubType;
					//int iTemp = 0;
					int iColor = iTemp/10;
					int iColorChange = iTemp%10;
					for ( int j = iCount; j >= 0; --j)
					{
						for ( int k = 0; k < 3; ++k)
						{
							Position[k] = o->Position[k] - ( float)j * o->Velocity[k] * 0.1f;
							Light[k] = ( float)( min( iCount - j, 10)) *
								( o->Light[(k+iColor)%3] * ( 10 - iColorChange) +
								o->Light[(k+iColor+1)%3] * iColorChange) *
								( ( float)min( o->LifeTime, 10) * 0.001f);
						}
						RenderSprite(o->TexType,Position,Width,Height,Light,o->Rotation);
					}
				}
				break;
            case BITMAP_FLARE:
				if ( o->SubType == 11 )
				{
					BITMAP_t* pBitmap = Bitmaps.GetTexture(o->TexType);
					Width  = pBitmap->Width * 0.5f * o->Scale;
					Height = pBitmap->Height * 0.4f;
					
    				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else if ( o->SubType!=4 )
                {
                    if( o->LifeTime!=60 )
    				    RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                }
                else
                {
    				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                }
                break;
			case BITMAP_FLARE_BLUE:
				if ( o->SubType == 0 )
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else if ( o->SubType == 1 )
				{
					BITMAP_t* pBitmap = Bitmaps.GetTexture(o->TexType);
					Width  = pBitmap->Width * 0.2f * o->Scale;
					Height = pBitmap->Height * 0.3f;
    				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
            case BITMAP_FLARE+1:
				if ( o->SubType == 0 )
				{
    				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_LIGHT+2:
				if (o->SubType == 3 || o->SubType == 4 || o->SubType == 6 )// || o->SubType == 7)
				{
					EnableAlphaBlendMinus();
				}
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;

//////////////////////////////////////////////////////////////////////////

			case BITMAP_MAGIC+1:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;

//////////////////////////////////////////////////////////////////////////
            case BITMAP_CLOUD :
                switch ( o->SubType )
                {
				case 10:
				case 12:
                case 7: //  흑색.
				case 14:	
				case 16:
         			EnableAlphaBlendMinus();
                case 0: //  좌/우로 회전.
				case 8:	// 사냥터 옅은안개
                case 3:
				case 18:
                    if( (i%2)==0 )
                    {
                        o->Rotation = (WorldTime*0.02f*o->TurningForce[0]) + o->StartPosition[1];
                    }
                    else
                    {
                        o->Rotation = (WorldTime*(-0.02f)*o->TurningForce[0]) + o->StartPosition[1];
                    }
                    break;
                }
				if(o->SubType==8 || o->SubType == 9
					|| o->SubType==20
#ifdef LDS_ADD_EMPIRE_GUARDIAN
					|| o->SubType==21
#endif //LDS_ADD_EMPIRE_GUARDIAN
					)	// 사냥터 옅은안개
				{
					vec3_t Light;
					Light[0] = o->Light[0] * o->Alpha;
					Light[1] = o->Light[1] * o->Alpha;
					Light[2] = o->Light[2] * o->Alpha;
					RenderSprite(o->TexType,o->Position,Width,Height,Light,o->Rotation);
				}
				else if(o->SubType == 17)
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else if(o->SubType == 18)
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else if(o->SubType == 19)
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
                break;

//---------------------------------------------------------------------------------------------------------

			case BITMAP_SPARK:
#ifdef PJH_FIX_CAOTIC
				if(o->SubType == 10)
					EnableAlphaBlendMinus();
//EnableAlphaBlend4();
#endif //PJH_FIX_CAOTIC
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;

//---------------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS
			case BITMAP_FLAME:
				if(o->SubType == 11)
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				else
				{
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
#endif // CSK_RAKLION_BOSS	
			case BITMAP_CURSEDTEMPLE_EFFECT_MASKER:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;
			case BITMAP_SHINY+6:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				RenderSprite(BITMAP_LIGHT,o->Position,Width,Height,o->Light,o->Rotation);
				break;
#ifdef KJH_ADD_EG_MONSTER_KATO_EFFECT
			case BITMAP_SMOKELINE2:
				{
					if( o->SubType == 3 )
					{
						EnableAlphaBlendMinus();
					}
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}break;
#endif // KJH_ADD_EG_MONSTER_KATO_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case BITMAP_SBUMB:
				{
					RenderSprite(o->TexType, o->Position, Width*0.25f, Height, o->Light, 0.0f, o->Frame%4*0.25f+0.005f, 0.0f, 0.25f-0.01f, 1.0f);
				}
				break;
			case BITMAP_DAMAGE1:
				{
					VectorScale(o->Light, 2.0f, o->Light);
					RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				}
				break;
			case BITMAP_SWORD_EFFECT_MONO:
				{
					vec3_t vPos;
					VectorCopy(o->Position, vPos);
					vPos[2] += 31.0f*o->Scale;
					RenderSprite(o->TexType, vPos, Width*0.9f, Height*1.1f, o->Light, o->Rotation);
				}
				break;
			case BITMAP_DAMAGE2:
				{
					vec3_t vLight;
					VectorCopy(o->Light, vLight);
					VectorScale(vLight, 1.4f, vLight);
					RenderSprite(o->TexType,o->Position,Width,Height,vLight,o->Rotation);
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
            case BITMAP_TRUE_FIRE:
            default:
				RenderSprite(o->TexType,o->Position,Width,Height,o->Light,o->Rotation);
				break;
			}
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_PARTICLES );
#endif // DO_PROFILING
}



