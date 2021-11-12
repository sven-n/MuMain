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




