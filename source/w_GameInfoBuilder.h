// w_GameInfoBuilder.h: interface for the GameInfoBuilder class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

#if !defined(AFX_W_GAMEINFOBUILDER_H__A33AA6A6_C067_4DC0_9E22_958D0DDD7535__INCLUDED_)
#define AFX_W_GAMEINFOBUILDER_H__A33AA6A6_C067_4DC0_9E22_958D0DDD7535__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
	게임 빌더는 게임에 필요한 정보를 읽어 들이는 부분이다.
	예를 들어 아이템, 케릭터 등등...
*/
#include "w_ShopUIInfo.h"
#include "w_ShopItemInfo.h"

BoostSmartPointer( GameInfoBuilder );
class GameInfoBuilder : public Build
{
public:
	enum eType
	{
		eNone,
		eShopUIInfo,
		eShopItemInfo,
		eCount,
	};

public:
	static GameInfoBuilderPtr Make();
	virtual ~GameInfoBuilder();

public:
	ShopUIInfo&	QueryShopUIInfo();
	ShopItemInfo& QueryShopItemInfo();

private:
	void Initialize();
	void Destroy();
	GameInfoBuilder();

private:
	ShopUIInfoPtr				m_ShopUIInfo;
	ShopItemInfoPtr				m_ShopItemInfo;

};

#endif // !defined(AFX_W_GAMEINFOBUILDER_H__A33AA6A6_C067_4DC0_9E22_958D0DDD7535__INCLUDED_)

#endif //NEW_USER_INTERFACE_BUILDER