//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <memory>

#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"

class CSPetSystem
{
protected:
    CHARACTER* m_PetOwner;
    CHARACTER* m_PetTarget;
    CHARACTER m_PetCharacter;
    PET_TYPE m_PetType;
    PET_INFO* m_pPetInfo;
    std::uint8_t m_byCommand;
    std::unique_ptr<vec34_t[]> m_BoneTransforms;

public:
    CSPetSystem();
    virtual ~CSPetSystem();

    PET_TYPE GetPetType(void)
    {
        return m_PetType;
    }
    void SetPetInfo(PET_INFO* pPetInfo)
    {
        m_pPetInfo = pPetInfo;
    };

    virtual void MovePet(void) = 0;
    virtual void CalcPetInformation(const PET_INFO& Petinfo) = 0;
    // Marks whether the frame's view shows the pet; part of the character prepare, before the draw.
    virtual void CullPet() {}
    virtual void RenderPet(int PetState = 0) const = 0;
    // The effects and state changes of the pet's draw, for a pet the last frame drew.
    virtual void UpdatePetEffects() = 0;

    virtual void Eff_LevelUp(void) = 0;
    virtual void Eff_LevelDown(void) = 0;

    void CreatePetPointer(int Type, unsigned char PositionX, unsigned char PositionY, float Rotation);
    bool PlayAnimation(OBJECT* o);

    void MoveInventory(void);
    void RenderInventory(void);

    void SetAI(int AI);
    void SetCommand(int Key, std::uint8_t cmd);
    void SetAttack(int Key, int attackType);

    int GetObjectType()
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

    void MovePet(void) override;
    void CalcPetInformation(const PET_INFO& Petinfo) override;
    void CullPet() override;
    void RenderPet(int PetState = 0) const override;
    void UpdatePetEffects() override;

    void Eff_LevelUp(void) override;
    void Eff_LevelDown(void) override;

    void AttackEffect(CHARACTER* c, OBJECT* o);
};
