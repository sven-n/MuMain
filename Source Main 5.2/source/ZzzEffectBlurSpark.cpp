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

#define MAX_BLURS      100
#define MAX_BLUR_TAILS 30
#define MAX_BLUR_LIFETIME 30

#define MAX_OBJECTBLURS		1000
#define MAX_OBJECTBLUR_TAILS 600
#define MAX_OBJECTBLUR_LIFETIME 30
//#define MAX_OBJECTBLUR_LIFETIME 11		// FIX

typedef struct
{
    bool      Live;
    int       Type;
    int       LifeTime;
    CHARACTER* Owner;
    int       Number;
    vec3_t    Light;
    vec3_t    p1[MAX_BLUR_TAILS];
    vec3_t    p2[MAX_BLUR_TAILS];
    int       SubType;
} BLUR;

BLUR Blur[MAX_BLURS];

void AddBlur(BLUR* b, vec3_t p1, vec3_t p2, vec3_t Light, int Type)
{
    b->Type = Type;
    VectorCopy(Light, b->Light);
    for (int i = b->Number - 1; i >= 0; i--)
    {
        VectorCopy(b->p1[i], b->p1[i + 1]);
        VectorCopy(b->p2[i], b->p2[i + 1]);
    }
    VectorCopy(p1, b->p1[0]);
    VectorCopy(p2, b->p2[0]);
    b->Number++;
    if (b->Number >= MAX_BLUR_TAILS - 1)
    {
        b->Number = MAX_BLUR_TAILS - 1;
    }
}

void CreateBlur(CHARACTER* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short, int SubType)
{
    for (int i = 0; i < MAX_BLURS; i++)
    {
        BLUR* b = &Blur[i];
        if (b->Live && b->Owner == Owner)
        {
            if (SubType > 0 && b->SubType != SubType)
                continue;
            AddBlur(b, p1, p2, Light, Type);
            return;
        }
    }

    for (int i = 0; i < MAX_BLURS; i++)
    {
        BLUR* b = &Blur[i];
        if (!b->Live)
        {
            b->Live = true;
            b->Owner = Owner;
            b->Number = 0;
            b->LifeTime = Short ? 15 : MAX_BLUR_LIFETIME;
            b->SubType = SubType;
            AddBlur(b, p1, p2, Light, Type);
            return;
        }
    }
}

void MoveBlurs()
{
    for (int i = 0; i < MAX_BLURS; i++)
    {
        BLUR* b = &Blur[i];
        if (b->Live)
        {
            b->LifeTime--;
            b->Number--;

            for (int i = b->Number - 1; i >= 0; i--)
            {
                VectorCopy(b->p1[i], b->p1[i + 1]);
                VectorCopy(b->p2[i], b->p2[i + 1]);
            }
            if (b->LifeTime <= 0)
            {
                b->Number--;
                if (b->Number <= 0)
                {
                    b->Live = false;
                }
            }
        }
    }
    MoveObjectBlurs();
}

void RenderBlurs()
{
    int Type;
    //DisableCullFace();
    for (int i = 0; i < MAX_BLURS; i++)
    {
        BLUR* b = &Blur[i];
        if (b->Live)
        {
            Type = b->Type;

            int nTexture = BITMAP_BLUR + Type;
            if (Type == 3)
            {
                nTexture = BITMAP_BLUR2;
            }
            else if (Type == 4)
            {
                nTexture = BITMAP_BLUR;
            }
            else if (Type == 5)
            {
                nTexture = BITMAP_BLUR + 3;
            }
            if (b->Owner->Level == 0 && (Type <= 3 || (Type >= 5 && Type <= 10)))
            {
                EnableAlphaBlend();
            }
            else
            {
                EnableAlphaBlendMinus();
            }

            if (Type > 2) Type = Type - 3;
            if (b->Number >= 2)
            {
                BindTexture(nTexture);
                for (int j = 0; j < b->Number - 1; j++)
                {
                    glBegin(GL_TRIANGLE_FAN);
                    float Light;
                    float TexU;
                    if (b->Owner->Level == 0)
                        Light = (b->Number - j) / (float)b->Number;
                    else
                        Light = 1.f;
                    glColor3f(b->Light[0] * Light, b->Light[1] * Light, b->Light[2] * Light);
                    TexU = (j) / (float)b->Number;
                    glTexCoord2f(TexU, 1.f); glVertex3fv(b->p1[j]);
                    glTexCoord2f(TexU, 0.f); glVertex3fv(b->p2[j]);
                    if (b->Owner->Level == 0)
                        Light = (b->Number - (j + 1)) / (float)b->Number;
                    else
                        Light = 1.f;
                    glColor3f(b->Light[0] * Light, b->Light[1] * Light, b->Light[2] * Light);
                    TexU = (j + 1) / (float)b->Number;
                    glTexCoord2f(TexU, 0.f); glVertex3fv(b->p2[j + 1]);
                    glTexCoord2f(TexU, 1.f); glVertex3fv(b->p1[j + 1]);
                    glEnd();
                }
            }
        }
    }
    RenderObjectBlurs();
}

