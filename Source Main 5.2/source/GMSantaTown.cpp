// GMSantaTown.cpp: implementation of the CGMSantaTown class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"



CGMSantaTownPtr CGMSantaTown::Make()
{
    CGMSantaTownPtr santatown(new CGMSantaTown);
    santatown->Init();
    return santatown;
}

CGMSantaTown::CGMSantaTown()
{
}

CGMSantaTown::~CGMSantaTown()
{
    Destroy();
}

void CGMSantaTown::Init()
{
}

void CGMSantaTown::Destroy()
{
}

bool CGMSantaTown::CreateObject(OBJECT* o)
{
    switch (o->Type)
    {
    case 21: case 18:
    case 19: case 12: case 13:
    case 25:
        o->CollisionRange = -300;
        return true;
    }

    return false;
}

CHARACTER* CGMSantaTown::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case 465:
        OpenNpc(MODEL_XMAS2008_SANTA_NPC);
        pCharacter = CreateCharacter(Key, MODEL_XMAS2008_SANTA_NPC, PosX, PosY);
        pCharacter->Object.Scale = 1.7f;
        break;
    case 467:
        OpenNpc(MODEL_XMAS2008_SNOWMAN_NPC);
        pCharacter = CreateCharacter(Key, MODEL_XMAS2008_SNOWMAN_NPC, PosX, PosY);
        pCharacter->Object.Scale = 1.3f;
        break;
    }

    return pCharacter;
}

bool CGMSantaTown::MoveObject(OBJECT* o)
{
    if (IsSantaTown() == false)
        return false;

    switch (o->Type)
    {
    case 16:
    {
        o->Velocity = 0.06f;
        return true;
    }
    break;
    case 26:
    case 27:
    case 28:
    {
        o->HiddenMesh = -2;
        return true;
    }
    break;
    }

    return false;
}

bool CGMSantaTown::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (IsSantaTown() == false)
        return false;

    // 	switch(o->Type)
    // 	{
    // 	}

    return false;
}

void CGMSantaTown::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    // 	switch(o->Type)
    // 	{
    // 	}
}

bool CGMSantaTown::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsSantaTown() == false)
        return false;

    switch (o->Type)
    {
    case 16:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.0f, 0.5f, 0.5f, b->BodyLight);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        return true;
    }
    }

    return false;
}

bool CGMSantaTown::RenderObjectVisual(OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case 26:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.005f) + 1.0f) * 0.1f + 0.9f;
        vec3_t vLightFire;
        Vector(fLumi * 0.8f, fLumi * 0.2f, fLumi * 0.0f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 8.0f * o->Scale, vLightFire, o);
        return true;
    }
    break;
    case 27:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.005f) + 1.0f) * 0.1f + 0.9f;
        vec3_t vLightFire;
        Vector(fLumi * 0.0f, fLumi * 0.2f, fLumi * 0.8f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 8.0f * o->Scale, vLightFire, o);
        return true;
    }
    break;
    case 28:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.005f) + 1.0f) * 0.1f + 0.9f;
        vec3_t vLightFire;
        Vector(fLumi * 0.0f, fLumi * 0.6f, fLumi * 0.6f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 8.0f * o->Scale, vLightFire, o);
        return true;
    }
    break;
    }

    return false;
}

bool CGMSantaTown::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    // 	vec3_t vPos, vRelative, vLight;
    //
    // 	switch(o->Type)
    // 	{
    // 	}

    return true;
}

void CGMSantaTown::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
}

bool CGMSantaTown::CreateSnow(PARTICLE* o)
{
    if (IsSantaTown() == false)
        return false;

    o->Type = BITMAP_LEAF1;
    o->Scale = (float)(rand() % 10 + 5);
    if (rand_fps_check(10))
    {
        o->Type = BITMAP_LEAF2;
        o->Scale = 12.f;
    }
    Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
        Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
        Hero->Object.Position[2] + (float)(rand() % 200 + 200),
        o->Position);
    Vector(-20.f, 0.f, 0.f, o->Angle);
    vec3_t Velocity;
    Vector(0.f, 0.f, -(float)(rand() % 8 + 4), Velocity);
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    VectorRotate(Velocity, Matrix, o->Velocity);

    return true;
}

bool IsSantaTown()
{
    if (gMapManager.WorldActive == WD_62SANTA_TOWN)
    {
        return true;
    }

    return false;
}

bool CGMSantaTown::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (IsSantaTown() == false)
        return false;

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

bool CGMSantaTown::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (IsSantaTown() == false)
        return false;

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

bool CGMSantaTown::PlayMonsterSound(OBJECT* o)
{
    if (IsSantaTown() == false)
        return false;

    // 	float fDis_x, fDis_y;
    // 	fDis_x = o->Position[0] - Hero->Object.Position[0];
    // 	fDis_y = o->Position[1] - Hero->Object.Position[1];
    // 	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
    //
    // 	if (fDistance > 500.0f)
    // 		return true;
    //
    // 	switch(o->Type)
    // 	{
    // 	}

    return false;
}

void CGMSantaTown::PlayObjectSound(OBJECT* o)
{
}

void CGMSantaTown::PlayBGM()
{
    if (IsSantaTown())
    {
        PlayMp3(MUSIC_SANTA_TOWN);
    }
}