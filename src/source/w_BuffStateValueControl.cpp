// w_BuffStateControl.cpp: implementation of the BuffStateControl class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIManager.h"
#include "ItemAddOptioninfo.h"
#include "w_BuffStateValueControl.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BuffStateValueControlPtr BuffStateValueControl::Make()
{
    BuffStateValueControlPtr buffstatecontrol(new BuffStateValueControl);
    return buffstatecontrol;
}

BuffStateValueControl::BuffStateValueControl()
{
    Initialize();
}

BuffStateValueControl::~BuffStateValueControl()
{
    Destroy();
}

void BuffStateValueControl::Initialize()
{
    for (int i = eBuff_Attack; i < eBuff_Count; ++i)
    {
        CheckValue(static_cast<eBuffState>(i));
    }
}

void BuffStateValueControl::Destroy()
{
}

eBuffValueLoadType BuffStateValueControl::CheckValue(eBuffState bufftype)
{
    if (bufftype >= eBuff_Attack && bufftype <= eBuff_GMEffect)
    {
        return eBuffValueLoad_None;
    }
    else
    {
        return eBuffValueLoad_ItemAddOption;
    }
}

void BuffStateValueControl::SetValue(eBuffState bufftype, BuffStateValueInfo& valueinfo)
{
    eBuffValueLoadType loadtype = CheckValue(bufftype);

    if (eBuffValueLoad_ItemAddOption == loadtype)
    {
        const BuffInfo buffinfo = g_BuffInfo(bufftype);
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEMINDEX(buffinfo.s_ItemType, buffinfo.s_ItemIndex));

        valueinfo.s_Value1 = Item_data.m_byValue1;
        valueinfo.s_Value2 = Item_data.m_byValue2;
        valueinfo.s_Value2 = Item_data.m_Time;
    }
    else
    {
        valueinfo.s_Value1 = 0;
        valueinfo.s_Value2 = 0;
        valueinfo.s_Value2 = 0;
    }
}

const BuffStateValueControl::BuffStateValueInfo BuffStateValueControl::GetValue(eBuffState bufftype)
{
    BuffStateValueInfo tempvalueinfo;

    auto iter = m_BuffStateValue.find(bufftype);

    if (iter == m_BuffStateValue.end())
    {
        const BuffInfo buffinfo = g_BuffInfo(bufftype);

        if (buffinfo.s_ItemType != 255)
        {
            SetValue(bufftype, tempvalueinfo);
        }

        m_BuffStateValue.insert(std::make_pair(bufftype, tempvalueinfo));
    }
    else
    {
        tempvalueinfo = (*iter).second;
    }

    return tempvalueinfo;
}

void BuffStateValueControl::GetBuffInfoString(std::list<std::wstring>& outstr, eBuffState bufftype)
{
    BuffStateValueInfo tempvalueinfo;
    tempvalueinfo = GetValue(bufftype);

    const BuffInfo buffinfo = g_BuffInfo(bufftype);

    outstr = buffinfo.s_BuffDescriptlist;
    outstr.push_front(L"\n");
    outstr.push_front(buffinfo.s_BuffName);

    /*
        if( tempvalueinfo.s_Value1 != 0 && tempvalueinfo.s_Value2 != 0 )
        {
            outstr.push_back(buffinfo.s_BuffName);
            outstr.push_back(( format( buffinfo.s_BuffDescript ) % tempvalueinfo.s_Value1 % tempvalueinfo.s_Value2 ).str());
        }
        else if( tempvalueinfo.s_Value1 != 0 && tempvalueinfo.s_Value2 == 0 )
        {
            outstr.push_back(buffinfo.s_BuffName);
            outstr.push_back(( format( buffinfo.s_BuffDescript ) % tempvalueinfo.s_Value1 ).str());
        }
        else if( tempvalueinfo.s_Value1 == 0 && tempvalueinfo.s_Value2 != 0 )
        {
            outstr.push_back(buffinfo.s_BuffName);
            outstr.push_back(( format( buffinfo.s_BuffDescript ) % tempvalueinfo.s_Value2 ).str());
        }
        else if( tempvalueinfo.s_Value1 != 0 && tempvalueinfo.s_Value2 != 0 )
        {
            outstr.push_back(buffinfo.s_BuffName);
            outstr.push_back(( format( buffinfo.s_BuffDescript ) ).str());
        }
    */
}

void BuffStateValueControl::GetBuffValueString(std::wstring& outstr, eBuffState bufftype)
{
    BuffStateValueInfo tempvalueinfo;
    tempvalueinfo = GetValue(bufftype);

    wchar_t buff[60];

    if (tempvalueinfo.s_Value1 != 0)
    {
        swprintf(buff, L"%d", tempvalueinfo.s_Value1);
        outstr = buff;
    }
    else
    {
        outstr = L"";
    }
}