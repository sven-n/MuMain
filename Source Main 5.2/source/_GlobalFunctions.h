#ifndef _GLOBAL_FUNCTIONS_H
#define _GLOBAL_FUNCTIONS_H

#include "ZzzScene.h"
#include "w_BuffStateSystem.h"

class BuffStateSystem;
class BuffScriptLoader;
class BuffTimeControl;
class BuffStateValueControl;

extern BuffStateSystemPtr			g_BuffSystem;

BuffScriptLoader& TheBuffInfo();

BuffStateSystem& TheBuffStateSystem();

BuffTimeControl& TheBuffTimeControl();

BuffStateValueControl& TheBuffStateValueControl();

//Character Buff
#define g_isNotCharacterBuff( o ) \
	o->m_BuffMap.isBuff()

#define g_isCharacterBuff( o, bufftype ) \
	o->m_BuffMap.isBuff( bufftype )

#define g_isCharacterBufflist( o, bufftypelist ) \
	o->m_BuffMap.isBuff( bufftypelist )

#define g_TokenCharacterBuff( o, bufftype ) \
	o->m_BuffMap.TokenBuff( bufftype )

#define g_CharacterBuffCount( o, bufftype ) \
	o->m_BuffMap.GetBuffCount( bufftype )

#define g_CharacterBuffSize( o ) \
	o->m_BuffMap.GetBuffSize()

#define g_CharacterBuff( o, iterindex ) \
	o->m_BuffMap.GetBuff( iterindex )

#define g_CharacterRegisterBuff( o, bufftype ) \
	o->m_BuffMap.RegisterBuff( bufftype )

#define g_CharacterRegisterBufflist( o, bufftypelist ) \
	o->m_BuffMap.RegisterBuff( bufftypelist )

#define g_CharacterUnRegisterBuff( o, bufftype ) \
	o->m_BuffMap.UnRegisterBuff( bufftype )

#define g_CharacterUnRegisterBuffList( o, bufftypelist ) \
	o->m_BuffMap.UnRegisterBuff( bufftypelist )

#define g_CharacterCopyBuff( outObj, inObj ) \
	outObj->m_BuffMap.m_Buff = inObj->m_BuffMap.m_Buff

#define g_CharacterClearBuff( o ) \
	o->m_BuffMap.ClearBuff()

//TheBuffInfo
#define g_BuffInfo( buff ) \
	TheBuffInfo().GetBuffinfo( buff )

#define g_IsBuffClass( buff ) \
	TheBuffInfo().IsBuffClass( buff )

//TheBuffTimeControl
#define g_RegisterBuffTime( bufftype, curbufftime ) \
	TheBuffTimeControl().RegisterBuffTime( bufftype, curbufftime )

#define g_UnRegisterBuffTime( bufftype ) \
	TheBuffTimeControl().UnRegisterBuffTime( bufftype )

#define g_BuffStringTime( bufftype, timeText ) \
	TheBuffTimeControl().GetBuffStringTime( bufftype, timeText )

#define g_StringTime( time, timeText, issecond ) \
	TheBuffTimeControl().GetStringTime( time, timeText, issecond )

//TheBuffStateValueControl
#define g_BuffStateValue( type ) \
	TheBuffStateValueControl().GetValue( type )

#define g_BuffToolTipString( outstr, type ) \
	TheBuffStateValueControl().GetBuffInfoString( outstr, type )

#define g_BuffStateValueString( outstr, type ) \
	TheBuffStateValueControl().GetBuffValueString( outstr, type )

inline unsigned long RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    return (r)+(g << 8) + (b << 16) + (a << 24);
}
inline unsigned char GetAlpha(unsigned long rgba)
{
    return ((rgba) >> 24);
}
inline unsigned char GetRed(unsigned long rgba)
{
    return ((rgba) & 0xff);
}
inline unsigned char GetGreen(unsigned long rgba)
{
    return (((rgba) >> 8) & 0xff);
}
inline unsigned char GetBlue(unsigned long rgba)
{
    return (((rgba) >> 16) & 0xff);
}

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
extern bool g_bRenderBoundingBox;
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX

inline void __TraceF(const TCHAR* pFmt, ...)
{
#ifdef _DEBUG
    //TCHAR	szMsg[4096];
    //va_list	pArgList;
    //va_start(pArgList, pFmt);
    //_vsntprintf(szMsg, sizeof(szMsg) / sizeof(TCHAR), pFmt, pArgList);
    //va_end(pArgList);
    //OutputDebugString(szMsg);
#endif // _DEBUG
}

#endif	// _GLOBAL_FUNCTIONS_H