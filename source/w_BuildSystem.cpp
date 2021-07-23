// w_BuildSystem.cpp: implementation of the BuildSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_BuildSystem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

BuildSystemPtr BuildSystem::Make()
{
	BuildSystemPtr system( new BuildSystem() );
	system->Initialize( system );
	return system;
}

BuildSystem::BuildSystem()
{

}

BuildSystem::~BuildSystem()
{
	Destroy();
}

void BuildSystem::Initialize( BoostWeak_Ptr(BuildSystem) handler )
{
	try
	{
		m_ClientInfoBuilder = ClientInfoBuilder::Make();
		m_GameInfoBuilder = GameInfoBuilder::Make();
	}
	catch (...)
	{
		//예외 처리...
	}
}

void BuildSystem::Destroy()
{

}

#endif //NEW_USER_INTERFACE_BUILDER