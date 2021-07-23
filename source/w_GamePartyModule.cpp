// w_GamePartyModule.cpp: implementation of the GamePartyModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_GamePartyModule.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GamePartyModulePtr GamePartyModule::Make( Module::eType type )
{
	GamePartyModulePtr module( new GamePartyModule( type ) );
	module->Initialize( module );
	return module;
}

GamePartyModule::GamePartyModule( Module::eType type ) : Module(type)
{

}

GamePartyModule::~GamePartyModule()
{
	Destroy();
}

void GamePartyModule::Initialize( BoostWeak_Ptr(GamePartyModule) handler )
{
	
}

void GamePartyModule::Destroy()
{

}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM