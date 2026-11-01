#include "stdafx.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Core/Globals/_enum.h"

const wchar_t* Engine::Object::PlayerBodyModelDirectory(int classOffset, bool useS6PlayerModels)
{
    if (!useS6PlayerModels && classOffset >= 0 && classOffset < MAX_CLASS * MAX_CLASS_STAGES)
    {
        const int baseClass = classOffset % MAX_CLASS;
        const int stage = classOffset / MAX_CLASS;
        if (baseClass <= CLASS_DARK_LORD && (stage != 1 || baseClass <= CLASS_ELF))
            return L"Data\\Player\\Updated\\";
    }
    return L"Data\\Player\\";
}