typedef struct
{
    bool      Live;
    int       Type;
    int       LifeTime;
    OBJECT* Owner;
    int       Number;
    vec3_t    Light;
    int		  _iLimitLifeTime;
    vec3_t    p1[MAX_OBJECTBLUR_TAILS];
    vec3_t    p2[MAX_OBJECTBLUR_TAILS];
    int       SubType;
} OBJECT_BLUR;

OBJECT_BLUR ObjectBlur[MAX_OBJECTBLURS];

void ClearAllObjectBlurs()
{
    for (int i = 0; i < MAX_OBJECTBLURS; ++i)
    {
        ObjectBlur[i].Live = false;
    }
}

void AddObjectBlur(OBJECT_BLUR* b, vec3_t p1, vec3_t p2, vec3_t Light, int Type)
{
    b->Type = Type;
    VectorCopy(Light, b->Light);

    for (int i = b->Number - 1; i >= 0; i--)
    {
        VectorCopy(b->p1[i], b->p1[i + 1]);
        VectorCopy(b->p2[i], b->p2[i + 1]);
    }
    VectorCopy(p1, b->p1[0]);
    VectorCopy(p2, b->p2[0]);
    b->Number++;

    if (b->Number >= MAX_OBJECTBLUR_TAILS - 1)
    {
        b->Number = MAX_OBJECTBLUR_TAILS - 1;
    }
}

void CreateObjectBlur(OBJECT* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short, int SubType, int iLimitLifeTime)
{
    for (int i = 0; i < MAX_OBJECTBLURS; i++)
    {
        OBJECT_BLUR* b = &ObjectBlur[i];
        if (b->Live && b->Owner == Owner)
        {
            if (SubType > 0 && b->SubType != SubType)
                continue;
            AddObjectBlur(b, p1, p2, Light, Type);
            return;
        }
    }

    for (int i = 0; i < MAX_OBJECTBLURS; i++)
    {
        OBJECT_BLUR* b = &ObjectBlur[i];
        if (!b->Live)
        {
            b->Live = true;
            b->Owner = Owner;
            b->Number = 0;
            if (iLimitLifeTime > -1)
            {
                b->LifeTime = iLimitLifeTime;
                b->_iLimitLifeTime = iLimitLifeTime;
            }
            else
            {
                b->_iLimitLifeTime = Short ? 15 : MAX_OBJECTBLUR_LIFETIME;
                b->LifeTime = b->_iLimitLifeTime;
            }
            b->SubType = SubType;

            AddObjectBlur(b, p1, p2, Light, Type);
            return;
        }
    }
}

void MoveObjectBlurs()
{
    for (int i = 0; i < MAX_OBJECTBLURS; i++)
    {
        OBJECT_BLUR* b = &ObjectBlur[i];
        if (b->Live)
        {
            b->LifeTime--;
            b->Number--;

            if (b->LifeTime <= 0)
            {
                b->Number = 0;
                b->Live = false;
                continue;
            }

            for (int i = b->Number - 1; i >= 0; i--)
            {
                VectorCopy(b->p1[i], b->p1[i + 1]);
                VectorCopy(b->p2[i], b->p2[i + 1]);
            }
        }
    }
}

