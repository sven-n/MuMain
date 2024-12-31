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

void MoveParticle(OBJECT* o, int Turn);

#define MAX_PLANES 100

OBJECT Planes[MAX_PLANES];

void CreatePlane(int Type, vec3_t Position, vec3_t Light, float Angle)
{
    for (int i = 0; i < MAX_PLANES; i++)
    {
        OBJECT* o = &Planes[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Type = Type;
            VectorCopy(Position, o->Position);
            VectorCopy(Light, o->Light);
            Vector(0.6f, 1.f, 0.8f, o->Light);
            Vector(0.f, 0.f, Angle, o->Angle);
            o->LifeTime = 18;
            o->Scale = 70.f;
            Vector(0.f, 0.f, 10.f, o->Direction);
            return;
        }
    }
}

void RenderPlane(int Texture, vec3_t Position, float Scale, float Angle)
{
    BindTexture(Texture);
    EnableAlphaBlend();

    glPushMatrix();
    glTranslatef(Position[0], Position[1], Position[2]);
    glRotatef(Angle, 0.f, 0.f, 1.f);

    vec3_t BoundingVertices[4];
    Vector(-Scale, -Scale, 0.f, BoundingVertices[0]);
    Vector(Scale, -Scale, 0.f, BoundingVertices[1]);
    Vector(-Scale, Scale, 0.f, BoundingVertices[2]);
    Vector(Scale, Scale, 0.f, BoundingVertices[3]);

    glBegin(GL_QUADS);

    glTexCoord2f(0.f, 1.f); glVertex3fv(BoundingVertices[0]);
    glTexCoord2f(1.f, 1.f); glVertex3fv(BoundingVertices[2]);
    glTexCoord2f(1.f, 0.f); glVertex3fv(BoundingVertices[3]);
    glTexCoord2f(0.f, 0.f); glVertex3fv(BoundingVertices[1]);

    glEnd();

    glPopMatrix();
    DisableAlphaBlend();
}

void MovePlanes()
{
    for (int i = 0; i < MAX_PLANES; i++)
    {
        OBJECT* o = &Planes[i];
        if (o->Live)
        {
            o->LifeTime -= FPS_ANIMATION_FACTOR;
            if (o->LifeTime <= 0)
                o->Live = false;
            MoveParticle(o, true);
            if (o->LifeTime < 10)
            {
                float Light = (float)o->LifeTime * 0.1f;
                Vector(Light * 0.6f, Light, Light * 0.8f, o->Light);
            }
            o->Angle[2] -= o->LifeTime * FPS_ANIMATION_FACTOR;
        }
    }
}

void RenderPlanes()
{
    for (int i = 0; i < MAX_PLANES; i++)
    {
        OBJECT* o = &Planes[i];
        if (o->Live)
        {
            glColor3f(o->Light[0], o->Light[1], o->Light[2]);
            RenderPlane(o->Type, o->Position, o->Scale, o->Angle[2]);
        }
    }
}

