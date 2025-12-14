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
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSclient.h"
#include "Random.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
constexpr int MAX_BLURS = 100;
constexpr int MAX_BLUR_TAILS = 30;
constexpr int MAX_BLUR_LIFETIME = 30;

constexpr int MAX_OBJECT_BLURS = 1000;
constexpr int MAX_OBJECT_BLUR_TAILS = 600;
constexpr int MAX_OBJECT_BLUR_LIFETIME = 30;

constexpr int FLAG_WIDTH = 7;
constexpr int FLAG_HEIGHT = 10;
constexpr float FLAG_SCALE = 10.f;

struct Blur
{
    bool Live = false;
    int Type = 0;
    int LifeTime = 0;
    CHARACTER* Owner = nullptr;
    int Number = 0;
    vec3_t Light{};
    std::array<vec3_t, MAX_BLUR_TAILS> P1{};
    std::array<vec3_t, MAX_BLUR_TAILS> P2{};
    int SubType = 0;
};

struct ObjectBlur
{
    bool Live = false;
    int Type = 0;
    int LifeTime = 0;
    OBJECT* Owner = nullptr;
    int Number = 0;
    vec3_t Light{};
    int LimitLifeTime = 0;
    std::array<vec3_t, MAX_OBJECT_BLUR_TAILS> P1{};
    std::array<vec3_t, MAX_OBJECT_BLUR_TAILS> P2{};
    int SubType = 0;
};

struct physics_vertex
{
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
};

struct physics_face
{
    int vlist[4];
    vec3_t normal;
};

struct physics_boundbox
{
    physics_vertex vtx[8];
};

std::array<Blur, MAX_BLURS> g_blurs{};
std::array<ObjectBlur, MAX_OBJECT_BLURS> g_objectBlurs{};
std::array<physics_vertex, FLAG_HEIGHT * FLAG_WIDTH> g_flagVertices{};
std::array<physics_face, (FLAG_HEIGHT - 1) * (FLAG_WIDTH - 1)> g_flagFaces{};

} // namespace

void AddBlur(Blur* b, vec3_t p1, vec3_t p2, vec3_t Light, int Type)
{
    b->Type = Type;
    VectorCopy(Light, b->Light);
    for (int i = b->Number - 1; i >= 0; --i)
    {
        VectorCopy(b->P1[i], b->P1[i + 1]);
        VectorCopy(b->P2[i], b->P2[i + 1]);
    }
    VectorCopy(p1, b->P1[0]);
    VectorCopy(p2, b->P2[0]);
    b->Number = std::min<int>(b->Number + 1, MAX_BLUR_TAILS - 1);
}

void CreateBlur(CHARACTER* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short, int SubType)
{
    Blur* freeBlur = nullptr;
    for (auto& blur : g_blurs)
    {
        if (blur.Live)
        {
            if (blur.Owner == Owner)
            {
                if (SubType > 0 && blur.SubType != SubType)
                {
                    continue;
                }
                AddBlur(&blur, p1, p2, Light, Type);
                return;
            }
        }
        else if (freeBlur == nullptr)
        {
            freeBlur = &blur;
        }
    }

    if (freeBlur != nullptr)
    {
        freeBlur->Live = true;
        freeBlur->Owner = Owner;
        freeBlur->Number = 0;
        freeBlur->LifeTime = Short ? 15 : MAX_BLUR_LIFETIME;
        freeBlur->SubType = SubType;
        AddBlur(freeBlur, p1, p2, Light, Type);
    }
}

void MoveBlurs()
{
    for (auto& blur : g_blurs)
    {
        if (!blur.Live)
        {
            continue;
        }

        blur.LifeTime--;
        blur.Number = std::max<int>(blur.Number - 1, 0);

        for (int i = blur.Number - 1; i >= 0; --i)
        {
            VectorCopy(blur.P1[i], blur.P1[i + 1]);
            VectorCopy(blur.P2[i], blur.P2[i + 1]);
        }

        if (blur.LifeTime <= 0)
        {
            blur.Number = std::max<int>(blur.Number - 1, 0);
            if (blur.Number <= 0)
            {
                blur.Live = false;
            }
        }
    }
    MoveObjectBlurs();
}

