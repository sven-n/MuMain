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
#include "CSPetSystem.h"

extern float g_fBoneSave[10][3][4];

void CreateJointFpsChecked(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType, OBJECT* Target, float Scale, short PKKey,
    WORD SkillIndex, WORD SkillSerialNum, int iChaIndex, const float* vPriorColor, short int sTargetindex)
{
    if (rand_fps_check(1))
    {
        CreateJoint(Type, Position, TargetPosition, Angle, SubType, Target, Scale, PKKey, SkillIndex, SkillSerialNum, iChaIndex, vPriorColor, sTargetindex);
    }
}

void CreateJoint(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType, OBJECT* Target, float Scale, short PKKey,
    WORD SkillIndex, WORD SkillSerialNum, int iChaIndex, const float* vPriorColor, short int sTargetindex)
{
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        JOINT* o = &Joints[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Type = Type;
            o->TexType = o->Type;
            o->SubType = SubType;
            o->RenderType = RENDER_TYPE_ALPHA_BLEND;
            o->RenderFace = RENDER_FACE_ONE | RENDER_FACE_TWO;
            o->Collision = false;
            o->PKKey = PKKey;
            o->Skill = SkillIndex;
            o->Velocity = 0.f;
            o->Target = NULL;
            o->m_bCreateTails = true;
            o->byOnlyOneRender = 0;
            o->bTileMapping = false;
            o->m_byReverseUV = 0;
            o->m_bySkillSerialNum = (BYTE)SkillSerialNum;
            o->m_sTargetIndex = sTargetindex;
            VectorCopy(Position, o->Position);
            VectorCopy(Angle, o->Angle);
            if (vPriorColor)
            {
                VectorCopy(vPriorColor, o->Light);
            }
            else
            {
                Vector(1.f, 1.f, 1.f, o->Light);
            }
            Vector(0.f, 0.f, 0.f, o->Direction);
            if (Target == NULL)
            {
                VectorCopy(TargetPosition, o->TargetPosition);
            }
            else if (MODEL_SPEARSKILL == Type && o->SubType == 2)
            {
                VectorCopy(TargetPosition, o->TargetPosition);
                o->Target = Target;
            }
            else if (Type == MODEL_SPEARSKILL && (o->SubType == 4 || o->SubType == 9))
            {
                if (iChaIndex != -1)
                    o->m_iChaIndex = iChaIndex;
                o->Target = Target;
            }
            else if (Type == MODEL_SPEARSKILL && (o->SubType == 10 || o->SubType == 11))
            {
                if (iChaIndex != -1)
                {
                    o->m_iChaIndex = FindCharacterIndex(iChaIndex);
                }
                o->Target = Target;
            }
            else
            {
                o->Target = Target;
            }

            o->NumTails = 0;
            float Matrix[3][4];
            vec3_t Position, p;

            bool bCreateStartTail = true;
            if (Type == BITMAP_FLARE + 1 && o->SubType == 8)
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }
            else if (Type == BITMAP_SCOLPION_TAIL)
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }
            else if (Type == BITMAP_JOINT_ENERGY && (o->SubType == 10 || o->SubType == 11))
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }
            else if (Type == BITMAP_JOINT_ENERGY && (o->SubType == 14 || o->SubType == 15))
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }
            else if (Type == BITMAP_JOINT_ENERGY && (o->SubType == 55 || o->SubType == 56))
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }
            else if (Type == BITMAP_JOINT_ENERGY && o->SubType == 57)
            {
                bCreateStartTail = false;
                o->NumTails = -1;
            }

            if (Type == BITMAP_FLARE_FORCE && (o->SubType == 5 || o->SubType == 6 || o->SubType == 7))
            {
                bCreateStartTail = false;
            }

            if (Type == MODEL_SPEARSKILL && (o->SubType == 5 || o->SubType == 6 || o->SubType == 7))
            {
                bCreateStartTail = false;
            }

            if ((Type == BITMAP_FLASH) && (o->SubType == 6))
                bCreateStartTail = false;

            if (bCreateStartTail)
            {
                AngleMatrix(o->Angle, Matrix);
                Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][0]);
                Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][1]);
                Vector(0.f, 0.f, -o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][2]);
                Vector(0.f, 0.f, o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][3]);
            }

            vec3_t BitePosition;
            switch (Type)
            {
            case BITMAP_SCOLPION_TAIL:
                o->Scale = Scale;
                o->LifeTime = 120;
                o->MaxTails = 28;
                o->Velocity = 3.f;
                break;
            case BITMAP_JOINT_ENERGY:
                o->Scale = Scale;
                switch (o->SubType)
                {
                case 0:
                case 1:
                case 6:
                case 7:
                case 12:
                case 13:
                case 16:
                case 44:
                case 45:
                case 46:
                    o->LifeTime = 120;
                    o->MaxTails = 8;
                    o->Velocity = 3.f;
                    if (o->SubType == 44)
                    {
                        o->LifeTime = 45;
                        o->MaxTails = 20;
                    }
                    if (o->SubType == 12 || o->SubType == 13 || o->SubType == 16
                        || o->SubType == 46
                        )
                    {
                        if (rand() % 2)
                            o->MultiUse = 1;
                        else
                            o->MultiUse = -1;
                        o->Velocity = 10.f;
                        o->Angle[0] = (float)(rand() % 45);
                        o->Angle[1] = (float)(rand() % 45);
                    }
                    break;
                case 9:
                    Vector(0.2f, 0.2f, 0.2f, o->Light);
                    o->LifeTime = 120;
                    o->MaxTails = 8;
                    o->Velocity = 3.f;
                    break;
                case 17:
                    o->Velocity = 0.f;
                    o->LifeTime = 12;
                    VectorCopy(o->Target->EyeLeft, o->Position);
                    break;
                case 47:
                {
                    o->Velocity = 0.f;
                    o->LifeTime = 12;
                    VectorCopy(o->Target->EyeRight, o->Position);
                }break;
                case 2:
                case 3:
                case 4:
                case 5:
                case 8:
                case 10:
                case 11:
                case 14:
                case 15:
                case 18:
                case 19:
                case 20:
                case 21:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    if (o->SubType == 5) o->MaxTails = 10;
                    else if (o->SubType == 10 || o->SubType == 11) o->MaxTails = 5;
                    else if (o->SubType == 14 || o->SubType == 15) o->MaxTails = 15;
                    else o->MaxTails = 20;
                    if (o->SubType == 3 || o->SubType == 11 || o->SubType == 15)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeRight, o->Position);
                    }
                    else if (o->SubType == 18 || o->SubType == 28)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeLeft, o->Position)
                    }
                    else if (o->SubType == 19 || o->SubType == 29)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeRight, o->Position)
                    }
                    else if (o->SubType == 20 || o->SubType == 30)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeLeft2, o->Position)
                    }
                    else if (o->SubType == 21 || o->SubType == 31)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeRight2, o->Position)
                    }
                    else if (o->SubType == 26 || o->SubType == 32)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeLeft3, o->Position)
                    }
                    else if (o->SubType == 27 || o->SubType == 33)
                    {
                        o->MaxTails = 20;
                        VectorCopy(o->Target->EyeRight3, o->Position)
                    }
                    else
                    {
                        VectorCopy(o->Target->EyeLeft, o->Position);
                    }
                    o->TexType = BITMAP_JOINT_ENERGY;
                    if ((o->SubType >= 28 && o->SubType <= 33)
                        )
                    {
                        o->TexType = BITMAP_FLARE;
                    }
                    break;
                case 22:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    o->MaxTails = 10;
                    VectorCopy(o->Target->EyeLeft, o->Position);
                    break;
                case 23:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    o->MaxTails = 10;
                    VectorCopy(o->Target->EyeRight, o->Position);
                    break;
                case 24:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    o->MaxTails = 10;
                    VectorCopy(o->Target->EyeLeft, o->Position);
                    break;
                case 25:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    o->MaxTails = 10;
                    VectorCopy(o->Target->EyeRight, o->Position);
                    break;
                case 40:
                case 41:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    if (o->SubType == 5) o->MaxTails = 10;
                    VectorCopy(TargetPosition, o->Position);
                    break;
                case 42:
                {
                    o->LifeTime = 100;
                    o->MaxTails = 6;
                    o->Velocity = 3.f;
                }
                break;
                case 43:
                {
                    o->LifeTime = 100;
                    o->MaxTails = 6;
                    o->Velocity = 3.f;
                    o->Scale = Scale;
                }
                break;
                case 48:
                case 49:
                case 50:
                case 51:
                case 52:
                case 53:
                    o->LifeTime = 999999999;
                    o->MaxTails = 3;
                    Vector(0.5f, 0.5f, 0.9f, o->Light);
                    break;
                case 54:
                    o->Velocity = 0.f;
                    o->LifeTime = 999999999;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_SPARK + 1;
                    switch (o->PKKey)
                    {
                    case 0:
                        VectorCopy(o->Target->EyeRight2, o->Position);
                        break;
                    case 1:
                        VectorCopy(o->Target->EyeLeft2, o->Position);
                        break;
                    case 2:
                        VectorCopy(o->Target->EyeRight3, o->Position);
                        break;
                    case 3:
                        VectorCopy(o->Target->EyeLeft3, o->Position);
                        break;
                    }
                    break;
                case 55:
                case 56:
                {
                    o->Velocity = 0.f;
                    o->MaxTails = 8;
                    o->LifeTime = 10;
                    o->TexType = BITMAP_JOINT_ENERGY;

                    switch (o->SubType)
                    {
                    case 55: VectorCopy(o->Target->EyeLeft, o->Position); break; //left
                    case 56: VectorCopy(o->Target->EyeRight, o->Position); break; //rifht
                    }
                }
                break;
                case 57:
                {
                    o->Velocity = 0.f;
                    o->MaxTails = iChaIndex;
                    o->LifeTime = 10;
                    o->TexType = BITMAP_JOINT_ENERGY;
                }
                break;
                }

                if (!vPriorColor)
                {
                    switch (o->SubType)
                    {
                    case 0:Vector(0.4f, 0.3f, 0.2f, o->Light); break;
                    case 1:Vector(0.1f, 0.1f, 0.5f, o->Light); break;
                    case 2:
                    case 3:Vector(0.5f, 0.1f, 1.f, o->Light); break;
                    case 4:Vector(0.3f, 0.15f, 0.1f, o->Light); break;
                    case 5:Vector(0.5f, 0.1f, 1.f, o->Light); break;
                    case 6:Vector(0.4f, 0.3f, 0.2f, o->Light); break;
                    case 8:Vector(1.f, 0.f, 0.5f, o->Light); break;
                    case 10:
                    case 11:Vector(1.f, 0.3f, 0.1f, o->Light); break;
                    case 12:
                    case 13:Vector(0.7f, 0.3f, 1.f, o->Light); break;
                    case 14:
                    case 15:Vector(1.0f, 0.1f, 0.1f, o->Light); break;
                    case 16:Vector(0.4f, 0.2f, 0.4f, o->Light); break;
                    case 17:Vector(0.8f, 0.2f, 1.0f, o->Light); break;
                    case 18:
                    case 19:
                    case 20:
                    case 21:
                    case 26:
                    case 27: Vector(0.8f, 0.5f, 1.0f, o->Light); break;
                    case 42: Vector(0.0f, 0.0f, 0.0f, o->Light); break;
                    case 43: Vector(2.5f, 0.0f, 0.0f, o->Light); break;
                    case 46: Vector(0.1f, 0.25f, 0.1f, o->Light); break;
                    case 47: Vector(0.9f, 0.f, 0.f, o->Light); break;
                    }
                }

                break;

                //////////////////////////////////////////////////////////////////////////

            case BITMAP_JOINT_HEALING:
                o->LifeTime = 12;
                o->Scale = Scale;
                o->MaxTails = 2;
                o->Velocity = 0.f;

                if (o->SubType == 4)
                {
                    o->LifeTime = 30;
                    Vector((float)(rand() % 64 - 32), -10.f, 0.f, o->TargetPosition);
                }
                else if (o->SubType == 5)
                {
                    o->Scale += rand() % 10 - 5;
                    o->Velocity = (float)(rand() % 20 + 6);
                    o->LifeTime = rand() % 8 + 8;
                    o->MaxTails = 8;
                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (o->SubType == 6)
                {
                    o->MaxTails = 4;
                    Vector(1.f, 1.f, 0.5f, o->Light);
                }
                else if (o->SubType == 7)
                {
                    o->MaxTails = 4;
                    Vector(1.f, 1.f, 0.f, o->Light);
                }
                else if (o->SubType == 8)
                {
                    o->LifeTime = 17;
                    o->MaxTails = 3;
                    o->Light[0] = 0.5f;
                    o->Light[1] = 0.5f;
                    o->Light[2] = (rand() % 128) / 255.f + 0.5f;
                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->TargetPosition[2] += 100.f;
                }
                else if (o->SubType == 9)
                {
                    o->LifeTime = 90;
                    o->MaxTails = 20;
                    o->NumTails = 0;
                    o->MultiUse = (int)o->Angle[2];
                    o->Velocity = 0.f;
                    o->Direction[0] = 50.f;
                    o->Light[0] = 1.f / (11.f);
                    o->Light[1] = 0.5f / (11.f);
                    o->Light[2] = 1.f / (11.f);
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    VectorCopy(o->TargetPosition, o->Position);
                }
                else if (o->SubType == 10)
                {
                    o->LifeTime = 80;
                    o->MaxTails = 20;
                    o->NumTails = 0;
                    o->MultiUse = (int)o->Angle[2];
                    o->Velocity = 0.f;
                    o->Direction[0] = 80.f;
                    if (o->MultiUse == 225 || o->MultiUse == 405)
                    {
                        o->Collision = true;
                    }
                    else
                    {
                        o->Collision = false;
                    }
                    o->Light[0] = 1.f / (11.f);
                    o->Light[1] = 0.5f / (11.f);
                    o->Light[2] = 1.f / (11.f);
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    VectorCopy(o->TargetPosition, o->Position);
                }
                else if (o->SubType == 13)
                {
                    o->LifeTime = 17;
                    o->MaxTails = 10;
                    o->Light[0] = 1.0f;
                    o->Light[1] = 0.3f;
                    o->Light[2] = 0.2f;
                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->TargetPosition[2] += 200.f;
                }
                else if (o->SubType == 14)
                {
                    o->LifeTime = 10;
                    o->MaxTails = 10;
                    o->Light[0] = 0.8f;
                    o->Light[1] = 1.0f;
                    o->Light[2] = 0.8f;
                    VectorCopy(TargetPosition, o->TargetPosition);
                }
                else if (o->SubType == 15)
                {
                    o->LifeTime = 10;
                    o->Scale = Scale;
                    o->MaxTails = 2;
                    o->Velocity = 0.f;
                }
                else if (o->SubType == 16)
                {
                    o->LifeTime = 10;
                    o->Scale = Scale;
                    o->MaxTails = 2;
                    o->Velocity = 0.f;
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_MINUS;
                }
                else if (o->SubType == 17)
                {
                    o->LifeTime = 17;
                    o->MaxTails = 3;
                    o->Light[0] = (rand() % 128) / 255.f + 0.6f;
                    o->Light[1] = 0.1f;
                    o->Light[2] = 0.f;
                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->TargetPosition[2] += 100.f;
                }
                break;

            case BITMAP_2LINE_GHOST:
            {
                if (o->SubType == 0)
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_MINUS;
                    o->Velocity = 40.f + rand() % 20;
                    if (rand_fps_check(2))
                        o->LifeTime = 67;
                    else
                        o->LifeTime = 75;
                    o->MaxTails = 26;
                    o->Scale = Scale + (rand() % 200 + 1);
                    o->Direction[0] = 0;
                    o->m_sTargetIndex = 2;
                    //				CreateEffect(MODEL_DESAIR,o->Position,o->Angle,o->Light,0, (OBJECT*)o);
                    if (rand() % 3 < 2)
                    {
                        Vector(0.5f, 0.5f, 0.5f, o->Light);
                        CreateEffect(MODEL_DESAIR, o->Position, o->Angle, o->Light, 0, NULL, -1, 0, 0, 0, 1.f, i);
                    }
                }
                else if (o->SubType == 1)
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->LifeTime = 25 + rand() % 10;
                    o->MaxTails = 15;
                    o->Velocity = 20.f + rand() % 10;
                    VectorCopy(o->Position, o->TargetPosition);

                    vec3_t vAngle, vDir, vRandPos;
                    Vector(0.f, 0.f, (float)(rand() % 360), vAngle);
                    AngleMatrix(vAngle, Matrix);
                    Vector(0.f, 300.0f, 0.f, vDir);
                    VectorRotate(vDir, Matrix, vRandPos);

                    o->Position[0] += vRandPos[0];
                    o->Position[1] += vRandPos[1];
                    o->Position[2] = (rand() % 100) + 200;
                    o->TargetPosition[2] = o->Position[2] - (rand() % 100 - 100) * (rand_fps_check(2) ? 1 : -1);

                    VectorSubtract(o->TargetPosition, o->Position, o->Direction);

                    o->Angle[2] = CreateAngle2D(o->Position, o->TargetPosition);
                }
            }
            break;
            case BITMAP_JOINT_SPIRIT:
            case BITMAP_JOINT_SPIRIT2:
                o->RenderType = RENDER_TYPE_ALPHA_BLEND_MINUS;
                switch (o->SubType)
                {
                case 0:
                    o->Weapon = CharacterMachine->PacketSerial;
                    o->Velocity = 70.f;
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 6;
                    break;
                case 1:
                    o->Velocity = 70.f;
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 6;
                    PlayBuffer(SOUND_BRANDISH_SWORD03);
                    break;
                case 2:
                case 21:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->Velocity = 50.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 3;
                    Vector(0.5f, 0.5f, 0.5f, o->Light);
                    VectorCopy(o->Position, o->StartPosition);
                    //PlayBuffer();
                    break;
                case 3:
                    o->Velocity = 140.f;
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    if (o->Type == BITMAP_JOINT_SPIRIT2)
                    {
                        Vector(1.f, 1.f, 1.f, o->Light);
                    }
                    else
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                        Vector(1.0f, 0.5f, 0.1f, o->Light);
                    }
                    break;
                case 4:
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->LifeTime = 0;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(0.3f, 0.3f, 1.f, o->Light);
                    o->StartPosition[0] = (TargetPosition[0] - o->Position[0]) / o->MaxTails;
                    o->StartPosition[1] = (TargetPosition[1] - o->Position[1]) / o->MaxTails;
                    o->StartPosition[2] = (TargetPosition[2] - o->Position[2]) / o->MaxTails;
                    VectorCopy(o->StartPosition, Position);

                    AngleMatrix(o->Angle, Matrix);

                    for (int i = 0; i < (o->MaxTails - 1); i++)
                    {
                        if (o->Target == NULL)
                        {
                            Position[0] = o->StartPosition[0];// + rand()%4-2;
                            Position[1] = o->StartPosition[1];// + rand()%4-2;
                        }
                        else
                        {
                            Position[0] = o->StartPosition[0];// + rand()%8-4;
                            Position[1] = o->StartPosition[1];// + rand()%8-4;
                        }
                        VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                        CreateTail(o, Matrix);
                        o->Position[0] -= Position[0];
                        o->Position[0] += o->StartPosition[0];
                        o->Position[1] -= Position[1];
                        o->Position[1] += o->StartPosition[1];
                    }
                    VectorCopy(o->TargetPosition, o->Position);
                }
                break;
                case 5:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Weapon = CharacterMachine->PacketSerial;
                    o->Velocity = 30.f;
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 12;
                    break;
                case 6:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->Velocity = 50.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 5;
                    o->PKKey = rand() % 5;

                    if (o->PKKey)
                    {
                        o->m_bCreateTails = false;
                    }

                    switch (o->Skill)
                    {
                    case 0:
                        Vector(0.3f, 0.3f, 1.f, o->Light);
                        break;
                    case 1:
                        o->Velocity = 10.f;
                        Vector(0.5f, 0.5f, 0.5f, o->Light);
                        break;
                    }
                    VectorCopy(o->Position, o->StartPosition);
                    //PlayBuffer();
                    break;
                case 7:
                    o->RenderFace = 0;
                    o->Velocity = 10.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 5;
                    switch (o->Skill)
                    {
                    case 0:
                        Vector(0.3f, 0.3f, 1.f, o->Light);
                        break;
                    case 1:
                        o->Velocity = 10.f;
                        Vector(0.5f, 0.5f, 0.5f, o->Light);
                        break;
                    }
                    VectorCopy(o->Position, o->StartPosition);
                    //PlayBuffer();
                    break;
                case 8:
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    break;
                case 9:
                    o->RenderFace = RENDER_FACE_TWO;
                    o->Velocity = 10.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    Vector(1.f, 1.f, 1.f, o->Light);
                    Vector(-90.f, 0.f, 0.f, o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    VectorCopy(o->Position, o->StartPosition);
                    break;
                case 10:
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 6;
                    break;
                case 11:
                    o->Velocity = 0;//30.f;
                    o->LifeTime = 50;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    break;
                case 12:
                    o->Velocity = 0;//30.f;
                    o->LifeTime = 30;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    break;
                case 13:
                    if (o->Type == BITMAP_JOINT_SPIRIT)
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    }
                    o->Velocity = 40.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    Vector(1.0f, 0.6f, 0.4f, o->Light);
                    break;
                case 14:
                    o->Velocity = 0;
                    o->LifeTime = 10;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    o->Angle[0] = 20;
                    break;
                case 15:
                    o->Velocity = 0;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    break;
                case 16:
                    o->Velocity = 0;
                    o->LifeTime = 50;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    break;
                case 17:
                    o->Velocity = 0;
                    o->LifeTime = 100;
                    o->Scale = Scale;
                    o->MaxTails = 1;
                    break;
                case 18:
                    if (o->Type == BITMAP_JOINT_SPIRIT2)
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    }
                    o->Velocity = 50.f;
                    o->LifeTime = 39;
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    Vector(0.7f, 0.7f, 0.7f, o->Light);
                    break;
                case 19:
                    if (o->Type == BITMAP_JOINT_SPIRIT)
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    }
                    o->Velocity = 70.f;
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 9;
                    Vector(0.1f, 0.5f, 0.2f, o->Light);
                    break;
                case 20:
                    if (o->Type == BITMAP_JOINT_SPIRIT2)
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    }
                    o->LifeTime = 49;
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    break;
                case 22:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->Velocity = 60.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 0;
                    VectorCopy(o->Position, o->StartPosition);
                    break;
                case 23:
                    o->RenderFace = 0;
                    o->Velocity = 10.f;
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->MaxTails = 0;
                    VectorCopy(o->Position, o->StartPosition);
                    break;
                case 24:
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->Velocity = 10.f;
                    o->LifeTime = 160;
                    o->Scale = Scale;
                    o->MaxTails = 40;
                    Vector(0.0f, 0.0f, 0.0f, o->Light);

                    o->Position[0] += (float)(rand() % 300 - 150);
                    o->Position[1] += (float)(rand() % 300 - 150);

                    float fRargleScale = o->Scale / 70.f;
                    if (0.9f <= fRargleScale)
                    {
                        vec3_t vLight;
                        Vector(0.0f, 0.0f, 0.1f, vLight);
                        CreateEffect(MODEL_SUMMONER_SUMMON_LAGUL, o->Position, o->Angle,
                            vLight, 1, (OBJECT*)o, -1, 0, 0, 0, fRargleScale);
                    }
                }
                break;
                case 25:
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->TexType = BITMAP_SHINY;
                    o->Velocity = 9.f;
                    o->LifeTime = 26;
                    o->Scale = Scale;
                    o->MaxTails = 30;
                    o->Skill = SkillIndex;
                    Vector(0.9f, 0.8f, 1.f, o->Light);
                    Vector(-90.f, 0.f, 0.f, o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    VectorCopy(o->Position, o->StartPosition);
                }
                break;
                }
                break;
            case BITMAP_JOINT_LASER:
                o->bTileMapping = true;
                o->Velocity = 70.f;
                o->LifeTime = 49;
                o->Scale = Scale;
                o->MaxTails = 6;
                break;
            case BITMAP_JOINT_SPARK:
                switch (o->SubType)
                {
                case 0:
                    o->Scale = 2.f;
                    o->Velocity = (float)(rand() % 20 + 6);
                    o->LifeTime = rand() % 8 + 8;
                    o->MaxTails = 2;
                    Vector(1.f, 1.f, 1.f, o->Light);
                    break;
                case 1:
                    o->Scale = 2.f;
                    o->Velocity = (float)(rand() % 20 + 16);
                    o->LifeTime = rand() % 4 + 4;
                    o->MaxTails = 2;
                    Vector(1.f, 1.f, 1.f, o->Light);
                    break;
                case 2:
                    o->Scale = 4.f;
                    o->Velocity = 30.0f;
                    o->LifeTime = rand() % 4 + 4;
                    o->MaxTails = 2;
                    Vector(0.3f, 0.3f, 1.f, o->Light);
                    break;
                case 3:
                    o->Scale = 2.f;
                    o->Velocity = (float)(rand() % 20 + 6);
                    o->LifeTime = rand() % 8 + 8;
                    o->MaxTails = 2;
                    Vector(1.f, 1.f, 1.f, o->Light);
                    break;
                case 4:
                    o->Scale = Scale * 2 + 4.f;
                    o->Velocity = (float)(rand() % 5 + 4);
                    o->LifeTime = rand() % 10 + 8;
                    o->MaxTails = 4;
                    Vector(1.f, 0.2f, 0.2f, o->Light);
                    break;
                case 5:
                    o->Scale = 2.f;
                    o->Velocity = (float)(rand() % 20 + 6);
                    o->LifeTime = rand() % 8 + 8;
                    o->MaxTails = 2;
                    Vector(0.7f, 0.1f, 0.1f, o->Light);
                    break;
                }
                break;

            case BITMAP_SMOKE:
                o->Scale = Scale;
                Vector(1.f, 1.f, 1.f, o->Light);
                if (o->SubType == 0)
                {
                    o->MaxTails = 20;
                    o->LifeTime = 20;
                    o->Velocity = 0.f;
                    o->TexType = BITMAP_FLARE;
                }
                else if (o->SubType == 1)
                {
                    o->MaxTails = 15;
                    o->LifeTime = 15;
                    o->Velocity = 0.f;
                }
                else if (o->SubType == 2)
                {
                    o->MaxTails = 25;
                    o->LifeTime = 22;
                    o->Velocity = 0.f;
                    o->TexType = BITMAP_FLARE;
                    Vector(0.1f, 0.3f, 1.0f, o->Light);
                }
                break;
            case MODEL_FENRIR_SKILL_THUNDER:
                o->Scale = Scale;
                o->MaxTails = 50;
                o->Velocity = 50.f;
                o->LifeTime = 20;
                o->bTileMapping = true;

                if (o->SubType == 0)
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                    Vector(0.7f, 1.0f, 0.7f, o->Light);
                }
                else if (o->SubType == 1)
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                    Vector(1.0f, 0.6f, 0.6f, o->Light);
                }
                else if (o->SubType == 2)
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                    Vector(0.7f, 0.7f, 1.0f, o->Light);
                }
                else if (o->SubType == 3)
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                    Vector(0.9f, 0.9f, 0.3f, o->Light);
                }
                else if (o->SubType == 4)
                {
                    o->TexType = BITMAP_FLASH;
                    Vector(0.1f, 0.8f, 0.1f, o->Light);
                }
                else if (o->SubType == 5)
                {
                    o->TexType = BITMAP_FLASH;
                    Vector(1.0f, 0.3f, 0.2f, o->Light);
                }
                else if (o->SubType == 6)
                {
                    o->TexType = BITMAP_FLASH;
                    Vector(0.2f, 0.3f, 1.0f, o->Light);
                }
                else if (o->SubType == 7)
                {
                    o->TexType = BITMAP_FLASH;
                    Vector(0.8f, 0.8f, 0.1f, o->Light);
                }
                break;
            case BITMAP_JOINT_LASER + 1:
                o->bTileMapping = true;
                break;
            case BITMAP_BLUR + 1:
                o->Scale = 60.f;
                o->Velocity = 40.f;
                o->MaxTails = 50;
                o->LifeTime = 2;
                if (o->SubType == 0)
                {
                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (o->SubType == 3)
                {
                    o->LifeTime = 20;
                    o->Velocity = 0.f;
                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else
                {
                    if (o->Type == BITMAP_JOINT_LASER + 1)
                    {
                        Vector(.35f, .1f, 1.f, o->Light);
                    }
                    else
                    {
                        Vector(0.f, 0.3f, 1.f, o->Light);
                    }
                }
                break;
            case BITMAP_JOINT_THUNDER:
                o->Scale = Scale;
                o->MaxTails = 50;
                o->Velocity = 50.f;
                switch (o->SubType)
                {
                case 0:
                case 20:
                    o->LifeTime = 2;
                    break;

                case 1:
                    o->Velocity = 20.f + (float)(rand() % 10);
                    o->LifeTime = rand() % 8 + 8;
                    break;

                case 2:
                    o->LifeTime = 2;
                    Vector(1.f, 0.1f, 0.f, o->Light);
                    Vector(0.f, -150.f, 0.f, BitePosition);//42
                    Models[MODEL_PLAYER].TransformPosition(o->Target->BoneTransform[33], BitePosition, o->TargetPosition, true);
                    break;

                case 3:
                    o->Velocity = SkillIndex;
                    o->PKKey = -1;
                    o->LifeTime = PKKey;
                    o->MaxTails = SkillIndex;
                    Vector(0.5f, 0.5f, 1.f, o->Light);
                    VectorCopy(o->Position, o->StartPosition);
                    break;

                case 4:
                    o->Velocity = 60.f;
                    o->LifeTime = 20;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    Vector(0.5f, 0.5f, 1.f, o->Light);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    VectorCopy(o->Position, o->StartPosition);
                    break;

                case 5:
                    o->Velocity = -60.f;
                    o->LifeTime = 20;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    Vector(0.5f, 0.5f, 1.f, o->Light);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    VectorCopy(o->Position, o->StartPosition);
                    o->TargetPosition[2] += 600.f;
                    break;

                case 6:
                    o->LifeTime = rand() % 20 + 6;
                    o->m_bCreateTails = false;
                    o->Velocity = 15.f + (float)(rand() % 10);

                    o->Light[0] = (rand() % 10) / 15.f + 0.1f;
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                    VectorCopy(o->Position, o->StartPosition);
                    break;

                case 7:
                    o->bTileMapping = TRUE;
                    o->Velocity = 5.f;
                    o->MaxTails = 3;
                    o->LifeTime = 1;
                    Vector(0.5f, 0.5f, 1.f, o->Light);
                    VectorCopy(o->Position, o->StartPosition);

                    MoveHumming(o->Position, o->Angle, o->TargetPosition, 360.f);
                    break;

                case 8:
                    o->Scale = 5.f;
                    o->Velocity = (float)(rand() % 10 + 6);
                    o->LifeTime = rand() % 8 + 8;
                    o->MaxTails = 2;
                    Vector(1.f, 1.f, 1.f, o->Light);
                    break;

                case 9:
                {
                    o->Velocity = 80.f + (float)(rand() % 20);
                    o->LifeTime = 30;

                    o->Light[0] = (rand() % 10) / 15.f + 0.1f;
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                    VectorCopy(o->Position, o->StartPosition);
                }
                break;

                case 10:
                {
                    o->bTileMapping = TRUE;
                    o->LifeTime = 0;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(0.3f, 0.3f, 1.f, o->Light);
                    o->StartPosition[0] = (TargetPosition[0] - o->Position[0]) / o->MaxTails;
                    o->StartPosition[1] = (TargetPosition[1] - o->Position[1]) / o->MaxTails;
                    o->StartPosition[2] = (TargetPosition[2] - o->Position[2]) / o->MaxTails;
                    VectorCopy(o->StartPosition, Position);

                    AngleMatrix(o->Angle, Matrix);

                    for (int i = 0; i < (o->MaxTails - 1); i++)
                    {
                        if (o->Target == NULL)
                        {
                            Position[0] = o->StartPosition[0] + rand() % 8 - 4;
                            Position[1] = o->StartPosition[1] + rand() % 8 - 4;
                        }
                        else
                        {
                            Position[0] = o->StartPosition[0] + rand() % 16 - 8;
                            Position[1] = o->StartPosition[1] + rand() % 16 - 8;
                        }
                        VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                        CreateTail(o, Matrix);
                        o->Position[0] -= Position[0];
                        o->Position[0] += o->StartPosition[0];
                        o->Position[1] -= Position[1];
                        o->Position[1] += o->StartPosition[1];
                    }
                    VectorCopy(o->TargetPosition, o->Position);
                    //                        CreateTail ( o, Matrix );
                }
                break;
                case 11:
                    Vector(1.0f, 0.5f, 0.1f, o->Light);
                    o->LifeTime = 2;
                    break;
                case 12:
                    Vector(1.0f, 0.1f, 0.1f, o->Light);
                    o->LifeTime = 10;
                    break;
                case 13:
                    Vector(1.0f, 0.1f, 0.1f, o->Light);
                    o->LifeTime = 2;
                    o->SubType = 11;
                    break;
                case 14:
                {
                    o->LifeTime = 0;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(0.3f, 0.3f, 1.f, o->Light);
                    o->StartPosition[0] = (TargetPosition[0] - o->Position[0]) / o->MaxTails;
                    o->StartPosition[1] = (TargetPosition[1] - o->Position[1]) / o->MaxTails;
                    o->StartPosition[2] = (TargetPosition[2] - o->Position[2]) / o->MaxTails;
                    VectorCopy(o->StartPosition, Position);

                    AngleMatrix(o->Angle, Matrix);

                    for (int i = 0; i < (o->MaxTails - 1); i++)
                    {
                        if (o->Target == NULL)
                        {
                            Position[0] = o->StartPosition[0];
                            Position[1] = o->StartPosition[1];
                        }
                        else
                        {
                            Position[0] = o->StartPosition[0];
                            Position[1] = o->StartPosition[1];
                        }
                        VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                        CreateTail(o, Matrix);
                        o->Position[0] -= Position[0];
                        o->Position[0] += o->StartPosition[0];
                        o->Position[1] -= Position[1];
                        o->Position[1] += o->StartPosition[1];
                    }
                    VectorCopy(o->TargetPosition, o->Position);
                    //                        CreateTail ( o, Matrix );
                }
                break;
                case 15:
                    o->RenderFace = 0;
                    o->LifeTime = 80;
                    o->MaxTails = 0;
                    o->MultiUse = 0;

                    for (int j = 0; j < MAX_CHARACTERS_CLIENT; j++)
                    {
                        CHARACTER* tc = &CharactersClient[j];
                        OBJECT* to = &tc->Object;

                        float dx = o->Position[0] - to->Position[0];
                        float dy = o->Position[1] - to->Position[1];
                        float Distance = sqrtf(dx * dx + dy * dy);
                        if (Distance <= 400 && to->Live && tc->Dead == 0 && to->Kind == KIND_MONSTER && to->Visible && to != Target)
                        {
                            o->TargetIndex[(int)o->MultiUse] = j;
                            o->MultiUse += FPS_ANIMATION_FACTOR;
                        }

                        if (o->MultiUse > 5) break;
                    }
                    o->Weapon = o->MultiUse * 15;
                    o->MultiUse = 0;

                    VectorCopy(TargetPosition, o->StartPosition);
                    o->StartPosition[2] += 150.f;
                    break;
                case 16:
                    o->Velocity = 20.f + (float)(rand() % 10);
                    o->LifeTime = rand() % 2 + 2;
                    break;
                case 17:
                    o->Velocity = 20.f + (float)(rand() % 10);
                    o->LifeTime = 10;
                    break;
                case 18:
                    o->Velocity = 80.f;
                    o->MaxTails = 5;
                    o->LifeTime = 10;
                    o->Light[0] = o->Light[1] = o->Light[2] = 1.0f;
                    VectorCopy(o->Position, o->StartPosition);
                    break;
                case 19:
                {
                    o->MaxTails = 30;
                    o->LifeTime = 2;
                    o->Velocity = o->Scale * 3.0f;
                    VectorCopy(o->Position, o->StartPosition);
                }
                break;
                case 21:
                    o->LifeTime = 2;
                    Vector(1.f, 0.5f, 0.4f, o->Light);
                    Vector(0.f, -150.f, 0.f, BitePosition);//42
                    Models[MODEL_PLAYER].TransformPosition(o->Target->BoneTransform[33], BitePosition, o->TargetPosition, true);
                    break;
                    // ChainLighting
                case 22:
                case 23:
                case 24:
                    o->LifeTime = 15;
                    o->Scale = Scale + (rand() % 50 + 50) * 0.1f;
                    o->MaxTails = 30;
                    o->Velocity = 20.f;
                    break;
                case 25:
                {
                    o->LifeTime = 20;
                    o->MaxTails = 8;
                    //o->Velocity = 50.f;
                    o->Velocity = 20.f + (float)(rand() % 10);
                }
                break;
                case 26:
                    o->Velocity = 20.f + (float)(rand() % 10);
                    o->LifeTime = rand() % 8 + 8;
                    //o->MaxTails = 10;
                    break;
                case 27:
                case 28:
                {
                    o->LifeTime = 2;
                    VectorCopy(vPriorColor, o->Light);
                }
                break;
                case 33:
                    Vector(0.3f, 0.3f, 1.0f, o->Light);
                    o->Velocity = 20.f + (float)(rand() % 10);
                    o->LifeTime = rand() % 2 + 2;
                    break;
                }
                break;

            case BITMAP_JOINT_THUNDER + 1:
                o->Scale = Scale;
                o->TexType = BITMAP_JOINT_THUNDER;

                if (o->SubType == 0)
                {
                    o->MaxTails = 50;

                    o->Velocity = 80.f + (float)(rand() % 20);
                    o->LifeTime = 20;

                    o->Light[2] = (float)o->MaxTails;
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 1 || o->SubType == 2 || o->SubType == 3 || o->SubType == 5 || o->SubType == 6 || o->SubType == 7)
                {
                    o->MaxTails = 50;
                    o->LifeTime = 20;
                    o->Position[0] += (float)(rand() % 10 - 5);
                    o->Position[1] += rand() % 10 - 5;

                    if (o->SubType == 7)
                    {
                        o->m_bCreateTails = false;
                        o->Position[2] += 1050.0f;
                    }
                    else
                    {
                        o->Position[2] += 800.f;
                    }
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    VectorCopy(TargetPosition, o->TargetPosition);

                    if (o->SubType == 1)
                    {
                        o->m_bCreateTails = false;
                    }
                    VectorCopy(o->Position, o->StartPosition);

                    if (o->SubType == 5)
                    {
                        Vector(1.f, 0.5f, 0.2f, o->Light);
                    }
                    else if (o->SubType == 6)
                    {
                        o->Velocity = 0.3f;
                        Vector(1.0f, 1.0f, 1.0f, o->Light);
                        o->Position[2] += 700.f;
                    }
                }
                else if (o->SubType == 4)
                {
                    o->MaxTails = 50;
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;
                    o->byOnlyOneRender = 2;
                    o->TargetIndex[0] = (int)(o->MaxTails / 1.5f);
                    o->TargetIndex[1] = o->MaxTails - 1;
                    o->StartPosition[0] = (float)(1.f / o->MaxTails);
                    o->StartPosition[1] = (float)(1.f / (o->MaxTails - o->TargetIndex[0] - 1));
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(1.f, 0.8f, 1.f, o->Light);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->TargetPosition[2] += 100.f;
                    Angle[2] = CreateAngle2D(o->Position, o->TargetPosition);
                }
                else if (o->SubType == 8)
                {
                    o->MaxTails = 50;
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;

                    o->Position[0] += (float)(rand() % 10 - 5);
                    o->Position[1] += rand() % 10 - 5;
                    o->Position[2] += 1100.0f;

                    Vector(0.f, 0.f, 0.f, o->Angle);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 9)
                {
                    o->MaxTails = 50;
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;

                    o->Position[0] -= 50.0f;

                    Vector(0.f, 0.f, 0.f, o->Angle);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 10)
                {
                    o->MaxTails = 50;
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;

                    o->Position[1] -= 350.0f;

                    Vector(0.f, 0.f, 0.f, o->Angle);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 11)
                {
                    o->Velocity = 80.f + (float)(rand() % 20);
                    o->Light[2] += 0.2f;
                    o->LifeTime = 5;
                    o->MaxTails = 30;
                    VectorCopy(o->Position, o->StartPosition);
                    VectorCopy(TargetPosition, o->TargetPosition);
                }
                else if (o->SubType == 12)
                {
                    o->Velocity = 80.f + (float)(rand() % 20);
                    o->Light[2] = (float)o->MaxTails;
                    o->MaxTails = 30;
                    o->LifeTime = 4;
                    o->m_bCreateTails = false;
                    VectorCopy(o->Position, o->StartPosition);

                    vec3_t p1, p2;
                    Vector(0.f, o->Scale / 1.0f, 0.f, p1);
                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(p1, Matrix, p2);

                    Vector(p2[0] * o->Scale / 2.0f, p2[1] * o->Scale / 2.0f, p2[2] * o->Scale / 2.0f, p2);
                    VectorAddScaled(o->Position, p2, o->Position, FPS_ANIMATION_FACTOR);
                    VectorCopy(o->Position, o->StartPosition);

                    int iScale = 1;
                    iScale = (int)(o->Scale / 4.0f);
                    o->TargetPosition[0] = TargetPosition[0] + rand() % (iScale * 2) - iScale;
                    o->TargetPosition[1] = TargetPosition[1] + rand() % (iScale * 2) - iScale;
                    o->TargetPosition[2] = TargetPosition[2] + rand() % (iScale * 2) - iScale;
                }
                break;
            case BITMAP_JOINT_FIRE:
                o->Scale = 70.f;
                o->Velocity = 50.f;
                o->MaxTails = 8;
                o->LifeTime = 20;
                Vector(0.f, 0.f, 130.f, BitePosition);
                VectorAdd(o->TargetPosition, BitePosition, o->TargetPosition);
                //PlayBuffer(SOUND_MAGIC_FIRE01);
                break;
            case BITMAP_SPARK + 1:
                o->LifeTime = 100;
                o->MaxTails = 20;
                o->Scale = 10.f;
                if (o->SubType == 0)
                {
                    o->Direction[2] = (float)(rand() % 20 + 35);
                    o->Scale = rand() % 20 + 20.f;
                    o->LifeTime = 25;
                }
                else
                    if (o->SubType == 1)
                    {
                        o->Scale = Scale;
                        o->Velocity = (float)(rand() % 55 + 14);
                        o->LifeTime = rand() % 10 + 8;
                        o->MaxTails = 4;
                    }
                Vector(1.f, 1.f, 1.f, o->Light);
                VectorCopy(o->Position, o->TargetPosition);
                break;
            case MODEL_SPEARSKILL:
                VectorCopy(o->Target->Position, o->TargetPosition);
                switch (o->SubType)
                {
                case 0:
                    Vector(1.f, 1.f, 1.f, o->Light);
                    o->LifeTime = 999999;//30 * 24;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_FLARE_BLUE;
                    break;
                case 4:
                case 9:
                    Vector(.4f, .8f, .2f, o->Light);
                    o->LifeTime = 10000;//30 * 24;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_FLARE_BLUE;
                    break;
                case 10:
                case 11:
                    Vector(1.f, 0.6f, 0.6f, o->Light);
                    o->LifeTime = 10000;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_LUCKY_SEAL_EFFECT;
                    break;
                case 1:
                    Vector(.2f, .2f, .2f, o->Light);
                    o->LifeTime = 999999;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_JOINT_SPIRIT;
                    break;
                case 2:
                    VectorCopy(TargetPosition, o->TargetPosition);
                    Vector(1.0f, .3f, .3f, o->Light);
                    o->LifeTime = 20;
                    o->MaxTails = 5;
                    o->TexType = BITMAP_FLARE_FORCE;
                    break;
                case 3:
                    if (o->Target != NULL)
                    {
                        VectorCopy(o->Target->Light, o->Light);
                    }
                    else
                    {
                        Vector(0.5f, 0.f, 0.f, o->Light);
                    }
                    Vector(-90.f, 0.f, 0.f, o->Angle);
                    Vector(0.f, (float)(rand() % 500), 0.f, o->Direction);
                    o->Velocity = rand() % 5 + 5.f;
                    o->LifeTime = 999999;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_JOINT_SPIRIT;
                    break;
                case 5:
                case 6:
                case 7:
                    o->RenderFace = RENDER_FACE_ONE;
                    o->LifeTime = 60;
                    o->MaxTails = 30;
                    o->Weapon = 0;
                    o->m_bCreateTails = false;

                    if (o->SubType == 5)
                    {
                        Vector(1.f, 1.f, 0.8f, o->Light);
                        o->TexType = BITMAP_FLARE + 1;
                    }
                    else if (o->SubType == 6)
                    {
                        Vector(1.f, 0.8f, 1.f, o->Light);
                        o->TexType = BITMAP_FLARE + 1;
                    }
                    else if (o->SubType == 7)
                    {
                        Vector(0.8f, 1.0f, 1.0f, o->Light);
                        o->TexType = BITMAP_FLARE + 1;
                    }
                    Vector(0.f, 800.f, 0.f, o->Direction);
                    VectorCopy(TargetPosition, o->StartPosition);

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->Position);

                    o->NumTails = -1;
                    CreateTail(o, Matrix);
                    break;
                case 8:
                    o->RenderFace = RENDER_FACE_ONE;
                    o->LifeTime = 40;
                    o->MaxTails = 30;
                    o->m_bCreateTails = false;

                    Vector(0.5f, 0.5f, 0.5f, o->Light);
                    o->TexType = BITMAP_LIGHT;
                    Vector(0.f, -40.f, 0.f, o->Direction);
                    VectorCopy(TargetPosition, o->StartPosition);

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->Position);
                    break;
                case 14:
                    VectorCopy(vPriorColor, o->Light);
                    o->LifeTime = 100;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_LIGHT;
                    break;
                case 15:
                    Vector(1.0f, 1.0f, 1.0f, o->Light);
                    o->LifeTime = 100;
                    o->MaxTails = 20;
                    o->TexType = BITMAP_JOINT_SPIRIT;
                    VectorCopy(o->Target->Owner->Position, o->StartPosition);
                    break;
                case 16:
                    Vector(1.f, 1.f, 0.f, o->Light);
                    o->LifeTime = 100;
                    o->MaxTails = 30;
                    o->TexType = BITMAP_LIGHT;
                    break;
                case 17:
                    Vector(0.7f, 0.2f, 1.f, o->Light);
                    o->LifeTime = 100;
                    o->MaxTails = 20;
                    o->TexType = BITMAP_JOINT_SPIRIT;
                    VectorCopy(o->Target->Owner->Position, o->StartPosition);
                    break;
                }
                o->Scale = Scale;
                break;
            case BITMAP_FLARE:
            case BITMAP_FLARE_BLUE:
                o->LifeTime = 100;
                o->MaxTails = 20;
                o->Scale = 10.f;
                if (o->SubType == 0 || o->SubType == 18)
                {
                    if (o->Type == BITMAP_FLARE && o->SubType == 18)
                    {
                        o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    }

                    o->Scale = Scale;
                    o->Direction[2] = (rand() % 150) / 100.f;
                    o->Direction[1] = (float)(rand() % 500 - 250);
                    o->Velocity = 40.f;

                    if (o->Scale > 10.f)
                    {
                        o->LifeTime = 50;
                        o->Direction[2] = (rand() % 250 + 200) / 100.f;
                    }

                    VectorCopy(o->Target->Light, o->Light);
                }
                else if (o->SubType == 20)
                {
                    if (o->Target == NULL)
                    {
                        o->Live = false;
                        break;
                    }
                    o->LifeTime = 30;
                    o->MaxTails = 10;
                    o->Scale = Scale;
                    o->TexType = BITMAP_FIRECRACKER;

                    Vector(0.8f, 0.3f, 1.f, o->Light);

                    BMD* b = &Models[o->Target->Type];
                    vec3_t p;
                    Vector(0.0f, 0.0f, 0.0f, p);
                    b->TransformPosition(o->Target->BoneTransform[33], p, o->Position, true);
                }
                else if (o->SubType == 10)
                {
                    o->Scale = Scale;
                    o->Direction[2] = (rand() % 150) / 100.f;
                    o->Direction[1] = (float)(rand() % 500 - 250);
                    if (o->SubType == 25)
                        o->Velocity = 140.f;
                    else
                        o->Velocity = 40.f;

                    if (o->Scale > 10.f)
                    {
                        o->LifeTime = 50;
                        o->Direction[2] = (rand() % 250 + 200) / 100.f;
                    }

                    VectorCopy(o->Target->Light, o->Light);
                }
                else if (o->SubType == 14 || o->SubType == 15)
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->TexType = BITMAP_JOINT_SPIRIT;
                    o->Direction[2] = (rand() % 150) / 100.f;
                    o->Direction[1] = (float)(rand() % 500 - 250);
                    o->Velocity = 30.f;

                    //VectorCopy ( o->Target->Light, o->Light );
                    Vector(0.2f, 0.4f, 0.8f, o->Light);

                    if (o->Scale > 10.f)
                    {
                        o->Direction[2] = (rand() % 250 + 150) / 100.f;
                    }
                    o->LifeTime = 20;

                    o->Position[2] += rand() % 25;
                }
                else if (o->SubType == 16)
                {
                    o->LifeTime = 60;
                    o->m_bCreateTails = false;
                    Vector(0.1f, 0.1f, 0.1f, o->Light);
                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->Scale = Scale;
                    for (int j = 0; j < o->MaxTails; ++j)
                    {
                        o->Position[2] += 50.f;

                        CreateTail(o, Matrix);
                    }
                }
                else if (o->SubType == 17)
                {
                    o->Position[0] += rand() % 500 - 500;
                    o->Position[1] += rand() % 500 - 500;

                    o->Direction[0] = 0.f;
                    o->Direction[1] = 0.f;
                    o->Direction[2] = 0.f;//(float)(rand()%5+2);
                    o->Velocity = (rand() % 200 + 10) / 25.f;
                    o->Scale = Scale;
                    o->LifeTime = 20 + rand() % 10;

                    Vector(0.f, 0.f, 1.f, o->Light);
                }
                else if (o->SubType == 2 || o->SubType == 24 || o->SubType == 50 || o->SubType == 51)
                {
                    if (o->SubType == 24)
                        o->TexType = BITMAP_FLARE_RED;
                    else
                        if (o->SubType == 50 || o->SubType == 51)
                            o->TexType = BITMAP_FLARE_BLUE;
                    o->Direction[2] = (float)(rand() % 20 + 35);
                    o->Scale = Scale;
                    o->LifeTime = 25 + rand() % 50;

                    Vector(1.f, 1.f, 1.f, o->Light);
                }