void RenderShpere(int Type, vec3_t ShperePosition, float Scale, vec3_t ShpereLight, float Rotation, float TextureV)
{
    vec3_t ObjectPosition;
    VectorCopy(ShperePosition, ObjectPosition);
    ObjectPosition[2] += Scale * FPS_ANIMATION_FACTOR;
    BindTexture(Type);

    float Width = 18.f;
    float Height = 9.f;
    for (float y = 0.f; y < Height; y += 1.f)
    {
        for (float x = 0.f; x < Width; x += 1.f)
        {
            float UV[4][2];
            TEXCOORD(UV[0], (x) * (1.f / Width), (y) * (1.f / Height));
            TEXCOORD(UV[1], (x + 1.f) * (1.f / Width), (y) * (1.f / Height));
            TEXCOORD(UV[2], (x + 1.f) * (1.f / Width), (y + 1.f) * (1.f / Height));
            TEXCOORD(UV[3], (x) * (1.f / Width), (y + 1.f) * (1.f / Height));

            vec3_t Angle;
            float Matrix1[3][4];
            float Matrix2[3][4];
            float Matrix3[3][4];
            float Matrix4[3][4];
            Vector(90.f, (x) * 20.f + Rotation, (y) * 20.f, Angle);
            AngleIMatrix(Angle, Matrix1);
            Vector(90.f, (x + 1.f) * 20.f + Rotation, (y) * 20.f, Angle);
            AngleIMatrix(Angle, Matrix2);
            Vector(90.f, (x + 1.f) * 20.f + Rotation, (y + 1.f) * 20.f, Angle);
            AngleIMatrix(Angle, Matrix3);
            Vector(90.f, (x) * 20.f + Rotation, (y + 1.f) * 20.f, Angle);
            AngleIMatrix(Angle, Matrix4);

            vec3_t p, Position[4];
            Vector(0.f, Scale, 0.f, p);
            VectorRotate(p, Matrix1, Position[0]);
            VectorRotate(p, Matrix2, Position[1]);
            VectorRotate(p, Matrix3, Position[2]);
            VectorRotate(p, Matrix4, Position[3]);

            glBegin(GL_QUADS);
            //glBegin(GL_LINE_STRIP);
            for (int i = 0; i < 4; i++)
            {
                glTexCoord2f(UV[i][0], UV[i][1] + TextureV);

                float Luminosity = 0.5f + 0.5f * (-Position[i][1]) / VectorLength(Position[i]);
                vec3_t Light;
                VectorScale(ShpereLight, Luminosity, Light);
                glColor3fv(Light);

                VectorAdd(ObjectPosition, Position[i], Position[i]);
                glVertex3fv(Position[i]);
            }
            glEnd();
        }
    }
}

typedef struct
{
    int LifeTime;
    int x, y;
} MAGIC_POINT;

MAGIC_POINT MagicPoint[100];

void MagicBitmap(int Texture)
{
    BITMAP_t* b = &Bitmaps[Texture];
    if (b->Buffer == NULL)
    {
        b->Width = 128.f;
        b->Height = 128.f;
        b->Buffer = new unsigned char[(int)b->Width * (int)b->Height * 3];
    }
    int Index = 0;
    for (int y = 0; y < (int)b->Height; y++)
    {
        for (int x = 0; x < (int)b->Width; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                if (b->Buffer[Index + c] < 16) b->Buffer[Index + c] = 10;
                else b->Buffer[Index + c] -= 6;
            }
            Index += 3;
        }
    }
    for (int i = 0; i < 50; i++)
    {
        MAGIC_POINT* p = &MagicPoint[i];
        if (p->LifeTime <= 0)
        {
            p->LifeTime = (rand() % 16 + 16);
            p->x = rand() % 128;
            p->y = rand() % 128;
        }
        else
        {
            p->LifeTime--;
            //p->x += rand()%8-4;
            //p->y += rand()%8-4;
            p->y = p->y * 8 / 10;
            if (p->y == 0)
                p->x = p->x * 8 / 10;
            Index = (p->y * 128 + p->x) * 3;
            b->Buffer[Index] = 255;
            b->Buffer[Index + 1] = 255;
            b->Buffer[Index + 2] = 255;
        }
    }

    glGenTextures(1, &(b->TextureNumber));
    glBindTexture(GL_TEXTURE_2D, b->TextureNumber);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, (int)b->Width, (int)b->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, b->Buffer);
}

