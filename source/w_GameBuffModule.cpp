// w_GameBuffModule.cpp: implementation of the GameBuffModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_GameBuffModule.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GameBuffModulePtr GameBuffModule::Make( Module::eType type )
{
	GameBuffModulePtr module( new GameBuffModule(type) );
	module->Initialize( module );
	return module;
}

GameBuffModule::GameBuffModule( Module::eType type ) : Module( type )
{

}

GameBuffModule::~GameBuffModule()
{
	Destroy();
}

void GameBuffModule::Initialize( BoostWeak_Ptr(GameBuffModule) handler )
{

}

void GameBuffModule::Destroy()
{

}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
