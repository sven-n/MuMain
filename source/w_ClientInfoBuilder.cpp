// w_ClientInfoBuilder.cpp: implementation of the ClientInfoBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ClientInfoBuilder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

ClientInfoBuilderPtr ClientInfoBuilder::Make()
{
	ClientInfoBuilderPtr builder( new ClientInfoBuilder() );
	return builder;
}

ClientInfoBuilder::ClientInfoBuilder()
{
	Initialize();
}

ClientInfoBuilder::~ClientInfoBuilder()
{
	Destroy();
}

void ClientInfoBuilder::Initialize()
{

}

void ClientInfoBuilder::Destroy()
{

}

#endif //NEW_USER_INTERFACE_BUILDER