void RenderMagicBox(int Texture, vec3_t Position, float Angle)
{
    MagicBitmap(Texture);

    BindTexture(Texture);
    EnableAlphaBlend();
    glPushMatrix();
    glTranslatef(Position[0], Position[1], Position[2]);
    glRotatef(Angle, 0.f, 0.f, 1.f);
    vec3_t BoundingBoxMin;
    vec3_t BoundingBoxMax;
    Vector(-60.f, -60.f, 0.f, BoundingBoxMin);
    Vector(60.f, 60.f, 210.f, BoundingBoxMax);
    Vector(-200.f, -200.f, 0.f, BoundingBoxMin);
    Vector(200.f, 200.f, 250.f, BoundingBoxMax);
    vec3_t BoundingVertices[8];
    Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[0]);
    Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[1]);
    Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[2]);
    Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[3]);
    Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[4]);
    Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[5]);
    Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[6]);
    Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[7]);

    float Color = sinf(WorldTime * 0.002f);
    glColor3f(Color, Color, Color);
    glBegin(GL_QUADS);

    glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[5]);

    glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[2]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);

    glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[6]);

    glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[1]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);

    glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[4]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);

    /*glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
    glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
    glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);
    glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[3]);*/

    glEnd();

    glPopMatrix();
    DisableAlphaBlend();
}

void RenderFog(vec3_t Position, int Flag, float Height, float u, float v)
{
    float UV[4][2];
    TEXCOORD(UV[0], 0.f + u, 0.f - v);
    TEXCOORD(UV[1], 0.f + u, 2.f - v);
    TEXCOORD(UV[2], 2.f + u, 2.f - v);
    TEXCOORD(UV[3], 2.f + u, 0.f - v);
    float Alpha[4];
    Alpha[0] = 1.f;
    Alpha[1] = 0.f;
    Alpha[2] = 0.f;
    Alpha[3] = 1.f;

    vec3_t p[4];
    if (Flag == 0)
    {
        Vector(Position[0] - 1200.f, Position[1] + 400.f, Position[2] + Height, p[0]);
        Vector(Position[0] - 1200.f, Position[1] - 1000.f, Position[2] + Height, p[1]);
        Vector(Position[0] + 1200.f, Position[1] - 1000.f, Position[2] + Height, p[2]);
        Vector(Position[0] + 1200.f, Position[1] + 400.f, Position[2] + Height, p[3]);
    }
    else
    {
        Vector(Position[0] - 800.f, Position[1] + 600.f, Position[2] + 400.f, p[0]);
        Vector(Position[0] - 800.f, Position[1] + 600.f, Position[2] - 10.f, p[1]);
        Vector(Position[0] + 800.f, Position[1] + 600.f, Position[2] - 10.f, p[2]);
        Vector(Position[0] + 800.f, Position[1] + 600.f, Position[2] + 400.f, p[3]);
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < 4; i++)
    {
        glColor4f(1.f, 1.f, 1.f, Alpha[i]);
        glTexCoord2f(UV[i][0], UV[i][1]);
        glVertex3fv(p[i]);
    }
    glEnd();
}

int FireFrame = 0;

void MappingEffect(int SrcIndex, int DstIndex, int Flag)
{
    BITMAP_t* src = &Bitmaps[SrcIndex];
    BITMAP_t* dst = &Bitmaps[DstIndex];
    unsigned char* s = src->Buffer;
    unsigned char* d = dst->Buffer;

    int i, j, k;
    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 256; j++)
        {
            int Index = (i * 256 + j) * 3;
            if (s[Index] >= 96)
            {
                //if(rand_fps_check(2))
                {
                    for (k = 0; k < 3; k++)
                    {
                        int Light = s[Index + k] + rand() % 64;
                        if (Light > 255) Light = 255;
                        d[Index + k] = Light;
                        //d[Index+k] = rand()%256;
                    }
                }
            }
        }
    }
    /*for(i=0;i<256;i++)
    {
        for(j=0;j<256;j++)
        {
            int Index = (i*256+j)*3;
            for(k=0;k<4;k++)
            {
                int Color;
                Color = (d[Index+256*3+k]+d[Index+256*3-3+k]+d[Index+k])/3-2;
                if(Color > 0) d[Index+k] = Color;
            }
        }
    }*/
    FireFrame++;

    glGenTextures(1, &(dst->TextureNumber));

    glBindTexture(GL_TEXTURE_2D, dst->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, src->Components, (int)src->Width, (int)src->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, dst->Buffer);
}

