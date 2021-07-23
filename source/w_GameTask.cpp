// w_GameTask.cpp: implementation of the GameTask class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_GameTask.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzLodTerrain.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "NewUISystem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GameTaskPtr GameTask::Make( Task::eType type )
{
	BoostSmart_Ptr(GameTask) task( new GameTask( type ) );
	task->Initialize( task );
	TheGameServerProxy().RegisterHandler( task );
	TheShopServerProxy().RegisterHandler( task );
	return task;
}

GameTask::GameTask( Task::eType type ) : Task( type )
{

}

GameTask::~GameTask()
{
	Destroy();
}

void GameTask::Initialize( BoostWeak_Ptr(GameTask) handler )
{
	//자기 자신 포인터를 가지고 있다가 모듈들이 필요하면 넘겨 줄까..--;;별로 좋지 않음..프록시로 통하는게 좋을꺼 같음.
	//그런데 상속을 마니 받아야함...ㅜㅜ
	m_Handler = handler;
}

void GameTask::Destroy()
{
	
}

void GameTask::OnShopIn( const eShopType shoptype )
{
	ShopMake( shoptype );
	g_pNewUISystem->HideAll();
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
	g_pNewUISystem->Show( SEASON3B::INTERFACE_PARTCHARGE_SHOP );
#else // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
	g_pNewUISystem->Toggle( SEASON3B::INTERFACE_PARTCHARGE_SHOP );
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
}

void GameTask::OnShopOunt( const eShopType shoptype )
{
	ShopClear;
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
	MessageBoxClear;
	g_pNewUISystem->Hide( SEASON3B::INTERFACE_PARTCHARGE_SHOP );
#else // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
	g_pNewUISystem->Toggle( SEASON3B::INTERFACE_PARTCHARGE_SHOP );
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM