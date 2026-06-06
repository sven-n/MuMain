#include "stdafx.h"
#include "UI/Chat/ChatInput.h"

// Includes mirror ZzzInterface.cpp, the unit this was extracted from.
#include <imm.h>
#include "UI/Legacy/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/PlayerActionState.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/SkillCast.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "Core/Input/ImeInput.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "Engine/Pathing/ZzzPath.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CSParts.h"
#include "UI/Legacy/UIMapName.h"	// rozy
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Events/Event.h"
#include "UI/NewUI/NewUISystem.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "UI/Legacy/UIControls.h"
#include "GameLogic/Social/PartyManager.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "GameLogic/Combat/DuelMgr.h"

namespace UI::Chat
{
void CheckChatText(wchar_t* Text)
{
    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;
    if (FindText(Text, I18N::Game::Hello) || FindText(Text, I18N::Game::Hi) || FindText(Text, I18N::Game::Welcome) || FindText(Text, I18N::Game::Welcome) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks))
    {
        SetActionClass(c, o, PLAYER_GREETING1, AT_GREETING1);
        SendRequestAction(Hero->Object, AT_GREETING1);
    }
    else if (FindText(Text, I18N::Game::EnjoyTheGame) || FindText(Text, I18N::Game::Bye) || FindText(Text, I18N::Game::Bye280))
    {
        SetActionClass(c, o, PLAYER_GOODBYE1, AT_GOODBYE1);
        SendRequestAction(Hero->Object, AT_GOODBYE1);
    }
    else if (FindText(Text, I18N::Game::Good) || FindText(Text, I18N::Game::Good) || FindText(Text, I18N::Game::Wow) || FindText(Text, I18N::Game::Wow) || FindText(Text, I18N::Game::Nice) || FindText(Text, I18N::Game::Nice))
    {
        SetActionClass(c, o, PLAYER_CLAP1, AT_CLAP1);
        SendRequestAction(Hero->Object, AT_CLAP1);
    }
    else if (FindText(Text, I18N::Game::Here) || FindText(Text, I18N::Game::Here) || FindText(Text, I18N::Game::Come) || FindText(Text, I18N::Game::Come))
    {
        SetActionClass(c, o, PLAYER_GESTURE1, AT_GESTURE1);
        SendRequestAction(Hero->Object, AT_GESTURE1);
    }
    else if (FindText(Text, I18N::Game::There) || FindText(Text, I18N::Game::There) || FindText(Text, I18N::Game::That) || FindText(Text, I18N::Game::That))
    {
        SetActionClass(c, o, PLAYER_DIRECTION1, AT_DIRECTION1);
        SendRequestAction(Hero->Object, AT_DIRECTION1);
    }
    else if (FindText(Text, I18N::Game::Not) || FindText(Text, I18N::Game::Not) || FindText(Text, I18N::Game::Never) || FindText(Text, I18N::Game::Never) || FindText(Text, I18N::Game::DoNot) || FindText(Text, I18N::Game::DoNot) || FindText(Text, I18N::Game::DoNot302))
    {
        SetActionClass(c, o, PLAYER_UNKNOWN1, AT_UNKNOWN1);
        SendRequestAction(Hero->Object, AT_UNKNOWN1);
    }
    else if (FindText(Text, L";") || FindText(Text, I18N::Game::Sorry) || FindText(Text, I18N::Game::Sorry) || FindText(Text, I18N::Game::Sorry))
    {
        SetActionClass(c, o, PLAYER_AWKWARD1, AT_AWKWARD1);
        SendRequestAction(Hero->Object, AT_AWKWARD1);
    }
    else if (FindText(Text, L"ㅠ.ㅠ") || FindText(Text, L"ㅜ.ㅜ") || FindText(Text, L"T_T") || FindText(Text, I18N::Game::Sad) || FindText(Text, I18N::Game::Sad) || FindText(Text, I18N::Game::Cry) || FindText(Text, I18N::Game::Cry))
    {
        SetActionClass(c, o, PLAYER_CRY1, AT_CRY1);
        SendRequestAction(Hero->Object, AT_CRY1);
    }
    else if (FindText(Text, L"ㅡ.ㅡ") || FindText(Text, L"ㅡ.,ㅡ") || FindText(Text, L"ㅡ,.ㅡ") || FindText(Text, L"-.-") || FindText(Text, L"-_-") || FindText(Text, I18N::Game::Huh) || FindText(Text, I18N::Game::Pooh))
    {
        SetActionClass(c, o, PLAYER_SEE1, AT_SEE1);
        SendRequestAction(Hero->Object, AT_SEE1);
    }
    else if (FindText(Text, L"^^") || FindText(Text, L"^.^") || FindText(Text, L"^_^") || FindText(Text, I18N::Game::Haha) || FindText(Text, I18N::Game::Hehe) || FindText(Text, I18N::Game::Hoho) || FindText(Text, I18N::Game::Hoho) || FindText(Text, I18N::Game::Hihi))
    {
        SetActionClass(c, o, PLAYER_SMILE1, AT_SMILE1);
        SendRequestAction(Hero->Object, AT_SMILE1);
    }
    else if (FindText(Text, I18N::Game::Great) || FindText(Text, I18N::Game::OhYeah) || FindText(Text, I18N::Game::OhYeah320) || FindText(Text, I18N::Game::BeatIt))
    {
        SetActionClass(c, o, PLAYER_CHEER1, AT_CHEER1);
        SendRequestAction(Hero->Object, AT_CHEER1);
    }
    else if (FindText(Text, I18N::Game::Win) || FindText(Text, I18N::Game::Win) || FindText(Text, I18N::Game::Victory) || FindText(Text, I18N::Game::Victory))
    {
        SetActionClass(c, o, PLAYER_WIN1, AT_WIN1);
        SendRequestAction(Hero->Object, AT_WIN1);
    }
    else if (FindText(Text, I18N::Game::Sleep) || FindText(Text, I18N::Game::Sleep) || FindText(Text, I18N::Game::Tired) || FindText(Text, I18N::Game::Tired))
    {
        SetActionClass(c, o, PLAYER_SLEEP1, AT_SLEEP1);
        SendRequestAction(Hero->Object, AT_SLEEP1);
    }
    else if (FindText(Text, I18N::Game::Cold) || FindText(Text, I18N::Game::Cold) || FindText(Text, I18N::Game::Hurt) || FindText(Text, I18N::Game::Hurt) || FindText(Text, I18N::Game::Hurt))
    {
        SetActionClass(c, o, PLAYER_COLD1, AT_COLD1);
        SendRequestAction(Hero->Object, AT_COLD1);
    }
    else if (FindText(Text, I18N::Game::Again) || FindText(Text, I18N::Game::Again) || FindText(Text, I18N::Game::OK) || FindText(Text, I18N::Game::Great))
    {
        SetActionClass(c, o, PLAYER_AGAIN1, AT_AGAIN1);
        SendRequestAction(Hero->Object, AT_AGAIN1);
    }
    else if (FindText(Text, I18N::Game::Respect) || FindText(Text, I18N::Game::Respect) || FindText(Text, I18N::Game::Defeated))
    {
        SetActionClass(c, o, PLAYER_RESPECT1, AT_RESPECT1);
        SendRequestAction(Hero->Object, AT_RESPECT1);
    }
    else if (FindText(Text, I18N::Game::Sir) || FindText(Text, I18N::Game::Sir) || FindText(Text, L"/ㅡ") || FindText(Text, L"ㅡ^"))
    {
        SetActionClass(c, o, PLAYER_SALUTE1, AT_SALUTE1);
        SendRequestAction(Hero->Object, AT_SALUTE1);
    }
    else if (FindText(Text, I18N::Game::Rush) || FindText(Text, I18N::Game::Rush) || FindText(Text, I18N::Game::GoGo) || FindText(Text, I18N::Game::GoGo))
    {
        SetActionClass(c, o, PLAYER_RUSH1, AT_RUSH1);
        SendRequestAction(Hero->Object, AT_RUSH1);
    }
    else if (FindText(Text, I18N::Game::Hustle) || FindText(Text, L"hustle"))
    {
        SetActionClass(c, o, PLAYER_HUSTLE, AT_HUSTLE);
        SendRequestAction(Hero->Object, AT_HUSTLE);
    }
    else if (FindText(Text, I18N::Game::ComeOn))
    {
        SetActionClass(c, o, PLAYER_PROVOCATION, AT_PROVOCATION);
        SendRequestAction(Hero->Object, AT_PROVOCATION);
    }
    else if (FindText(Text, I18N::Game::Great))
    {
        SetActionClass(c, o, PLAYER_CHEERS, AT_CHEERS);
        SendRequestAction(Hero->Object, AT_CHEERS);
    }
    else if (FindText(Text, I18N::Game::LookAround))
    {
        SetActionClass(c, o, PLAYER_LOOK_AROUND, AT_LOOK_AROUND);
        SendRequestAction(Hero->Object, AT_LOOK_AROUND);
    }
    else if (FindText(Text, I18N::Game::TheForehead))
    {
        ITEM* pItem_rr = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        ITEM* pItem_rl = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];