#ifdef GUILD_WAR_EVENT
                else if (o->SubType == 21)
                {
                    o->Direction[2] = (float)(rand() % 20 + 35);
                    o->Scale = Scale;
                    o->LifeTime = 40 + rand() % 10;
                    o->Light[0] = 0.7f;
                    o->Light[1] = 0.5f + (float)(rand() % 127) / 255.f;
                    o->Light[2] = 0.5f + (float)(rand() % 127) / 255.f;
                }
                else if (o->SubType == 22)
                {
                    VectorCopy(o->Target->Position, o->StartPosition);
                    o->Direction[2] = (float)(rand() % 20 + 35);
                    o->Scale = Scale;
                    o->LifeTime = 40 + rand() % 10;
                    o->Light[0] = 0.5f + (float)(rand() % 127) / 255.f;
                    o->Light[1] = 0.2f + (float)(rand() % 204) / 255.f;
                    o->Light[2] = 0.2f + (float)(rand() % 204) / 255.f;
                }
                else if (o->SubType == 40)
                {
                    o->Scale = Scale;
                    o->LifeTime = 50;
                    o->MaxTails = 50;
                    Vector(1.f, 1.f, 1.f, o->Light);

                    o->Direction[0] = -2.0f * (float)sinf(-o->Angle[2] * Q_PI / 180.0f);
                    o->Direction[1] = -2.0f * (float)cosf(-o->Angle[2] * Q_PI / 180.0f);
                    o->Direction[2] = 0.f;
                }
#endif //GUILD_WAR_EVENT

                else if (o->SubType == 41)
                {
                    o->Scale = Scale;
                    o->LifeTime = 40;
                    o->MaxTails = 50;
                    float rbias = (float)(rand() % 300) / 1000;
                    float gbias = (float)(rand() % 300) / 1000;
                    Vector(1.f - rbias, 1.f - gbias, 1.f, o->Light);
                    o->Direction[2] = (float)(rand() % 5 + 5);
                }
                else if (o->SubType == 42)
                {
                    o->Direction[1] = -15.f;
                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                    VectorCopy(o->Position, o->StartPosition);
                    o->Direction[1] = -50.f;
                    o->m_bCreateTails = false;
                    o->Scale = Scale;
                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (o->SubType == 19)
                {
                    o->Direction[2] = -(float)(rand() % 20 + 35);
                    o->Scale = Scale;
                    o->LifeTime = 25 + rand() % 50;

                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (o->SubType == 3)
                {
                    o->Velocity = 50.f;
                    o->Scale = Scale;
                    o->LifeTime = 5;
                    o->MaxTails = 10;

                    float Matrix[3][4];
                    Vector(0.f, 45.f, -90.f, o->Angle);
                    AngleMatrix(o->Angle, Matrix);
                    Vector(0.f, 100, 0.f, p);
                    VectorRotate(p, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

                    CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 8 + 8) * 0.3f, o->Light, NULL, (float)(rand() % 360));

                    Vector(1.f, 1.f, 1.f, o->Light);
                    o->Angle[0] *= -1.f;
                    o->Angle[1] *= -1.f;
                    o->Angle[2] *= -1.f;
                }
                else if (o->SubType == 4 || o->SubType == 6 || o->SubType == 12)
                {
                    o->Scale = Scale;
                    if (o->SubType == 12)
                    {
                        o->LifeTime = 70;
                        o->MaxTails = 50;
                        //Vector(1.f,1.f,1.f,o->Light);
                        Vector(0.1f, 0.1f, 1.f, o->Light);

                        //o->Direction[1] = rand()%150/100.f;
                        o->Direction[0] = (float)(rand() % 360);
                        o->Direction[1] = -4.f * (float)sinf(-o->Angle[2] * Q_PI / 180.0f);
                        o->Direction[2] = -4.f * (float)cosf(-o->Angle[2] * Q_PI / 180.0f);
                    }
                    else
                    {
                        if (o->SubType == 6)
                        {
                            o->LifeTime = 20;
                            o->MaxTails = 30;
                            if (o->Type == BITMAP_FLARE_BLUE)
                            {
                                o->LifeTime = 15;
                                o->MaxTails = 15;
                            }
                            Vector(1.f, 1.f, 1.f, o->Light);
                        }
                        else
                        {
                            o->LifeTime = 110;
                            o->MaxTails = 200;

                            VectorCopy(o->Target->Light, o->Light);
                        }
                        o->Direction[0] = (float)(rand() % 360);
                        o->Direction[1] = -2.0f * (float)sinf(-o->Angle[2] * Q_PI / 180.0f);
                        o->Direction[2] = -2.0f * (float)cosf(-o->Angle[2] * Q_PI / 180.0f);
                    }
                }
                else if (o->SubType == 5)
                {
                    o->LifeTime = 2;
                    o->MaxTails = 3;
                    o->Direction[2] = -(float)(rand() % 3 + 40);

                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (o->SubType == 7)
                {
                    Vector(0.2f, 0.2f, 1.0f, o->Light);
                    o->MultiUse = rand() % 10;
                    o->LifeTime = 30 + o->MultiUse;
                    o->MaxTails = 15;
                    o->Direction[0] = (float)(rand() % 3000);
                    o->Scale = 30.f;
                }
                else if (o->SubType == 8)
                {
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    VectorCopy(o->Position, o->StartPosition);

                    Vector(0.f, -50.f, 0.f, p);
                    AngleMatrix(o->TargetPosition, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->Position);
                }
                else if (o->SubType == 9)   //
                {
                    o->LifeTime = 0;
                    o->Scale = Scale;
                    o->MaxTails = 10;
                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(0.3f, 0.3f, 1.f, o->Light);
                    o->StartPosition[0] = (TargetPosition[0] - o->Position[0]) / o->MaxTails;
                    o->StartPosition[1] = (TargetPosition[1] - o->Position[1]) / o->MaxTails;
                    o->StartPosition[2] = (TargetPosition[2] - o->Position[2]) / o->MaxTails;

                    AngleMatrix(o->Angle, Matrix);

                    for (int i = 0; i < o->MaxTails; i++)
                    {
                        VectorAdd(o->Position, o->StartPosition, o->Position);
                        CreateTail(o, Matrix);
                    }
                }
                else if (o->SubType == 11)
                {
                    Vector(0.2f, 0.2f, 1.0f, o->Light);
                    o->MultiUse = 0;
                    o->LifeTime = 30;
                    o->MaxTails = 15;
                    o->Direction[0] = (float)(rand() % 3000);
                    o->Scale = 30.f;
                }
                else if (o->SubType == 25)
                {
                    Vector(0.9f, 0.4f, 0.6f, o->Light);
                    o->MultiUse = 0;
                    o->LifeTime = 30;
                    o->MaxTails = 15;
                    o->Direction[0] = (float)(rand() % 3000);
                    o->Scale = 30.f;
                }
                else if (o->SubType == 13)
                {
                    o->Direction[2] = (float)(rand() % 20 + 35);
                    o->Scale = Scale;
                    o->LifeTime = 25 + rand() % 50;

                    Vector(0.5f, 0.5f, 0.5f, o->Light);
                }
                else if (o->SubType == 23)
                {
                    o->LifeTime = 20 + (4 - PKKey);
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    o->NumTails = -1;
                    o->Velocity = 0.f;
                    o->Direction[0] = 1.f;
                    o->Direction[1] = 5.f;
                    o->Direction[2] = 1.f;
                    o->m_bCreateTails = false;

                    VectorCopy(o->Angle, o->HeadAngle);

                    o->MultiUse = PKKey;
                    switch ((int)o->MultiUse)
                    {
                    case 0: o->HeadAngle[2] += 90.f; o->Position[2] += 200.f; break;
                    case 1: o->HeadAngle[2] += 90.f; o->Position[2] += 10.f; break;
                    case 2: o->HeadAngle[2] -= 90.f; o->Position[2] += 200.f; break;
                    case 3: o->HeadAngle[2] -= 90.f; o->Position[2] += 10.f; break;
                    case 4: o->HeadAngle[2] += 180.f; o->Position[2] += 100.f; break;
                    case 5:
                        o->HeadAngle[0] = 90.f;
                        o->Position[2] += 100.f;
                        o->LifeTime = 10;
                        o->MaxTails = 20;
                        o->RenderFace = RENDER_FACE_TWO;
                        break;
                    }
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 43)
                {
                    o->LifeTime = 100;
                    o->MaxTails = 0;
                    o->m_bCreateTails = false;
                }
                else if (o->SubType == 44)
                {
                    o->Scale = Scale;
                    o->LifeTime = 15;
                    o->MaxTails = 30;
                    o->m_bCreateTails = true;
                    o->Direction[2] = (float)(rand() % 2 + 2);
                }
                else if (o->SubType == 45 || o->SubType == 46)
                {
                    Vector(0.2f, 0.2f, 1.0f, o->Light);
                    o->MultiUse = rand() % 10;
                    o->LifeTime = 30 + o->MultiUse;
                    o->MaxTails = 15;
                    o->Direction[0] = (float)(rand() % 3000);
                    o->Scale = 30.f;
                }
                else
                    if (o->SubType == 47)
                    {
                        Vector(0.7f, 0.7f, 1.0f, o->Light);
                        o->MultiUse = rand() % 10;
                        o->LifeTime = 30 + o->MultiUse;
                        o->MaxTails = 15;
                        o->Direction[0] = (float)(rand() % 3000);
                        o->Scale = 30.f;
                    }
                    else
                    {
                        o->m_bCreateTails = false;
                    }
                VectorCopy(TargetPosition, o->TargetPosition);
                break;

            case BITMAP_FLARE + 1:
                o->LifeTime = PKKey;
                o->PKKey = 0;
                o->MaxTails = 50;
                switch (o->Skill)
                {
                case 0:
                    o->Scale = 20.f;
                    Vector(0.5f, 0.5f, 0.5f, o->Light);
                    break;

                case 1:
                    o->Scale = 40.f;
                    Vector(1.f, 0.5f, 0.f, o->Light);
                    break;
                case 3:
                    o->Scale = 20.f;
                    VectorCopy(Target->Light, o->Light);
                    break;
                }
                switch (o->SubType)
                {
                case 0:
                    o->Velocity = 0.f;
                    Vector(Scale * 1.5f, 0.f, 0.f, o->Direction);
                    break;

                case 1:
                    o->Velocity = 90.f;
                    Vector(Scale * 1.5f, 0.f, 0.f, o->Direction);
                    break;

                case 2:
                    o->Velocity = 180.f;
                    Vector(Scale * 1.5f, 0.f, 0.f, o->Direction);
                    break;

                case 3:
                    o->Velocity = 240.f;
                    Vector(Scale * 1.5f, 0.f, 0.f, o->Direction);
                    break;

                case 4:
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_MINUS;
                    o->LifeTime = 0;
                    o->Scale = Scale;
                    o->MaxTails = 10;

                    Vector(0.f, 0.f, 0.f, o->Angle);
                    Vector(1.f, 1.f, 1.f, o->Light);
                    o->StartPosition[0] = (TargetPosition[0] - o->Position[0]) / o->MaxTails;
                    o->StartPosition[1] = (TargetPosition[1] - o->Position[1]) / o->MaxTails;
                    o->StartPosition[2] = (TargetPosition[2] - o->Position[2]) / o->MaxTails;

                    AngleMatrix(o->Angle, Matrix);

                    for (int i = 0; i < o->MaxTails; i++)
                    {
                        VectorAdd(o->Position, o->StartPosition, o->Position);
                        CreateTail(o, Matrix);
                    }
                }
                break;

                case 5:
                    Vector(1.f, 1.f, 1.f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->LifeTime = 50;
                    o->MaxTails = 8;
                    o->Velocity = 3.f;
                    o->byOnlyOneRender = 2;
                    break;

                case 6:
                    Vector(1.f, 1.f, 1.f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 5.f;
                    break;

                case 7:
                    Vector(1.f, 1.f, 1.f, o->Light);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 10.f;
                    o->TexType = BITMAP_FLARE;
                    o->Direction[0] = 15.f;
                    o->Position[0] = TargetPosition[0] + (float)cos((float)(rand() % 360)) * 40;
                    o->Position[1] = TargetPosition[1] - (float)sin((float)(rand() % 360)) * 40;
                    VectorCopy(o->Position, o->StartPosition);
                    break;
                case 8:
                case 9:
                    o->RenderFace = RENDER_FACE_TWO;
                case 10:
                case 11:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->MaxTails = 20;
                    o->LifeTime = 100;
                    o->Velocity = 0.f;
                    o->byOnlyOneRender = 1;
                    o->MultiUse = SkillIndex;

                    if (o->SubType == 10 || o->SubType == 11)
                    {
                        Vector(0.7f, 0.7f, 0.7f, o->Light);
                        o->MaxTails = 10;
                    }
                    else
                    {
                        Vector(1.f, 1.f, 1.f, o->Light);
                    }
                    Vector(0.f, 0.f, 0.f, o->Direction);

                    if (o->Target == NULL)
                    {
                        o->Live = false;
                    }
                    VectorCopy(o->Target->Position, o->Position);
                    VectorCopy(o->Target->Angle, o->Angle);
                    if (o->SubType == 8)
                    {
                        o->Position[2] = 300.f;
                    }
                    else if (o->SubType == 9)
                    {
                        o->Position[2] += o->MultiUse;
                    }
                    break;

                case 12:
                    Vector(0.6f, 0.2f, 0.8f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 70.f;
                    break;
                case 13:
                    Vector(0.7f, 0.7f, 0.3f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 70.f;
                    break;
                case 14:
                    Vector(1.0f, 1.0f, 1.0f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 70.f;
                    break;
                case 15:
                    Vector(0.4f, 0.9f, 0.5f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->MaxTails = 16;
                    o->Velocity = 70.f;
                    break;
                case 16:
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND_OTHER;
                    o->RenderFace = RENDER_FACE_TWO;
                    o->LifeTime = 35;
                    o->MaxTails = 8;
                    o->Velocity = 7.f;
                    o->byOnlyOneRender = 2;
                    Vector(1.f, 0.5f, 0.3f, o->Light);
                    //					VectorCopy(Target->Light, o->Light);
                    break;
                case 17:
                    Vector(1.0f, 1.0f, 1.0f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;//+50.f;
                    o->MaxTails = 16;
                    o->Velocity = 70.f;
                    break;
                case 18:
                {
                    if (vPriorColor)
                    {
                        VectorCopy(vPriorColor, o->Light);
                    }
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->MaxTails = 7;
                    o->Velocity = 70.f;
                }
                break;
                case 19:
                {
                    Vector(0.5f, 0.5f, 1.0f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->MaxTails = 14;
                    o->Velocity = 30.f;
                    o->LifeTime = 50;
                }
                break;
                case 20:
                {
                    Vector(0.5f, 0.5f, 1.0f, o->Light);
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                    o->Scale = Scale;
                    o->MaxTails = 14;
                    o->Velocity = 30.f;
                    o->LifeTime = 50;
                }
                break;
                }
                VectorCopy(TargetPosition, o->TargetPosition);
                break;
            case BITMAP_JOINT_FORCE:
                if (o->SubType == 0 || o->SubType == 10)
                {
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;
                    o->Scale = Scale;
                    o->Velocity = 0.f;
                    o->MaxTails = 18;
                    o->NumTails = -1;

                    Vector(0.f, -180.f, 0.f, p);
                    VectorCopy(o->Position, o->TargetPosition);
                    o->Angle[2] += 30.f;
                    VectorCopy(o->Angle, o->Direction);

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->Position, Position, o->Position);
                    //					Vector(0.f,1.f,0.f,o->Light);
                }
                else if (o->SubType == 1)
                {
                    o->Scale = Scale;
                    o->MaxTails = 15;
                    o->LifeTime = 30;
                    o->Velocity = 0.f;
                    o->byOnlyOneRender = 1;
                    o->Weapon = 0;

                    Vector(1.f, 1.f, 1.f, o->Light);
                    Vector(0.f, 0.f, 0.f, o->Direction);

                    if (o->Target == NULL)
                    {
                        o->Live = false;
                    }
                    VectorCopy(o->Target->Position, o->Position);
                    VectorCopy(o->Target->Angle, o->Angle);
                }
                else if (o->SubType >= 2 && o->SubType <= 6)
                {
                    o->LifeTime = 20;
                    o->Scale = Scale;
                    o->Velocity = 8.f;
                    o->Direction[0] = 3.5f;
                    o->Direction[2] = 1.f;
                    o->MaxTails = 12;
                    o->NumTails = -1;
                    o->MultiUse = 5;
                    if (o->SubType == 3)
                    {
                        o->TexType = BITMAP_FLARE;
                        o->MultiUse = 10;
                        o->Velocity = 10.f;
                        o->Direction[2] = 2.f;
                        o->m_byReverseUV = 0;
                        o->Direction[0] = 10.5f;
                    }
                    else if (o->SubType == 4)
                    {
                        o->LifeTime = 20;
                        o->MaxTails = 13;
                        o->TexType = BITMAP_HOLE;
                        o->Direction[0] = 15.5f;
                        o->Direction[2] = 3.f;
                        o->MultiUse = 10;
                    }
                    else if (o->SubType == 5)
                    {
                        o->TexType = BITMAP_LAVA;
                    }
                    else if (o->SubType == 6)
                    {
                        o->TexType = BITMAP_LAVA;
                        o->Velocity = 20.f;
                        o->LifeTime = 16;
                    }
                    else
                    {
                        o->Position[2] += 10.f;
                        o->TexType = BITMAP_INFERNO;
                        o->LifeTime = 15;
                        o->MultiUse = 5;
                        o->Direction[0] = 5.f;
                        o->Direction[2] = 5.f;
                    }
                    Vector(0.f, 0.f, o->Angle[2], o->HeadAngle);
                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 7)
                {
                    o->Scale = Scale;
                    o->Velocity = 8.f;
                    o->Direction[0] = 3.5f;
                    o->Direction[2] = 1.f;
                    o->MaxTails = 13;
                    o->NumTails = -1;
                    o->MultiUse = 5;

                    o->TexType = BITMAP_INFERNO;
                    o->Velocity = 10.f;
                    o->LifeTime = 20;
                }
                else if (o->SubType == 8)	// SubType : 7
                {
                    o->LifeTime = 20;
                    o->m_bCreateTails = false;
                    o->Scale = Scale;
                    o->Velocity = 0.f;
                    o->MaxTails = 18;
                    o->NumTails = -1;

                    Vector(0.f, -180.f, 0.f, p);
                    VectorCopy(o->Position, o->TargetPosition);
                    o->Angle[2] += 30.f;
                    VectorCopy(o->Angle, o->Direction);

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                    //					Vector(1.f,1.f,1.f,o->Light);
                }
                else if (o->SubType == 20)	// SubType : 7
                {
                    o->Scale = Scale;
                    o->Velocity = 8.f;
                    o->Direction[0] = 3.5f;
                    o->Direction[2] = 1.f;
                    o->MaxTails = 15;
                    o->NumTails = -1;
                    o->MultiUse = 5;

                    o->TexType = BITMAP_INFERNO;
                    o->Velocity = 10.f;
                    o->LifeTime = 20;
                }
                break;
            case BITMAP_LIGHT:
                o->m_bCreateTails = false;
                if (o->SubType == 0)
                {
                    o->LifeTime = 20;
                    o->MaxTails = 30;
                    o->Scale = Scale;

                    o->Velocity = rand() % 10 + 10.f;
                    Vector(-(float)(rand() % 40) + 30.f, 0.f, (float)(rand() % 360), o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    Vector(0.1f, 0.5f, 1.f, o->Light);

                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 1)
                {
                    o->LifeTime = 10;
                    o->MaxTails = 20;
                    o->Scale = Scale;
                    o->Skill = rand() % 5;

                    o->Velocity = rand() % 5 + 1.f;
                    Vector(30.f, 0.f, (float)(rand() % 360), o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    Vector(1.f, 0.8f, 0.6f, o->Light);

                    VectorCopy(o->Position, o->StartPosition);
                }
                break;

            case BITMAP_PIERCING:
                o->m_bCreateTails = false;
                if (o->SubType == 0)
                {
                    o->LifeTime = 10;
                    o->MaxTails = 30;
                    o->Scale = Scale;

                    o->Velocity = 20.f;
                    Vector(-90.f, 0.f, Angle[2], o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    if (rand_fps_check(2))
                    {
                        Vector(1.f, 0.8f, 0.6f, o->Light);
                    }
                    else
                    {
                        Vector(1.f, 1.f, 1.f, o->Light);
                    }

                    VectorCopy(o->Position, o->StartPosition);
                }
                else if (o->SubType == 1)
                {
                    o->LifeTime = 10;
                    o->bTileMapping = TRUE;
                    o->MaxTails = 30;
                    o->Scale = Scale;

                    o->Velocity = 20.f;
                    Vector(-90.f, 0.f, Angle[2], o->Angle);
                    Vector(0.f, 0.f, 0.f, o->Direction);
                    if (rand_fps_check(2))
                    {
                        Vector(1.f, 0.8f, 0.6f, o->Light);
                    }
                    else
                    {
                        Vector(1.f, 1.f, 1.f, o->Light);
                    }

                    VectorCopy(o->Position, o->StartPosition);
                }
                break;
            case BITMAP_FLARE_FORCE:
                o->LifeTime = 20;
                o->m_bCreateTails = false;
                o->MaxTails = 30;
                o->Scale = Scale;
                o->MultiUse = 1;
                o->Velocity = -3.f;
                o->Position[2] += 150.f * FPS_ANIMATION_FACTOR;
                Vector(0.f, 0.f, 0.f, o->Direction);
                Vector(0.f, 0.f, 0.f, o->TargetPosition);
                Vector(1.f, 0.8f, 1.f, o->Light);
                VectorCopy(o->Position, o->StartPosition);
                o->Weapon = 0;

                if (o->SubType >= 1 && o->SubType <= 4
                    || (o->SubType >= 11 && o->SubType <= 13)
                    )
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                    o->Velocity = -3.f;
                    o->TargetPosition[0] = 80.f;
                    o->TargetPosition[1] = 180.f;
                    o->Weapon = rand() % 3 + 2;
                    if (o->SubType == 2 || o->SubType == 4
                        || (o->SubType >= 11 && o->SubType <= 13)
                        )
                    {
                        switch (o->SubType)
                        {
                        case 11:
                            Vector(0.7f, 1.0f, 0.7f, o->Light);
                            break;
                        case 12:
                            Vector(1.0f, 0.6f, 0.6f, o->Light);
                            break;
                        case 13:
                            Vector(0.7f, 0.7f, 1.0f, o->Light);
                            break;
                        }

                        o->TargetPosition[1] = -180.f;
                    }

                    if (o->SubType == 3 || o->SubType == 4)
                    {
                        o->Weapon = 0;
                    }

                    o->LifeTime += o->Weapon;

                    AngleMatrix(o->TargetPosition, Matrix);
                    Vector(0.f, 0.f, o->TargetPosition[0], p);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->Position);
                }
                else if (o->SubType != 0)
                {
                    o->LifeTime = 15;
                    o->Skill = 0;
                    o->MaxTails = 1;
                    o->Weapon = 30;
                    o->MultiUse = SkillIndex;
                    o->Velocity = 0.f;
                    o->TargetPosition[0] = 30.f;
                    o->TargetPosition[2] = (float)(o->SubType * 90);
                    Vector(1.f, 0.8f, 1.f, o->Light);
                    Vector(0.f, -4.f, 0.f, o->Direction);
                    VectorCopy(o->Position, o->StartPosition);

                    if (o->PKKey != -1)
                        o->Direction[1] = 0.f;
                }

                if (o->SubType >= 5 && o->SubType <= 7)
                {
                    o->TexType = BITMAP_FIRECRACKER;
                }
                else if (o->SubType == 0)
                {
                    o->TexType = BITMAP_JOINT_THUNDER;
                }
                break;
            case BITMAP_FLASH:
                if (o->SubType <= 3 || o->SubType == 5)
                {
                    o->MaxTails = 10;
                    o->LifeTime = 40;
                    o->Velocity = 70.f;
                    o->Scale = Scale;
                    o->m_byReverseUV = rand() % 2;
                    o->MultiUse = 150;
                    o->PKKey = 0;
                    if (o->SubType == 2)
                    {
                        o->Velocity = 30.f;
                        o->MultiUse = 300;
                        o->TexType = BITMAP_FLARE_FORCE;
                    }
                    else if (o->SubType == 3)
                    {
                        o->Velocity = 30.f;
                        o->MultiUse = 300;
                        o->TexType = BITMAP_FLARE_BLUE;
                    }

                    if (o->SubType == 5)
                    {
                        o->TexType = BITMAP_FLARE;
                        o->MaxTails = 15;
                        o->Velocity = rand() % 20 + 10.f;
                        o->m_byReverseUV = 2;
                    }
                    else
                    {
                        Vector(90.f, 0.f, 0.f, o->Angle);
                    }
                    VectorCopy(o->Angle, o->HeadAngle);
                }
                else if (o->SubType == 4)
                {
                    o->Scale = Scale;
                    o->MaxTails = 20;
                    o->LifeTime = 30;
                    o->Velocity = 0.f;
                    o->byOnlyOneRender = 1;
                    o->Weapon = 0;
                    o->TexType = BITMAP_FLARE_BLUE;

                    Vector(1.f, 1.f, 1.f, o->Light);
                    Vector(0.f, 0.f, 0.f, o->Direction);

                    if (o->Target == NULL)
                    {
                        o->Live = false;
                    }
                    VectorCopy(o->Target->Position, o->Position);
                    VectorCopy(o->Target->Angle, o->Angle);
                }
                else if (o->SubType == 6)
                {
                    o->Scale = Scale;
                    o->MaxTails = 30;
                    o->LifeTime = 25;
                    o->Weapon = o->LifeTime;
                    o->Velocity = (float)(70 + rand() % 3) * Q_PI / 180;

                    Vector(0.8f, 0.8f, 1.0f, o->Light);
                    Vector(0.f, -(40.f + (float)(rand() % 4)), sinf(o->Velocity) * 10.f, o->Direction);
                    o->Position[1] -= 20.f;
                    o->Position[2] += 130.f;
                    VectorCopy(o->Position, o->StartPosition);
                    VectorCopy(Angle, o->Angle);
                    AngleMatrix(o->Angle, Matrix);
                    o->m_bCreateTails = false;
                    o->m_byReverseUV = 3;
                    CreateTailAxis(o, Matrix, 1);
                    o->NumTails = 0;
                }
                else if (o->SubType == 7)
                {
                    //6 Opener
                    o->m_bCreateTails = false;
                    o->LifeTime = 0;
                    for (int i = 0; i < 3; ++i)
                    {
                        vec3_t Pos;
                        AngleMatrix(o->Angle, Matrix);
                        Vector((i - 1) * 100, -50.f, 0.f, Pos);
                        VectorRotate(Pos, Matrix, Position);
                        VectorAdd(Position, o->Position, Pos);
                        CreateJoint(BITMAP_FLASH, Pos, Pos, o->Angle, 6, NULL, 40.f);
                    }
                }
                break;
            case BITMAP_DRAIN_LIFE_GHOST:
            {
                switch (o->SubType)
                {
                case 0:
                {
                    o->RenderType = RENDER_TYPE_ALPHA_BLEND;

                    // o->Target
                    if (rand_fps_check(2))
                    {
                        o->Angle[2] += 90.f;
                    }
                    else
                    {
                        o->Angle[2] -= 90.f;
                    }

                    VectorCopy(TargetPosition, o->TargetPosition);
                    o->Angle[0] += (float)((rand() % 100) - 50);				// -140 ~ 140
                    o->Angle[1] += (float)((rand() % 100) - 50);				// -140 ~ 140
                    o->Angle[2] += (float)((rand() % 100) - 50);				// -140 ~ 140
                    o->Velocity = (float)(1 + ((float)(rand() % 10) * 0.2f));		// 20 ~ 40
                    o->LifeTime = (float)(30 + (rand() % 20 - 10));				// 20 ~ 30
                    o->Scale = Scale + (float)((rand() % 60 - 30));			// Scale-30 ~ Scale+30
                    o->MaxTails = (float)(20 + (rand() % 10 - 5));				// 5 ~ 15
                }
                break;
                }
            }
            break;
            case BITMAP_PIN_LIGHT:
            {
                o->Scale = Scale;
                o->Velocity = (float)(rand() % 60 + 15);
                o->LifeTime = rand() % 3 + 1;
                o->MaxTails = 5;
            }
            break;
            case BITMAP_FORCEPILLAR:
            {
                o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                o->RenderFace = RENDER_FACE_TWO;
                o->Scale = Scale;
                o->LifeTime = 7;
                o->MaxTails = 5;
                o->m_bCreateTails = true;
                Vector(0.95f, 0.72f, 0.48f, o->Light);
            }
            break;
            case BITMAP_SWORDEFF:
            {
                o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                o->RenderFace = RENDER_FACE_TWO;
                o->Scale = Scale;
                o->LifeTime = 10;
                o->MaxTails = 50;
                o->m_bCreateTails = true;
                Vector(0.6f, 0.6f, 1.0f, o->Light);
            }
            break;
            case BITMAP_GROUND_WIND:
            {
                o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                o->RenderFace = RENDER_FACE_TWO;
                o->Scale = Scale;
                o->LifeTime = 10;
                o->MaxTails = 8;
                o->m_bCreateTails = true;
                Vector(0.7f, 0.7f, 1.0f, o->Light);
            }
            break;
            case BITMAP_LAVA:
            {
                o->RenderType = RENDER_TYPE_ALPHA_BLEND;
                o->RenderFace = RENDER_FACE_TWO;
                o->Scale = Scale;
                o->LifeTime = 10;
                o->MaxTails = 10;
                o->m_bCreateTails = true;
                Vector(1.0f, 1.0f, 1.0f, o->Light);
                o->Velocity = Models[o->Target->Type].Actions[o->Target->CurrentAction].PlaySpeed;
            }
            break;
            }

            // Because the Tails will be too short when FPS is high, we need to
            // increase the MaxTails.
            o->MaxTails = static_cast<int>(o->MaxTails / FPS_ANIMATION_FACTOR);
            if (o->MaxTails > MAX_TAILS)
            {
                o->MaxTails = MAX_TAILS;
            }

            return;
        }
    }
}

void DeleteJoint(int Type, OBJECT* Target, int SubType)
{
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        JOINT* o = &Joints[i];

        if (Target == NULL)
        {
            if (o->Live && o->Type == Type)
            {
                if (SubType == -1 || o->SubType == SubType)
                {
                    o->Live = false;
                }
            }
        }
        else if (o->Live && o->Type == Type && o->Target == Target)
        {
            if (SubType == -1 || o->SubType == SubType)
                o->Live = false;
        }
    }
}

bool SearchJoint(int Type, OBJECT* Target, int SubType)
{
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        JOINT* o = &Joints[i];
        if (o->Live && o->Type == Type && o->Target == Target)
        {
            if (SubType == -1 || o->SubType == SubType)
                return true;
        }
    }
    return false;
}

void CreateTailAxis(JOINT* o, float Matrix[3][4], BYTE axis)
{
    o->NumTails++;

    if (o->NumTails > o->MaxTails - 1)
    {
        o->NumTails = o->MaxTails - 1;
    }

    for (int j = (int)(o->NumTails - 1); j >= 0; j--)
    {
        for (int k = 0; k < 4; k++)
            VectorCopy(o->Tails[j][k], o->Tails[j + 1][k]);
    }

    vec3_t Position, p;
    if (axis == 0)
    {
        Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][0]);
        Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][1]);
        Vector(0.f, 0.f, -o->Scale * 0.5f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][2]);
        Vector(0.f, 0.f, o->Scale * 0.5f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][3]);
    }
    else
    {
        Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][0]);
        Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][1]);
        Vector(0.f, -o->Scale * 0.5f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][2]);
        Vector(0.f, o->Scale * 0.5f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][3]);
    }
}

void CreateTail(JOINT* o, float Matrix[3][4], bool Blur)
{
    //return; // TODO
    if (Blur == true)
    {
        int i = 0;
        for (i = 0; i < 2; i++)
        {
            o->NumTails++;
            if (o->NumTails > o->MaxTails - 1)
            {
                o->NumTails = o->MaxTails - 1;
            }

            for (int j = o->NumTails - 1; j >= 0; j--)
            {
                for (int k = 0; k < 4; k++)
                    VectorCopy(o->Tails[j][k], o->Tails[j + 1][k]);
            }

            if (i == 0 && (int)o->NumTails > 1)
            {
                vec3_t Position, p;
                Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][0]);
                o->Tails[0][0][0] = (o->Tails[0][0][0] + o->Tails[1][0][0]) / 2.f;
                o->Tails[0][0][1] = (o->Tails[0][0][1] + o->Tails[1][0][1]) / 2.f;
                o->Tails[0][0][2] = (o->Tails[0][0][2] + o->Tails[1][0][2]) / 2.f;
                Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][1]);
                o->Tails[0][1][0] = (o->Tails[0][1][0] + o->Tails[1][1][0]) / 2.f;
                o->Tails[0][1][1] = (o->Tails[0][1][1] + o->Tails[1][1][1]) / 2.f;
                o->Tails[0][1][2] = (o->Tails[0][1][2] + o->Tails[1][1][2]) / 2.f;
                Vector(0.f, 0.f, -o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][2]);
                o->Tails[0][2][0] = (o->Tails[0][2][0] + o->Tails[1][2][0]) / 2.f;
                o->Tails[0][2][1] = (o->Tails[0][2][1] + o->Tails[1][2][1]) / 2.f;
                o->Tails[0][2][2] = (o->Tails[0][2][2] + o->Tails[1][2][2]) / 2.f;
                Vector(0.f, 0.f, o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][3]);
                o->Tails[0][3][0] = (o->Tails[0][3][0] + o->Tails[1][3][0]) / 2.f;
                o->Tails[0][3][1] = (o->Tails[0][3][1] + o->Tails[1][3][1]) / 2.f;
                o->Tails[0][3][2] = (o->Tails[0][3][2] + o->Tails[1][3][2]) / 2.f;
            }
            else
            {
                vec3_t Position, p;

                Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][0]);

                Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][1]);

                Vector(0.f, 0.f, -o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][2]);

                Vector(0.f, 0.f, o->Scale * 0.5f, Position);
                VectorRotate(Position, Matrix, p);
                VectorAdd(o->Position, p, o->Tails[0][3]);
            }
        }
    }
    else
    {
        o->NumTails++;
        if (o->NumTails > o->MaxTails - 1)
        {
            o->NumTails = o->MaxTails - 1;
        }

        // TODO: Instead of copying, leave it and add new tail values at the end
        // We need something like a circular buffer here ...
        for (int j = o->NumTails - 1; j >= 0; j--)
        {
            for (int k = 0; k < 4; k++)
                VectorCopy(o->Tails[j][k], o->Tails[j + 1][k]);
        }

        vec3_t Position, p;

        Vector(-o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][0]);

        Vector(o->Scale * 0.5f, 0.f, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][1]);

        Vector(0.f, 0.f, -o->Scale * 0.5f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][2]);

        Vector(0.f, 0.f, o->Scale * 0.5f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAdd(o->Position, p, o->Tails[0][3]);
    }
}


/*void MoveShpere(vec3_t Position,vec3_t Angle)
{
OBJECT *to = &Hero->Object;
vec3_t Range;
VectorSubtract(Position,to->Position,Range);
float Distance = Range[0]*Range[0]+Range[1]*Range[1];
float CollisionRange = 100.f*100.f;
if(Distance <= CollisionRange)
{
float Rotation = 360.f-CreateAngle2D(Position, to->Position);
Angle[2] = TurnAngle2(Angle[2],Rotation,FarAngle(Angle[2],Rotation)*0.2f);
}
else Angle[2] = TurnAngle2(Angle[2],0.f,FarAngle(Angle[2],0.f)*0.5f);
}*/