void RenderObjectBlurs()
{
    int Type;
    for (int i = 0; i < MAX_OBJECTBLURS; i++)
    {
        OBJECT_BLUR* b = &ObjectBlur[i];
        if (b->Live)
        {
            Type = b->Type;
            int nTexture = BITMAP_BLUR + Type;
            if (Type == 3)
            {
                nTexture = BITMAP_BLUR2;
            }
            else if (Type == 4)
            {
                nTexture = BITMAP_BLUR;
            }
            else if (Type == 5)
            {
                nTexture = BITMAP_LAVA;
            }

            EnableAlphaBlend();

            if (Type > 2) Type = Type - 3;
            if (b->Number >= 2)
            {
                BindTexture(nTexture);
                for (int j = 0; j < b->Number - 1; j++)
                {
                    float Data = 300.f;
                    if (b->SubType == 113 || b->SubType == 114)
                    {
                        if (abs(b->p1[j][0] - b->p1[j + 1][0]) > Data || abs(b->p1[j][1] - b->p1[j + 1][1]) > Data || abs(b->p1[j][2] - b->p1[j + 1][2]) > Data ||
                            abs(b->p1[j][0] - b->p2[j + 1][0]) > Data || abs(b->p1[j][1] - b->p2[j + 1][1]) > Data || abs(b->p2[j][2] - b->p2[j + 1][2]) > Data)
                            continue;
                    }

                    glBegin(GL_TRIANGLE_FAN);
                    float Light;
                    float TexU;
                    Light = (b->Number - j) / (float)b->Number;
                    glColor3f(b->Light[0] * Light, b->Light[1] * Light, b->Light[2] * Light);
                    TexU = (j) / (float)b->Number;
                    glTexCoord2f(TexU, 1.f); glVertex3fv(b->p1[j]);
                    glTexCoord2f(TexU, 0.f); glVertex3fv(b->p2[j]);
                    Light = (b->Number - (j + 1)) / (float)b->Number;
                    glColor3f(b->Light[0] * Light, b->Light[1] * Light, b->Light[2] * Light);
                    TexU = (j + 1) / (float)b->Number;
                    glTexCoord2f(TexU, 0.f); glVertex3fv(b->p2[j + 1]);
                    glTexCoord2f(TexU, 1.f); glVertex3fv(b->p1[j + 1]);
                    glEnd();
                }
            }
        }
    }
}

void RemoveObjectBlurs(OBJECT* Owner, int SubType)
{
    for (int i = 0; i < MAX_OBJECTBLURS; i++)
    {
        OBJECT_BLUR* b = &ObjectBlur[i];
        if (b->Live && b->Owner == Owner)
        {
            ObjectBlur[i].Live = false;
        }
    }
}

void CreateSpark(int Type, CHARACTER* tc, vec3_t Position, vec3_t Angle)
{
    OBJECT* to = &tc->Object;
    vec3_t Light;
    Vector(1.f, 1.f, 1.f, Light);
    CreateParticle(BITMAP_SPARK + 1, Position, to->Angle, Light);
    vec3_t p, p2;
    float Matrix[3][4];
    Vector(0.f, 50.f, 0.f, p);
    AngleMatrix(Angle, Matrix);
    VectorRotate(p, Matrix, p2);
    VectorAdd(p2, Position, p2);
    for (int i = 0; i < 20; i++)
    {
        vec3_t a;
        Vector((float)(rand() % 360), 0.f, (float)(rand() % 360), a);
        VectorAdd(a, Angle, a);
        //CreateJoint(BITMAP_JOINT_SPARK,p2,p2,a);
        CreateParticle(BITMAP_SPARK, Position, to->Angle, Light);
    }
}

void CreateBlood(OBJECT* o)
{
    int BoneHead = Models[o->Type].BoneHead;
    if (BoneHead != -1)
    {
        if (o->Type == MODEL_ICE_MONSTER)
        {
            o->Live = false;
            for (int i = 0; i < 10; i++)
                CreateEffect(MODEL_ICE_SMALL, o->Position, o->Angle, o->Light);
        }
        else if (o->Type != MODEL_GHOST && o->Type != MODEL_ASSASSIN && o->Type != MODEL_ICE_QUEEN)
        {
            vec3_t p, Position;
            for (int i = 0; i < 2; i++)
            {
                Vector((float)(rand() % 100 - 50), (float)(rand() % 100 - 50), 0.f, p);
                Models[o->Type].TransformPosition(o->BoneTransform[BoneHead], p, Position, true);
                CreatePointer(BITMAP_BLOOD, Position, (float)(rand() % 360), o->Light, (float)(rand() % 4 + 8) * 0.1f);
            }
        }
    }
}

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

physics_vertex flag_vertex[FLAG_HEIGHT * FLAG_WIDTH];
physics_face   flag_face[(FLAG_HEIGHT - 1) * (FLAG_WIDTH - 1)];

