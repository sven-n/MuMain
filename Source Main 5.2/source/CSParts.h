//////////////////////////////////////////////////////////////////////////
//  CSParts.h
//////////////////////////////////////////////////////////////////////////
#ifndef __CSPARTS_H__
#define __CSPARTS_H__

#include "zzzBmd.h"
#include "zzzinfomation.h"
#include "zzzobject.h"
#include "zzzcharacter.h"

class CSIPartsMDL
{
protected:
    OBJECT  m_pObj;
    int     m_iBoneNumber;
    vec3_t  m_vOffset;

public:
    CSIPartsMDL(void) : m_iBoneNumber(-1) {};
    virtual void    IRender(CHARACTER* c) = 0;
    inline OBJECT* GetObject() { return &m_pObj; }
};

class CSParts : public CSIPartsMDL
{
private:

public:
    CSParts(int Type, int BoneNumber, bool bBillBoard = false, float x = 0.f, float y = 0.f, float z = 0.f, float ax = 0.f, float ay = 0.f, float az = 0.f);
    virtual void    IRender(CHARACTER* c);
};

class CSAnimationParts : public CSIPartsMDL
{
private:

public:
    CSAnimationParts(int Type, int BoneNumber, bool bBillBoard = false, float x = 0.f, float y = 0.f, float z = 0.f, float ax = 0.f, float ay = 0.f, float az = 0.f);
    void    Animation(CHARACTER* c);
    virtual void    IRender(CHARACTER* c);
};

class CSParts2D : public CSIPartsMDL
{
public:
    CSParts2D(int Type, int SubType, int BoneNumber, float x = 0.f, float y = 0.f, float z = 0.f);
    virtual void    IRender(CHARACTER* c);
};

void    CreatePartsFactory(CHARACTER* c);
void    RenderParts(CHARACTER* c);
void    DeleteParts(CHARACTER* c);

#endif// __CSPARTS_H__