void RenderSky3()
{
    float Height = 800.f;
    float Num = 20.f;
    for (float x = 0.f; x < Num; x += 1.f)
    {
        float UV[4][2];
        TEXCOORD(UV[0], (x) * (1.f / Num), 1.f);
        TEXCOORD(UV[1], (x + 1.f) * (1.f / Num), 1.f);
        TEXCOORD(UV[2], (x + 1.f) * (1.f / Num), 0.f);
        TEXCOORD(UV[3], (x) * (1.f / Num), 0.f);

        vec3_t Light[4];
        Vector(1.f, 1.f, 1.f, Light[0]);
        Vector(1.f, 1.f, 1.f, Light[1]);
        Vector(1.f, 1.f, 1.f, Light[2]);
        Vector(1.f, 1.f, 1.f, Light[3]);

        vec3_t Angle;
        float Matrix1[3][4];
        float Matrix2[3][4];
        Angle[0] = 0.f;
        Angle[1] = 0.f;
        Angle[2] = (x - 0.5f) * 18.f + 140.f;
        AngleIMatrix(Angle, Matrix1);
        Angle[2] = (x + 0.5f) * 18.f + 140.f;
        AngleIMatrix(Angle, Matrix2);

        vec3_t p, Position[4];
        Vector(0.f, CameraViewFar * 0.5f, -Height, p);
        VectorRotate(p, Matrix1, Position[0]);
        VectorAdd(CameraPosition, Position[0], Position[0]);
        Vector(0.f, CameraViewFar * 0.5f, -Height, p);
        VectorRotate(p, Matrix2, Position[1]);
        VectorAdd(CameraPosition, Position[1], Position[1]);
        Vector(0.f, CameraViewFar * 0.5f, Height, p);
        VectorRotate(p, Matrix2, Position[2]);
        VectorAdd(CameraPosition, Position[2], Position[2]);
        Vector(0.f, CameraViewFar * 0.5f, Height, p);
        VectorRotate(p, Matrix1, Position[3]);
        VectorAdd(CameraPosition, Position[3], Position[3]);

        glBegin(GL_QUADS);
        for (int i = 0; i < 4; i++)
        {
            glColor3f(Light[i][0], Light[i][1], Light[i][2]);
            glTexCoord2f(UV[i][0], UV[i][1]);
            glVertex3f(Position[i][0], Position[i][1], Position[i][2] + 100.f);
        }
        glEnd();
    }
}

static  int     g_waveCount = 0;
static  int     g_backLightIndex[4 * 4 * 6];
static  int     g_gridNum = 4;
static	float   g_Height = 32.f;
static  float   g_Width = 32.f;
static  float   g_vertX[5 * 5];
static  float   g_vertX2[5 * 5];
static	float   g_vertY[5 * 5];
static  float   g_vertZ[5 * 5];
static  float   g_U[5 * 5];
static  float   g_V[5 * 5];
static  bool    g_aniBackLight = true;
static  bool    g_initBackLight = false;

void initBackLight(void)
{
    int count = 0;
    int dx, dy;

    for (dy = 0; dy < g_gridNum; dy++)
    {
        for (dx = 0; dx < g_gridNum; dx++)
        {
            g_backLightIndex[count + 0] = dx + (dy * (g_gridNum + 1));
            g_backLightIndex[count + 1] = dx + 1 + (dy * (g_gridNum + 1));
            g_backLightIndex[count + 2] = dx + ((dy + 1) * (g_gridNum + 1));
            count += 3;

            g_backLightIndex[count + 0] = dx + ((dy + 1) * (g_gridNum + 1));
            g_backLightIndex[count + 1] = dx + 1 + (dy * (g_gridNum + 1));
            g_backLightIndex[count + 2] = (dx + 1) + ((dy + 1) * (g_gridNum + 1));
            count += 3;
        }
    }
}