vec3_t Gravity;
float Damping = 0.04f;
float Ks = 5.f; //hook's spring
float Kd = 0.1f; //spring dumping
float Kr = 0.8f;
float DeltaTime = 0.1f;

void CreateFlag()
{
    for (int i = 0; i < FLAG_HEIGHT; i++)
    {
        for (int j = 0; j < FLAG_WIDTH; j++)
        {
            int index = i * FLAG_WIDTH + j;
            physics_vertex* v = &flag_vertex[index];
            v->p[0] = -(float)i * FLAG_SCALE;
            v->p[1] = 0.f;
            v->p[2] = (float)j * FLAG_SCALE;
            if (i == 0)
            {
                if (j == 0)
                {
                    v->p[0] -= 5.f;
                    v->p[2] += 5.f;
                }
                if (j == 6)
                {
                    v->p[0] -= 5.f;
                    v->p[2] -= 5.f;
                }
                v->p[1] += 10.f;
            }
            v->link_num = 0;
            if (j - 1 >= 0) { v->link[v->link_num] = (i)*FLAG_WIDTH + (j - 1); v->link_num++; }
            if (j + 1 <= FLAG_WIDTH - 1) { v->link[v->link_num] = (i)*FLAG_WIDTH + (j + 1); v->link_num++; }
            if (i - 1 >= 0) { v->link[v->link_num] = (i - 1) * FLAG_WIDTH + (j); v->link_num++; }
            if (i + 1 <= FLAG_HEIGHT - 1) { v->link[v->link_num] = (i + 1) * FLAG_WIDTH + (j); v->link_num++; }
        }
    }

    for (int i = 0; i < FLAG_HEIGHT - 1; i++)
    {
        for (int j = 0; j < FLAG_WIDTH - 1; j++)
        {
            physics_face* f = &flag_face[i * (FLAG_WIDTH - 1) + j];
            int index = i * FLAG_WIDTH + j;
            f->vlist[0] = index;
            f->vlist[1] = index + FLAG_WIDTH;
            f->vlist[2] = index + FLAG_WIDTH + 1;
            f->vlist[3] = index + 1;
        }
    }
}

bool InitFlag = false;
int  wind_frame = 0;

void AnimationFlag()
{
    if (!InitFlag)
    {
        InitFlag = true;
        CreateFlag();
    }
    wind_frame++;

    Gravity[0] = -0.2f;
    Gravity[1] = 0.f;
    Gravity[2] = 0.f;

    for (int i = 2; i < FLAG_HEIGHT; i++)
    {
        for (int j = 0; j < FLAG_WIDTH; ++j)
        {
            int index = i * FLAG_WIDTH + j;
            physics_vertex* v = &flag_vertex[index];

            v->f[0] = 0.f;
            v->f[1] = 0.f;
            v->f[2] = 0.f;

            VectorAdd(v->f, Gravity, v->f);
            VectorMA(v->f, -Damping, v->v, v->f);

            //v->f[2] += (float)sin(j*0.4f+wind_frame*0.03f)*(j*0.08f);
            if (i < 5)
            {
                if (wind_frame % 128 < 64)
                {
                    v->f[1] += 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
                    v->f[0] -= 1.f;//-(float)sin(i*10.f+(j-3)*1.f+wind_frame*0.06f)*0.5f;
                }
            }
            if (i == 9)
            {
                if (wind_frame % 128 < 64)
                {
                    v->f[1] -= 0.8f - (float)sin(i * 10.f + (j - 3) * 1.f + wind_frame * 0.06f) * 0.8f;
                }
            }

            for (int k = 0; k < v->link_num; k++)
            {
                physics_vertex* v1 = &flag_vertex[index];
                physics_vertex* v2 = &flag_vertex[v->link[k]];

                vec3_t DeltaP;
                VectorSubtract(v1->p, v2->p, DeltaP);
                float dist = VectorLength(DeltaP);
                float Hterm = (dist - FLAG_SCALE) * Ks;

                vec3_t DeltaV;
                VectorSubtract(v1->v, v2->v, DeltaV);
                float Dterm = (DotProduct(DeltaV, DeltaP) * Kd) / dist;

                vec3_t SpringForce;
                VectorScale(DeltaP, 1.f / dist, SpringForce);
                VectorScale(SpringForce, -(Hterm + Dterm), SpringForce);
                VectorAdd(v1->f, SpringForce, v1->f);
                if ((index + 1) / FLAG_WIDTH > (v->link[k] + 1) / FLAG_WIDTH)
                {
                    VectorSubtract(v2->f, SpringForce, v2->f);
                }
            }
        }
    }

    for (int i = 2; i < FLAG_HEIGHT; i++)
    {
        for (int j = 0; j < FLAG_WIDTH; j++)
        {
            int index = i * FLAG_WIDTH + j;
            physics_vertex* v = &flag_vertex[index];
            VectorMA(v->v, DeltaTime, v->f, v->v);
            VectorMA(v->p, DeltaTime, v->v, v->p);
        }
    }
}

