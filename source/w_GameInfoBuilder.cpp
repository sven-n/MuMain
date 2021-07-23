// w_GameInfoBuilder.cpp: implementation of the GameInfoBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_GameInfoBuilder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER 

GameInfoBuilderPtr GameInfoBuilder::Make()
{
	GameInfoBuilderPtr builder( new GameInfoBuilder() );
	return builder;
}

GameInfoBuilder::GameInfoBuilder()
{
	Initialize();
}

GameInfoBuilder::~GameInfoBuilder()
{
	Destroy();
}

void GameInfoBuilder::Initialize()
{
	m_ShopUIInfo = ShopUIInfo::Make();
	m_ShopItemInfo = ShopItemInfo::Make();
}

void GameInfoBuilder::Destroy()
{

}

ShopUIInfo&	GameInfoBuilder::QueryShopUIInfo()
{
	if( !m_ShopUIInfo )
	{
		m_ShopUIInfo = ShopUIInfo::Make();
		return *m_ShopUIInfo;
	}
	else
	{
		return *m_ShopUIInfo;
	}
}

ShopItemInfo& GameInfoBuilder::QueryShopItemInfo()
{
	if( !m_ShopItemInfo )
	{
		m_ShopItemInfo = ShopItemInfo::Make();
		return *m_ShopItemInfo;
	}
	else
	{
		return *m_ShopItemInfo;
	}
}

#endif //NEW_USER_INTERFACE_BUILDER