#include "Engine/Object/AdditionalOptionComparison.h"
#include "Engine/Object/CompareItemResult.h"

namespace Engine::Items
{
int CompareAdditionalOption(bool firstHasIt, int firstValue, bool secondHasIt, int secondValue)
{
    if (firstHasIt < secondHasIt || firstValue < secondValue)
        return CompareResult::SecondIsBetter;
    if (firstHasIt > secondHasIt || firstValue > secondValue)
        return CompareResult::FirstIsBetter;
    return CompareResult::Same;
}
} // namespace Engine::Items
