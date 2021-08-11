#include "stdafx.h"
#include "_GlobalFunctions.h"

#ifdef LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
#include "GMEmpireGuardian1.h"
#endif // LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
bool g_bRenderBoundingBox = false; 
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX

BuffStateSystemPtr			g_BuffSystem;

BuffStateSystem& TheBuffStateSystem() 
{
	assert( g_BuffSystem ); 
	return *g_BuffSystem;
}

BuffScriptLoader& TheBuffInfo() {
	return TheBuffStateSystem().GetBuffInfo();
}

BuffTimeControl& TheBuffTimeControl() {
	return TheBuffStateSystem().GetBuffTimeControl();
}

BuffStateValueControl& TheBuffStateValueControl() {
	return TheBuffStateSystem().GetBuffStateValueControl();
}

#ifdef NEW_USER_INTERFACE_FUNCTIONS

ShellPtr		g_shell;

Shell& TheShell()
{	
	assert( g_shell );
	return *g_shell;
}

BuildSystem& TheBuildSystem()
{
	return TheShell().GetBuildSystem();
}

ClientInfoBuilder& TheClientInfoBuilder()
{
	return TheBuildSystem().GetClientInfoBuilder();
}

GameInfoBuilder& TheGameInfoBuilder()
{
	return TheBuildSystem().GetGameInfoBuilder();
}

ServerProxySystem& TheSerProxySystem()
{
	return TheShell().GetServerProxySystem();
}

InputProxy&	TheInputProxy()
{
	return TheSerProxySystem().GetInputProxy();
}

input::InputSystem& TheInputSystem()
{
	return TheShell().GetInputSystem();
}

ui::UISystem& TheUISystem()
{
	return TheShell().GetUISystem();
}

GameServerProxy& TheGameServerProxy()
{
	return TheSerProxySystem().GetGameServerProxy();
}

ShopServerProxy& TheShopServerProxy()
{
	return TheSerProxySystem().GetShopServerProxy();
}

ClientSystem& TheClientSystem()
{
	return TheShell().GetClientSystem();
}

#endif //NEW_USER_INTERFACE_FUNCTIONS

#ifdef PSW_ADD_MAPSYSTEM



#endif //PSW_ADD_MAPSYSTEM