void RenderBlurs()
{
    int Type;
    for (auto& blur : g_blurs)
    {
        if (!blur.Live)
        {
            continue;
        }

        Type = blur.Type;

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
        if (blur.Owner->Level == 0 && (Type <= 3 || (Type >= 5 && Type <= 10)))
        {
            EnableAlphaBlend();
        }
        else
        {
            EnableAlphaBlendMinus();
        }

        if (Type > 2)
        {
            Type = Type - 3;
        }
        if (blur.Number >= 2)
        {
            BindTexture(nTexture);
            for (int j = 0; j < blur.Number - 1; j++)
            {
                glBegin(GL_TRIANGLE_FAN);
                float Light;
                float TexU;
                if (blur.Owner->Level == 0)
                    Light = (blur.Number - j) / static_cast<float>(blur.Number);
                else
                    Light = 1.f;
                glColor3f(blur.Light[0] * Light, blur.Light[1] * Light, blur.Light[2] * Light);
                TexU = (j) / static_cast<float>(blur.Number);
                glTexCoord2f(TexU, 1.f);
                glVertex3fv(blur.P1[j]);
                glTexCoord2f(TexU, 0.f);
                glVertex3fv(blur.P2[j]);
                if (blur.Owner->Level == 0)
                    Light = (blur.Number - (j + 1)) / static_cast<float>(blur.Number);
                else
                    Light = 1.f;
                glColor3f(blur.Light[0] * Light, blur.Light[1] * Light, blur.Light[2] * Light);
                TexU = (j + 1) / static_cast<float>(blur.Number);
                glTexCoord2f(TexU, 0.f);
                glVertex3fv(blur.P2[j + 1]);
                glTexCoord2f(TexU, 1.f);
                glVertex3fv(blur.P1[j + 1]);
                glEnd();
            }
        }
    }
    RenderObjectBlurs();
}

void ClearAllObjectBlurs()
{
    for (auto& blur : g_objectBlurs)
    {
        blur.Live = false;
    }
}

void AddObjectBlur(ObjectBlur* b, vec3_t p1, vec3_t p2, vec3_t Light, int Type)
{
    b->Type = Type;
    VectorCopy(Light, b->Light);

    for (int i = b->Number - 1; i >= 0; --i)
    {
        VectorCopy(b->P1[i], b->P1[i + 1]);
        VectorCopy(b->P2[i], b->P2[i + 1]);
    }
    VectorCopy(p1, b->P1[0]);
    VectorCopy(p2, b->P2[0]);
    b->Number = std::min<int>(b->Number + 1, MAX_OBJECT_BLUR_TAILS - 1);
}

void CreateObjectBlur(OBJECT* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short, int SubType, int iLimitLifeTime)
{
    ObjectBlur* freeBlur = nullptr;
    for (auto& blur : g_objectBlurs)
    {
        if (blur.Live)
        {
            if (blur.Owner == Owner)
            {
                if (SubType > 0 && blur.SubType != SubType)
                {
                    continue;
                }
                AddObjectBlur(&blur, p1, p2, Light, Type);
                return;
            }
        }
        else if (freeBlur == nullptr)
        {
            freeBlur = &blur;
        }
    }

    if (freeBlur != nullptr)
    {
        freeBlur->Live = true;
        freeBlur->Owner = Owner;
        freeBlur->Number = 0;
        if (iLimitLifeTime > -1)
        {
            freeBlur->LifeTime = iLimitLifeTime;
            freeBlur->LimitLifeTime = iLimitLifeTime;
        }
        else
        {
            freeBlur->LimitLifeTime = Short ? 15 : MAX_OBJECT_BLUR_LIFETIME;
            freeBlur->LifeTime = freeBlur->LimitLifeTime;
        }
        freeBlur->SubType = SubType;

        AddObjectBlur(freeBlur, p1, p2, Light, Type);
    }
}

void MoveObjectBlurs()
{
    for (auto& blur : g_objectBlurs)
    {
        if (!blur.Live)
        {
            continue;
        }

        blur.LifeTime--;
        blur.Number = std::max<int>(blur.Number - 1, 0);

        if (blur.LifeTime <= 0)
        {
            blur.Number = 0;
            blur.Live = false;
            continue;
        }

        for (int i = blur.Number - 1; i >= 0; --i)
        {
            VectorCopy(blur.P1[i], blur.P1[i + 1]);
            VectorCopy(blur.P2[i], blur.P2[i + 1]);
        }
    }
}

