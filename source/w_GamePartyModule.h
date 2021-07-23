
#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM
// w_GamePartyModule.h: interface for the GamePartyModule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_GAMEPARTYMODULE_H__00369957_94C4_4B72_93E1_1B04EF78FE15__INCLUDED_)
#define AFX_W_GAMEPARTYMODULE_H__00369957_94C4_4B72_93E1_1B04EF78FE15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer( GamePartyModule );
class GamePartyModule : public Module, public GameServerProxyHandler 
{
public:
	static GamePartyModulePtr	Make( Module::eType type );
	virtual ~GamePartyModule();

private:
	void Initialize( BoostWeak_Ptr(GamePartyModule) handler );
	void Destroy();
	GamePartyModule( Module::eType type );
};

#endif // !defined(AFX_W_GAMEPARTYMODULE_H__00369957_94C4_4B72_93E1_1B04EF78FE15__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM