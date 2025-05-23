//*****************************************************************************
// file    : GM_PK_Field.h
// producer: BGPARK
//*****************************************************************************

#ifndef _GM_PK_FIELD_H_
#define _GM_PK_FIELD_H_

class BMD;

#include "w_BaseMap.h"
#include "./Time/Timer.h"

SmartPointer(CGM_PK_Field);
class CGM_PK_Field : public BaseMap
{
public:
    static CGM_PK_FieldPtr Make();
    virtual ~CGM_PK_Field();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);

public:
    bool CreateFireSpark(PARTICLE* o);

public:
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);

public:
    virtual bool PlayMonsterSound(OBJECT* o);
    virtual void PlayObjectSound(OBJECT* o);
    void PlayBGM();

public:
    void Init();
    void Destroy();

private:
    CGM_PK_Field();
    bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);
};

extern bool IsPKField();

#endif //_GM_PK_FIELD_H_