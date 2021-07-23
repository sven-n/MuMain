#include "stdafx.h"
#include "_GlobalFunctions.h"

#ifdef LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
#include "GMEmpireGuardian1.h"
#endif // LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
bool g_bRenderBoundingBox = false; 
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX

BuffStateSystemPtr			g_BuffSystem;

BuffStateSystem& TheBuffStateSystem() 
{
	assert( g_BuffSystem ); 
	return *g_BuffSystem;
}

BuffScriptLoader& TheBuffInfo() {
	return TheBuffStateSystem().GetBuffInfo();
}

BuffTimeControl& TheBuffTimeControl() {
	return TheBuffStateSystem().GetBuffTimeControl();
}

BuffStateValueControl& TheBuffStateValueControl() {
	return TheBuffStateSystem().GetBuffStateValueControl();
}

#ifdef NEW_USER_INTERFACE_FUNCTIONS

ShellPtr		g_shell;

Shell& TheShell()
{	
	assert( g_shell );
	return *g_shell;
}

BuildSystem& TheBuildSystem()
{
	return TheShell().GetBuildSystem();
}

ClientInfoBuilder& TheClientInfoBuilder()
{
	return TheBuildSystem().GetClientInfoBuilder();
}

GameInfoBuilder& TheGameInfoBuilder()
{
	return TheBuildSystem().GetGameInfoBuilder();
}

ServerProxySystem& TheSerProxySystem()
{
	return TheShell().GetServerProxySystem();
}

InputProxy&	TheInputProxy()
{
	return TheSerProxySystem().GetInputProxy();
}

input::InputSystem& TheInputSystem()
{
	return TheShell().GetInputSystem();
}

ui::UISystem& TheUISystem()
{
	return TheShell().GetUISystem();
}

GameServerProxy& TheGameServerProxy()
{
	return TheSerProxySystem().GetGameServerProxy();
}

ShopServerProxy& TheShopServerProxy()
{
	return TheSerProxySystem().GetShopServerProxy();
}

ClientSystem& TheClientSystem()
{
	return TheShell().GetClientSystem();
}

#endif //NEW_USER_INTERFACE_FUNCTIONS

#ifdef PSW_ADD_MAPSYSTEM



#endif //PSW_ADD_MAPSYSTEM

#ifdef PBG_FIX_SKILL_DEMENDCONDITION
namespace SKILLCONDITION
{
	//스킬의 스텟요구량과 캐릭터의 스텟량을 비교하는 함수.
	BOOL skillVScharactorCheck( const DemendConditionInfo& basicInfo, const DemendConditionInfo& heroInfo )
	{
		//스크립트파일내의 에너지량만을 처리한다.
		if( basicInfo <= heroInfo )
		{
			//캐릭의 스텟량이 스킬의 스텟요구량보다 높다면.
			return true;
		}
		return false;
	}
	
