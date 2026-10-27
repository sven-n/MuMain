#include "stdafx.h"

#include "GameLogic/Pets/CompanionCleanup.h"
#include "GameLogic/Pets/w_BasePet.h"

namespace GameLogic::Pets
{
void RemovePetObjects(std::list<std::shared_ptr<PetObject>>& pets, CHARACTER* owner, int itemType, bool allDelete)
{
    if (owner == nullptr)
        return;

    constexpr int AnyItem = -1;
    for (auto iter = pets.begin(); iter != pets.end();)
    {
        const auto& pet = *iter;
        const bool matches = pet && (itemType == AnyItem ? pet->IsSameOwner(&owner->Object)
                                                         : pet->IsSameObject(&owner->Object, itemType));
        if (!matches)
        {
            ++iter;
            continue;
        }

        pet->Release();
        iter = pets.erase(iter);
        if (!allDelete)
            return;
    }
}
} // namespace GameLogic::Pets