//LONG FAR PASCAL WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void MoveJoint(JOINT* o, int iIndex)
{
    float Height;
    vec3_t Light;
    float Luminosity;
    vec3_t Position, p;

    float Distance;
    float dx = o->Position[0] - o->TargetPosition[0];
    float dy = o->Position[1] - o->TargetPosition[1];

    float Matrix[3][4];

    if (!(o->Type == BITMAP_JOINT_FORCE && o->SubType == 0) && !(o->Type == BITMAP_JOINT_THUNDER && o->SubType == 15) && o->Type != BITMAP_LIGHT
        && o->Type != BITMAP_PIERCING && o->Type != BITMAP_FLARE_FORCE
        && !((o->Type == BITMAP_FLARE || o->Type == BITMAP_FLARE_BLUE) && o->SubType == 1)
        && !(o->Type == MODEL_SPEARSKILL && (o->SubType == 5 || o->SubType == 6 || o->SubType == 7 || o->SubType == 8))
        && !((o->Type == BITMAP_FLARE) && (o->SubType == 42))
        && !((o->Type == BITMAP_FLASH) && (o->SubType == 6))
        )
    {
        AngleMatrix(o->Angle, Matrix);
        if (o->Velocity != 0.0f)
        {
            Vector(0.f, -o->Velocity, 0.f, p);
            VectorRotate(p, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
        }
    }

    switch (o->Type)
    {
    case BITMAP_JOINT_LASER:
        VectorCopy(o->Target->Position, o->TargetPosition);
        o->TargetPosition[2] += 130.f;

        Distance = sqrtf(dx * dx + dy * dy);
        MoveHumming(o->Position, o->Angle, o->TargetPosition, 3000.f / Distance);
        //MoveHumming(o->Position,o->Angle,o->TargetPosition,10.f);
        //MoveHumming(o->Position,o->Angle,o->TargetPosition,0.f);
        if (!o->Collision && Distance <= o->Velocity * 2.f * FPS_ANIMATION_FACTOR)
        {
            o->Collision = true;
            o->LifeTime = 5;
        }
        //light
        Luminosity = o->LifeTime * 0.1f;
        Vector(Luminosity, Luminosity, Luminosity, o->Light);
        Luminosity = -(float)(rand() % 4 + 4) * 0.01f;
        Vector(Luminosity, Luminosity, Luminosity, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        break;
    case BITMAP_SCOLPION_TAIL:
        VectorCopy(o->Target->EyeLeft, o->Position);
        if (!o->Target->Live)
        {
            o->Live = false;
            return;
        }
        break;
    case BITMAP_JOINT_ENERGY:
        switch (o->SubType)
        {
        case 2:
        case 3:
        case 4:
        case 5:
        case 8:
        case 10:
        case 11:
        case 14:
        case 15:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 22:
        case 23:
        case 24:
        case 25:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
            if (!o->Target->Live)
            {
                o->Live = false;
                return;
            }
            switch (o->SubType)
            {
                //. Left
            case 2:
            case 4:
            case 5:
            case 8:
            case 10:
            case 14:
            case 17:
            case 18:
            case 28:
            case 22:
            case 24:
                VectorCopy(o->Target->EyeLeft, o->Position);
                if (o->SubType == 8)
                {
                    o->Scale += (10.1f) * FPS_ANIMATION_FACTOR;
                }
                break;
            case 20:
            case 30:
                VectorCopy(o->Target->EyeLeft2, o->Position);
                break;
            case 26:
            case 32:
                VectorCopy(o->Target->EyeLeft3, o->Position);
                break;
                //. Right
            case 3:
            case 11:
            case 15:
            case 19:
            case 29:
            case 23:
            case 25:
            case 47:
                VectorCopy(o->Target->EyeRight, o->Position);
                break;
            case 21:
            case 31:
                VectorCopy(o->Target->EyeRight2, o->Position);
                break;
            case 54:
                switch (o->PKKey)
                {
                case 0:
                    VectorCopy(o->Target->EyeRight2, o->Position);
                    break;
                case 1:
                    VectorCopy(o->Target->EyeLeft2, o->Position);
                    break;
                case 2:
                    VectorCopy(o->Target->EyeRight3, o->Position);
                    break;
                case 3:
                    VectorCopy(o->Target->EyeLeft3, o->Position);
                    break;
                }
                break;
            case 27:
            case 33:
                VectorCopy(o->Target->EyeRight3, o->Position);
                break;
            case 55:
                VectorCopy(o->Target->EyeLeft, o->Position);
                break;
            case 56:
                VectorCopy(o->Target->EyeRight, o->Position);
                break;
            case 57:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, o->PKKey);
                break;
            case 48:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 24);
                break;
            case 49:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 28);
                break;
            case 50:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 32);
                break;
            case 51:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 44);
                break;
            case 52:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 48);
                break;
            case 53:
                Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame,
                    o->Target->PriorAnimationFrame, o->Target->PriorAction, o->Target->Angle,
                    o->Target->HeadAngle, false, false);
                Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 52);
                break;
            }
            switch (o->SubType)
            {
            case 2:
            case 3:
                Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.8f;
                Vector(Luminosity * 0.5f, Luminosity * 0.1f, Luminosity, Light);
                VectorMul(Light, o->Light, Light);
                CreateParticle(BITMAP_LIGHTNING + 1, o->Position, o->Angle, Light);
                break;
            case 14:
            case 15:
                Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.8f;
                Vector(Luminosity, Luminosity * 0.1f, Luminosity * 0.1f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, Light, 4);
                break;
            case 22:
            case 23:
                Luminosity = sinf(WorldTime * 0.002f) * 0.2f + 0.8f;
                Vector(Luminosity, Luminosity, Luminosity, Light);
                VectorMul(Light, o->Light, Light);
                CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, Light, 4);
                break;
            case 24:
            case 25:
                Luminosity = sinf(WorldTime * 0.002f) * 0.1f + 0.2f;
                Vector(Luminosity, Luminosity, Luminosity, Light);
                VectorMul(Light, o->Light, Light);
                CreateSprite(BITMAP_FLARE + 1, o->Position, Luminosity, Light, o->Target);
                break;
            case 54:
                Luminosity = sinf(WorldTime * 0.002f) * 0.1f;
                Vector(Luminosity, Luminosity, Luminosity, Light);
                VectorMul(Light, o->Light, Light);
                CreateSprite(BITMAP_BLUR, o->Position, Luminosity, Light, o->Target);
                break;
            }
            break;
        case 0:
        case 1:
        case 6:
        case 9:
        case 12:
        case 13:
        case 16:
        case 44:
        case 45:
        case 46:
        {
            if (o->LifeTime >= 100 && o->SubType != 44)
            {
                if (o->SubType == 0)
                {
                    o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (6.f) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 45)
                {
                    o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (6.f) * FPS_ANIMATION_FACTOR;
                    //VectorCopy(o->Target->Position, o->TargetPosition);
                    //o->TargetPosition[2] += (120.f);
                    //Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
                else if (o->SubType == 12)
                {
                    o->Angle[2] += (15.f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] += (o->MultiUse * (rand() % 5)) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (o->MultiUse * (rand() % 5)) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (6.f) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 16
                    || o->SubType == 46
                    )
                {
                    o->Angle[2] -= (o->MultiUse * 20.f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] += (o->MultiUse * (rand() % 5 + 3)) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (o->MultiUse * (rand() % 5 + 3)) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (8.f) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 13)
                {
                    o->Angle[2] -= (20.f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] += (o->MultiUse * (rand() % 5)) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (o->MultiUse * (rand() % 5)) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (6.f) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 44)
                {
                    o->Angle[2] -= (20.f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] += (o->MultiUse * (rand() % 5) + cosf(Q_PI / (float)(rand() % 180) * 20.0f)) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (o->MultiUse * (rand() % 5) + sinf(Q_PI / (float)(rand() % 180) * 20.0f)) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (6.f) * FPS_ANIMATION_FACTOR;
                }
                else
                {
                    o->Angle[2] -= (10.f) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (5.f) * FPS_ANIMATION_FACTOR;
                }
            }
            else if (o->SubType == 44 && o->LifeTime <= 120 && o->LifeTime >= 10)
            {
                if (o->LifeTime >= 20)
                {
                    o->Velocity += FPS_ANIMATION_FACTOR * 10.f;
                    if (o->Velocity >= 20.f) o->Velocity = 20.f;

                    VectorCopy(o->Target->Position, o->TargetPosition);
                    o->TargetPosition[2] += 120.f;
                    Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
                else
                {
                    o->Velocity += FPS_ANIMATION_FACTOR * 12.f;
                    if (o->Velocity >= 20.f) o->Velocity = 20.f;

                    //VectorCopy(o->Target->Position, o->TargetPosition);

                    //o->Angle[0] = o->TargetPosition[0]-o->Position[0];
                    //o->Angle[1] = o->TargetPosition[1]-o->Position[1];
                    //o->Angle[2] = o->TargetPosition[2]-o->Position[2];

                    o->Position[0] += (cosf(Q_PI / (float)(rand() % 180 + 150))) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (sinf(Q_PI / (float)(rand() % 180 + 150))) * FPS_ANIMATION_FACTOR;
                    //o->Position[2] += (cosf(Q_PI / (float)(rand()%180))) * FPS_ANIMATION_FACTOR;

                    Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
            }
            else
            {
                if (o->SubType == 12 || o->SubType == 13 || o->SubType == 16
                    || o->SubType == 46
                    )
                {
                    o->Velocity += FPS_ANIMATION_FACTOR * 7.f;
                    if (o->Velocity >= 40.f)
                        o->Velocity = 40.f;
                }
                else
                {
                    o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                    if (o->Velocity >= 30.f)
                        o->Velocity = 30.f;
                }

                float fOldAngle = o->Angle[2];
                if (o->SubType == 6 || o->SubType == 9)
                {
                    VectorCopy(o->TargetPosition, Position);
                    Distance = MoveHumming(o->Position, o->Angle, Position, o->Velocity);
                }
                else if (o->SubType == 7)
                {
                    o->Position[2] += (o->Velocity) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 44)
                {
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    o->TargetPosition[2] += 120.f;
                    Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
                else
                {
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    o->TargetPosition[2] += 120.f;
                    Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
                if (Distance <= 35.f)
                {
                    o->Live = false;
                    CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light, 1);
                    PlayBuffer(SOUND_GET_ENERGY);
                }
                else if (Distance <= 70.0f && fabs(fOldAngle - o->Angle[2]) > 20.0f)
                {
                    if (o->Velocity >= 20.f)
                        o->Velocity -= FPS_ANIMATION_FACTOR * 10.f;
                }
            }
            Luminosity = (float)(rand() % 4 + 8) * 0.03f;
            Vector(Luminosity * 0.4f, Luminosity, Luminosity * 0.8f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

            if (o->SubType == 6 || o->SubType == 9)
            {
                CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light, 3, 0.05f);
            }
            else if (o->SubType == 12 || o->SubType == 13)
            {
                CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, NULL);
                CreateSprite(BITMAP_SHINY + 1, o->Position, 1.f, o->Light, NULL, (float)(rand() % 360));
            }
            else if (o->SubType == 44)
            {
                CreateSprite(BITMAP_LIGHT, o->Position, 0.2f, o->Light, NULL);
                CreateSprite(BITMAP_SHINY + 1, o->Position, 0.1f, o->Light, NULL, (float)(rand() % 360));
            }
            else if (o->SubType == 16)
            {
                Vector(0.4f, 0.8f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 31, 0.8f);
            }
            else if (o->SubType == 46)
            {
                Vector(0.4f, 1.0f, 0.4f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 31, 0.8f);
            }
            else if (o->SubType == 45)
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateSprite(BITMAP_FLARE_RED, o->Position, 0.3f, Light, o->Target);
            }
            else
            {
                CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light);
            }
        }
        break;
        case 42:
        {
            if (o->LifeTime >= 60)
            {
                o->Position[2] += (1.f) * FPS_ANIMATION_FACTOR;
                o->Light[0] += (0.4f * 0.005f) * FPS_ANIMATION_FACTOR;
                o->Light[1] += (0.3f * 0.005f) * FPS_ANIMATION_FACTOR;
                o->Light[2] += (2.2f * 0.005f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                Vector(0.4f, 0.3f, 2.2f, o->Light);
                o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                if (o->Velocity >= 30.f)
                {
                    o->Velocity = 30.f;
                }
            }

            float fOldAngle = o->Angle[2];
            VectorCopy(o->Target->Position, o->TargetPosition);
            o->TargetPosition[2] += 260.f;
            Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);

            if (Distance <= 35.f)
            {
                o->Live = false;
                PlayBuffer(SOUND_GET_ENERGY);
            }
            else if (Distance <= 70.0f && fabs(fOldAngle - o->Angle[2]) > 20.0f)
            {
                if (o->Velocity >= 20.f)
                    o->Velocity -= FPS_ANIMATION_FACTOR * 10.f;
            }

            Luminosity = (float)(rand() % 4 + 8) * 0.03f;
            Vector(Luminosity * 0.4f, Luminosity, Luminosity * 0.8f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

            CreateParticleFpsChecked(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light, 5, 0.12f);
        }
        break;
        case 43:
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
            if (o->Velocity >= 40.f)
            {
                o->Velocity = 40.f;
            }

            VectorCopy(o->Target->Position, o->TargetPosition);
            o->TargetPosition[2] += 100.f;
            float TargetAngle;
            TargetAngle = CreateAngle2D(o->Position, o->TargetPosition);
            o->Angle[2] = TargetAngle;
            Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);

            if (Distance <= 60.f)
            {
                o->Live = false;
                vec3_t vPos;
                VectorCopy(o->Position, vPos);
                vPos[2] -= (60.f) * FPS_ANIMATION_FACTOR;
                CreateBomb(vPos, true, 4);
            }
            else if (Distance >= 550.0f)
            {
                o->Live = false;
            }

            Luminosity = (float)(rand() % 4 + 8) * 0.03f;
            Vector(Luminosity * 1.0f, Luminosity * 0.4, Luminosity * 0.3f, Light);
            //Vector(Luminosity*0.3f,Luminosity*0.4,Luminosity*1.0f,Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);

            float fLumi;
            vec3_t vPos, vLight;
            VectorCopy(o->Position, vPos);
            fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.1f;
            Vector(2.0f + fLumi, 1.0f + fLumi, 1.0f + fLumi, vLight);
            CreateSprite(BITMAP_POUNDING_BALL, vPos, 0.7f + fLumi, vLight, NULL, (WorldTime / 10.0f));
            fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 1.0f;
            Vector(2.0f + (rand() % 10) * 0.03f, 0.4f + (rand() % 10) * 0.03f, 0.4f + (rand() % 10) * 0.03f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, NULL, -(WorldTime * 0.1f));
            CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, NULL, (WorldTime * 0.12f));
            Vector(1.0f, 0.6f, 0.4f, vLight);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 31, 1.0f);
        }
        break;
        }
        break;

    case BITMAP_JOINT_HEALING:
        if (o->SubType == 4)
        {
            VectorCopy(o->Target->Position, o->Position);
            VectorAdd(o->TargetPosition, o->Position, o->Position);

            o->Position[2] += o->Velocity * FPS_ANIMATION_FACTOR;
            o->Velocity += FPS_ANIMATION_FACTOR * 10.f;
            Luminosity = (12 - o->LifeTime) * 0.1f;
            Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, o->Light);
        }
        else if (o->SubType == 5)
        {
            Luminosity = (12 - o->LifeTime) * 0.1f;
            Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, o->Light);
        }
        else if (o->SubType == 9)
        {
            // Ancient blur
            VectorSubtract(o->TargetPosition, o->Target->Position, Position);
            VectorCopy(o->Target->Position, o->TargetPosition);
            for (int j = o->NumTails - 1; j >= 0; j--)
            {
                for (int k = 0; k < 4; k++)
                    VectorSubtract(o->Tails[j][k], Position, o->Tails[j][k]);
            }

            o->Position[0] = o->TargetPosition[0] + sinf(o->MultiUse * 0.1f) * o->Direction[0];
            o->Position[1] = o->TargetPosition[1] + cosf(o->MultiUse * 0.1f) * o->Direction[0];
            o->Position[2] = o->TargetPosition[2] + 10 + (90 - o->LifeTime);

            o->MultiUse += 2 * FPS_ANIMATION_FACTOR;
            o->Direction[0] -= 1.f * FPS_ANIMATION_FACTOR;

            if (o->Direction[0] < 0)
            {
                o->Direction[0] = 0.f;
                o->LifeTime = 0;
            }

            if (o->LifeTime < 50)
            {
                VectorScale(o->Light, powf(1.f / 1.1f, FPS_ANIMATION_FACTOR), o->Light);
            }
            else if (o->LifeTime > 80)
            {
                VectorScale(o->Light, powf(1.25f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 10)
        {
            VectorSubtract(o->TargetPosition, o->Target->Position, Position);
            VectorCopy(o->Target->Position, o->TargetPosition);
            for (int j = o->NumTails - 1; j >= 0; j--)
            {
                for (int k = 0; k < 4; k++)
                    VectorSubtract(o->Tails[j][k], Position, o->Tails[j][k]);
            }

            o->Position[0] = o->TargetPosition[0] + sinf(o->MultiUse * 0.1f) * o->Direction[0];
            o->Position[1] = o->TargetPosition[1] + cosf(o->MultiUse * 0.1f) * o->Direction[0];
            o->Position[2] = o->TargetPosition[2] + (300 * (o->LifeTime / 80.f));

            if (o->Collision)
            {
                o->MultiUse += (2) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->MultiUse -= (2) * FPS_ANIMATION_FACTOR;
            }
            o->Direction[0] -= (1.f) * FPS_ANIMATION_FACTOR;

            if (o->Direction[0] < 0)
            {
                o->Direction[0] = 0.f;
                o->LifeTime = 0;
            }

            if (o->LifeTime < 40)
            {
                VectorScale(o->Light, powf(1.f / 1.1f, FPS_ANIMATION_FACTOR), o->Light);
            }
            else if (o->LifeTime > 68)
            {
                VectorScale(o->Light, powf(1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 14)
        {
            if (o->LifeTime < 5)
            {
                float fLumi = o->LifeTime / 5.f;
                VectorScale(o->Light, powf(fLumi, FPS_ANIMATION_FACTOR), o->Light);
            }
            o->Scale *= powf(1.05f, FPS_ANIMATION_FACTOR);
            VectorCopy(o->Target->Position, o->Position);
        }
        else if (o->SubType == 15)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 4.f;

            VectorCopy(o->Target->Position, p);

            MoveHumming(o->Position, o->Angle, p, 10.f);

            VectorScale(o->Light, powf(1.f / 1.08f, FPS_ANIMATION_FACTOR), o->Light);

            CreateSprite(BITMAP_SHINY + 1, p, (float)(rand() % 4 + 4) * 0.2f, o->Light, o->Target, (float)(rand() % 360));
        }
        else if (o->SubType == 16)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 4.f;

            VectorCopy(o->Target->Position, p);

            MoveHumming(o->Position, o->Angle, p, 10.f);

            VectorScale(o->Light, powf(1.f / 1.08f, FPS_ANIMATION_FACTOR), o->Light);

            CreateSprite(BITMAP_SHINY + 1, p, (float)(rand() % 4 + 4) * 0.2f, o->Light, o->Target, (float)(rand() % 360), 1);
        }
        else
        {
            if (o->SubType == 6 || o->SubType == 7)
            {
                o->Velocity += FPS_ANIMATION_FACTOR * 2.f;
            }
            else
            {
                o->Velocity += FPS_ANIMATION_FACTOR * 4.f;
            }

            if (o->SubType == 6 || o->SubType == 7 || o->SubType == 12)
            {
                VectorCopy(o->TargetPosition, p);
            }
            else
            {
                if (o->SubType == 8 || o->SubType == 13 || o->SubType == 17)
                {
                    VectorCopy(o->TargetPosition, p);

                    if (o->LifeTime > 10)
                    {
                        MoveHumming(o->Position, o->Angle, p, 10.f);
                    }
                    else if (o->LifeTime < 6)
                    {
                        VectorScale(o->Light, powf(1.f / 1.8f, FPS_ANIMATION_FACTOR), o->Light);
                    }

                    if (o->SubType == 13)
                        CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, NULL);
                    else
                        CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, NULL);
                }
                else
                {
                    VectorCopy(o->Target->Position, p);
                    p[2] += 120.f * FPS_ANIMATION_FACTOR;

                    MoveHumming(o->Position, o->Angle, p, 10.f);
                }
            }
            Luminosity = (12 - o->LifeTime) * 0.1f;
            switch (o->SubType)
            {
            case 1:Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, o->Light); break;
            case 2:Vector(Luminosity * 0.4f, Luminosity * 1.f, Luminosity * 0.6f, o->Light); break;
            case 3:Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.4f, o->Light); break;
            case 11:Vector(Luminosity * 0.9f, Luminosity * 0.49f, Luminosity * 0.04f, o->Light); break;
            case 12:Vector(Luminosity * 0.9f, Luminosity * 0.39f, Luminosity * 0.03f, o->Light); break;
            }
            if (o->SubType == 6)
            {
                if (o->LifeTime <= 10)
                {
                    float fAlpha = (float)(6 - abs(o->LifeTime - 6)) * 0.15f;
                    vec3_t Light;
                    Light[0] = Light[1] = Light[2] = fAlpha;
                    CreateSprite(BITMAP_SHINY + 1, p, (float)(rand() % 8 + 8) * 0.05f, Light, o->Target, (float)(rand() % 360));
                }
            }
            else if (o->SubType != 7 && o->SubType != 8 && o->SubType != 12)
            {
                if ((int)o->LifeTime == 1)
                {
                    CreateSprite(BITMAP_SHINY + 1, p, (float)(rand() % 8 + 8) * 0.2f, o->Light, o->Target, (float)(rand() % 360));
                }
                if (o->SubType == 11)
                {
                    Luminosity = (float)(rand() % 4 + 4) * 0.01f;
                    Vector(Luminosity, Luminosity, Luminosity, Light);
                    AddTerrainLight(p[0], p[1], Light, 1, PrimaryTerrainLight);
                }
            }
        }
        break;
    case BITMAP_2LINE_GHOST:
    {
        if (o->SubType == 0)
        {
            if ((int)o->LifeTime % 16 <= 7)
            {
                o->m_sTargetIndex = 10;
                o->Angle[2] += (o->m_sTargetIndex) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->m_sTargetIndex = -10;
                o->Angle[2] += (o->m_sTargetIndex) * FPS_ANIMATION_FACTOR;
            }
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 59, 1.0f);
        }
        else if (o->SubType == 1)
        {
            float TargetAngleH;
            TargetAngleH = CreateAngle2D(o->Position, o->TargetPosition);
            o->Angle[2] = TargetAngleH - 65;

            float TargetAngleV;
            vec3_t Range;
            VectorSubtract(o->Position, o->TargetPosition, Range);
            float Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
            if (Distance > 60)
            {
                TargetAngleV = 360.f - CreateAngle(o->Position[2], Distance, o->TargetPosition[2], 0.f);
                o->Angle[0] = TurnAngle2(o->Angle[0], TargetAngleV, 3.0f);
            }
            else
            {
                o->Angle[2] = TargetAngleH - 90;
            }
        }
    }
    break;
    case BITMAP_JOINT_SPIRIT:
    case BITMAP_JOINT_SPIRIT2:
        if (0 == o->SubType || o->SubType == 5 || o->SubType == 19)
        {
            if (o->Scale == 80.f)
            {
                if (o->SubType == 5)
                    CreateEffectFpsChecked(MODEL_LASER, o->Position, o->Angle, o->Light, 3);
                else
                {
                    CreateEffectFpsChecked(MODEL_LASER, o->Position, o->Angle, o->Light);
                }

                if (battleCastle::IsBattleCastleStart())
                {
                    DWORD att = TERRAIN_ATTRIBUTE(o->Position[0], o->Position[1]);
                    if ((att & TW_NOATTACKZONE) == TW_NOATTACKZONE)
                    {
                        o->Velocity = 0.f;
                        o->LifeTime *= powf(1.f / 5.f, FPS_ANIMATION_FACTOR);
                        break;
                    }
                }

#ifndef CSK_EVIL_SKILL
                if ((int)o->LifeTime % 15 == 0)
                {
                    if (o->Target == &Hero->Object)
                        AttackCharacterRange(o->Skill, o->Position, 150.f, o->Weapon, o->PKKey, o->m_bySkillSerialNum);
                }
#endif // CSK_EVIL_SKILL
            }
            else if (o->SubType == 19)
            {
                CreateEffect(MODEL_SKULL, o->Position, o->Angle, o->Light);
            }
            VectorCopy(o->Target->Position, o->TargetPosition);
            o->TargetPosition[2] += 80.f;

            Distance = sqrtf(dx * dx + dy * dy);
            if (o->SubType == 5)
            {
                MoveHumming(o->Position, o->Angle, o->TargetPosition, 2.f);
            }
            else
            {
                MoveHumming(o->Position, o->Angle, o->TargetPosition, 10.f);
            }
            if (!o->Collision && Distance <= o->Velocity * 2.f * FPS_ANIMATION_FACTOR)
            {
                o->Collision = true;
            }

            if (o->SubType == 5)
            {
            }
            else
            {
                o->Direction[0] += ((float)(rand() % 32 - 16) * 0.2f);
                o->Direction[2] += ((float)(rand() % 32 - 16) * 0.8f);
                o->Angle[0] += (o->Direction[0]) * FPS_ANIMATION_FACTOR;
                o->Angle[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
                o->Direction[0] *= 0.6f;// powf(0.6f, FPS_ANIMATION_FACTOR);
                o->Direction[2] *= 0.8f; // powf(0.8f, FPS_ANIMATION_FACTOR);
            }

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height + 100.f)
            {
                o->Direction[0] = 0.f;
                o->Angle[0] = -5.f;
            }
            if (o->Position[2] > Height + 400.f)
            {
                o->Direction[0] = 0.f;
                o->Angle[0] = 5.f;
            }

            //light
            if (o->SubType != 19)
            {
                Luminosity = o->LifeTime * 0.1f;
                Vector(Luminosity, Luminosity, Luminosity, o->Light);
                Luminosity = -(float)(rand() % 4 + 4) * 0.01f;
                Vector(Luminosity, Luminosity, Luminosity, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
            }
        }
        else if (1 == o->SubType)
        {
            Vector(.8f, 0.4f, 1.f, Light);
            CreateSprite(BITMAP_LIGHT, o->Position, 4.0f, Light, o->Target, (float)(rand() % 360), 0);
        }
        else if (o->SubType == 3 || o->SubType == 13)
        {
            float Scale = 3.f;
            if (o->SubType != 13 || o->LifeTime > 28)
            {
                vec3_t vRot;
                GetMagicScrew(iIndex, vRot);
                VectorScale(vRot, 50.f, vRot);
                VectorAdd(o->Position, vRot, o->Position);
                Vector(1.0f, 0.5f, 0.1f, Light);
            }
            else if (o->Target != NULL)
            {
                if (rand_fps_check(2))
                {
                    if (o->Angle[0] < -90)
                        o->Angle[0] += (20.f);
                    else
                        o->Angle[0] -= (20.f);
                }
                float Distance;
                if (o->SubType == 13)
                {
                    VectorCopy(o->Target->Position, Position);
                    Position[0] += (rand() % 150 - 70.f) * FPS_ANIMATION_FACTOR;
                    Position[1] += (rand() % 150 - 70.f) * FPS_ANIMATION_FACTOR;
                    Position[2] += (50.f) * FPS_ANIMATION_FACTOR;
                    Distance = MoveHumming(o->Position, o->Angle, Position, (o->Velocity - 20));
                }
                else
                {
                    Distance = MoveHumming(o->Position, o->Angle, o->Target->Position, (o->Velocity - 20));
                }
                if (o->Velocity > 50.f)
                    o->Velocity = 50.f;
                else
                    o->Velocity += FPS_ANIMATION_FACTOR;

                if (Distance < 50.f)
                {
                    o->LifeTime -= FPS_ANIMATION_FACTOR;
                }
                if (o->LifeTime < 10)
                {
                    VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
                    VectorCopy(o->Light, Light);
                }
                if (o->SubType == 13)
                {
                    VectorCopy(o->Light, Light);

                    Scale = 2.f * (o->LifeTime / 10) + 2.f;
                }
            }
            CreateSprite(BITMAP_LIGHT, o->Position, Scale, Light, o->Target, (float)(rand() % 360), 0);
            CreateSprite(BITMAP_SHINY + 1, o->Position, Scale / 2.f, Light, o->Target, (float)(rand() % 360), 0);
        }
        else if (o->SubType == 2 || o->SubType == 6 || o->SubType == 7 || o->SubType == 21
            || o->SubType == 22 || o->SubType == 23)
        {
            vec3_t Angle;

            switch (o->SubType)
            {
            case 2:	Vector(1.f, 0.5f, 0.1f, Light); break;
            case 21: Vector(0.1f, 0.5f, 1.f, Light); break;
            case 22:
            case 23:
                switch (o->Skill)
                {
                case 0: Vector(1.0f, 1.0f, 1.0f, Light); break;
                case 1: Vector(1.0f, 1.0f, 1.0f, Light); break;
                }
                break;
            case 7:
            case 6:
                switch (o->Skill)
                {
                case 0: Vector(0.3f, 0.3f, 1.f, Light); break;
                case 1: Vector(0.5f, 0.5f, 0.5f, Light); break;
                }
                break;
            }

            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
                VectorCopy(o->Light, Light);
            }
            else if (o->LifeTime > 18 && o->LifeTime < 20 && (o->SubType == 2 || o->SubType == 21))
            {
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);

                Position[0] = o->StartPosition[0] + rand() % 200 - 100;
                Position[1] = o->StartPosition[1] + rand() % 200 - 100;
                Position[2] = o->StartPosition[2] - 200;

                if (o->SubType == 2)
                    CreateJointFpsChecked(BITMAP_FLARE, Position, Position, Angle, 2, NULL, 40);
            }

            if (o->SubType == 22 || o->SubType == 23)
            {
                CreateSprite(BITMAP_FLAME, o->Position, (o->Scale + (20 - o->LifeTime) / 5), Light, o->Target, (float)(rand() % 360), 0);
            }
            else
                CreateSprite(BITMAP_LIGHT, o->Position, (4.0f + (20 - o->LifeTime) / 5), Light, o->Target, (float)(rand() % 360), 0);

            o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
        }
        else if (8 == o->SubType)
        {
            if (o->LifeTime < 42)
            {
                o->Velocity = 50.f;
                if (o->Angle[0] <= 180.0f)
                    o->Angle[0] = 180.0f;
                if (o->Angle[0] <= 360.0f)
                    o->Angle[0] += (rand() % 3 + 2.0f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Velocity = 10.f;
                o->Angle[0] = 110.0f;
            }
        }
        else if (10 == o->SubType)
        {
            o->Velocity = 50.f;
            o->Angle[2] -= (50.0f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 9)
        {
            o->Position[0] += (rand() % 10 - 5.f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (rand() % 10 - 5.f) * FPS_ANIMATION_FACTOR;
            o->Position[2] += (o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Velocity += FPS_ANIMATION_FACTOR * 1.f;

            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.15f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 11)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position;
            VectorCopy(o->Position, Position);

            o->Angle[0] = (float)o->LifeTime;	// 임시로 -_-
            CreateParticleFpsChecked(BITMAP_FIRE + 1, Position, o->Angle, Light, 5, 0.9f);
            if (rand_fps_check(200))
            {
                CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, o->Angle, 12, NULL, 0.9f);
            }
            //			CreateSprite(BITMAP_FIRE+1,Position,2.0f,Light,o->Target,(float)(rand()%360),0);
            o->Position[0] += (cosf(o->Angle[2]) * 20.0f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (sinf(o->Angle[2]) * 20.0f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 12)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position;
            VectorCopy(o->Position, Position);

            Vector(1.0f, 0.2f, 0.1f, Light);
            //if (rand()%2 == 1)
            CreateParticleFpsChecked(BITMAP_SMOKE + 3, Position, o->Angle, Light, 0, (float)(rand() % 32 + 48) * 0.01f);

            Vector(1, 1, 1, Light);
            CreateParticleFpsChecked(BITMAP_FIRE + 1, Position, o->Angle, Light, 6, 0.6f);
        }
        else if (o->SubType == 14)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position;
            VectorCopy(o->Position, Position);

            o->Angle[0] -= (2.0f) * FPS_ANIMATION_FACTOR;
            int r = rand() % 5;
            if (r == 0)
                CreateParticleFpsChecked(BITMAP_WATERFALL_3, o->Position, o->Angle, o->Light, 2);
            else if (r <= 2)
                CreateParticleFpsChecked(BITMAP_WATERFALL_4, o->Position, o->Angle, o->Light, 2);
            else
                CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, o->Light, 4);
        }
        else if (o->SubType == 15)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position;
            VectorCopy(o->Position, Position);
            o->PKKey += (30) * FPS_ANIMATION_FACTOR;

            if (o->LifeTime < 35)
            {
                CreateParticleFpsChecked(BITMAP_FIRE, Position, o->Angle, Light, 7, 1.0f);

                if (rand_fps_check(2))//if ((int)o->LifeTime % 2 == 0)
                {
                    Position[0] -= (sinf(o->Angle[2]) * (40.0f - o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
                    Position[1] += (cosf(o->Angle[2]) * (40.0f - o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
                    Position[2] = 350;
                    vec3_t Angle;

                    for (int i = 0; i < 2; ++i)
                    {
                        Vector(0.f, 0.f, i * 3.f, Angle);
                        CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 14, NULL, 0.9f);
                    }
                }
                if (o->Target != NULL)
                {
                    if (o->Target->Angle[0] != 0)
                        o->Target->Angle[0] = 0;
                    if (o->Target->LifeTime > 5)
                        o->Target->Alpha = 1.0f;
                    VectorCopy(o->Position, Position);
                    Position[0] += (sinf(o->Angle[2]) * (30.0f + o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
                    Position[1] -= (cosf(o->Angle[2]) * (30.0f + o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
                    //Position[2] = 350;
                    VectorCopy(Position, o->Target->Position);
                    if (o->Target->Angle[0] != 0)
                        o->Target->Angle[0] = 0;
                    o->Target->Angle[1] = 0;
                    //					VectorCopy(o->Angle, o->Target->Angle);
                }
            }
            //CreateParticle(BITMAP_FIRE+1,Position,o->Angle,Light,5,0.9f);
            //			CreateSprite(BITMAP_FIRE+1,Position,2.0f,Light,o->Target,(float)(rand()%360),0);
            o->Position[0] += (sinf(o->Angle[2]) * (30.0f + o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
            o->Position[1] -= (cosf(o->Angle[2]) * (30.0f + o->PKKey * 0.1f)) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 16)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position;
            VectorCopy(o->Position, Position);

            o->Angle[0] = (float)o->LifeTime;
            CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, o->Light, 4);
            o->Position[0] += (cosf(o->Angle[2]) * 30.0f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (sinf(o->Angle[2]) * 30.0f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 17)
        {
            Vector(1, 1, 1, Light);
            vec3_t Position, Angle;
            VectorCopy(o->Position, Position);

            if ((int)o->LifeTime == 100)//% 10 == 0)
            {
                for (int i = 0; i < 120; ++i)
                {
                    Vector(0.f, 0.f, i * 3.f, Angle);
                    CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, o->Position, Angle, 16, NULL, 0.9f);
                }
            }
        }
        else if (o->SubType == 18)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 3.0f;
            o->Angle[0] = -90.0f;

            int add = (int)o->LifeTime % 12;
            if (add <= 5)
                o->PKKey += (6) * FPS_ANIMATION_FACTOR;
            else
                o->PKKey -= (6) * FPS_ANIMATION_FACTOR;

            o->Angle[0] -= (o->PKKey) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (10) * FPS_ANIMATION_FACTOR;

            if (o->Target != NULL)
            {
                if (o->Target->LifeTime > 5)
                    o->Target->Alpha = 1.0f;
                VectorCopy(o->Position, o->Target->Position);
            }

            VectorCopy(o->Position, Position);
            Vector(1, 1, 1, Light);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 19, 7.f);
        }
        else if (o->SubType == 20)
        {
            if (o->LifeTime < 42)
            {
                o->Velocity = 50.f;
                if (o->Angle[0] <= 180.0f)
                    o->Angle[0] = 180.0f;
                if (o->Angle[0] <= 360.0f)
                    o->Angle[0] += (rand() % 3 + 2.0f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Velocity = 10.f;
                o->Angle[0] = 110.0f;
            }
        }
        else if (o->SubType == 24)
        {
            if (o->Target == NULL || o->Target->Live == false)
            {
                o->Live = false;
                break;
            }

            vec3_t vPos;
            VectorCopy(o->TargetPosition, vPos);
            vPos[2] += (80.f) * FPS_ANIMATION_FACTOR;

            MoveHumming(o->Position, o->Angle, vPos, 5.f);

            o->Direction[0] += ((float)(rand() % 32 - 16) * 0.2f) * FPS_ANIMATION_FACTOR;
            o->Direction[2] += ((float)(rand() % 32 - 16) * 0.8f) * FPS_ANIMATION_FACTOR;
            o->Angle[0] += (o->Direction[0]) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            o->Direction[0] *= powf(0.6f, FPS_ANIMATION_FACTOR);
            o->Direction[2] *= powf(0.8f, FPS_ANIMATION_FACTOR);

            if (o->LifeTime < 30)
                Luminosity = o->LifeTime / 30.f;
            else if (o->LifeTime > 144)
                Luminosity = (160 - o->LifeTime) / 15.f;
            else
                Luminosity = 1.0f;
            Vector(0.7f * Luminosity, 0.7f * Luminosity, 0.9f * Luminosity, o->Light);

            if (o->Target != NULL)
            {
                VectorCopy(o->Position, o->Target->Position);
                VectorCopy(o->Angle, o->Target->Angle);
            }
        }
        else if (o->SubType == 25)
        {
            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.45f, FPS_ANIMATION_FACTOR), o->Light);

                if (o->Light[0] < 0.2f)
                    o->Live = false;
            }
            else
            {
                o->Position[0] += (rand() % 16 - 8.f) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % 16 - 8.f) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (o->Velocity) * FPS_ANIMATION_FACTOR;
                //o->Velocity += FPS_ANIMATION_FACTOR * 1.f;

                o->Scale += (3.0f) * FPS_ANIMATION_FACTOR;
            }

            CreateSprite(BITMAP_LIGHT, o->Position, 0.5f, o->Light, o->Target);
            CreateSprite(BITMAP_DS_SHOCK, o->Position, 0.15f, o->Light, o->Target);

            if ((int)o->LifeTime == 10)
            {
                CreateEffectFpsChecked(BITMAP_FIRECRACKER0002, o->Position, o->Angle, o->Light, o->Skill);
            }
        }
        break;
    case BITMAP_JOINT_SPARK:
        if (o->SubType == 1)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 0.3f;
            o->Light[0] *= powf(1.f / 1.4f, FPS_ANIMATION_FACTOR);
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
        }
        else if (o->SubType == 3)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 0.1f;
            o->Light[0] *= powf(1.f / 1.1f, FPS_ANIMATION_FACTOR);
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
        }
        else if (o->SubType == 4)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 0.1f;
            o->Light[0] *= powf(1.f / 1.1f, FPS_ANIMATION_FACTOR);
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
        }
        break;
    case BITMAP_JOINT_FIRE:
        Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 0.f);
        float Matrix[3][4];
        AngleMatrix(o->Angle, Matrix);
        CreateTail(o, Matrix);
        if (Distance <= o->Velocity)
        {
            o->Live = false;
            CreateParticle(BITMAP_EXPLOTION, o->Position, o->Angle, o->Light);
            /*o->TargetCharacter->TargetCharacter = o->OwnerCharacter;
            if(o->TargetCharacter->Object.CurrentAction != PLAYER_SHOCK)
            {
            Damage(o->Position,o->TargetCharacter,o->Velocity*2.f,rand()%15+5,true);
        }*/
            break;
        }
        Luminosity = (float)(rand() % 4 + 4) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, Position);
        VectorRotate(Position, Matrix, p);
        VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
        break;
    case MODEL_SPEARSKILL:	// 방어막
        CHARACTER* c;
        if (o->m_iChaIndex != -1)
        {
            c = &CharactersClient[o->m_iChaIndex];
            if (o->SubType == 4)
            {
                if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                    o->SubType = 9;
            }
            else if (o->SubType == 9)
            {
                if (c->Helper.Type != MODEL_HORN_OF_FENRIR || c->SafeZone)
                    o->SubType = 4;
            }
            else if (o->SubType == 10)
            {
                if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA
                    || c->Helper.Type == MODEL_HORN_OF_DINORANT
                    || c->Helper.Type == MODEL_DARK_HORSE_ITEM
                    || c->Helper.Type == MODEL_HORN_OF_FENRIR) && c->SafeZone == false)
                {
                    o->SubType = 11;
                }
            }
            else if (o->SubType == 11)
            {
                if ((c->Helper.Type != MODEL_HORN_OF_UNIRIA
                    && c->Helper.Type != MODEL_HORN_OF_DINORANT
                    && c->Helper.Type != MODEL_DARK_HORSE_ITEM
                    && c->Helper.Type != MODEL_HORN_OF_FENRIR) || c->SafeZone == true)
                {
                    o->SubType = 10;
                }
            }
        }
        if (2 == o->SubType)
        {
            if (!o->Target || !o->Target->Live)
            {
                o->Target = NULL;
                break;
            }
            o->Scale = (float)o->LifeTime * 3.0f;
            float fRate1 = max(0.0f, min((float)(o->LifeTime - 10) / (float)10, 1.0f));
            float fRate2 = 1.0f - fRate1;

            vec3_t MagicPos;
            GetMagicScrew(iIndex * 17721, MagicPos, 1.4f);
            //VectorScale(MagicPos, 300.0f, MagicPos);
            VectorAdd(MagicPos, o->TargetPosition, MagicPos);
            vec3_t TargetPos;
            VectorCopy(o->Target->m_vPosSword, TargetPos);
            //TargetPos[2] += (120.0f) * FPS_ANIMATION_FACTOR;

            for (int i = 0; i < 3; ++i)
            {
                o->Position[i] = fRate2 * TargetPos[i] + fRate1 * MagicPos[i];
            }
        }
        else if (o->SubType == 3)
        {
            if (!o->Target->Live)
            {
                o->LifeTime = 0;
                o->Live = false;
                break;
            }
            if (rand_fps_check(5))
            {
                o->LifeTime -= 5 * FPS_ANIMATION_FACTOR;
            }
            else if (rand_fps_check(2))
            {
                o->LifeTime += FPS_ANIMATION_FACTOR;
            }
            o->Direction[0] = sinf((o->LifeTime - o->Direction[1]) * 0.05f) * 3.f;
            o->Position[0] += (o->Direction[0]) * FPS_ANIMATION_FACTOR;

            float fAlpha;
            if (o->Target != NULL)
            {
                if (o->Target->LifeTime > 50)
                {
                    fAlpha = (100 - o->Target->LifeTime) / 40.f;
                }
                else
                {
                    fAlpha = o->Target->LifeTime / 10.f;
                }

                if (fAlpha > 1.f) fAlpha = 1.f;

                o->Light[0] = o->Target->Light[0] * fAlpha;
                o->Light[1] = o->Target->Light[1] * fAlpha;
                o->Light[2] = o->Target->Light[2] * fAlpha;
            }
        }
        else if (o->SubType == 5 || o->SubType == 6 || o->SubType == 7)
        {
            for (int i = 0; i < 3; ++i)
            {
                o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(o->Direction, Matrix, Position);
                VectorAdd(o->StartPosition, Position, o->Position);
                CreateTail(o, Matrix);

                if (o->Weapon == 0)
                {
                    o->Direction[1] *= powf(0.95f, FPS_ANIMATION_FACTOR);
                    if (o->Direction[1] < 10.f) o->Direction[1] = -10.f;
                }
                if (o->Weapon > 40)
                {
                    o->Direction[1] -= (20.f) * FPS_ANIMATION_FACTOR;
                    o->Angle[2] -= (5.f) * FPS_ANIMATION_FACTOR;
                    o->Scale += (15.f) * FPS_ANIMATION_FACTOR;
                }
                if (o->Direction[1] < 0)
                {
                    if (o->Weapon == 40 && o->SubType <= 6)
                    {
                        CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->StartPosition, o->Angle, o->Light, 3);
                    }
                    o->Weapon += FPS_ANIMATION_FACTOR;

                    if (o->Weapon < 20)
                    {
                        o->Direction[1] = -30.f;
                        o->Scale = 40.f;
                    }
                    else
                    {
                        o->StartPosition[2] += (5.f) * FPS_ANIMATION_FACTOR;
                    }
                }
            }

            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }

            o->Scale -= (5.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 8)
        {
            o->Angle[2] += (25.f) * FPS_ANIMATION_FACTOR;
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAdd(o->StartPosition, Position, o->Position);
            CreateTail(o, Matrix);

            o->StartPosition[2] += (15.f) * FPS_ANIMATION_FACTOR;
        }
        else
        {
            if (!o->Target->Live)
            {
                o->LifeTime = 0;
                o->Live = false;
                break;
            }

            if (o->SubType == 10 || o->SubType == 11)
            {
                float fLumi = (float)(rand() % 4 + 4) * 0.1f;
                Vector(fLumi * 1.5f, fLumi * 0.6f, fLumi * 0.6f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);

                if (rand_fps_check(10))
                {
                    fLumi *= powf(0.2f, FPS_ANIMATION_FACTOR);
                    Vector(0.9f + fLumi, 0.5f + fLumi, 0.5f + fLumi, Light);
                    CreateParticle(BITMAP_SPARK + 1, o->Position, o->Angle, Light, 19);
                }
            }

            if (o->SubType == 4 || o->SubType == 9)
            {
                if (g_isCharacterBuff(o->Target, eBuff_Defense) || g_isCharacterBuff(o->Target, eBuff_HelpNpc))
                {
                    o->LifeTime = 100;
                }
                else
                {
                    o->LifeTime = 0;
                    o->Live = false;
                    break;
                }
            }

            if (0 == o->SubType || o->SubType == 4 || o->SubType == 9
                || o->SubType == 10 || o->SubType == 11
                || o->SubType == 14
                || o->SubType == 16
                )
            {
                for (int j = o->NumTails - 1; j >= 0; j--)
                {
                    for (int k = 0; k < 4; k++)
                        VectorSubtract(o->Tails[j][k], o->TargetPosition, o->Tails[j][k]);
                }
            }

            if (o->SubType == 14)
            {
                vec3_t vRelative;
                Vector(0, 0, 0, vRelative);
                BMD* pModel = &Models[o->Target->Type];
                pModel->TransformPosition(o->Target->BoneTransform[37], vRelative, o->TargetPosition, false);
                VectorScale(o->TargetPosition, pModel->BodyScale, o->TargetPosition);
                VectorAdd(o->Target->Position, o->TargetPosition, o->TargetPosition);
            }
            else if (o->SubType == 15 || o->SubType == 17)
            {
                if (o->Target->Owner == NULL)
                {
                    o->Live = false;
                    break;
                }

                for (int j = o->NumTails - 1; j >= 0; j--)
                {
                    for (int k = 0; k < 4; k++)
                        VectorSubtract(o->Tails[j][k], o->StartPosition, o->Tails[j][k]);
                }
                VectorCopy(o->Target->Owner->Position, o->StartPosition);
                VectorCopy(o->Target->Position, o->TargetPosition);

                for (int j = o->NumTails - 1; j >= 0; j--)
                {
                    for (int k = 0; k < 4; k++)
                        VectorAdd(o->Tails[j][k], o->StartPosition, o->Tails[j][k]);
                }
            }
            else
            {
                VectorCopy(o->Target->Position, o->TargetPosition);
                o->TargetPosition[2] += 10.f;
            }

            if (0 == o->SubType || o->SubType == 4 || o->SubType == 9
                || o->SubType == 10 || o->SubType == 11
                || o->SubType == 14
                || o->SubType == 16
                )
            {
                for (int j = o->NumTails - 1; j >= 0; j--)
                {
                    for (int k = 0; k < 4; k++)
                        VectorAdd(o->Tails[j][k], o->TargetPosition, o->Tails[j][k]);
                }
            }
            int iFrame = static_cast<int>(WorldTime / 40.f);

            iFrame = ((iIndex % 2) ? iFrame : -iFrame) + iIndex * 53731;

            vec3_t vDir, vDirTemp;
            float fSpeed[3] = { 0.048f, 0.0613f, 0.1113f };
            if (o->SubType == 1)
            {
                fSpeed[0] *= 0.5f;
                fSpeed[1] *= 0.5f;
                fSpeed[2] *= 0.5f;
            }
            vDirTemp[0] = sinf((float)(iFrame + 55555) * fSpeed[0]) * cosf((float)iFrame * fSpeed[1]);
            vDirTemp[1] = sinf((float)(iFrame + 55555) * fSpeed[0]) * sinf((float)iFrame * fSpeed[1]);
            vDirTemp[2] = cosf((float)(iFrame + 55555) * fSpeed[0]);

            float fSinAdd = sinf((float)(iFrame + 11111) * fSpeed[2]);
            float fCosAdd = cosf((float)(iFrame + 11111) * fSpeed[2]);
            vDir[2] = vDirTemp[0];
            vDir[1] = fSinAdd * vDirTemp[1] + fCosAdd * vDirTemp[2];
            vDir[0] = fCosAdd * vDirTemp[1] - fSinAdd * vDirTemp[2];
            // VectorScale(vDir, FPS_ANIMATION_FACTOR, vDir);

            switch (o->SubType)
            {
            case 0:
            case 4:
                o->Position[0] = o->TargetPosition[0] + vDir[0] * 80.0f;// + ( float)( rand() % 11 - 5);
                o->Position[1] = o->TargetPosition[1] + vDir[1] * 80.0f;// + ( float)( rand() % 11 - 5);
                o->Position[2] = 110.0f + o->TargetPosition[2] + vDir[2] * 120.0f;
                break;
            case 9:
                o->Position[0] = o->TargetPosition[0] + vDir[0] * 80.0f;// + ( float)( rand() % 11 - 5);
                o->Position[1] = o->TargetPosition[1] + vDir[1] * 80.0f;// + ( float)( rand() % 11 - 5);
                o->Position[2] = 140.0f + o->TargetPosition[2] + vDir[2] * 120.0f;
                break;
            case 10:
                o->Position[0] = o->TargetPosition[0] + vDir[0] * 60.0f;// + ( float)( rand() % 11 - 5);
                o->Position[1] = o->TargetPosition[1] + vDir[1] * 60.0f;// + ( float)( rand() % 11 - 5);
                o->Position[2] = 50.0f + o->TargetPosition[2] + vDir[2] * 60.0f;
                break;
            case 11:
                o->Position[0] = o->TargetPosition[0] + vDir[0] * 60.0f;// + ( float)( rand() % 11 - 5);
                o->Position[1] = o->TargetPosition[1] + vDir[1] * 60.0f;// + ( float)( rand() % 11 - 5);
                o->Position[2] = 100.0f + o->TargetPosition[2] + vDir[2] * 60.0f;
                break;
            case 1:
                o->Position[0] = o->TargetPosition[0] + vDir[0] * 70.0f;// + ( float)( rand() % 11 - 5);
                o->Position[1] = o->TargetPosition[1] + vDir[1] * 70.0f;// + ( float)( rand() % 11 - 5);
                o->Position[2] = o->TargetPosition[2] + vDir[2] * 140.0f;

                Vector(0.2f, 0.2f, 0.4f + 0.2f * fSinAdd, o->Light);
                break;
            case 14:	// 소환 손목링
                if (o->Target != NULL)
                {
                    if (o->Target->Live)
                        o->LifeTime = 100.f; //무한
                    else
                    {
                        DeleteJoint(MODEL_SPEARSKILL, o->Target, 14);
                        break;
                    }

                    o->Position[0] = o->TargetPosition[0] + vDir[0] * 15.0f;
                    o->Position[1] = o->TargetPosition[1] + vDir[1] * 15.0f;
                    o->Position[2] = o->TargetPosition[2] + vDir[2] * 15.0f;
                }
                break;
            case 15:
                if (o->Target != NULL)
                {
                    if (o->Target->Live)
                    {
                        o->LifeTime = 100.f;
                        Vector(o->Target->Alpha, o->Target->Alpha, o->Target->Alpha, o->Light);
                    }
                    else
                    {
                        DeleteJoint(MODEL_SPEARSKILL, o->Target, 15);
                        break;
                    }
                    VectorCopy(o->TargetPosition, o->Position);
                }
                break;
            case 16:
            {
                if (o->Target != NULL)
                {
                    o->Position[0] = o->TargetPosition[0] + vDir[0] * 20.0f;// + ( float)( rand() % 11 - 5);
                    o->Position[1] = o->TargetPosition[1] + vDir[1] * 20.0f;// + ( float)( rand() % 11 - 5);
                    o->Position[2] = 100.0f + o->TargetPosition[2] + vDir[2] * 40.0f;
                }
            }
            break;
            case 17:
                if (o->Target != NULL)
                {
                    if (o->Target->Live)
                    {
                        o->LifeTime = 100.f;
                    }
                    else
                    {
                        DeleteJoint(MODEL_SPEARSKILL, o->Target, 17);
                        break;
                    }
                    VectorCopy(o->TargetPosition, o->Position);
                }
                break;
            }
        }
        break;
    case BITMAP_SMOKE:
        if (o->Target != NULL && o->Target->Live && (o->Target->Type == MODEL_MAYASTONE1 || o->Target->Type == MODEL_MAYASTONE2 || o->Target->Type == MODEL_MAYASTONE3 || o->Target->Type == MODEL_FIRE))
        {
            VectorCopy(o->Target->Position, o->Position);
        }
        break;
    case MODEL_FENRIR_SKILL_THUNDER:
        for (int j = 0; j < o->MaxTails; j++)
        {
            if (o->Target)
            {
                VectorCopy(o->Target->Position, o->TargetPosition);
                o->TargetPosition[2] += 80.f;
            }

            Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 50.f);

            o->Direction[0] = (float)(rand() % 1024 - 512) / o->Scale;
            o->Direction[2] = (float)(rand() % 1024 - 512) / o->Scale;

            vec3_t Angle;
            VectorAdd(o->Angle, o->Direction, Angle);
            float Matrix[3][4];
            AngleMatrix(Angle, Matrix);
            CreateTail(o, Matrix);

            if (Distance < o->Velocity * 1.5f)
            {
                if (o->Scale == 50.f)
                {
                    CreateParticleFpsChecked(BITMAP_ENERGY, o->Position, o->Angle, o->Light);
                    vec3_t Position;
                    if (rand_fps_check(8))
                    {
                        Vector((float)(rand() % 64 - 32), (float)(rand() % 64 - 32), (float)(rand() % 64 - 32), Position);
                        VectorAdd(Position, o->TargetPosition, Position);
                        CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light);
                    }
                }
                break;
            }

            if (o->Scale >= 50.f)
            {
                Luminosity = (float)(rand() % 4 + 4) * 0.04f;
                Vector(Luminosity * 0.4f, Luminosity * 0.4f, Luminosity * 0.4f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            }

            Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, Position);
            VectorRotate(Position, Matrix, p);
            VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
        }
        break;
    case BITMAP_BLUR + 1:
    case BITMAP_JOINT_LASER + 1:

        for (int j = 0; j < o->MaxTails; j++)
        {
            if (o->SubType == 2)
            {
            }
            else
            {
                VectorCopy(o->Target->Position, o->TargetPosition);
                o->TargetPosition[2] += 80.f;
            }

            Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 25.f);
            //float Distance = MoveHumming(o,0.f);
            o->Direction[0] += (float)(FPS_ANIMATION_FACTOR * (rand() % 256 - 128) / o->Scale);
            o->Direction[2] += (float)(FPS_ANIMATION_FACTOR * (rand() % 256 - 128) / o->Scale);
            VectorScale(o->Direction, powf(0.8f, FPS_ANIMATION_FACTOR), o->Direction);

            vec3_t Angle;
            VectorAdd(o->Angle, o->Direction, Angle);
            float Matrix[3][4];
            AngleMatrix(Angle, Matrix);
            CreateTail(o, Matrix);

            if (Distance <= o->Velocity * 2.f * FPS_ANIMATION_FACTOR)
            {
                vec3_t Position;
                if (rand_fps_check(2))
                {
                    Vector((float)(rand() % 64 - 32), (float)(rand() % 64 - 32), (float)(rand() % 64 - 32), Position);
                    VectorAdd(Position, o->TargetPosition, Position);
                    CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, o->Light);
                }
                //CreateParticle(BITMAP_ENERGY,o->Position,o->Angle,o->Light);
                /*o->TargetCharacter->TargetCharacter = o->OwnerCharacter;
                if(o->TargetCharacter->Object.CurrentAction != PLAYER_SHOCK)
                {
                Damage(o->Position,o->TargetCharacter,o->Velocity*2.f,rand()%15+5,true);
            }*/
                break;
            }
            //light
            Luminosity = (float)(rand() % 4 + 4) * 0.05f;
            if (o->Type == BITMAP_JOINT_LASER + 1)
            {
                if (o->SubType == 1)
                {
                    Vector(Luminosity * 1.f, Luminosity * .1f, Luminosity * .1f, Light);
                }
                else
                {
                    Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.3f, Light);
                }
            }
            else
            {
                Vector(Luminosity * 0.f, Luminosity * 0.1f, Luminosity * 0.2f, Light);
            }
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, Position);
            VectorRotate(Position, Matrix, p);
            VectorScale(p, FPS_ANIMATION_FACTOR, p)
            VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
        }
        break;
    case BITMAP_JOINT_THUNDER:
        if ((o->SubType == 6 && o->LifeTime > 4) || o->SubType == 8)
            break;

        switch (o->SubType)
        {
        case 4:
        case 5:
            VectorCopy(o->TargetPosition, o->Position);
            if (o->SubType == 4)
            {
                VectorCopy(o->TargetPosition, Position);
                Position[2] += (30.f) * FPS_ANIMATION_FACTOR;
                CreateSprite(BITMAP_SHINY + 1, Position, (float)(rand() % 8 + 8) * 0.2f, o->Light, NULL, (float)(rand() % 360));
            }
            break;

        case 9:
            Position[0] = o->TargetPosition[0] + rand() % 200 - 100;
            Position[1] = o->TargetPosition[1] + rand() % 200 - 100;
            Position[2] = o->TargetPosition[2];
        case 6:
        case 7:
        case 18:
        case 19:
            VectorCopy(o->StartPosition, o->Position);
            break;
        }

        for (int j = 0; j < o->MaxTails; j++)
        {
            if (o->SubType == 15)
                break;

            switch (o->SubType)
            {
            case 0:
            case 1:
            case 2:
            case 16:
            case 21:
            case 27:
            case 28:
            case 33:
                if (o->Target)
                {
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    o->TargetPosition[2] += 80.f;
                }

                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 50.f);

                break;
            case 3:
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 50.f);
                break;

            case 4:
                Position[0] = o->TargetPosition[0];
                Position[1] = o->TargetPosition[1];
                Position[2] = o->TargetPosition[2] - 300.f;

                Distance = MoveHumming(o->Position, o->Angle, Position, -10);//-25.f);
                break;
            case 5:
                Position[0] = o->TargetPosition[0];
                Position[1] = o->TargetPosition[1];
                Position[2] = o->TargetPosition[2] + 600.f;

                Distance = MoveHumming(Position, o->Angle, o->Position, -10);//-25.f);
                break;

            case 6:
                VectorCopy(o->Target->Position, o->TargetPosition);
                o->TargetPosition[0] += ((2050.f + rand() % 200)) * FPS_ANIMATION_FACTOR;
                o->TargetPosition[1] += ((2050.f + rand() % 200)) * FPS_ANIMATION_FACTOR;
                o->TargetPosition[2] -= (10000.f) * FPS_ANIMATION_FACTOR;

                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, (float)(rand() % 100 + 50));//-25.f);
                break;

            case 7:
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, (float)(rand() % 100 + 50));
                break;

            case 9:
                Distance = MoveHumming(o->Position, o->Angle, Position, (float)(rand() % 80 + 60.f));
                break;
            case 12:
            {
                BMD* b = &Models[o->Target->Type];
                vec3_t Pos1;
                Vector(0.0f, -100.0f, 0.0f, Pos1);
                b->TransformPosition(g_fBoneSave[3], Pos1, o->Position, true);
            }
            break;
            case 18:
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 110.f);
                break;
            case 19:
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 25.f + o->Scale);
                break;
            case 20:
                o->TargetPosition[2] += 100.f;
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 100.f);
                break;
                // ChainLighting
            case 22:
            case 23:
            case 24:
            {
                if (o->Target)
                {
                    if (j == 0)			// ????
                    {
                        OBJECT* pSourceObj = o->Target;
                        CHARACTER* pTargetChar = &CharactersClient[FindCharacterIndex(o->m_sTargetIndex)];
                        OBJECT* pTargetObj = &pTargetChar->Object;
                        vec3_t vRelativePos, vPos, vAngle;
                        BMD* pModel = &Models[pSourceObj->Type];
                        Vector(0.f, 0.f, 0.f, vRelativePos);
                        if (o->SubType == 22)
                        {
                            pModel->TransformPosition(pSourceObj->BoneTransform[37], vRelativePos, vPos, true);
                        }
                        else if (o->SubType == 23)
                        {
                            pModel->TransformPosition(pSourceObj->BoneTransform[28], vRelativePos, vPos, true);
                        }
                        else if (o->SubType == 24)
                        {
                            VectorCopy(pSourceObj->Position, vPos);
                            vPos[2] += (80.0f) * FPS_ANIMATION_FACTOR;
                        }

                        o->Direction[0] = (float)(rand() % 1024 - 512 / o->Scale);
                        o->Direction[2] = (float)(rand() % 1024 - 512 / o->Scale);
                        VectorAdd(o->Angle, o->Direction, vAngle);
                        float fMatrix[3][4];
                        AngleMatrix(vAngle, fMatrix);
                        CreateTail(o, fMatrix);

                        VectorCopy(vPos, o->Position);
                        VectorCopy(pTargetObj->Position, o->TargetPosition);

                        o->TargetPosition[2] += 80.f;
                    }

                    Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, o->Velocity);
                }
                else
                {
                    assert(!"디버깅");
                }
            }
            break;
            case 25:
            {
                if (o->Target)
                {
                    VectorCopy(o->Target->Position, o->TargetPosition);
                    o->TargetPosition[2] += 80.f;
                }

                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 40.f);
            }
            break;
            default:
                Distance = MoveHumming(o->Position, o->Angle, o->TargetPosition, 25.f);
                break;
            }

            if (o->SubType == 1 || o->SubType == 28)
            {
                o->Direction[0] = (float)(rand() % 256 - 128);
                o->Direction[2] = (float)(rand() % 256 - 128);
            }
            else if (o->SubType == 4)
            {
                o->Direction[0] = (float)(rand() % 64 - 32);
            }
            else if (o->SubType == 5)
            {
                o->Direction[0] = (float)(rand() % 32 - 16);
            }
            else if (o->SubType == 6)
            {
                o->Direction[0] = (float)(rand() % 100 + 20);
                o->Direction[2] = (float)(rand() % 100 + 20);
            }
            else if (o->SubType == 7)
            {
                o->Direction[0] = (float)(rand() % 100 + 20);
                o->Direction[2] = (float)(rand() % 100 + 20);
            }
            else if (o->SubType == 11)
            {
                o->Direction[0] = (float)(rand() % 1024 - 512) * 0.7f / o->Scale;
                o->Direction[2] = (float)(rand() % 1024 - 512) * 0.7f / o->Scale;
            }
            else if (o->SubType == 18)
            {
                o->Direction[0] = (float)(rand() % 64 - 32);
                o->Direction[2] = (float)(rand() % 64 - 32);
            }
            else if (o->SubType == 19)
            {
                o->Direction[0] = (float)(rand() % 1024 - 512) / o->Scale;
                o->Direction[1] = (float)(rand() % 1024 - 512) / o->Scale;
                o->Direction[2] = (float)(rand() % 1024 - 512) / o->Scale;
            }
            else
            {
                o->Direction[0] = (float)(rand() % 1024 - 512) / o->Scale;
                o->Direction[2] = (float)(rand() % 1024 - 512) / o->Scale;
            }

            vec3_t Angle;
            VectorAddScaled(o->Angle, o->Direction, Angle, FPS_ANIMATION_FACTOR);
            float Matrix[3][4];
            AngleMatrix(Angle, Matrix);
            CreateTail(o, Matrix);

            if (o->SubType == 3)
            {
                if (Distance > 150)
                {
                    o->LifeTime = 0;
                }
            }
            else if (Distance < o->Velocity * 1.5f &&
                (o->SubType != 6 && o->SubType != 9 && o->SubType != 7 && o->SubType != 18 && o->SubType != 19))
            {
                if (o->Scale == 50.f)
                {
                    CreateParticleFpsChecked(BITMAP_ENERGY, o->Position, o->Angle, o->Light);
                    vec3_t Position;
                    if (rand_fps_check(8))
                    {
                        Vector((float)(rand() % 64 - 32), (float)(rand() % 64 - 32), (float)(rand() % 64 - 32), Position);
                        VectorAdd(Position, o->TargetPosition, Position);
                        CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light);
                    }
                    if (o->SubType == 0 || o->SubType == 27)
                    {
                        if ((int)WorldTime % 1000 < 500 && rand_fps_check(16))
                        {
                            Vector((float)(rand() % 100 - 50), (float)(rand() % 100 - 50), (float)(rand() % 120 - 60), Position);
                            VectorAdd(Position, o->TargetPosition, Position);
                            CreateJoint(BITMAP_JOINT_THUNDER, Position, o->TargetPosition, o->Angle, 28, o->Target, 6.0f + (float)(rand() % 8), -1, 0, 0, 0, o->Light);
                        }
                    }
                }
                break;
            }

            if (o->Scale >= 50.f && o->SubType != 4 && o->SubType != 7)
            {
                if (o->SubType == 0 || o->SubType == 11 || o->SubType == 2 || o->SubType == 18 || o->SubType == 27)
                {
                    Luminosity = (float)(rand() % 4 + 4) * 0.04f;

                    if (o->SubType == 0 || o->SubType == 11 || o->SubType == 18 || o->SubType == 27)
                    {
                        Vector(Luminosity * 0.1f, Luminosity * 0.1f, Luminosity * 0.5f, Light);
                    }
                    else
                    {
                        Vector(Luminosity * 0.4f, Luminosity * 0.1f, Luminosity * 0.1f, Light);
                    }

                    if (o->SubType == 27)
                        VectorCopy(o->Light, Light);

                    AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
                }
            }

            Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, Position);
            VectorRotate(Position, Matrix, p);
            VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
        }
        if (o->SubType == 7 || o->SubType == 18)
        {
            VectorCopy(o->TargetPosition, o->Position);
        }

        else if (o->SubType == 15 && ((int)o->LifeTime % 2) == 0)
        {
            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->StartPosition, o->Angle, 0, NULL, 50.f);
            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->StartPosition, o->Angle, 0, NULL, 10.f);
            CreateParticleFpsChecked(BITMAP_ENERGY, o->Position, o->Angle, Light);

            vec3_t Angle;
            VectorCopy(o->StartPosition, Position);
            Position[2] += (100.f) * FPS_ANIMATION_FACTOR;

            for (int i = 0; i < o->MultiUse; ++i)
            {
                if ((i % 15) == 0)
                {
                    CHARACTER* tc = &CharactersClient[o->TargetIndex[i / 15]];
                    OBJECT* to = &tc->Object;

                    if (to->Live && tc->Dead == 0 && to->Kind == KIND_MONSTER && to->Visible)
                    {
                        VectorCopy(to->Position, o->TargetPosition);
                        VectorCopy(o->Angle, Angle);
                        o->TargetPosition[2] += 100.f;
                        Angle[2] = CreateAngle2D(o->Position, o->TargetPosition);

                        CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Position, o->TargetPosition, Angle, 0, NULL, 50.f);
                        CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Position, o->TargetPosition, Angle, 0, NULL, 10.f);

                        VectorCopy(o->TargetPosition, Position);
                    }
                }
            }

            if (o->MultiUse < o->Weapon)
            {
                if (((int)o->LifeTime % 2) == 0)
                    o->MultiUse+=FPS_ANIMATION_FACTOR;
            }
        }
        break;
    case BITMAP_JOINT_THUNDER + 1:
        if (o->SubType == 0)
        {
            if (o->LifeTime > 15)
            {
                Position[0] = o->TargetPosition[0] + rand() % 200 - 100;
                Position[1] = o->TargetPosition[1] + rand() % 100 - 50;
                Position[2] = o->TargetPosition[2];

                VectorCopy(o->StartPosition, o->Position);

                for (int j = 0; j < o->MaxTails; j++)
                {
                    Distance = MoveHumming(o->Position, o->Angle, Position, (float)(rand() % 80 + 60.f));

                    o->Direction[0] = (float)(rand() % 1400 - 700) / o->Scale;
                    o->Direction[2] = (float)(rand() % 1400 - 700) / o->Scale;

                    vec3_t Angle;
                    VectorAdd(o->Angle, o->Direction, Angle);
                    float Matrix[3][4];
                    AngleMatrix(Angle, Matrix);
                    CreateTail(o, Matrix);

                    Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                    vec3_t vTempPos;
                    VectorCopy(p, vTempPos);
                    VectorRotate(vTempPos, Matrix, p);
                    VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
                }
            }
            else
            {
                o->Light[2] -= (10.12f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 1 || o->SubType == 2 || o->SubType == 3 || o->SubType == 5 || o->SubType == 6 || o->SubType == 7) //  위에서 아래로 내려오는 번개.

        {
            VectorCopy(o->StartPosition, o->Position);
            AngleMatrix(o->Angle, Matrix);
            for (int i = 0; i < o->MaxTails - 5; ++i)
            {
                if (o->SubType == 6)
                {
                    o->Position[0] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                    o->Position[2] -= (13.f) * FPS_ANIMATION_FACTOR;
                }
                else
                {
                    o->Position[0] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;

                    if (o->SubType == 7)
                    {
                        o->Position[2] -= (20.0f) * FPS_ANIMATION_FACTOR;
                    }
                    else
                    {
                        o->Position[2] -= (16.f) * FPS_ANIMATION_FACTOR;
                    }
                }
                CreateTail(o, Matrix);
            }
            VectorSubtract(o->TargetPosition, o->Position, Position);
            VectorScale(Position, 0.2f, Position);

            for (int i = o->MaxTails - 5; i < o->MaxTails - 1; ++i)
            {
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                o->Position[0] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);

            if (o->LifeTime < 4)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }

            if (o->SubType == 2)
            {
                Vector(o->Light[0] * 0.5f, o->Light[1] * 0.6f, o->Light[2], Light);

                CreateSprite(BITMAP_SHINY + 1, o->TargetPosition, 1.5f, Light, NULL, (float)((rand() % 360)));
                CreateSprite(BITMAP_SHINY + 1, o->TargetPosition, 1.5f, Light, NULL, (float)((rand() % 360)));
                CreateParticleFpsChecked(BITMAP_TRUE_BLUE, o->TargetPosition, o->Angle, Light);
            }
            else if (o->SubType == 3)
            {
                Vector(o->Light[0] * 0.5f, o->Light[1] * 0.6f, o->Light[2], Light);

                CreateEffect(MODEL_ICE, o->TargetPosition, o->Angle, Light);
            }
            else if (o->SubType == 6)
            {
                o->TargetPosition[2] = RequestTerrainHeight(o->TargetPosition[0], o->TargetPosition[1]) + 30.f;
                CreateParticleFpsChecked(BITMAP_TRUE_BLUE, o->TargetPosition, o->Angle, Light, 0, 2.0f);
            }
        }
        else if (o->SubType == 4)
        {
            if (o->Target != NULL)
            {
                VectorCopy(o->Target->Position, o->Position);
                AngleMatrix(o->Angle, Matrix);

                VectorSubtract(o->TargetPosition, o->Position, Position);
                VectorScale(Position, o->StartPosition[0], Position);

                for (int i = 0; i < o->TargetIndex[0]; ++i)
                {
                    CreateTail(o, Matrix);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                    o->Position[0] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                }

                float width = o->TargetIndex[1] / 2.f;

                VectorSubtract(o->TargetPosition, o->Position, Position);
                VectorScale(Position, o->StartPosition[1], Position);

                for (int i = o->TargetIndex[0]; i < o->TargetIndex[1]; ++i)
                {
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                    CreateTail(o, Matrix);
                }
                VectorCopy(o->TargetPosition, o->Position);
                CreateTail(o, Matrix);

                CreateSprite(BITMAP_SHINY + 1, o->TargetPosition, 1.5f, o->Light, NULL, (float)((rand() % 360)));
                CreateSprite(BITMAP_SHINY + 1, o->TargetPosition, 1.5f, o->Light, NULL, (float)((rand() % 360)));
                if (o->LifeTime < 8)
                {
                    VectorScale(o->Light, powf(1.f / 1.4f, FPS_ANIMATION_FACTOR), o->Light);
                }
            }
            else
            {
                o->Live = false;
            }
        }
        else if (o->SubType == 8)
        {
            VectorCopy(o->StartPosition, o->Position);
            AngleMatrix(o->Angle, Matrix);
            for (int i = 0; i < o->MaxTails - 5; ++i)
            {
                o->Position[0] += (rand() % 10 - 5) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % 10 - 5) * FPS_ANIMATION_FACTOR;
                o->Position[2] -= (20.0f) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorSubtract(o->TargetPosition, o->Position, Position);
            VectorScale(Position, 0.2f, Position);

            for (int i = o->MaxTails - 5; i < o->MaxTails - 1; ++i)
            {
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                o->Position[0] += (rand() % 10 - 5) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % 10 - 5) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);

            if (o->LifeTime < 4)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 9)
        {
            VectorCopy(o->StartPosition, o->Position);
            AngleMatrix(o->Angle, Matrix);
            for (int i = 0; i < o->MaxTails - 5; ++i)
            {
                o->Position[0] += (21.0f) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorSubtract(o->TargetPosition, o->Position, Position);
            VectorScale(Position, 0.2f, Position);

            for (int i = o->MaxTails - 5; i < o->MaxTails - 1; ++i)
            {
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                o->Position[1] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);

            if (o->LifeTime < 4)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 10)
        {
            VectorCopy(o->StartPosition, o->Position);
            AngleMatrix(o->Angle, Matrix);
            for (int i = 0; i < o->MaxTails - 5; ++i)
            {
                o->Position[1] += (20.0f) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorSubtract(o->TargetPosition, o->Position, Position);
            VectorScale(Position, 0.2f, Position);

            for (int i = o->MaxTails - 5; i < o->MaxTails - 1; ++i)
            {
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                o->Position[0] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (rand() % 20 - 10) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);

            if (o->LifeTime < 4)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 11)
        {
            vec3_t p1, p2;
            VectorCopy(o->StartPosition, o->Position);
            Vector(0.f, o->Scale / 1.3f, 0.f, p1);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(p1, Matrix, p2);

            for (int i = 0; i < o->MaxTails - 5; i++)
            {
                VectorAddScaled(o->Position, p2, o->Position, FPS_ANIMATION_FACTOR);
                CreateTail(o, Matrix);
                VectorCopy(o->Position, o->TargetPosition);
            }
            VectorCopy(o->StartPosition, o->Position);

            for (int i = 0; i < o->MaxTails - 5; i++)
            {
                int iScale = 1;
                VectorAddScaled(o->Position, p2, o->Position, FPS_ANIMATION_FACTOR);
                iScale = (int)(o->Scale / 8.0f);
                o->Position[0] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);
        }
        else if (o->SubType == 12)
        {
            VectorCopy(o->StartPosition, o->Position);
            AngleMatrix(o->Angle, Matrix);
            for (int i = 0; i < o->MaxTails - 1; ++i)
            {
                int iScale = 1;
                iScale = (int)(o->Scale / 5.0f);
                o->Position[0] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (rand() % (iScale * 2) - iScale) * FPS_ANIMATION_FACTOR;
                CreateTail(o, Matrix);
                VectorSubtract(o->TargetPosition, o->Position, Position);
                VectorScale(Position, 0.08f, Position);
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
            }
            VectorCopy(o->TargetPosition, o->Position);
            CreateTail(o, Matrix);
        }
        else if (o->SubType == 0
            || o->SubType == 2 || o->SubType == 3 || o->SubType == 4 || o->SubType == 5
            || o->SubType == 6 || o->SubType == 8 || o->SubType == 9 || o->SubType == 10)
        {
            if (o->LifeTime <= 10)
            {
                o->m_bCreateTails = false;
            }
            else
            {
                o->m_bCreateTails = true;
            }
        }
        break;
    case BITMAP_SPARK + 1:
        if (o->SubType == 0)
        {
            o->Direction[2] += (5.f) * FPS_ANIMATION_FACTOR;
            o->Position[0] = o->TargetPosition[0];
            o->Position[1] = o->TargetPosition[1];
            o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
        }
        if (o->SubType == 1)
        {
            o->Velocity += FPS_ANIMATION_FACTOR * 0.1f;
            o->Light[0] *= powf(1.f / 1.1f, FPS_ANIMATION_FACTOR);
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
        }
        else

            if (o->LifeTime < 5)
            {
                VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
            }
        break;
    case BITMAP_FLARE:
    case BITMAP_FLARE_BLUE:
        if (o->SubType == 0 || o->SubType == 10 || o->SubType == 18)
        {
            float count;
            if (o->PKKey != -1)
            {
                count = (o->Direction[1] + o->LifeTime) / (float)(o->PKKey);
            }
            else if (o->SubType == 18)
            {
                count = (o->Direction[1] + (o->LifeTime * 2)) * 0.1f;
                if (o->Skill == 1)
                {
                    count *= powf(-1, FPS_ANIMATION_FACTOR);
                }
            }
            else
            {
                count = (o->Direction[1] + o->LifeTime) * 0.1f;
            }
            o->Position[0] = o->TargetPosition[0] + cosf(count) * o->Velocity;
            o->Position[1] = o->TargetPosition[1] - sinf(count) * o->Velocity;

            if (o->SubType == 0)
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            else if (o->SubType == 10)
                o->Position[2] -= (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            else if (o->SubType == 18)
            {
                o->Position[2] += ((o->Direction[2] * 1.1f)) * FPS_ANIMATION_FACTOR;
                o->Velocity += FPS_ANIMATION_FACTOR * 0.1f;
                o->Scale += (0.2f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 20)
        {
            if (o->Target == NULL)
            {
                o->Live = false;
                break;
            }

            BMD* b = &Models[o->Target->Type];
            vec3_t p;
            Vector(0.0f, 0.0f, 0.0f, p);
            b->TransformPosition(o->Target->BoneTransform[33], p, o->Position, true);
        }
        else if (o->SubType == 14 || o->SubType == 15)
        {
            if (o->Target != NULL)
            {
                float count;

                count = o->Direction[1] * 0.1f;
                if (o->SubType == 15)
                {
                    count *= powf(-1, FPS_ANIMATION_FACTOR);
                }
                o->Position[0] = o->TargetPosition[0] + cosf(count) * o->Velocity;
                o->Position[1] = o->TargetPosition[1] - sinf(count) * o->Velocity;
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
                if (o->LifeTime < 20)
                {
                    o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                }
                o->Scale += (1) * FPS_ANIMATION_FACTOR;
                o->Direction[1] += (4) * FPS_ANIMATION_FACTOR;
                o->Direction[2] += ((rand() % 200 + 200) / 100.f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 2 || o->SubType == 24 || o->SubType == 50 || o->SubType == 51)
        {
            if (o->SubType == 51)
            {
                o->Direction[2] += (10.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
            if (o->LifeTime <= 25 || o->SubType == 50)
            {
                o->Direction[2] += (5.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
        }
#ifdef GUILD_WAR_EVENT
        else if (o->SubType == 21)
        {
            if (o->LifeTime <= 25)
            {
                o->Direction[2] += (5.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 22)
        {
            if (o->LifeTime <= 25)
            {
                o->Direction[2] += (5.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
        }
#endif //GUILD_WAR_EVENT
        else if (o->SubType == 41)
        {
            if (o->LifeTime <= 20)
            {
                o->Direction[2] += (1.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
                VectorScale(o->Light, powf(1.f / 1.1f, FPS_ANIMATION_FACTOR), o->Light);
            }
            AddTerrainLight(o->Position[0], o->Position[1], o->Light, 1, PrimaryTerrainLight);
        }
        else if (o->SubType == 42)
        {
            o->Angle[2] += (15.f) * FPS_ANIMATION_FACTOR;

            if (o->Target->Live)
                o->LifeTime = 100.f;
            else
            {
                DeleteJoint(BITMAP_FLARE, o->Target, 42);
                break;
            }

            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAdd(o->StartPosition, Position, o->Position);

            int tBias = rand() % 100;
            if (((int)o->Angle[2] % 45 == 0) && tBias <= 50)
            {
                vec3_t Angle;
                Vector(0.f, 0.f, 0.f, Angle);
                CreateJointFpsChecked(BITMAP_FLARE, o->Position, o->Position, Angle, 41, NULL, o->Scale);
            }
            AddTerrainLight(o->TargetPosition[0], o->TargetPosition[1], o->Light, 2, PrimaryTerrainLight);
        }
        else if (o->SubType == 19)
        {
            if (o->LifeTime <= 25)
            {
                o->Direction[2] -= (5.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 40)
        {
            static int lMax = o->LifeTime;
            int radious = 50.f;
            vec3_t vPos;
            vec3_t backPos;
            o->m_bCreateTails = false;
            o->NumTails = 0;
            

            VectorCopy(o->Position, backPos);
            for (int i = 0; i < MAX_TAILS; ++i)
            {
                float theta = i * Q_PI / 6;
                float rCos = radious * cosf(theta);
                float rSin = radious * sinf(theta);
                vPos[0] = rCos * cosf((90.f - o->Angle[2]) * Q_PI / 180.f);
                vPos[1] = rCos * sinf((90.f - o->Angle[2]) * Q_PI / 180.f);
                vPos[2] = rSin;
                o->TargetPosition[0] += (o->Direction[0] * (lMax - o->LifeTime)) * FPS_ANIMATION_FACTOR;
                o->TargetPosition[1] += (o->Direction[1] * (lMax - o->LifeTime)) * FPS_ANIMATION_FACTOR;
                VectorAdd(o->TargetPosition, vPos, o->Position);
                AngleMatrix(o->Angle, Matrix);
                CreateTail(o, Matrix);
            }
            VectorCopy(backPos, o->Position);
            o->Position[0] += (o->Direction[0] * 12) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (o->Direction[1] * 12) * FPS_ANIMATION_FACTOR;
            VectorCopy(o->Position, o->TargetPosition);

            o->Scale *= powf(1.1f, FPS_ANIMATION_FACTOR);
            o->Light[0] = 0.5f + (float)(rand() % 64) / 255;
            o->Light[1] = 0.5f + (float)(rand() % 64) / 255;
            o->Light[2] = 0.5f + (float)(rand() % 128) / 255;
        }
        else if (o->SubType == 4 || o->SubType == 6 || o->SubType == 12)
        {
            float count;
            vec3_t vPos;
            if (o->SubType == 6)
            {
                if (o->Type == BITMAP_FLARE_BLUE)
                {
                    count = (o->Direction[0] + (o->LifeTime * 0.8f));
                }
                else
                {
                    count = (o->Direction[0] + (o->LifeTime * 0.5f));
                }
                vPos[0] = -cosf(count) * ((float)max(o->LifeTime, 1) * 2.f);
                vPos[1] = sinf(count) * ((float)max(o->LifeTime, 1) * 2.f);
            }
            else if (o->SubType == 12)
            {
                count = (o->Direction[0] + o->LifeTime) * 0.1f;
                vPos[0] = -cosf(count) * (40.0f * .65f);
                vPos[1] = sinf(count) * (40.0f * .65f) - (float)(90 - o->LifeTime) * 0.3f;
            }
            else
            {
                count = (o->Direction[0] + o->LifeTime) * 0.1f;
                vPos[0] = -cosf(count) * ((float)max(o->LifeTime + 40, 10) * .65f);
                vPos[1] = sinf(count) * ((float)max(o->LifeTime + 40, 10) * .65f);
            }
            vPos[2] = 0.0f;
            o->TargetPosition[0] += (o->Direction[1]) * FPS_ANIMATION_FACTOR;
            o->TargetPosition[1] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            o->Position[0] = vPos[0] * sinf((90.0f - o->Angle[2]) * Q_PI / 180.0f) + o->TargetPosition[0];
            o->Position[1] = vPos[0] * cosf((90.0f - o->Angle[2]) * Q_PI / 180.0f) + o->TargetPosition[1];
            o->Position[2] = vPos[1] + o->TargetPosition[2];
        }
        else if (o->SubType == 5)
        {
            o->Direction[2] -= (60.f) * FPS_ANIMATION_FACTOR;
            o->Position[0] = o->TargetPosition[0];
            o->Position[1] = o->TargetPosition[1];
            o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            o->Scale = rand() % 4 + 6.f;
        }
        else if (o->SubType == 7 || o->SubType == 11 || o->SubType == 25 || o->SubType == 45 || o->SubType == 46 || o->SubType == 47)
        {
            if (o->SubType == 11 || o->SubType == 25)
            {
                if (o->Target->Live == false)
                {
                    o->Live = 0;
                    o->LifeTime = -1;
                    break;
                }
            }

            int iFrame = static_cast<int>(WorldTime / 40.f);

            iFrame = ((iIndex % 2) ? iFrame : -iFrame) + iIndex * 53731;

            vec3_t vDir, vDirTemp;
            //float fSpeed[3] = { 0.097f, 0.1213f, 0.2313f};
            float fSpeed[3] = { 0.048f, 0.0613f, 0.1113f };
            if (o->SubType == 11)
            {
                fSpeed[0] *= 1.5f;
                fSpeed[1] *= 1.5f;
                fSpeed[2] *= 1.5f;
            }
            vDirTemp[0] = sinf((float)(iFrame + 55555) * fSpeed[0]) * cosf((float)iFrame * fSpeed[1]);
            vDirTemp[1] = sinf((float)(iFrame + 55555) * fSpeed[0]) * sinf((float)iFrame * fSpeed[1]);
            vDirTemp[2] = cosf((float)(iFrame + 55555) * fSpeed[0]);

            float fSinAdd = sinf((float)(iFrame + 11111) * fSpeed[2]);
            float fCosAdd = cosf((float)(iFrame + 11111) * fSpeed[2]);
            vDir[2] = vDirTemp[0];
            vDir[1] = fSinAdd * vDirTemp[1] + fCosAdd * vDirTemp[2];
            vDir[0] = fCosAdd * vDirTemp[1] - fSinAdd * vDirTemp[2];

            // VectorScale(vDir, FPS_ANIMATION_FACTOR, vDir);

            float fLife = (float)o->LifeTime * 40.f / 30.f;
            float fPos;

            if (o->SubType == 11 || o->SubType == 25)
            {
                if (fLife < 20.f)
                {
                    fPos = fLife * 4.f;
                }
                else
                {
                    fPos = fLife * 1.0f + 60.f;
                }
            }
            else
            {
                if (fLife < 10.f)
                {	// 끝
                    fPos = fLife * 7.0f;
                }
                else
                {
                    fPos = fLife * 1.0f + 60.f;
                }
            }
            fPos = fPos / (float)(30 + o->MultiUse) * 30.f;
            if ((int)o->LifeTime == 30)
            {
                PlayBuffer(SOUND_METEORITE01);
            }

            float fCircle;
            if (o->SubType == 11 || o->SubType == 25)
            {
                fCircle = min(max(0.f, fLife - 10) * 5.f, 150.f);
            }
            else
            {
                fCircle = min(max(0.f, 40.f - fLife) * 15.f, 150.f);
            }
            o->Position[0] = o->TargetPosition[0] + vDir[0] * fCircle;
            o->Position[1] = o->TargetPosition[1] + vDir[1] * fCircle;
            o->Position[2] = o->TargetPosition[2] + vDir[2] * fCircle;

            float fLastTarget;
            for (int k = 0; k < 3; ++k)
            {
                if (o->SubType == 11 || o->SubType == 25)
                    fLastTarget = (100.f - fPos) * (o->Target->StartPosition[k] + 25.f * cosf((float)(iIndex * 51231 + k * 3711 + iFrame / 10) * 0.01f));
                else
                    fLastTarget = (100.f - fPos) * (o->Target->Position[k] + 25.f * cosf((float)(iIndex * 51231 + k * 3711 + iFrame / 10) * 0.01f));

                o->Position[k] = (fPos * o->Position[k] + fLastTarget) * 0.01f;
            }
            if (o->SubType != 11 && o->SubType != 25)
            {
                o->Position[2] += 100.0f;
            }

            vec3_t Light = { .5f, .5f, 1.0f };

            if (o->SubType == 11)
            {
                Vector(0.3f, 0.3f, 0.5f, Light);
                if (o->Skill == 1)
                {
                    VectorCopy(o->Position, o->Target->Position);
                    CheckClientArrow(o->Target);
                }
            }
            if (o->SubType == 25)
            {
                Vector(0.9f, 0.4f, 0.6f, Light);
                if (o->Skill == 1)
                {
                    VectorCopy(o->Position, o->Target->Position);
                    CheckClientArrow(o->Target);
                }
            }
            if (o->SubType == 45 && o->LifeTime <= 1)
            {
                CreateEffectFpsChecked(MODEL_CHANGE_UP_EFF, o->Target->Position, o->Target->Angle, o->Target->Light, 1, o->Target);
                CreateEffectFpsChecked(MODEL_CHANGE_UP_CYLINDER, o->Target->Position, o->Target->Angle, o->Target->Light, 1, o->Target);
            }
            {
                if (o->SubType != 47)
                {
                    CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 2 + 8) * 0.10f, Light, o->Target, (float)(rand() % 360));
                    CreateSprite(BITMAP_LIGHT, o->Position, (float)(rand() % 2 + 8) * 0.18f, Light, o->Target, (float)(rand() % 360));
                    CreateSprite(BITMAP_LIGHT, o->Position, (float)(rand() % 2 + 8) * 0.18f, Light, o->Target, (float)(rand() % 360));
                }
            }

            if (o->SubType != 11 && 1 == (int)o->LifeTime && o->SubType != 45 && o->SubType != 46 && o->SubType != 47)
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f };
                vec3_t Light = { 1.f, 1.f, 1.f };
                CreateParticleFpsChecked(BITMAP_EXPLOTION, o->Position, Angle, Light, 1, 0.6f);
            }
        }
        else if (o->SubType == 8)
        {
            Vector(0.f, -50.f, 0.f, p);
            AngleMatrix(o->TargetPosition, Matrix);
            VectorRotate(p, Matrix, Position);
            VectorAdd(o->StartPosition, Position, o->Position);
            AngleMatrix(o->Angle, Matrix);

            o->TargetPosition[2] += 10.f;
        }
        else if (o->SubType == 9)
        {
            AngleMatrix(o->Angle, Matrix);

            for (int i = 0; i < o->MaxTails; i++)
            {
                VectorAdd(o->Position, o->StartPosition, o->Position);
                CreateTail(o, Matrix);
            }
        }
        else if (o->SubType == 13)
        {
            if (o->LifeTime <= 25)
            {
                o->Direction[2] += (1.f) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0];
                o->Position[1] = o->TargetPosition[1];
                o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 17)
        {
            if (((o->Target->CurrentAction >= PLAYER_WALK_MALE /*&& o->Target->CurrentAction<=PLAYER_RUN_RIDE_WEAPON*/) ||
                (o->Target->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->Target->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD5)
                && (o->Target->CurrentAction != PLAYER_RAGE_UNI_STOP_ONE_RIGHT)))
            {
                o->LifeTime -= 10 * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Direction[0] += (sinf(rand() % 360 * 0.1f) * 4.f) * FPS_ANIMATION_FACTOR;
                o->Direction[1] += (cosf(rand() % 360 * 0.1f) * 4.f) * FPS_ANIMATION_FACTOR;
                o->Direction[2] += (o->Velocity) * FPS_ANIMATION_FACTOR;
                o->Position[0] = o->TargetPosition[0] + o->Direction[0];
                o->Position[1] = o->TargetPosition[1] + o->Direction[1];
                o->Position[2] = o->TargetPosition[2] + o->Direction[2];
            }

            if (o->LifeTime < 12)
            {
                VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 16 && o->Target != NULL)
        {
            VectorSubtract(o->TargetPosition, o->Target->Position, Position);
            VectorCopy(o->Target->Position, o->TargetPosition);

            for (int j = o->NumTails - 1; j >= 0; j--)
            {
                for (int k = 0; k < 4; k++)
                    VectorSubtract(o->Tails[j][k], Position, o->Tails[j][k]);
            }

            if (o->LifeTime < 20)
            {
                VectorScale(o->Light, powf(1.f / 1.1f, FPS_ANIMATION_FACTOR), o->Light);
            }
            else if (o->LifeTime < 40)
            {
                o->Light[0] = 0.25f;
                o->Light[1] = 0.25f;
                o->Light[2] = 0.25f;
            }
            else
            {
                o->Light[0] *= powf(1.04f, FPS_ANIMATION_FACTOR);
                o->Light[1] *= powf(1.04f, FPS_ANIMATION_FACTOR);
                o->Light[2] *= powf(1.04f, FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 23)
        {
            if (o->LifeTime < 20)
            {
                o->Velocity = 200.f;
                if (o->LifeTime < 8.f)
                {
                    o->Velocity = 0.f;
                    AngleMatrix(o->HeadAngle, Matrix);
                    CreateTail(o, Matrix);
                }
                else
                {
                    for (float i = 0; i < o->Direction[2]; ++i)
                    {
                        o->Direction[1] += (1.5f) * FPS_ANIMATION_FACTOR;
                        switch ((int)o->MultiUse)
                        {
                        case 0: o->HeadAngle[0] += 3.f * FPS_ANIMATION_FACTOR; o->HeadAngle[2] -= o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        case 1: o->HeadAngle[0] -= 3.f * FPS_ANIMATION_FACTOR; o->HeadAngle[2] -= o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        case 2: o->HeadAngle[0] += 3.f * FPS_ANIMATION_FACTOR; o->HeadAngle[2] += o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        case 3: o->HeadAngle[0] -= 3.f * FPS_ANIMATION_FACTOR; o->HeadAngle[2] += o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        case 4: o->HeadAngle[2] -= o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        case 5: o->HeadAngle[0] -= o->Direction[1] * FPS_ANIMATION_FACTOR; break;
                        }
                        if (o->Direction[1] > 15.f)
                        {
                            o->Direction[1] = 15.f;
                        }

                        AngleMatrix(o->HeadAngle, Matrix);
                        Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                        VectorRotate(p, Matrix, Position);
                        VectorAdd(o->StartPosition, Position, o->Position);

                        CreateTail(o, Matrix);

                        if (o->NumTails >= o->MaxTails - 1)
                        {
                            o->Velocity = 0.f;
                            break;
                        }
                    }
                    o->Direction[2] += (4.f) * FPS_ANIMATION_FACTOR;
                }
            }
        }
        else if (o->SubType == 43)
        {
            if (o->Target->Live)
            {
                o->LifeTime = 100.f; //무한

                if (rand_fps_check(3))
                {
                    VectorCopy(o->Target->Position, o->Position);
                    o->Position[2] += (30.f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] += ((rand() % 60 - 30)) * FPS_ANIMATION_FACTOR;
                    o->Position[1] += ((rand() % 60 - 30)) * FPS_ANIMATION_FACTOR;
                    CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 44, o->Target, o->Scale, 0, 0, 0, 0, o->Light);
                }
            }
            else
            {
                DeleteJoint(BITMAP_FLARE, o->Target, 43);
                break;
            }
        }
        else if (o->SubType == 44)
        {
            if (o->Target->Live == false)
            {
                o->Live = false;
                break;
            }
            else
            {
                if (o->LifeTime <= 20)
                {
                    o->Direction[2] += (1.0f) * FPS_ANIMATION_FACTOR;
                    o->Position[0] = o->TargetPosition[0];
                    o->Position[1] = o->TargetPosition[1];
                    o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
                    VectorScale(o->Light, powf(1.f / 1.05f, FPS_ANIMATION_FACTOR), o->Light);

                    CreateSprite(BITMAP_PIN_LIGHT, o->Position, 1.5f, o->Light, NULL);
                    CreateSprite(BITMAP_PIN_LIGHT, o->Position, 0.5f, o->Light, NULL);

                    if (o->Position[2] >= 400.f)
                    {
                        o->Live = false;
                    }
                }
            }
        }

        if (o->SubType != 5 && o->SubType != 7 && o->SubType != 11 || (o->SubType == 14 || o->SubType == 15 || o->SubType == 16))
        {
            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }

        if ((o->SubType == 0 || o->SubType == 10 || o->SubType == 18) && o->Scale < 40.f)
        {
            if (((o->Target->CurrentAction >= PLAYER_WALK_MALE && o->Target->CurrentAction <= PLAYER_RUN_RIDE_WEAPON) ||
                (o->Target->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->Target->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD5)
                || (o->Target->CurrentAction == PLAYER_RAGE_UNI_RUN || o->Target->CurrentAction == PLAYER_RAGE_UNI_RUN_ONE_RIGHT)))
            {
                o->SubType = 1;
                o->m_bCreateTails = false;
                o->LifeTime = min(10, o->LifeTime);
            }
        }
        break;
    case BITMAP_FLARE + 1:
        if (o->SubType == 5)
        {
            if (o->LifeTime >= 25)
            {
                o->Angle[2] -= (10.f) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (2.5f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                if (o->Velocity >= 30.f)
                    o->Velocity = 30.f;

                float fOldAngle = o->Angle[2];
                VectorCopy(o->TargetPosition, Position);
                Distance = MoveHumming(o->Position, o->Angle, Position, o->Velocity);

                if (Distance <= 70.0f && fabs(fOldAngle - o->Angle[2]) > 20.0f)
                {
                    if (o->Velocity >= 20.f)
                        o->Velocity -= FPS_ANIMATION_FACTOR * 5.f;
                }

                if (o->LifeTime < 10)
                {
                    o->Light[0] *= powf(1.0f / 1.2f, FPS_ANIMATION_FACTOR);
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                }
            }
            Luminosity = (float)(rand() % 4 + 8) * 0.03f;
            Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 0.4f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        }
        else if (o->SubType == 16)
        {
            if (o->LifeTime >= 25)
            {
                o->Angle[2] -= (10.f) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (2.5f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                if (o->Velocity >= 30.f)
                    o->Velocity = 30.f;

                float fOldAngle = o->Angle[2];
                VectorCopy(o->Target->Position, Position);
                //				VectorCopy(o->TargetPosition,Position);
                Distance = MoveHumming(o->Position, o->Angle, Position, o->Velocity);

                if (Distance <= 70.0f && fabs(fOldAngle - o->Angle[2]) > 20.0f)
                {
                    if (o->Velocity >= 20.f)
                        o->Velocity -= FPS_ANIMATION_FACTOR * 10.f;
                }

                if (o->LifeTime < 10)
                {
                    o->Light[0] *= powf(1.0f / 1.2f, FPS_ANIMATION_FACTOR);
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                }
            }
            Luminosity = 1.f;
            Vector(Luminosity * 1.0f, Luminosity * 1.0f, Luminosity * 1.0f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);
        }
        else if (o->SubType == 6)
        {
            if (o->Target != NULL)
            {
                if (o->LifeTime >= 30)
                {
                    o->Angle[2] -= (10.f) * FPS_ANIMATION_FACTOR;
                    o->Position[2] += (2.5f) * FPS_ANIMATION_FACTOR;
                }

                o->Velocity += FPS_ANIMATION_FACTOR * 5.f;
                if (o->Velocity >= 30.f)
                    o->Velocity = 30.f;

                float fOldAngle = o->Angle[2];
                VectorCopy(o->Target->Position, Position);
                Position[2] += (150.f) * FPS_ANIMATION_FACTOR;
                Distance = MoveHumming(o->Position, o->Angle, Position, o->Velocity);

                //박종훈테스트
                //					CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 34, 1.0f);
                //					CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 35, 1.0f);
                //박종훈테스트
                if (Distance <= 70.0f && fabs(fOldAngle - o->Angle[2]) > 20.0f)
                {
                    if (o->Velocity >= 20.f)
                        o->Velocity -= FPS_ANIMATION_FACTOR * 10.f;
                }

                if (o->LifeTime < 15)
                {
                    o->Light[0] *= powf(1.0f / 1.2f, FPS_ANIMATION_FACTOR);
                    o->Light[1] = o->Light[0];
                    o->Light[2] = o->Light[0];
                }
                Luminosity = (float)(rand() % 4 + 8) * 0.03f;
                Vector(Luminosity * 0.4f, Luminosity, Luminosity * 0.8f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            }
            else
            {
                o->Live = false;
            }
        }
        else if (o->SubType == 7)
        {
            o->Position[0] = o->StartPosition[0] + sinf(o->LifeTime / 2.5f) * o->Direction[0];
            o->Position[1] = o->StartPosition[1];
            o->Position[2] += o->Velocity * FPS_ANIMATION_FACTOR;
            o->Velocity -= FPS_ANIMATION_FACTOR * 0.1f;
            o->Direction[0] -= 0.1f * FPS_ANIMATION_FACTOR;

            if (o->LifeTime < 15)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 8 || o->SubType == 9)
        {
            if (o->Target == NULL)
            {
                o->Live = false;
                break;
            }
            o->Live = o->Target->Live;
            VectorCopy(o->Target->Position, o->Position);
            //            Vector ( 0.f, 0.f, o->Target->Angle[2], o->Angle );

            if (o->SubType == 8)
            {
                o->Position[2] = 300.f;
            }
            else if (o->SubType == 9)
            {
                o->Position[2] += (o->MultiUse) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 12 || o->SubType == 13 || o->SubType == 14 || o->SubType == 17 || o->SubType == 15 || o->SubType == 18)
        {
            if (o->Target != NULL)
            {
                if (o->Target->Live == false)
                {
                    o->Live = false;
                }
                if (o->SubType == 17 && o->Live == true)
                {
                    Vector(1.0f, 1.0f, 0.2f, o->Light);
                }
                if (o->SubType == 14 && o->Live == true)
                {
                    CreateSprite(BITMAP_SPARK + 1, o->Position, 5.0f, o->Light, NULL);

                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 34, 1.0f);
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 35, 1.0f);
                    //CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, o->Light, 12, 1.0f);
                    Vector(0.7f, 0.3f, 0.3f, o->Light);
                    CreateParticleFpsChecked(BITMAP_SPARK + 1, o->Position, o->Angle, o->Light, 10, 4.0f);
                    Vector(1.0f, 1.0f, 0.2f, o->Light);
                }
                else if (o->SubType == 15 && o->Live == true)
                {
                    CreateParticleFpsChecked(BITMAP_SHINY, o->Position, o->Angle, o->Light, 3, 0.5f);
                    CreateParticleFpsChecked(BITMAP_SHINY, o->Position, o->Angle, o->Light, 3, 0.5f);
                }

                if (o->LifeTime < 15)
                {
                    VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
                }

                if (o->SubType != 18)
                {
                    Luminosity = (float)(rand() % 4 + 8) * 0.03f;
                    Vector(Luminosity * 0.4f, Luminosity, Luminosity * 0.8f, Light);
                    AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
                }
            }
            else
            {
                o->Live = false;
            }
        }
        else if (o->SubType == 19)
        {
            vec3_t  pos;
            float   Mat[3][4];

            vec3_t  Angle;
            VectorSubtract(o->Target->Position, o->Target->StartPosition, pos);
            VectorCopy(o->Target->StartPosition, o->TargetPosition);
            //VectorCopy( o->Target->HeadAngle, Angle );
            AngleMatrix(o->Angle, Mat);

            pos[0] /= 3.f;
            pos[1] /= 3.f;
            pos[2] /= 3.f;
            //Angle[1] += (o->Velocity - 90) * FPS_ANIMATION_FACTOR;

            for (int j = 0; j < 3; j++)
            {
                Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;

                VectorAdd(o->TargetPosition, pos, o->TargetPosition);

                vec3_t  position;
                AngleMatrix(Angle, Matrix);
                VectorRotate(o->Direction, Matrix, position);
                VectorAdd(o->TargetPosition, position, o->Position);

                if ((int)o->NumTails < (o->MaxTails - 1) || o->Skill != 0)
                    CreateTail(o, Mat);
            }
        }
        else if (o->SubType == 20)
        {
            vec3_t  pos;
            float   Mat[3][4];

            vec3_t  Angle;
            VectorSubtract(o->Target->Position, o->Target->StartPosition, pos);
            VectorCopy(o->Target->StartPosition, o->TargetPosition);
            //VectorCopy( o->Target->HeadAngle, Angle );
            AngleMatrix(o->Angle, Mat);

            pos[0] /= 3.f;
            pos[1] /= 3.f;
            pos[2] /= 3.f;
            //Angle[1] += (o->Velocity - 90) * FPS_ANIMATION_FACTOR;

            for (int j = 0; j < 3; j++)
            {
                VectorAdd(o->TargetPosition, pos, o->TargetPosition);

                vec3_t  position;
                AngleMatrix(Angle, Matrix);
                VectorRotate(o->Direction, Matrix, position);
                VectorAdd(o->TargetPosition, position, o->Position);

                if ((int)o->NumTails < (o->MaxTails - 1) || o->Skill != 0)
                    CreateTail(o, Mat);
            }
        }
        else
        {
            vec3_t  pos;
            float   Mat[3][4];
            if (o->Target->Live)
            {
                vec3_t  Angle;
                VectorSubtract(o->Target->Position, o->Target->StartPosition, pos);
                VectorCopy(o->Target->StartPosition, o->TargetPosition);
                VectorCopy(o->Target->HeadAngle, Angle);
                AngleMatrix(o->Angle, Mat);

                pos[0] /= 3.f; pos[1] /= 3.f; pos[2] /= 3.f;
                Angle[1] += (o->Velocity - 90) * FPS_ANIMATION_FACTOR;

                for (int j = 0; j < 3; j++)
                {
                    Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;
                    VectorAdd(o->TargetPosition, pos, o->TargetPosition);

                    vec3_t  position;
                    AngleMatrix(Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, position);
                    VectorAdd(o->TargetPosition, position, o->Position);

                    if ((int)o->NumTails < (o->MaxTails - 1) || o->Skill != 0)
                        CreateTail(o, Mat);
                }

                if (o->LifeTime < 15)
                {
                    VectorScale(o->Light, powf(1.f / 1.5f, FPS_ANIMATION_FACTOR), o->Light);
                }
            }
            else
            {
                o->LifeTime = -1;
                VectorCopy(o->TargetPosition, o->Position);
                o->Position[2] += (50.f) * FPS_ANIMATION_FACTOR;
            }
        }
        break;

    case BITMAP_JOINT_FORCE:
        if (o->SubType == 10 || o->SubType == 8)
        {
            Vector(0.f, -145.f, 0.f, p);

            for (int j = 0; j < 8; ++j)
            {
                if ((int)o->NumTails < o->MaxTails - 1)
                {
                    VectorCopy(o->Position, Light);
                    AngleMatrix(o->Direction, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->TargetPosition, Position, o->Position);
                    AngleMatrix(o->Angle, Matrix);
                    CreateTail(o, Matrix);

                    o->Direction[2] -= (11.f) * FPS_ANIMATION_FACTOR;
                    if (o->SubType == 10)
                    {
                        if (rand_fps_check(2))
                        {
                            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, o->Light, 0);
                        }

                        CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Light, o->Position, o->Angle, 3, NULL, rand() % 10 + 5.f, 5, 10);
                        CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Light, o->Position, o->Angle, 3, NULL, rand() % 8 + 4.f, 5, 10);
                    }
                    else
                        if (o->SubType == 8)
                        {
                            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 3, NULL, rand() % 10 + 5.f, 5, 10);
                            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 3, NULL, rand() % 8 + 4.f, 5, 10);
                        }
                }
                if (o->SubType == 10)
                {
                    if (o->Target == &Hero->Object && o->LifeTime > 18)
                    {
                        if ((j % 5) == 0)
                        {
                            if (battleCastle::IsBattleCastleStart())
                            {
                                DWORD att = TERRAIN_ATTRIBUTE(o->Position[0], o->Position[1]);
                                if ((att & TW_NOATTACKZONE) == TW_NOATTACKZONE)
                                {
                                    o->Velocity = 0.f;
                                    o->LifeTime *= powf(1.f / 5.f, FPS_ANIMATION_FACTOR);
                                    break;
                                }
                            }
                            AttackCharacterRange(o->Skill, o->Position, 225.f, o->Weapon, o->PKKey);
                        }
                    }
                }
            }
            if (o->SubType == 10)
            {
                if (o->m_bySkillSerialNum == 0)
                {
                    if (o->LifeTime < 10)
                        Luminosity = o->Light[0] / 1.5f;
                    else
                        Luminosity = o->LifeTime / 30.f;
                    Vector(Luminosity, Luminosity, Luminosity, o->Light);
                }
                else
                {
                    if (o->LifeTime < 10)
                    {
                        o->Light[0] = o->Light[0] / 1.5f;
                        o->Light[1] = o->Light[1] / 1.5f;
                        o->Light[2] = o->Light[2] / 1.5f;
                    }
                }
            }
        }
        else
            if (o->SubType == 0)
            {
                Vector(0.f, -145.f, 0.f, p);

                for (int j = 0; j < 8; ++j)
                {
                    if ((int)o->NumTails < o->MaxTails - 1)
                    {
                        VectorCopy(o->Position, Light);
                        AngleMatrix(o->Direction, Matrix);
                        VectorRotate(p, Matrix, Position);
                        VectorAdd(o->TargetPosition, Position, o->Position);
                        AngleMatrix(o->Angle, Matrix);
                        CreateTail(o, Matrix);

                        o->Direction[2] -= (11.f) * FPS_ANIMATION_FACTOR;
                        if (o->SubType == 0)
                        {
                            if (rand_fps_check(2))
                            {
                                CreateParticle(BITMAP_FIRE, o->Position, o->Angle, o->Light, 0);
                            }

                            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Light, o->Position, o->Angle, 3, NULL, rand() % 10 + 5.f, 5, 10); //  전기
                            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Light, o->Position, o->Angle, 3, NULL, rand() % 8 + 4.f, 5, 10); //  전기
                        }
                    }
                    if (o->SubType == 0)
                    {
#ifndef SV_RANGE_ATTACK_CHECK
                        if (o->Target == &Hero->Object && o->LifeTime > 18)
                        {
                            if ((j % 5) == 0)
                            {
                                if (battleCastle::IsBattleCastleStart())
                                {
                                    DWORD att = TERRAIN_ATTRIBUTE(o->Position[0], o->Position[1]);
                                    if ((att & TW_NOATTACKZONE) == TW_NOATTACKZONE)
                                    {
                                        o->Velocity = 0.f;
                                        o->LifeTime *= powf(1.f / 5.f, FPS_ANIMATION_FACTOR);
                                        break;
                                    }
                                }
                                AttackCharacterRange(o->Skill, o->Position, 150.f, o->Weapon, o->PKKey);
                            }
                        }
#endif // SV_RANGE_ATTACK_CHECK
                    }
                }
                if (o->LifeTime < 10)
                    Luminosity = o->Light[0] / 1.5f;
                else
                    Luminosity = o->LifeTime / 30.f;
                Vector(Luminosity, Luminosity, Luminosity, o->Light);
            }
            else if (o->SubType == 1)
            {
                if (o->Target == NULL)
                {
                    o->Live = false;
                    break;
                }
                o->Live = o->Target->Live;
                VectorCopy(o->Target->Position, o->Position);

                o->Scale -= (1.f) * FPS_ANIMATION_FACTOR;
                o->Position[2] += (100.f) * FPS_ANIMATION_FACTOR;

                if (o->LifeTime < 15)
                {
                    VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
                }
            }
            else if (o->SubType >= 2 && o->SubType <= 6)
            {
                o->Velocity += FPS_ANIMATION_FACTOR * o->Direction[2];
                if (o->LifeTime < 15)
                {
                    o->Direction[2] += (0.5f) * FPS_ANIMATION_FACTOR;
                }
                else
                {
                    o->Direction[2] += (o->Direction[0]) * FPS_ANIMATION_FACTOR;
                }
                if (o->SubType != 4)
                {
                    if (o->NumTails >= o->MaxTails - 1)
                    {
                        o->m_bCreateTails = false;
                    }
                }
                if (o->LifeTime < o->MultiUse)
                {
                    VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
                }
                if ((int)o->LifeTime % 5 == 0 && o->SubType == 2)
                {
                    CreateEffectFpsChecked(MODEL_SKILL_INFERNO, o->StartPosition, o->HeadAngle, o->Light, 6, NULL, 20, 0);
                }
                else if (o->SubType == 4)
                {
                    Vector(0.1f, 0.6f, 1.f, Light);

                    CreateJointFpsChecked(BITMAP_FLARE_BLUE, o->Position, o->Position, o->Angle, 6, NULL, 30.0f);
                    CreateSprite(BITMAP_LIGHT, o->Position, 1.6f, Light, NULL);
                    CreateSprite(BITMAP_SHINY + 1, o->Position, 1.5f, Light, NULL, WorldTime * 0.1f);
                }
            }
            else if (o->SubType == 7 || o->SubType == 20)
            {
                o->Velocity += FPS_ANIMATION_FACTOR * o->Direction[2];
                if (o->LifeTime < 20)
                {
                    o->Direction[2] += (0.5f) * FPS_ANIMATION_FACTOR;
                }
                else
                {
                    o->Direction[2] += (o->Direction[0]) * FPS_ANIMATION_FACTOR;
                }
                if (o->NumTails >= o->MaxTails - 1)
                {
                    o->m_bCreateTails = false;
                }
                if (o->LifeTime < o->MultiUse)
                {
                    VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
                }
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 3.f;
            }
        break;

    case BITMAP_LIGHT:
        if (o->SubType == 0)
        {
            if (o->LifeTime > 16)
            {
                AngleMatrix(o->Angle, Matrix);

                for (int i = 0; i < 10; ++i)
                {
                    CreateTail(o, Matrix);

                    Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                    vec3_t vPos;
                    VectorRotate(p, Matrix, vPos);
                    VectorAdd(o->Position, vPos, o->Position);
                }
            }
        }
        else if (o->SubType == 1)
        {
            if (o->Skill == 0 || o->LifeTime > 3)
            {
                AngleMatrix(o->Angle, Matrix);

                for (int i = 0; i < 5; ++i)
                {
                    CreateTail(o, Matrix);

                    Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                    vec3_t vPos;
                    VectorRotate(p, Matrix, vPos);
                    VectorAdd(o->Position, vPos, o->Position);
                    o->Velocity += FPS_ANIMATION_FACTOR * 0.5f;
                }
            }
        }

        VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
        break;
    case BITMAP_PIERCING:
        if (o->SubType == 0)
        {
            if (o->LifeTime > 9)
            {
                AngleMatrix(o->Angle, Matrix);

                for (int i = 0; i < 30; ++i)
                {
                    CreateTail(o, Matrix);

                    Vector(0.f, 0.f, -o->Velocity * FPS_ANIMATION_FACTOR, p);
                    VectorRotate(p, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                }
            }
            o->Velocity -= FPS_ANIMATION_FACTOR * 2.f;

            VectorScale(o->Light, powf(1.f / 1.4f, FPS_ANIMATION_FACTOR), o->Light);
        }
        else if (o->SubType == 1)
        {
            if (o->LifeTime > 9)
            {
                AngleMatrix(o->Angle, Matrix);

                for (int i = 0; i < 30; ++i)
                {
                    CreateTail(o, Matrix);

                    Vector(0.f, 0.f, -o->Velocity * FPS_ANIMATION_FACTOR, p);
                    VectorRotate(p, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
                }
            }
            o->Velocity -= FPS_ANIMATION_FACTOR * 2.f;

            VectorScale(o->Light, powf(1.f / 1.4f, FPS_ANIMATION_FACTOR), o->Light);
        }
        break;
    case BITMAP_FLARE_FORCE:
        if (o->SubType == 5 || o->SubType == 6 || o->SubType == 7)
        {
            if (o->Target != NULL)
            {
                vec3_t Direction, Angle;

                Vector(0.f, 20.f, 0.f, p);
                VectorCopy(o->Direction, Direction);
                BMD* b = &Models[o->Target->Type];
                VectorCopy(o->Target->Position, b->BodyOrigin);
                b->TransformPosition(o->Target->BoneTransform[(int)o->MultiUse], p, o->StartPosition, true);
                VectorCopy(o->StartPosition, o->Position);

                o->NumTails = 0;
                int MaxTails = min(o->MaxTails, o->Weapon);
                o->MaxTails = MaxTails;
                o->TargetPosition[1] = o->TargetPosition[2];

                for (int i = 0; i < MaxTails; ++i)
                {
                    VectorRotate(Direction, o->Target->BoneTransform[(int)o->MultiUse], Position);
                    VectorAddScaled(o->StartPosition, Position, o->StartPosition, FPS_ANIMATION_FACTOR);
                    // VectorAdd(o->StartPosition, Position, o->StartPosition);

                    if (o->SubType % 2)
                    {
                        o->TargetPosition[1] += (40.f);
                    }
                    else
                    {
                        o->TargetPosition[1] -= (40.f);
                    }

                    Vector(o->TargetPosition[1], 0.f, 0.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    Vector(0.f, 0.f, o->TargetPosition[0], p);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->Position);
                    // VectorAddScaled(o->StartPosition, Position, o->Position, FPS_ANIMATION_FACTOR);

                    CreateTail(o, o->Target->BoneTransform[0]);//(int)o->MultiUse]);

                    o->TargetPosition[0] -= (0.15f) * FPS_ANIMATION_FACTOR;

                    if (o->PKKey == -1)
                        Direction[1] -= (0.1f) * FPS_ANIMATION_FACTOR;

                    if ((i % 2) == 0)
                    {
                        CreateSprite(BITMAP_FLARE, o->StartPosition, o->Light[0] / 2.f, o->Light, NULL);
                    }
                }
                o->MaxTails++;
            }
            else
            {
                o->Live = false;
            }
            if (o->LifeTime < 7)
            {
                VectorScale(o->Light, powf(1.f / 1.5f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->NumTails < o->MaxTails - 1)
        {
            if (o->SubType == 0)
            {
                for (int i = 1; i < o->MultiUse; ++i)
                {
                    AngleMatrix(o->Angle, Matrix);

                    CreateTail(o, Matrix);
                    Vector(0.f, o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(o->StartPosition, Position, o->StartPosition);

                    o->Position[0] = o->StartPosition[0];
                    o->Position[1] = o->StartPosition[1];
                    o->Position[2] = o->StartPosition[2];
                    o->Velocity -= FPS_ANIMATION_FACTOR * 2.f;
                }
                o->MultiUse += (2) * FPS_ANIMATION_FACTOR;
            }
            else if (o->SubType >= 1 && o->SubType <= 4
                || (o->SubType >= 11 && o->SubType <= 13)
                )
            {
                if (o->Weapon <= 0)
                {
                    vec3_t Angle;
                    for (int i = 1; i < o->MultiUse; ++i)
                    {
                        AngleMatrix(o->Angle, Matrix);
                        CreateTail(o, Matrix);
                        Vector(0.f, o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
                        VectorRotate(p, Matrix, Position);
                        VectorAdd(o->StartPosition, Position, o->StartPosition);

                        if (o->SubType == 2 || o->SubType == 4)
                        {
                            o->TargetPosition[1] -= (20.f) * FPS_ANIMATION_FACTOR;
                        }
                        else if (o->SubType >= 11 || o->SubType <= 13)
                        {
                            o->TargetPosition[1] -= (20.f) * FPS_ANIMATION_FACTOR;
                        }
                        else
                        {
                            o->TargetPosition[1] += (20.f) * FPS_ANIMATION_FACTOR;
                        }
                        Vector(0.f, o->TargetPosition[1], o->Angle[2], Angle);
                        AngleMatrix(Angle, Matrix);
                        Vector(0.f, 0.f, o->TargetPosition[0], p);
                        VectorRotate(p, Matrix, Position);

                        if (o->SubType == 3 || o->SubType == 4)
                        {
                            VectorSubtract(o->StartPosition, Position, Position);
                        }
                        else
                        {
                            VectorAdd(o->StartPosition, Position, Position);
                        }

                        o->Position[0] = Position[0];
                        o->Position[1] = Position[1];
                        o->Position[2] = Position[2];
                        o->Velocity -= FPS_ANIMATION_FACTOR * 2.f;
                        o->TargetPosition[0] -= (2.5f) * FPS_ANIMATION_FACTOR;
                    }
                    o->MultiUse += (2) * FPS_ANIMATION_FACTOR;
                }
                else
                {
                    o->Weapon -= FPS_ANIMATION_FACTOR;
                }
            }
        }

        if ((o->SubType >= 0 && o->SubType <= 4
            || (o->SubType >= 11 && o->SubType <= 13)
            ) && o->LifeTime < 10)
        {
            VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
        }
        break;

    case BITMAP_FLASH:
        if (o->SubType <= 3 || o->SubType == 5)
        {
            if (o->SubType == 2)
            {
                o->Angle[2] += (rand() % 10 + 10.f) * FPS_ANIMATION_FACTOR;
            }

            o->Velocity += FPS_ANIMATION_FACTOR * 10.f;
            if (o->m_bCreateTails)
            {
                AngleMatrix(o->Angle, Matrix);
                CreateTail(o, Matrix);
            }

            o->Velocity += FPS_ANIMATION_FACTOR * 10.f;
            AngleMatrix(o->HeadAngle, Matrix);
            Vector(0.f, -o->Velocity * FPS_ANIMATION_FACTOR, 0.f, p);
            VectorRotate(p, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]) - o->MultiUse;
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height;
                if (o->m_bCreateTails && o->PKKey != 1)
                {
                    if (o->SubType < 2)
                    {
                        if (o->Target != NULL)
                        {
                            BMD* b = &Models[MODEL_SHADOW_BODY];
                            b->Animation(BoneTransform, 0.f, 0.f, 0, o->Target->Angle, o->Target->HeadAngle, false, true);
                            b->Transform(BoneTransform, o->Target->BoundingBoxMin, o->Target->BoundingBoxMax, &o->Target->OBB, false);

                            if (o->SubType == 0)
                            {
                                b->RenderMeshEffect(0, MODEL_SKIN_SHELL);
                            }
                            else if (o->SubType == 1)
                            {
                                b->RenderMeshEffect(0, MODEL_SKIN_SHELL, 1);
                            }
                        }
                    }
                    else
                    {
                        if (o->SubType == 5)
                        {
                            Vector(1.f, 0.8f, 0.3f, Light);
                        }
                        else
                        {
                            Vector(0.3f, 0.8f, 1.f, Light);
                        }
                        CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 4);
                    }
                }
                if (o->SubType == 5)
                {
                    o->PKKey = 1;
                    o->Velocity = -20.f;
                }
                else
                {
                    o->m_bCreateTails = false;
                }

                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
            }
            if (o->SubType < 2)
            {
                CreateSprite(BITMAP_FLARE, o->Position, 1.f, o->Light, NULL, (float)(rand() % 360));
            }
            else if (o->SubType == 5)
            {
                CreateSprite(BITMAP_FLARE, o->Position, 2.5f, o->Light, NULL, (float)(rand() % 360));
            }
            else
            {
                Vector(0.f, 1.f, 1.f, Light);
                CreateSprite(BITMAP_SHINY + 2, o->Position, 1.6f, Light, NULL, (float)(rand() % 360));
            }
        }
        else if (o->SubType == 4)
        {
            if (o->Target == NULL)
            {
                o->Live = false;
                break;
            }
            o->Live = o->Target->Live;
            VectorCopy(o->Target->Position, o->Position);

            if (o->LifeTime < 15)
            {
                VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        else if (o->SubType == 6)
        {
            int fr = rand() % 5 - 2;
            //			o->Direction[0] += (fr) * FPS_ANIMATION_FACTOR;
            o->Direction[1] += (fr) * FPS_ANIMATION_FACTOR;
            o->Direction[2] -= (10.f) * FPS_ANIMATION_FACTOR;

            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

            if ((o->Weapon - o->LifeTime) < 10)
            {
                CreateParticleFpsChecked(BITMAP_EXPLOTION + 1, o->StartPosition, o->Angle, o->Light, 0, 3.f);
                o->Scale = 100.f;
            }
            else
            {
                o->Scale = 40.f;
            }

            float height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] <= height)
            {
                vec3_t tPos;
                VectorCopy(o->Position, tPos);
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 4, NULL, 60.f);
                Vector(1.f, 1.f, 1.f, Light);
                if (o->Direction[2] <= 0) o->Direction[2] = sinf(o->Velocity) * 20.f;
                o->Position[2] = height + 5.f;

                tPos[2] = o->Position[2] + 30.f;
                CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, tPos, o->Angle, o->Light, 2, 3.f);
            }

            CreateTailAxis(o, Matrix, 1);

            if (o->LifeTime < (o->Weapon / 2))
            {
                VectorScale(o->Light, powf(1.f / 1.3f, FPS_ANIMATION_FACTOR), o->Light);
            }
        }
        break;
    case BITMAP_DRAIN_LIFE_GHOST:
    {
        switch (o->SubType)
        {
        case 0:
        {
            OBJECT* pSourceObj = o->Target;
            OBJECT* pTargetObj = pSourceObj->Owner;
            vec3_t vTargetPos;
            VectorCopy(pTargetObj->Position, vTargetPos);
            vTargetPos[2] = o->TargetPosition[2];

            Distance = MoveHumming(o->Position, o->Angle, vTargetPos, o->Velocity);

            o->Velocity += FPS_ANIMATION_FACTOR * 2.f;

            if (o->LifeTime < 10)
            {
                VectorScale(o->Light, powf(1.f / 1.2f, FPS_ANIMATION_FACTOR), o->Light);
                VectorCopy(o->Light, Light);
            }

            VectorCopy(o->Light, Light);
            float Scale = 2.f * (o->LifeTime / 10) + 2.f;

            //CreateSprite(BITMAP_LIGHT,o->Position,Scale,Light,o->Target,(float)(rand()%360),0);
            //CreateSprite(BITMAP_SHINY+1,o->Position,Scale/2.f,Light,o->Target,(float)(rand()%360),0);
        }
        break;
        }
    }break;
    case BITMAP_PIN_LIGHT:
    {
        o->Velocity += FPS_ANIMATION_FACTOR * 0.1f;
        VectorScale(o->Light, powf(0.9f, FPS_ANIMATION_FACTOR), o->Light);
    }
    break;
    case BITMAP_FORCEPILLAR:
    {
        if (!o->Target->Live || (o->Target->Type != MODEL_DOWN_ATTACK_DUMMY_L && o->Target->Type != MODEL_DOWN_ATTACK_DUMMY_R))
        {
            o->Live = false;
            return;
        }
        Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame, o->Target->PriorAnimationFrame,
            o->Target->PriorAction, o->Target->Angle, o->Target->HeadAngle, false, false);

        Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 0);
    }
    break;
    case BITMAP_SWORDEFF:
    {
        if (!o->Target->Live)
        {
            o->Live = false;
            return;
        }
        Models[o->Target->Type].Animation(BoneTransform, o->Target->AnimationFrame, o->Target->PriorAnimationFrame,
            o->Target->PriorAction, o->Target->Angle, o->Target->HeadAngle, false, false);

        Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 5);
    }
    break;
    case BITMAP_GROUND_WIND:
    {
        if (!o->Target->Live || (o->Target->Type != MODEL_DRAGON_KICK_DUMMY))
        {
            o->Live = false;
            return;
        }
        float TargetAniFrame = o->Target->AnimationFrame + (o->SubType * o->Target->AlphaTarget);
        Models[o->Target->Type].Animation(BoneTransform, TargetAniFrame, o->Target->PriorAnimationFrame,
            o->Target->PriorAction, o->Target->Angle, o->Target->HeadAngle, false, false);

        Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 2);

        o->Position[2] += (20.0f) * FPS_ANIMATION_FACTOR;
        VectorScale(o->Light, powf(0.7f, FPS_ANIMATION_FACTOR), o->Light);
    }
    break;
    case BITMAP_LAVA:
    {
        if (!o->Target->Live)
        {
            o->Live = false;
            return;
        }
        int TempFrame = 0;
        TempFrame = o->SubType;
        if (o->SubType >= 7)
            TempFrame -= 7;

        float TargetAniFrame = o->Target->AnimationFrame - (((o->Velocity * 10.0f) - TempFrame) * 0.1f);
        Models[o->Target->Type].Animation(BoneTransform, TargetAniFrame, o->Target->PriorAnimationFrame,
            o->Target->PriorAction, o->Target->Angle, o->Target->HeadAngle, false, false);

        if (o->SubType >= 7)
        {
            Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 28);
        }
        else
        {
            Models[o->Target->Type].TransformByObjectBone(o->Position, o->Target, 36);
        }
        VectorScale(o->Light, powf(0.8f, FPS_ANIMATION_FACTOR), o->Light);
    }
    break;
    }

    if (o->m_bCreateTails)
    {
        if (o->Type == BITMAP_JOINT_ENERGY && o->SubType == 54)
            CreateTail(o, Matrix, true);
        else
            CreateTail(o, Matrix);
    }

    o->LifeTime -= FPS_ANIMATION_FACTOR;
    if (o->LifeTime < 0)
    {
        o->Live = false;
    }
    else if (0 != (((int)o->LifeTime) % 12))
    {
        switch (o->Type)
        {
        case BITMAP_FLARE:
        case BITMAP_FLARE_BLUE:
            if (o->SubType == 4 || o->SubType == 12)
            {
                MoveJoint(o, iIndex);
            }
            break;
        }
    }
}

void MoveJoints()
{
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        JOINT* o = &Joints[i];
        if (o->Live)
        {
            MoveJoint(o, i);
        }
    }
}

void RenderJoints(BYTE bRenderOneMore)
{
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        JOINT* o = &Joints[i];
        if (o->Type == BITMAP_JOINT_ENERGY && o->SubType == 54 && o->Target->CurrentAction != MONSTER01_ATTACK1)
            continue;
        if (o->Live && o->NumTails > 0 && o->RenderFace != 0)
        {
            if (bRenderOneMore == 1 && o->byOnlyOneRender == 2) continue;
            else if (bRenderOneMore == 2 && o->byOnlyOneRender == 1) continue;

            switch (o->RenderType)
            {
            case RENDER_TYPE_ALPHA_BLEND:
                EnableAlphaBlend();
                break;

            case RENDER_TYPE_ALPHA_TEST:
                EnableAlphaTest();
                break;

            case RENDER_TYPE_ALPHA_BLEND_MINUS:
                EnableAlphaBlendMinus();
                break;

            case RENDER_TYPE_ALPHA_BLEND_OTHER:
                EnableAlphaBlend2();
                break;
            }

            if (o->Type == BITMAP_JOINT_HEALING && o->SubType == 8)
            {
                DisableDepthTest();
            }

            if (o->Type == MODEL_SPEARSKILL)
            {
                float fAlpha;
                switch (o->SubType)
                {
                case 0:
                case 1:
                case 2:
                case 4:
                case 9:
                case 10:
                    fAlpha = (float)min(o->LifeTime, 20) * 0.05f;
                    glColor3f(fAlpha * o->Light[0], fAlpha * o->Light[1], fAlpha * o->Light[2]);
                    break;
                case 3:
                case 5:
                case 6:
                case 7:
                case 8:
                case 16:
                case 14:
                case 17:
                    glColor3f(o->Light[0], o->Light[1], o->Light[2]);
                    break;
                case 15:
                    glColor3f(o->Light[0], o->Light[1], o->Light[2]);
                    EnableAlphaBlendMinus();
                    break;
                }
            }
            else if (o->Type == BITMAP_FLARE_BLUE && o->SubType == 20)
            {
                EnableAlphaBlend2();
                glColor3fv(o->Light);
            }
            else if (o->Type == BITMAP_SMOKE && o->SubType == 0)
            {
                float fAlpha = (float)min(o->LifeTime, 20) * 0.1f;
                glColor3f(fAlpha * o->Light[0], fAlpha * o->Light[1], fAlpha * o->Light[2]);
            }
            else if (o->Type == BITMAP_JOINT_SPARK)
            {
                if (o->SubType == 5)
                    BindTexture(o->TexType);
            }
            else
            {
                glColor3fv(o->Light);
            }

            BindTexture(o->TexType);

            for (int j = 0; j < (int)o->NumTails; j++)
            {
                if (o->Type == BITMAP_SMOKE && o->SubType == 0 && j < 1)
                {
                    continue;
                }

                if (o->Type == BITMAP_JOINT_HEALING && (o->SubType == 9 || o->SubType == 10) && j == (int)o->NumTails - 1)
                {
                    continue;
                }

                auto currentTail = o->Tails[j];
                auto nextTail = o->Tails[j + 1];

                float Light1, Light2;
                if (o->bTileMapping)
                {
                    Light1 = ((int)o->NumTails - (j)) / 16.f;
                    Light2 = ((int)o->NumTails - (j + 1)) / 16.f;
                }
                else if (o->m_byReverseUV == 3)
                {
                    Light1 = 1.f - (j) / (float)(o->MaxTails - 1);
                    Light2 = 1.f - (j + 1) / (float)(o->MaxTails - 1);
                }
                else
                {
                    Light1 = ((int)o->NumTails - (j)) / (float)(o->MaxTails - 1);
                    Light2 = ((int)o->NumTails - (j + 1)) / (float)(o->MaxTails - 1);
                }

                float Scroll = (float)((int)WorldTime % 1000) * 0.001f;
                if (o->Type == BITMAP_JOINT_THUNDER || o->Type == BITMAP_JOINT_THUNDER + 1)
                {
                    Light1 *= powf(2.f, FPS_ANIMATION_FACTOR);
                    Light2 *= powf(2.f, FPS_ANIMATION_FACTOR);
                    Light1 -= Scroll;
                    Light2 -= Scroll;
                }
                if (o->Type == BITMAP_FLARE_FORCE && o->SubType >= 0 && o->SubType <= 4
                    || (o->SubType >= 11 && o->SubType <= 13)	//^ 펜릴 스킬 관련
                    )
                {
                    Light1 = ((int)o->NumTails - (j)) / (float)((o->MaxTails - 1) / 2);
                    Light2 = ((int)o->NumTails - (j + 1)) / (float)((o->MaxTails - 1) / 2);
                    Light1 -= Scroll;
                    Light2 -= Scroll;
                }
                if (o->bTileMapping)
                {
                    Scroll *= powf(2.f, FPS_ANIMATION_FACTOR);
                    Light1 *= powf(2.f, FPS_ANIMATION_FACTOR);
                    Light2 *= powf(2.f, FPS_ANIMATION_FACTOR);
                    Light1 -= Scroll;
                    Light2 -= Scroll;
                }
                if (o->Type == BITMAP_JOINT_FORCE && o->SubType == 0)
                {
                    float Luminosity = ((float)((o->MaxTails - j) / (float)(o->MaxTails)) * 2);
                    Luminosity *= powf(o->Light[0], FPS_ANIMATION_FACTOR);
                    glColor3f(Luminosity, Luminosity, Luminosity);

                    glBegin(GL_QUADS);
                    glTexCoord2f(Light1, 0.f); glVertex3fv(currentTail[0]);
                    glTexCoord2f(Light1, 1.f); glVertex3fv(currentTail[1]);
                    glTexCoord2f(Light2, 1.f); glVertex3fv(nextTail[1]);
                    glTexCoord2f(Light2, 0.f); glVertex3fv(nextTail[0]);
                    glEnd();
                }
                else
                {
                    if (o->Type == MODEL_SPEARSKILL && (o->SubType == 0 || o->SubType == 4
                        || o->SubType == 9
                        ))
                    {
                        float scale = 0.7f;
                        vec3_t Light;
                        if (o->Target != NULL)
                        {
                            float fJointHeight;

                            if (o->SubType == 9)
                                fJointHeight = (currentTail[0][2] - (o->Target->Position[2] + 180)) * 0.01f;
                            else
                                fJointHeight = (currentTail[0][2] - (o->Target->Position[2] + 50)) * 0.01f;

                            if (fJointHeight > 0)
                            {
                                Vector(o->Light[0] - fJointHeight, o->Light[1] - fJointHeight, o->Light[2] - fJointHeight, Light);
                                glColor3fv(Light);
                            }
                            else
                            {
                                VectorCopy(o->Light, Light);
                                glColor3fv(o->Light);//1.f,1.f,1.f);
                            }
                        }
                        else
                        {
                            glColor3f(1.f, 1.f, 1.f);
                        }

                        if (j == ((int)o->NumTails / 2))
                        {
                            vec3_t  Position;

                            Vector(0.f, 0.f, 0.f, Position);
                            for (int k = 0; k < 4; ++k)
                            {
                                VectorAdd(currentTail[k], Position, Position);
                            }
                            VectorScale(Position, 0.25f, Position);

                            CreateSprite(BITMAP_FLARE_BLUE, Position, scale, Light, NULL);
                        }
                    }
                    else if (o->Type == BITMAP_JOINT_HEALING && (o->SubType == 9 || o->SubType == 10))
                    {
                        if (o->Target != NULL)
                        {
                            float  scale = 0.7f;
                            vec3_t Light;
                            float  fJointHeight = (j) * 0.01f;
                            VectorScale(o->Light, powf(0.9978f, FPS_ANIMATION_FACTOR), o->Light);
                            Vector(o->Light[0] - fJointHeight, o->Light[1] - fJointHeight, o->Light[2] - fJointHeight, Light);
                            glColor3fv(Light);

                            vec3_t  Position;

                            Vector(0.f, 0.f, 0.f, Position);
                            for (int k = 0; k < 4; ++k)
                            {
                                VectorAdd(currentTail[k], Position, Position);
                            }
                            VectorScale(Position, 0.25f, Position);

                            if (o->SubType == 9) scale = 0.5f;
                            CreateSprite(BITMAP_FLARE_BLUE, Position, scale, Light, NULL);
                        }
                    }
                    else if (o->Type == BITMAP_JOINT_THUNDER + 1 && o->SubType == 0)
                    {
                        int tail = (int)(o->Light[2]);
                        if (tail == j)
                        {
                            float l = o->Light[2] - j;
                            glColor3f(l, l, l);
                        }
                        else if (tail < j)
                        {
                            glColor3f(0.f, 0.f, 0.f);
                        }
                        else
                        {
                            glColor3f(0.7f, 0.7f, 0.7f);
                        }
                    }
                    else if (o->Type == BITMAP_FLARE + 1 && o->SubType == 6)
                    {
                        if (j == 0)
                        {
                            vec3_t  Position;
                            Vector(0.f, 0.f, 0.f, Position);
                            for (int k = 0; k < 4; ++k)
                            {
                                VectorAdd(currentTail[k], Position, Position);
                            }
                            VectorScale(Position, 0.25f, Position);

                            CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, o->Light, NULL, (float)(rand() % 360), 3);
                            CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, o->Light, NULL, (float)(rand() % 360), 3);
                        }
                    }
                    else if (o->Type == BITMAP_FLARE + 1 && o->SubType == 8)
                    {
                        if (j == 0)
                        {
                            vec3_t  Position;
                            Vector(0.f, 0.f, 0.f, Position);
                            for (int k = 0; k < 4; ++k)
                            {
                                VectorAdd(currentTail[k], Position, Position);
                            }
                            VectorScale(Position, 0.25f, Position);

                            CreateSprite(BITMAP_FLARE_BLUE, Position, 0.3f, o->Light, NULL, (float)(rand() % 360), 3);
                            CreateSprite(BITMAP_SHINY + 1, Position, 1.f, o->Light, NULL, (float)(rand() % 360), 3);
                        }
                    }
                    else if (o->Type == BITMAP_FLARE_FORCE && (o->SubType >= 0 && o->SubType <= 4)
                        || (o->SubType >= 11 && o->SubType <= 13)
                        )
                    {
                        float Luminosity = ((float)(((int)o->NumTails - 1 - j) / (float)(o->MaxTails)) * 2);

                        glColor3f(o->Light[0] * Luminosity, o->Light[1] * Luminosity, o->Light[2] * Luminosity);
                    }
                    else if (o->Type == BITMAP_JOINT_FORCE && o->SubType == 1)
                    {
                        float Luminosity = (1.f - ((int)o->NumTails - j) / (float)(o->NumTails)) * 2.f;

                        glColor3f(o->Light[0] * Luminosity, o->Light[1] * Luminosity, o->Light[2] * Luminosity);
                    }
#ifdef GUILD_WAR_EVENT
                    if (o->Type == BITMAP_FLARE && o->SubType == 22)
                    {
                        vec3_t t_bias;
                        VectorSubtract(o->Target->Position, o->StartPosition, t_bias);
                        glMatrixMode(GL_MODELVIEW);
                        glPushMatrix();
                        glTranslatef(t_bias[0], t_bias[1], t_bias[2]);

                        glBegin(GL_QUADS);
                        glTexCoord2f(Light1, 1.f); glVertex3fv(currentTail[2]);
                        glTexCoord2f(Light1, 0.f); glVertex3fv(currentTail[3]);
                        glTexCoord2f(Light2, 0.f); glVertex3fv(o->Tails[j + 1][3]);
                        glTexCoord2f(Light2, 1.f); glVertex3fv(o->Tails[j + 1][2]);
                        glTexCoord2f(Light1, 0.f); glVertex3fv(currentTail[0]);
                        glTexCoord2f(Light1, 1.f); glVertex3fv(currentTail[1]);
                        glTexCoord2f(Light2, 1.f); glVertex3fv(o->Tails[j + 1][1]);
                        glTexCoord2f(Light2, 0.f); glVertex3fv(o->Tails[j + 1][0]);
                        glEnd();

                        glPopMatrix();
                        continue;
                    }
#endif //GUILD_WAR_EVENT

                    float V1 = 0.f;
                    float V2 = 1.f;
                    float L1 = Light1;
                    float L2 = Light2;
                    if (o->m_byReverseUV == 1)
                    {
                        V1 = 1.f;
                        V2 = 0.f;
                    }
                    else if (o->m_byReverseUV == 2)
                    {
                        L1 = 1.f - L1;
                        L2 = 1.f - L2;
                    }

                    if ((o->RenderFace & RENDER_FACE_ONE) == RENDER_FACE_ONE)
                    {
                        glBegin(GL_QUADS);
                        glTexCoord2f(L1, V2); glVertex3fv(currentTail[2]);
                        glTexCoord2f(L1, V1); glVertex3fv(currentTail[3]);
                        glTexCoord2f(L2, V1); glVertex3fv(nextTail[3]);
                        glTexCoord2f(L2, V2); glVertex3fv(nextTail[2]);
                        glEnd();
                    }

                    if ((o->RenderFace & RENDER_FACE_TWO) == RENDER_FACE_TWO)
                    {
                        if (o->Type == BITMAP_JOINT_THUNDER || o->Type == BITMAP_JOINT_THUNDER + 1)
                        {
                            L1 += Scroll * 2.f;
                            L2 += Scroll * 2.f;
                        }
                        glBegin(GL_QUADS);
                        glTexCoord2f(L1, V1); glVertex3fv(currentTail[0]);
                        glTexCoord2f(L1, V2); glVertex3fv(currentTail[1]);
                        glTexCoord2f(L2, V2); glVertex3fv(nextTail[1]);
                        glTexCoord2f(L2, V1); glVertex3fv(nextTail[0]);
                        glEnd();
                    }
                }
            }

            if (o->Type == BITMAP_JOINT_HEALING && o->SubType == 8)
            {
                EnableDepthTest();
            }
            if (o->Type == BITMAP_JOINT_THUNDER + 1 && o->SubType == 6)
            { 
                vec3_t Light;
                EnableAlphaBlend();
                o->Velocity *= powf(1.f / 1.1f, FPS_ANIMATION_FACTOR);
                Vector(o->Velocity, o->Velocity, o->Velocity, Light);
                RenderTerrainAlphaBitmap(BITMAP_MAGIC + 1, o->TargetPosition[0], o->TargetPosition[1], 2.f, 2.f, Light);
                DisableAlphaBlend();
            }
        }
    }
}

void GetMagicScrew(int iParam, vec3_t vResult, float fSpeedRate)
{
    iParam += static_cast<int>(WorldTime / 40.f);

    vec3_t vDirTemp;
    float fSpeed[3] = { 0.048f, 0.0613f, 0.1113f };
    fSpeed[0] *= powf(fSpeedRate, FPS_ANIMATION_FACTOR);
    fSpeed[1] *= powf(fSpeedRate, FPS_ANIMATION_FACTOR);
    fSpeed[2] *= powf(fSpeedRate, FPS_ANIMATION_FACTOR);
    vDirTemp[0] = sinf((float)(iParam + 55555) * fSpeed[0]) * cosf((float)iParam * fSpeed[1]);
    vDirTemp[1] = sinf((float)(iParam + 55555) * fSpeed[0]) * sinf((float)iParam * fSpeed[1]);
    vDirTemp[2] = cosf((float)(iParam + 55555) * fSpeed[0]);
    float fSinAdd = sinf((float)(iParam + 11111) * fSpeed[2]);
    float fCosAdd = cosf((float)(iParam + 11111) * fSpeed[2]);
    vResult[2] = vDirTemp[0];
    vResult[1] = fSinAdd * vDirTemp[1] + fCosAdd * vDirTemp[2];
    vResult[0] = fCosAdd * vDirTemp[1] - fSinAdd * vDirTemp[2];
}