	BOOL DemendConditionCheckSkill(WORD SkillType)
	{
		if( SkillType >= MAX_SKILLS ) {
			assert( 0 );
			return false;
		}

		// 제국수호군 맵의 순간이동 계열 스킬은 모두 사용못하도록 합니다.
#ifdef LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
		if( (true == IsEmpireGuardian()) && 
			(SkillType == AT_SKILL_TELEPORT_B || SkillType == AT_SKILL_TELEPORT) )
		{
			return false;
		}
#endif // LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
		
		//에너지 요구치(스크립트내에 0값을 갖는것들)가 없다면 그냥 리턴한다.
 		if(SkillAttribute[SkillType].Energy == 0)
 		{
 			return true;
 		}

		// 마스터 스킬 강화의 경우 강화 이전의 스킬의 수치를 그대로 적용한다. - 
#ifdef KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL
		SkillType = MasterSkillToBaseSkillIndex(SkillType);
#else // KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL
#ifdef LDS_FIX_MASTERSKILLKEY_DISABLE_OVER_LIMIT_MAGICIAN		
		if(SkillType >= AT_SKILL_SOUL_UP &&				// 소울 바리어 강화
			SkillType <= AT_SKILL_SOUL_UP + 5)
		{
			SkillType = AT_SKILL_WIZARDDEFENSE;
		}
		else if(SkillType >= AT_SKILL_HELL_FIRE_UP &&		// 헬파이어 강화
			SkillType <= AT_SKILL_HELL_FIRE_UP + 5)
		{
			SkillType = AT_SKILL_HELL;
		}
		else if(SkillType >= AT_SKILL_EVIL_SPIRIT_UP &&	// 악령 강화 (흑마법사)
			SkillType <= AT_SKILL_EVIL_SPIRIT_UP + 5)
		{
			SkillType = AT_SKILL_EVIL;
		}
		else if(SkillType >= AT_SKILL_ICE_UP &&			// 서든 아이스 강화 
			SkillType <= AT_SKILL_ICE_UP + 5)
		{
			SkillType = AT_SKILL_BLAST_FREEZE;
		}
#endif // LDS_FIX_MASTERSKILLKEY_DISABLE_OVER_LIMIT_MAGICIAN
#endif // KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL


		bool result = true;	
		
		DemendConditionInfo BasicCharacterInfo;
		
		
		BasicCharacterInfo.SkillLevel     = SkillAttribute[SkillType].Level;
		BasicCharacterInfo.SkillStrength  = SkillAttribute[SkillType].Strength;
		BasicCharacterInfo.SkillDexterity = SkillAttribute[SkillType].Dexterity;
		BasicCharacterInfo.SkillVitality  = 0;
		//에너지의 스텟 요구치 계산공식.
		BasicCharacterInfo.SkillEnergy = (20 + ( SkillAttribute[SkillType].Energy * SkillAttribute[SkillType].Level ) * 0.04);
		BasicCharacterInfo.SkillCharisma = SkillAttribute[SkillType].Charisma;
		
		DemendConditionInfo HeroCharacterInfo;
		
		//캐릭터의 각종 스텟의 량을 얻어온다. 캐릭의 스텟+ 각종 장비착용후의 증가량을 합한다.
		HeroCharacterInfo.SkillLevel     = CharacterMachine->Character.Level; 
		HeroCharacterInfo.SkillStrength  = CharacterMachine->Character.Strength + CharacterMachine->Character.AddStrength;
		HeroCharacterInfo.SkillDexterity = CharacterMachine->Character.Dexterity + CharacterMachine->Character.AddDexterity; 
		HeroCharacterInfo.SkillVitality  = CharacterMachine->Character.Vitality + CharacterMachine->Character.AddVitality; 
		HeroCharacterInfo.SkillEnergy    = CharacterMachine->Character.Energy + CharacterMachine->Character.AddEnergy; 
		HeroCharacterInfo.SkillCharisma  = CharacterMachine->Character.Charisma + CharacterMachine->Character.AddCharisma;
		
		result = skillVScharactorCheck( BasicCharacterInfo, HeroCharacterInfo );

		return result;	
	}
};
#endif //PBG_FIX_SKILL_DEMENDCONDITION

#if defined PBG_ADD_MU_LOGO || defined LJH_MOD_TO_USE_ISBLUEMUSERVER_FUNC
// 블루뮤 로고가 들어가면 블루섭 
//(소스 분리작업으로 디파인으로 블루섭인가 여부를 확인) 
// 굳이 글로벌변수는 없어도 됨 프로토콜로 관리 할경우
namespace BLUE_MU
{
	bool g_bIsBlue_MU_Server = false;

	// 블루뮤서버인지 재확인이 필요할 경우
	BOOL IsBlueMuServer()
	{
#ifdef _BLUE_SERVER
		g_bIsBlue_MU_Server = true;
#endif //_BLUE_SERVER
		return g_bIsBlue_MU_Server;
	}
};
#endif //defined PBG_ADD_MU_LOGO || defined LJH_MOD_TO_USE_ISBLUEMUSERVER_FUNC


