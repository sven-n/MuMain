
#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

// w_GameBuffModule.h: interface for the GameBuffModule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_GAMEBUFFMODULE_H__5EA8058D_2E0D_49C1_BB2D_3277A41AD1DD__INCLUDED_)
#define AFX_W_GAMEBUFFMODULE_H__5EA8058D_2E0D_49C1_BB2D_3277A41AD1DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer( GameBuffModule );
class GameBuffModule : public Module, public GameServerProxyHandler
{
public:
	static GameBuffModulePtr Make( Module::eType type );
	virtual ~GameBuffModule();

private:
	void Initialize( BoostWeak_Ptr(GameBuffModule) handler );
	void Destroy();
	GameBuffModule( Module::eType type );
};

#endif // !defined(AFX_W_GAMEBUFFMODULE_H__5EA8058D_2E0D_49C1_BB2D_3277A41AD1DD__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM