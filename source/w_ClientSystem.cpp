// w_ClientSystem.cpp: implementation of the ClientSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ClientSystem.h"

//task
#include "w_GameTask.h"

//module
#include "w_GamePartyModule.h"
#include "w_GameBuffModule.h"
#include "w_ShopTopModule.h"
#include "w_ShopLeftModule.h"
#include "w_ShopMiddleModule.h"
#include "w_ShopRightModule.h"
#include "w_MessageBoxModule.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClientSystemPtr ClientSystem::Make()
{
	ClientSystemPtr system( new ClientSystem() );
	system->Initialize();
	return system;
}

ClientSystem::ClientSystem()
{

}

ClientSystem::~ClientSystem()
{
	Destroy();
}

void ClientSystem::Initialize()
{

}

void ClientSystem::Destroy()
{
	for( int i = Module::eCount-1; i > -1; --i )
	{
		if( m_CurModule[i] ) m_CurModule[i].reset();
	}

	m_CurTask.reset();
}

void ClientSystem::ChangeTask( Task::eType type )
{
	if( m_CurTask )
	{
		if( m_CurTask->GetType() == type ) return;
	}

	m_CurTask.reset();

	switch( type )
	{
	case Task::eGame: m_CurTask = GameTask::Make( Task::eGame ); break;
	default: assert( 0 );
	}
}

void ClientSystem::RegisterModule( Module::eType type, bool overwrite )
{
	if( overwrite )
	{
		m_CurModule[type].reset();
	}

    if( m_CurModule[type] )   return;

    BoostSmart_Ptr(Module)	   module;

	switch( type )
	{
	//game
	case Module::eGame_Party:		module = GamePartyModule::Make( Module::eGame_Party ); break;
	case Module::eGame_Buff:		module = GameBuffModule::Make( Module::eGame_Buff ); break;
	//shop
	case Module::eShop_Top:			module = ShopTopModule::Make( Module::eShop_Top ); break;
	case Module::eShop_Left:		module = ShopLeftModule::Make( Module::eShop_Left ); break;
	case Module::eShop_Middle:		module = ShopMiddleModule::Make( Module::eShop_Middle ); break;
	case Module::eShop_Right:		module = ShopRightModule::Make( Module::eShop_Right ); break;
	default: assert( 0 );
	}

	m_CurModule[type] = module;
}

void ClientSystem::RegisterMessageBoxModule( const MESSAGEBOXDATA& messageboxdata )
{
    if( m_CurModule[Module::eMessageBox] )
	{
		MessageBoxClear;
	}

	BoostSmart_Ptr(Module)	   module = MessageBoxModule::Make( messageboxdata );
	m_CurModule[Module::eMessageBox]  = module;
}

void ClientSystem::UnregisterModule( Module::eType type )
{
	m_CurModule[type].reset();
}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM