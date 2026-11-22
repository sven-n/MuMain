#include "stdafx.h"
#include "w_BuffStateValueControl.h"
#include "w_BuffScriptLoader.h"
#include "Core/Utilities/_GlobalFunctions.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BuffStateValueControlPtr BuffStateValueControl::Make()
{
    BuffStateValueControlPtr buffstatecontrol(new BuffStateValueControl);
    return buffstatecontrol;
}

BuffStateValueControl::BuffStateValueControl() = default;

BuffStateValueControl::~BuffStateValueControl() = default;

void BuffStateValueControl::GetBuffInfoString(std::list<std::wstring>& outstr, eBuffState bufftype)
{
    const BuffInfo buffinfo = g_BuffInfo(bufftype);

    outstr = buffinfo.s_BuffDescriptlist;
    outstr.push_front(L"\n");
    outstr.push_front(buffinfo.s_BuffName);
}
