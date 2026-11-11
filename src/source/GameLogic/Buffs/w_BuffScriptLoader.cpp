#include "stdafx.h"
#include "w_BuffScriptLoader.h"
#include "Data/GameData/BuffData/BuffDatabase.h"

namespace
{
BuffInfo MakeBuffInfo(const Data::Buffs::BuffDefinition& buff, const Data::Buffs::BuffDisplayTexts& texts)
{
    BuffInfo info;
    info.s_BuffIndex = static_cast<short>(buff.number);
    info.s_BuffEffectType = static_cast<BYTE>(buff.group);
    if (buff.item)
    {
        info.s_ItemType = static_cast<short>(buff.item->group);
        info.s_ItemIndex = static_cast<short>(buff.item->number);
    }
    info.s_BuffClassType =
        static_cast<BYTE>(buff.kind == Data::Buffs::BuffKind::Debuff ? eBuffClass_DeBuff : eBuffClass_Buff);
    wcsncpy(info.s_BuffName, texts.name.c_str(), MAX_BUFF_NAME_LENGTH - 1);
    info.s_BuffName[MAX_BUFF_NAME_LENGTH - 1] = L'\0';
    info.s_BuffDescriptlist.assign(texts.descriptionLines.begin(), texts.descriptionLines.end());
    return info;
}
} // namespace

BuffInfo::BuffInfo() : s_BuffIndex(0), s_BuffEffectType(0), s_ItemType(NoItem), s_ItemIndex(NoItem), s_BuffClassType(0)
{
    memset(&s_BuffName, 0, sizeof(wchar_t) * MAX_BUFF_NAME_LENGTH);
}

BuffInfo::~BuffInfo() {}

BuffScriptLoaderPtr BuffScriptLoader::Make()
{
    BuffScriptLoaderPtr info(new BuffScriptLoader());
    return info;
}

BuffScriptLoader::BuffScriptLoader() {}

BuffScriptLoader::~BuffScriptLoader() {}

const BuffInfo BuffScriptLoader::GetBuffinfo(eBuffState type) const
{
    const Data::Buffs::BuffDefinition* buff = type < eBuff_Count ? g_BuffDatabase.Find(type) : nullptr;
    const Data::Buffs::BuffDisplayTexts* texts = type < eBuff_Count ? g_BuffDatabase.FindTexts(type) : nullptr;
    return buff != nullptr && texts != nullptr ? MakeBuffInfo(*buff, *texts) : BuffInfo();
}

eBuffClass BuffScriptLoader::IsBuffClass(eBuffState type) const
{
    const Data::Buffs::BuffDefinition* buff = type < eBuff_Count ? g_BuffDatabase.Find(type) : nullptr;
    if (buff == nullptr)
    {
        return eBuffClass_Count;
    }
    return buff->kind == Data::Buffs::BuffKind::Debuff ? eBuffClass_DeBuff : eBuffClass_Buff;
}

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
namespace
{
// The buff the item gives, or nullptr. Of several buffs the item gives (13/43 gives 29 and 40), the lowest number, as
// the old buff info (by number) had it, whatever the file's order.
const Data::Buffs::BuffDefinition* FindItemBuff(int iItemCode)
{
    const Data::Buffs::BuffDefinition* found = nullptr;
    for (const Data::Buffs::BuffDefinition& buff : g_BuffDatabase.GetAll())
    {
        if (buff.number < eBuff_Count && buff.item && ITEMINDEX(buff.item->group, buff.item->number) == iItemCode &&
            (found == nullptr || buff.number < found->number))
        {
            found = &buff;
        }
    }
    return found;
}
} // namespace

int BuffScriptLoader::GetBuffIndex(int iItemCode)
{
    const Data::Buffs::BuffDefinition* buff = FindItemBuff(iItemCode);
    return buff != nullptr ? buff->number : -1;
}

int BuffScriptLoader::GetBuffType(int iItemCode)
{
    const Data::Buffs::BuffDefinition* buff = FindItemBuff(iItemCode);
    return buff != nullptr ? buff->group : -1;
}
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
