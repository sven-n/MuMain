// w_GameTask.h: interface for the GameTask class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_GAMETASK_H__9C3E0E5B_46E5_4C9C_8727_3CC998BCE928__INCLUDED_)
#define AFX_W_GAMETASK_H__9C3E0E5B_46E5_4C9C_8727_3CC998BCE928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//상속이 많으면 좋지 않다.
//부분적으로 나눠야 할것들은 나눠서 관리 하자

BoostSmartPointer( GameTask );
class GameTask : public Task,
				 public GameServerProxyHandler, 
				 public ShopServerProxyHandler
{
public:
	static GameTaskPtr Make( Task::eType type );
	virtual ~GameTask();

protected://ShopServerProxyHandler
	virtual void OnShopIn( const eShopType shoptype );
	virtual void OnShopOunt( const eShopType shoptype );

private:
	void Initialize( BoostWeak_Ptr(GameTask) handler );
	void Destroy();
	GameTask( Task::eType type );

private:
	BoostWeak_Ptr(GameTask)			m_Handler;
};

#endif // !defined(AFX_W_GAMETASK_H__9C3E0E5B_46E5_4C9C_8727_3CC998BCE928__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM