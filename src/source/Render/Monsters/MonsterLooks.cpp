#include "stdafx.h"

#include "Render/Monsters/MonsterLooks.h"

#include "Data/GameData/LookData/LookStore.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "Engine/Object/w_CharacterInfo.h"
#include "Engine/Object/w_ObjectInfo.h"

#include <limits>
#include <vector>

namespace Render::Monsters::Looks
{
namespace
{
// The names by number; number 0 is no look.
std::vector<std::string>& Names()
{
    static std::vector<std::string> names{std::string()};
    return names;
}

// The looks of each number as g_LookStore had them at `version`: found again when the looks change.
struct Found
{
    int version = -1;
    std::vector<const Data::Looks::LookDefinition*> renderStyles;
    std::vector<const Data::Looks::LookDefinition*> effects;
};

Found& Current()
{
    static Found found;
    const int version = Data::Looks::g_LookStore.GetVersion();
    const size_t count = Names().size();
    if (found.version != version || found.renderStyles.size() != count)
    {
        found.version = version;
        found.renderStyles.assign(count, nullptr);
        found.effects.assign(count, nullptr);
        for (size_t id = 1; id < count; ++id)
        {
            found.renderStyles[id] = Data::Looks::g_LookStore.FindRenderStyle(Names()[id]);
            found.effects[id] = Data::Looks::g_LookStore.FindEffect(Names()[id]);
        }
    }
    return found;
}

std::string_view ModelName(int modelSlot, std::string Data::Monsters::MonsterModelDefinition::* field)
{
    const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(modelSlot);
    return model != nullptr ? std::string_view(model->*field) : std::string_view();
}
} // namespace

LookId IdOf(std::string_view name)
{
    if (name.empty() || name == NoneName)
        return NoLook;
    std::vector<std::string>& names = Names();
    for (size_t id = 1; id < names.size(); ++id)
    {
        if (names[id] == name)
            return static_cast<LookId>(id);
    }
    if (names.size() > std::numeric_limits<LookId>::max())
        return NoLook;
    names.emplace_back(name);
    return static_cast<LookId>(names.size() - 1);
}

const std::string& NameOf(LookId id)
{
    const std::vector<std::string>& names = Names();
    return id < names.size() ? names[id] : names[NoLook];
}

const Data::Looks::LookDefinition* RenderStyleOf(LookId id)
{
    if (id == NoLook)
        return nullptr;
    const Found& found = Current();
    return id < found.renderStyles.size() ? found.renderStyles[id] : nullptr;
}

const Data::Looks::LookDefinition* EffectOf(LookId id)
{
    if (id == NoLook)
        return nullptr;
    const Found& found = Current();
    return id < found.effects.size() ? found.effects[id] : nullptr;
}

std::string_view RenderStyleName(const Data::Monsters::MonsterSetup& setup, int modelSlot)
{
    return setup.renderStyle ? std::string_view(*setup.renderStyle)
                             : ModelName(modelSlot, &Data::Monsters::MonsterModelDefinition::renderStyle);
}

std::string_view EffectName(const Data::Monsters::MonsterSetup& setup, int modelSlot)
{
    return setup.effect ? std::string_view(*setup.effect)
                        : ModelName(modelSlot, &Data::Monsters::MonsterModelDefinition::effect);
}

void Resolve(CHARACTER& character, const Data::Monsters::MonsterSetup& setup)
{
    OBJECT& object = character.Object;
    object.RenderStyleLook = IdOf(RenderStyleName(setup, object.Type));
    object.EffectLook = IdOf(EffectName(setup, object.Type));
}

const Data::Looks::LookDefinition* RenderStyleOf(const OBJECT& object)
{
    return RenderStyleOf(object.RenderStyleLook);
}

const Data::Looks::LookDefinition* EffectOf(const OBJECT& object)
{
    return EffectOf(object.EffectLook);
}
} // namespace Render::Monsters::Looks