#ifdef KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL
int MasterSkillToBaseSkillIndex(int iMasterSkillIndex)
{
	int iBaseSkillIndex = 0;
	switch( iMasterSkillIndex )
	{	
	case AT_SKILL_EVIL_SPIRIT_UP:
	case AT_SKILL_EVIL_SPIRIT_UP+1:
	case AT_SKILL_EVIL_SPIRIT_UP+2:
	case AT_SKILL_EVIL_SPIRIT_UP+3:
	case AT_SKILL_EVIL_SPIRIT_UP+4:
	case AT_SKILL_EVIL_SPIRIT_UP_M:
	case AT_SKILL_EVIL_SPIRIT_UP_M+1:
	case AT_SKILL_EVIL_SPIRIT_UP_M+2:
	case AT_SKILL_EVIL_SPIRIT_UP_M+3:
	case AT_SKILL_EVIL_SPIRIT_UP_M+4:
		{
			iBaseSkillIndex = AT_SKILL_EVIL;			// 악령
		}
		break;
	case AT_SKILL_SOUL_UP:
	case AT_SKILL_SOUL_UP+1:
	case AT_SKILL_SOUL_UP+2:
	case AT_SKILL_SOUL_UP+3:
	case AT_SKILL_SOUL_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_WIZARDDEFENSE;	// 소울바리어
		}
		break;	
	case AT_SKILL_HELL_FIRE_UP:
	case AT_SKILL_HELL_FIRE_UP+1:
	case AT_SKILL_HELL_FIRE_UP+2:
	case AT_SKILL_HELL_FIRE_UP+3:
	case AT_SKILL_HELL_FIRE_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_BLAST_HELL;		// 헬버스트
		}
		break;	
	case AT_SKILL_ICE_UP:
	case AT_SKILL_ICE_UP+1:
	case AT_SKILL_ICE_UP+2:
	case AT_SKILL_ICE_UP+3:
	case AT_SKILL_ICE_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_BLAST_FREEZE;	// 서든아이스
		}
		break;
	case AT_SKILL_TORNADO_SWORDA_UP:
	case AT_SKILL_TORNADO_SWORDA_UP+1:
	case AT_SKILL_TORNADO_SWORDA_UP+2:
	case AT_SKILL_TORNADO_SWORDA_UP+3:
	case AT_SKILL_TORNADO_SWORDA_UP+4:
	case AT_SKILL_TORNADO_SWORDB_UP:
	case AT_SKILL_TORNADO_SWORDB_UP+1:
	case AT_SKILL_TORNADO_SWORDB_UP+2:
	case AT_SKILL_TORNADO_SWORDB_UP+3:
	case AT_SKILL_TORNADO_SWORDB_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_WHEEL;			// 회오리베기
		}
		break;
	case AT_SKILL_BLOW_UP:
	case AT_SKILL_BLOW_UP+1:
	case AT_SKILL_BLOW_UP+2:
	case AT_SKILL_BLOW_UP+3:
	case AT_SKILL_BLOW_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ONETOONE;		// 블로우어택
		}
		break;	
	case AT_SKILL_ANGER_SWORD_UP:
	case AT_SKILL_ANGER_SWORD_UP+1:
	case AT_SKILL_ANGER_SWORD_UP+2:
	case AT_SKILL_ANGER_SWORD_UP+3:
	case AT_SKILL_ANGER_SWORD_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_FURY_STRIKE;		// 분노의일격
		}
		break;
	case AT_SKILL_LIFE_UP:
	case AT_SKILL_LIFE_UP+1:
	case AT_SKILL_LIFE_UP+2:
	case AT_SKILL_LIFE_UP+3:
	case AT_SKILL_LIFE_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_VITALITY;		// 스웰라이프
		}
		break;
	case AT_SKILL_HEAL_UP:
	case AT_SKILL_HEAL_UP+1:
	case AT_SKILL_HEAL_UP+2:
	case AT_SKILL_HEAL_UP+3:
	case AT_SKILL_HEAL_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_HEALING;			// 치료
		}
		break;
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_DEFENSE;			// 방어력증가
		}
		break;	
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
		{	
			iBaseSkillIndex = AT_SKILL_ATTACK;			// 공격력증가
		}
		break;
	case AT_SKILL_MANY_ARROW_UP:
	case AT_SKILL_MANY_ARROW_UP+1:
	case AT_SKILL_MANY_ARROW_UP+2:
	case AT_SKILL_MANY_ARROW_UP+3:
	case AT_SKILL_MANY_ARROW_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_CROSSBOW;		// 다발
		}
		break;
	case AT_SKILL_BLOOD_ATT_UP:
	case AT_SKILL_BLOOD_ATT_UP+1:
	case AT_SKILL_BLOOD_ATT_UP+2:
	case AT_SKILL_BLOOD_ATT_UP+3:
	case AT_SKILL_BLOOD_ATT_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_REDUCEDEFENSE;	// 블러드어택
		}
		break;
	case AT_SKILL_POWER_SLASH_UP:
	case AT_SKILL_POWER_SLASH_UP+1:
	case AT_SKILL_POWER_SLASH_UP+2:
	case AT_SKILL_POWER_SLASH_UP+3:
	case AT_SKILL_POWER_SLASH_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ICE_BLADE;		// 파워슬래시
		}
		break;	
	case AT_SKILL_BLAST_UP:
	case AT_SKILL_BLAST_UP+1:
	case AT_SKILL_BLAST_UP+2:
	case AT_SKILL_BLAST_UP+3:
	case AT_SKILL_BLAST_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_BLAST;			// 블래스트
		}
		break;
	case AT_SKILL_ASHAKE_UP:
	case AT_SKILL_ASHAKE_UP+1:
	case AT_SKILL_ASHAKE_UP+2:
	case AT_SKILL_ASHAKE_UP+3:
	case AT_SKILL_ASHAKE_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_DARK_HORSE;		// 어스퀘이크
		}
		break;
	case AT_SKILL_FIRE_BUST_UP:
	case AT_SKILL_FIRE_BUST_UP+1:
	case AT_SKILL_FIRE_BUST_UP+2:
	case AT_SKILL_FIRE_BUST_UP+3:
	case AT_SKILL_FIRE_BUST_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_LONGPIER_ATTACK;		// 파이어버스트
		}
		break;
	case AT_SKILL_FIRE_SCREAM_UP:
	case AT_SKILL_FIRE_SCREAM_UP+1:
	case AT_SKILL_FIRE_SCREAM_UP+2:
	case AT_SKILL_FIRE_SCREAM_UP+3:
	case AT_SKILL_FIRE_SCREAM_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_DARK_SCREAM;			// 파이어스크림
		}
		break;	
