#include "stdafx.h"

#include "NpcTalk.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "Engine/Object/w_CharacterInfo.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

#include <algorithm>
#include <array>

namespace GameLogic::Npcs
{
namespace
{
using Data::Monsters::MonsterTalkBlock;
using Data::Monsters::MonsterTalkDialog;
using Data::Monsters::MonsterTalkOk;

std::wstring TextOf(const Data::LocalizedString& text, const TalkWorld& world)
{
    return Core::Text::FromUtf8(text.Get(world.locale()));
}

// The talk request, as the map lets it: Kanturu 2nd's entrance and the
// Illusion Temple's NPCs have rules of their own.
void SendTalk(CHARACTER& npc, const TalkWorld& world)
{
    if (world.isKanturu2nd())
        world.kanturu2ndTalk(npc);
    else if (world.isCursedTemple())
        world.cursedTempleTalk(npc);
    else
        world.sendTalk(npc.Key);
}

// The OK of a dialog: its quest in one of the states, then the zen.
void RunOk(const MonsterTalkOk& ok, const TalkWorld& world)
{
    if (!ok.quest.empty())
    {
        const std::optional<int> quest = Data::Monsters::FindTalkName(Data::Monsters::GetTalkQuests(), ok.quest);
        const int state = quest ? world.questState(*quest) : -1;
        const bool inState = std::ranges::any_of(
            ok.questStates, [&](const std::string& name)
            { return Data::Monsters::FindTalkName(Data::Monsters::GetTalkQuestStates(), name) == state; });
        // The OK did nothing in another state, as the old disabled button.
        if (!inState)
            return;
    }
    if (world.gold() < ok.zen)
    {
        if (!ok.failure.IsEmpty())
            world.errorMessage(TextOf(ok.failure, world));
        return;
    }
    if (ok.request == "enterRefuge")
        world.sendEnterRefuge();
    else if (ok.request == "enterBarracks")
        world.sendEnterBarracks();
}

mu::ui::window::GenericDialogConfig DialogOf(const MonsterTalkDialog& dialog, const TalkWorld& world)
{
    mu::ui::window::GenericDialogConfig config;
    if (!dialog.title.IsEmpty())
        config.lines.push_back({TextOf(dialog.title, world), true});
    for (const Data::LocalizedString& line : dialog.lines)
        config.lines.push_back({TextOf(line, world), false});
    if (dialog.ok)
        config.onPrimary = [ok = *dialog.ok, &world] { RunOk(ok, world); };
    return config;
}

void RunBlock(const MonsterTalkBlock& block, CHARACTER& npc, const TalkWorld& world)
{
    switch (block.kind)
    {
    case MonsterTalkBlock::Kind::Dialog:
        world.showDialog(DialogOf(block.dialog, world));
        break;
    case MonsterTalkBlock::Kind::SendTalk:
        SendTalk(npc, world);
        break;
    case MonsterTalkBlock::Kind::SendPetInfo:
        if (world.hasDarkHorse())
            world.sendDarkHorseInfo();
        if (world.hasDarkRaven())
            world.sendDarkRavenInfo();
        break;
    case MonsterTalkBlock::Kind::Message:
        world.systemMessage(TextOf(block.message, world));
        break;
    case MonsterTalkBlock::Kind::QuestWindow:
        world.setNoQuestTitleFor(block.noQuestTitleFor);
        break;
    case MonsterTalkBlock::Kind::CrywolfAltarContract:
        // The step on an NPC that is no altar (the data allows it, Data
        // Checks warns): it talks.
        if (npc.Object.Type < MODEL_CRYWOLF_ALTAR1 || npc.Object.Type > MODEL_CRYWOLF_ALTAR5)
        {
            SendTalk(npc, world);
            break;
        }
        // Its contract while only elves may make it, nothing else on
        // Crywolf, the talk request elsewhere.
        if (world.isCrywolfElfTime())
        {
            // The list's reader and Data Checks report a set that is not
            // there; it shows empty texts then.
            const Data::Monsters::MonsterAltarContract* found = world.findTalkTexts(block.altarTexts);
            static const Data::Monsters::MonsterAltarContract None;
            const Data::Monsters::MonsterAltarContract& texts = found != nullptr ? *found : None;
            world.crywolfAltarTalk(npc, {TextOf(texts.canContract, world), TextOf(texts.proceed, world),
                                         TextOf(texts.onMount, world), TextOf(texts.needGuardian, world),
                                         TextOf(texts.destroyed, world), TextOf(texts.ongoing, world)});
        }
        else if (!world.isCrywolfMap())
        {
            SendTalk(npc, world);
        }
        break;
    }
}
} // namespace

std::wstring WithNumber(std::wstring text, int number)
{
    const std::wstring shown = std::to_wstring(number);
    for (size_t at = text.find(L"%d"); at != std::wstring::npos; at = text.find(L"%d", at + shown.size()))
        text.replace(at, 2, shown);
    return text;
}

bool Talk(CHARACTER& npc, const TalkWorld& world)
{
    const Data::Monsters::MonsterSetup setup = world.talkSetup(npc);
    if (!setup.talks.value_or(true))
        return true;

    if (setup.talkLevel && world.heroLevel() < setup.talkLevel->level)
    {
        world.systemMessage(WithNumber(TextOf(setup.talkLevel->message, world), setup.talkLevel->level));
        return false;
    }

    world.setRepairShop(setup.repairShop.value_or(false));
    world.beforeTalk();
    world.setNoQuestTitleFor({});

    if (setup.onTalk.empty())
    {
        SendTalk(npc, world);
        return true;
    }
    for (const MonsterTalkBlock& block : setup.onTalk)
        RunBlock(block, npc, world);
    return true;
}
} // namespace GameLogic::Npcs