void RenderFlagFace(OBJECT* o, int x, int y, vec3_t Light, int Tex1, int Tex2)
{
    int n = 4;
    float su = (float)x / 6.f;
    float sv = (float)y / 9.f;
    float TexCoord[4][2];
    TexCoord[0][0] = su;
    TexCoord[0][1] = sv;
    TexCoord[1][0] = su;
    TexCoord[1][1] = sv + 1.f / 9.f;
    TexCoord[2][0] = su + 1.f / 6.f;
    TexCoord[2][1] = sv + 1.f / 9.f;
    TexCoord[3][0] = su + 1.f / 6.f;
    TexCoord[3][1] = sv;

    float minz = 65536.f; //louis
    physics_face* f = &flag_face[y * (FLAG_WIDTH - 1) + x];

    for (int i = 0; i < n; i++)
    {
        int vlist = f->vlist[i];
        physics_vertex* v = &flag_vertex[vlist];
        v->light = (-v->normal[0] + v->normal[1]) * 0.5f + 0.5f;
    }

    BindTexture(Tex2);
    glBegin(GL_QUADS);

    for (int i = 0; i < n; i++)
    {
        int vlist = f->vlist[i];
        physics_vertex* v = &flag_vertex[vlist];
        glTexCoord2f(TexCoord[i][0], TexCoord[i][1]);
        glColor3f(Light[0] * v->light, Light[1] * v->light, Light[2] * v->light);
        vec3_t p, Position;
        Vector(v->p[0] + 9.f, v->p[1] - 12.f, v->p[2] - 35.f, p);
        Models[o->Type].TransformPosition(o->BoneTransform[19], p, Position, true);
        glVertex3f(Position[0], Position[1], Position[2]);
    }
    glEnd();

    BindTexture(Tex1);
    glBegin(GL_QUADS);

    for (int i = n - 1; i >= 0; i--)
    {
        int vlist = f->vlist[i];
        physics_vertex* v = &flag_vertex[vlist];
        glTexCoord2f(TexCoord[i][0], TexCoord[i][1]);
        glColor3f(Light[0] * v->light, Light[1] * v->light, Light[2] * v->light);
        vec3_t p, Position;
        Vector(v->p[0] + 9.f, v->p[1] - 12.f, v->p[2] - 35.f, p);
        Models[o->Type].TransformPosition(o->BoneTransform[19], p, Position, true);
        glVertex3f(Position[0], Position[1], Position[2]);
    }
    glEnd();
}

void RenderFlag(OBJECT* o, vec3_t Light, int Tex1, int Tex2)
{
    for (int y = 0; y < FLAG_HEIGHT - 1; y++)
    {
        for (int x = 0; x < FLAG_WIDTH - 1; x++)
        {
            physics_face* f = &flag_face[y * (FLAG_WIDTH - 1) + x];
            FaceNormalize(flag_vertex[f->vlist[0]].p,
                flag_vertex[f->vlist[1]].p,
                flag_vertex[f->vlist[2]].p,
                f->normal);
        }
    }
    for (int y = 0; y < FLAG_HEIGHT; y++)
    {
        for (int x = 0; x < FLAG_WIDTH; x++)
        {
            physics_vertex* v = &flag_vertex[y * FLAG_WIDTH + x];
            float num = 0.f;
            if (y < FLAG_HEIGHT - 1 && x < FLAG_WIDTH - 1)
            {
                VectorCopy(flag_face[y * (FLAG_WIDTH - 1) + x].normal, v->normal);
            }
            else
            {
                v->normal[0] = 0.f;
                v->normal[1] = 0.f;
                v->normal[2] = 0.f;
            }
        }
    }

    for (int y = 0; y < FLAG_HEIGHT - 1; y++)
    {
        for (int x = 0; x < FLAG_WIDTH - 1; x++)
        {
            RenderFlagFace(o, x, y, Light, Tex1, Tex2);
        }
    }
}