        if (pItem_rr->Type == ITEM_JACK_OLANTERN_TRANSFORMATION_RING || pItem_rl->Type == ITEM_JACK_OLANTERN_TRANSFORMATION_RING)
        {
            if (rand_fps_check(2))
            {
                SetAction(o, PLAYER_JACK_1);
                SendRequestAction(Hero->Object, AT_JACK1);
            }
            else
            {
                SetAction(o, PLAYER_JACK_2);
                SendRequestAction(Hero->Object, AT_JACK2);
            }

            o->m_iAnimation = 0;
        }
    }
    else if (FindText(Text, I18N::Game::Christmas))
    {
        ITEM* pItem_rr = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        ITEM* pItem_rl = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];

        if (pItem_rr->Type == ITEM_CHRISTMAS_TRANSFORMATION_RING || pItem_rl->Type == ITEM_CHRISTMAS_TRANSFORMATION_RING)
        {
            if (o->CurrentAction != PLAYER_SANTA_1 && o->CurrentAction != PLAYER_SANTA_2)
            {
                int i = rand() % 3;
                if (rand() % 2)
                {
                    SetAction(o, PLAYER_SANTA_1);
                    SendRequestAction(Hero->Object, AT_SANTA1_1 + i);
                    PlayBuffer(static_cast<ESound>(SOUND_XMAS_JUMP_SANTA + i));
                }
                else
                {
                    SetAction(o, PLAYER_SANTA_2);
                    SendRequestAction(Hero->Object, AT_SANTA2_1 + i);
                    PlayBuffer(SOUND_XMAS_TURN);
                }

                g_XmasEvent->CreateXmasEventEffect(c, o, i);
            }

            o->m_iAnimation = 0;
        }
    }
}
}