void RenderObjectBlurs()
{
    int Type;
    for (auto& blur : g_objectBlurs)
    {
        if (!blur.Live)
        {
            continue;
        }

        Type = blur.Type;
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

        if (Type > 2)
        {
            Type = Type - 3;
        }
        if (blur.Number >= 2)
        {
            BindTexture(nTexture);
            for (int j = 0; j < blur.Number - 1; j++)
            {
                const float Data = 300.f;
                if (blur.SubType == 113 || blur.SubType == 114)
                {
                    if (std::fabs(blur.P1[j][0] - blur.P1[j + 1][0]) > Data || std::fabs(blur.P1[j][1] - blur.P1[j + 1][1]) > Data || std::fabs(blur.P1[j][2] - blur.P1[j + 1][2]) > Data ||
                        std::fabs(blur.P1[j][0] - blur.P2[j + 1][0]) > Data || std::fabs(blur.P1[j][1] - blur.P2[j + 1][1]) > Data || std::fabs(blur.P1[j][2] - blur.P2[j + 1][2]) > Data)
                    {
                        continue;
                    }
                }

                glBegin(GL_TRIANGLE_FAN);
                float Light = (blur.Number - j) / static_cast<float>(blur.Number);
                float TexU = (j) / static_cast<float>(blur.Number);
                glColor3f(blur.Light[0] * Light, blur.Light[1] * Light, blur.Light[2] * Light);
                glTexCoord2f(TexU, 1.f);
                glVertex3fv(blur.P1[j]);
                glTexCoord2f(TexU, 0.f);
                glVertex3fv(blur.P2[j]);
                Light = (blur.Number - (j + 1)) / static_cast<float>(blur.Number);
                glColor3f(blur.Light[0] * Light, blur.Light[1] * Light, blur.Light[2] * Light);
                TexU = (j + 1) / static_cast<float>(blur.Number);
                glTexCoord2f(TexU, 0.f);
                glVertex3fv(blur.P2[j + 1]);
                glTexCoord2f(TexU, 1.f);
                glVertex3fv(blur.P1[j + 1]);
                glEnd();
            }
        }
    }
}

void RemoveObjectBlurs(OBJECT* Owner, int SubType)
{
    for (auto& blur : g_objectBlurs)
    {
        if (blur.Live && blur.Owner == Owner)
        {
            if (SubType > 0 && blur.SubType != SubType)
            {
                continue;
            }
            blur.Live = false;
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
        Vector(Random::RangeFloat(0.f, 360.f), 0.f, Random::RangeFloat(0.f, 360.f), a);
        VectorAdd(a, Angle, a);
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
                Vector(Random::RangeFloat(-50.f, 50.f), Random::RangeFloat(-50.f, 50.f), 0.f, p);
                Models[o->Type].TransformPosition(o->BoneTransform[BoneHead], p, Position, true);
                const float rotation = Random::RangeFloat(0.f, 360.f);
                const float scale = Random::RangeFloat(0.8f, 1.2f);
                CreatePointer(BITMAP_BLOOD, Position, rotation, o->Light, scale);
            }
        }
    }
}

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
            physics_vertex* v = &g_flagVertices[index];
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
            physics_face* f = &g_flagFaces[i * (FLAG_WIDTH - 1) + j];
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
            physics_vertex* v = &g_flagVertices[index];

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
                physics_vertex* v1 = &g_flagVertices[index];
                physics_vertex* v2 = &g_flagVertices[v->link[k]];

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
            physics_vertex* v = &g_flagVertices[index];
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
    physics_face* f = &g_flagFaces[y * (FLAG_WIDTH - 1) + x];

    for (int i = 0; i < n; i++)
    {
        int vlist = f->vlist[i];
        physics_vertex* v = &g_flagVertices[vlist];
        v->light = (-v->normal[0] + v->normal[1]) * 0.5f + 0.5f;
    }

    BindTexture(Tex2);
    glBegin(GL_QUADS);

    for (int i = 0; i < n; i++)
    {
        int vlist = f->vlist[i];
        physics_vertex* v = &g_flagVertices[vlist];
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
        physics_vertex* v = &g_flagVertices[vlist];
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
            physics_face* f = &g_flagFaces[y * (FLAG_WIDTH - 1) + x];
            FaceNormalize(g_flagVertices[f->vlist[0]].p,
                g_flagVertices[f->vlist[1]].p,
                g_flagVertices[f->vlist[2]].p,
                f->normal);
        }
    }
    for (int y = 0; y < FLAG_HEIGHT; y++)
    {
        for (int x = 0; x < FLAG_WIDTH; x++)
        {
            physics_vertex* v = &g_flagVertices[y * FLAG_WIDTH + x];
            float num = 0.f;
            if (y < FLAG_HEIGHT - 1 && x < FLAG_WIDTH - 1)
            {
                VectorCopy(g_flagFaces[y * (FLAG_WIDTH - 1) + x].normal, v->normal);
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