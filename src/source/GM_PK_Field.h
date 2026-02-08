//*****************************************************************************
// file    : GM_PK_Field.h
// producer: BGPARK
//*****************************************************************************

#pragma once

#include <memory>

#include "w_BaseMap.h"
#include "_struct.h"

class BMD;
struct OBJECT;
struct CHARACTER;

class CGM_PK_Field;
using CGM_PK_FieldPtr = std::shared_ptr<CGM_PK_Field>;

class CGM_PK_Field final : public BaseMap
{
public:
    static CGM_PK_FieldPtr Make();
    ~CGM_PK_Field() override;

    bool CreateObject(OBJECT* object) override;
    bool MoveObject(OBJECT* object) override;
    bool RenderObjectVisual(OBJECT* object, BMD* bmd) override;
    bool RenderObjectMesh(OBJECT* object, BMD* bmd, bool extraMon = false) override;
    void RenderAfterObjectMesh(OBJECT* object, BMD* bmd, bool extraMon = false) override;

    CHARACTER* CreateMonster(int type, int positionX, int positionY, int key) override;
    bool MoveMonsterVisual(OBJECT* object, BMD* bmd) override;
    void MoveBlurEffect(CHARACTER* character, OBJECT* object, BMD* bmd) override;
    bool RenderMonsterVisual(CHARACTER* character, OBJECT* object, BMD* bmd) override;

    bool PlayMonsterSound(OBJECT* object) override;
    void PlayObjectSound(OBJECT* object) override;

    void PlayBGM();
    bool CreateFireSpark(PARTICLE* particle);

    void Init();
    void Destroy();

private:
    CGM_PK_Field();

    bool RenderMonster(OBJECT* object, BMD* bmd, bool extraMon);
};

bool IsPKField();