#ifdef PJH_ADD_MASTERSKILL
	case AT_SKILL_ALICE_SLEEP_UP:
	case AT_SKILL_ALICE_SLEEP_UP+1:
	case AT_SKILL_ALICE_SLEEP_UP+2:
	case AT_SKILL_ALICE_SLEEP_UP+3:
	case AT_SKILL_ALICE_SLEEP_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ALICE_SLEEP;			// 슬립
		}
		break;
	case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
	case AT_SKILL_ALICE_CHAINLIGHTNING_UP+1:
	case AT_SKILL_ALICE_CHAINLIGHTNING_UP+2:
	case AT_SKILL_ALICE_CHAINLIGHTNING_UP+3:
	case AT_SKILL_ALICE_CHAINLIGHTNING_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ALICE_CHAINLIGHTNING;	// 체인라이트닝
		}
		break;
	case AT_SKILL_LIGHTNING_SHOCK_UP:
	case AT_SKILL_LIGHTNING_SHOCK_UP+1:
	case AT_SKILL_LIGHTNING_SHOCK_UP+2:
	case AT_SKILL_LIGHTNING_SHOCK_UP+3:
	case AT_SKILL_LIGHTNING_SHOCK_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ALICE_LIGHTNINGORB;		// 라이트닝오브(쇼크)
		}
		break;
	case AT_SKILL_ALICE_DRAINLIFE_UP:
	case AT_SKILL_ALICE_DRAINLIFE_UP+1:
	case AT_SKILL_ALICE_DRAINLIFE_UP+2:
	case AT_SKILL_ALICE_DRAINLIFE_UP+3:
	case AT_SKILL_ALICE_DRAINLIFE_UP+4:
		{
			iBaseSkillIndex = AT_SKILL_ALICE_DRAINLIFE;			// 드레인 라이프
		}
		break;
#endif // PJH_ADD_MASTERSKILL
	default:
		{
			iBaseSkillIndex = iMasterSkillIndex;
		}
	}

	return iBaseSkillIndex;
}
#endif // KJH_MOD_BTS184_REQUIRE_STAT_WHEN_SPELL_SKILL