void AnimationBackLight(void)
{
    if (!g_aniBackLight) return;

    if (!g_initBackLight)
    {
        g_initBackLight = true;
        initBackLight();
    }

    g_Height = 16.f;

    int     dx, dy;
    float   deltaX;
    float   maxU = g_Width * g_gridNum;
    float   maxV = g_Height * g_gridNum;
    float   x, y;
    for (dy = 0; dy < g_gridNum + 1; dy++)
    {
        for (dx = 0; dx < g_gridNum + 1; dx++)
        {
            int offset = dx + (dy * (g_gridNum + 1));

            deltaX = 0.f;//(float)(sin(g_waveCount))*18.f;

            x = g_Width * dx;
            y = g_Height * dy;

            g_vertX[offset] = x + deltaX;
            g_vertX2[offset] = x - deltaX;
            g_vertY[offset] = y - 5;
            g_vertZ[offset] = 0.f;

            g_U[offset] = x / maxU;
            g_V[offset] = y / maxV;

            g_waveCount++;
        }
    }

    g_aniBackLight = false;
}

void RenderBackLight(OBJECT* o, vec3_t Position, vec3_t Light1, vec3_t Light2)
{
    vec3_t  position;
    int     offset;
    int     iVertex = g_gridNum * g_gridNum * 6;

    EnableAlphaBlend();
    BeginSprite();

    BindTexture(BITMAP_SHINY + 1);//BITMAP_ENERGY);
    float light = sinf(g_waveCount / 1000.f) * 0.5f + 0.8f;

    VectorTransform(Position, CameraMatrix, position);
    position[0] -= (g_Width * (g_gridNum / 2.f));
    position[1] -= 25.f;
    position[2] += 30.f;

    glBegin(GL_TRIANGLES);
    glColor3f(Light1[0] * light, Light1[1] * light, Light1[2] * light);
    for (int i = 0; i < iVertex; i++)
    {
        offset = g_backLightIndex[i];

        glTexCoord2f(g_U[offset], g_V[offset]);
        glVertex3f(position[0] + g_vertX[offset], position[1] + g_vertY[offset], position[2] + g_vertZ[offset]);
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(Light2[0], Light2[1], Light2[2]);

    for (int i = 0; i < iVertex; i++)
    {
        offset = g_backLightIndex[i];

        glTexCoord2f(g_U[offset], g_V[offset]);
        glVertex3f(position[0] + g_vertX2[offset], position[1] + g_vertY[offset], position[2] + g_vertZ[offset]);
    }
    glEnd();

    EndSprite();
    DisableAlphaBlend();

    if ((o->CurrentAction<PLAYER_WALK_MALE || o->CurrentAction>PLAYER_RUN_RIDE_WEAPON)
        && (o->CurrentAction != PLAYER_RAGE_UNI_RUN && o->CurrentAction != PLAYER_RAGE_UNI_RUN_ONE_RIGHT)
        && light > 1.1f)
    {
        vec3_t  a, Light;

        Vector(-90.f, 0.f, 0.f, a);

        position[0] = Position[0] + rand() % 80 - 40;
        position[1] = Position[1] + rand() % 80 - 40;
        position[2] = Position[2] + 10;

        if (rand_fps_check(2))
        {
            CreateParticle(BITMAP_ENERGY, position, o->Angle, Light1, 1, 0.05f);
        }
        else if (rand_fps_check(2))
        {
            CreateParticle(BITMAP_ENERGY, position, o->Angle, Light2, 1, 0.05f);
        }

        Vector(Light1[0] * light + Light2[0] * (1.f - light), Light1[1] * light + Light2[1] * (1.f - light), Light1[2] * light + Light2[2] * (1.f - light), Light);

        CreateParticleFpsChecked(BITMAP_FLARE, position, o->Angle, Light1, 1, 0.2f);
    }
    g_aniBackLight = true;
}