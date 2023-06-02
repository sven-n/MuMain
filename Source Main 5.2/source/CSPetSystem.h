//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

class CSPetSystem
{
protected:
    CHARACTER* m_PetOwner;
    CHARACTER* m_PetTarget;
    CHARACTER   m_PetCharacter;
    PET_TYPE    m_PetType;
    PET_INFO* m_pPetInfo;
    BYTE        m_byCommand;

public:
    CSPetSystem() {};
    virtual ~CSPetSystem();

    PET_TYPE    GetPetType(void) { return m_PetType; }
    void		SetPetInfo(PET_INFO* pPetInfo) { m_pPetInfo = pPetInfo; };

    virtual void    MovePet(void) = 0;
    virtual void	CalcPetInformation(const PET_INFO& Petinfo) = 0;
    virtual void    RenderPetInventory(void) = 0;
    virtual void    RenderPet(int PetState = 0) = 0;

    virtual void    Eff_LevelUp(void) = 0;
    virtual void    Eff_LevelDown(void) = 0;

    void    CreatePetPointer(int Type, unsigned char PositionX, unsigned char PositionY, float Rotation);
    bool    PlayAnimation(OBJECT* o);

    void    MoveInventory(void);
    void    RenderInventory(void);

    void    SetAI(int AI);
    void    SetCommand(int Key, BYTE cmd);
    void    SetAttack(int Key, int attackType);

    int		GetObjectType()
    {
        return m_PetCharacter.Object.Type;
    }
};

class CSPetDarkSpirit : public CSPetSystem
{
private:

public:
    CSPetDarkSpirit(CHARACTER* c);
    virtual ~CSPetDarkSpirit(void);

    virtual void MovePet(void);
    virtual void CalcPetInformation(const PET_INFO& Petinfo);
    virtual void RenderPetInventory(void);
    virtual void RenderPet(int PetState = 0);

    virtual void Eff_LevelUp(void);
    virtual void Eff_LevelDown(void);

    void    AttackEffect(CHARACTER* c, OBJECT* o);
    void    RenderCmdType(void);
};
