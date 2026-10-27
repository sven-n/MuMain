#pragma once

#include <list>
#include <memory>

class PetObject;
class CHARACTER;

namespace GameLogic::Pets
{
void RemovePetObjects(std::list<std::shared_ptr<PetObject>>& pets, CHARACTER* owner, int itemType, bool allDelete);
}
