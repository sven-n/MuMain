// w_BuffStateSystem.cpp: implementation of the BuffStateSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_BuffStateSystem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BuffStateSystemPtr BuffStateSystem::Make()
{
    BuffStateSystemPtr buffstatesystem(new BuffStateSystem);
    buffstatesystem->Initialize();
    return buffstatesystem;
}

BuffStateSystem::BuffStateSystem()
{
}

BuffStateSystem::~BuffStateSystem()
{
    Destroy();
}

void BuffStateSystem::Initialize()
{
    // �ӽ� �ڵ�
    m_BuffInfo = BuffScriptLoader::Make();
    m_BuffTimeControl = BuffTimeControl::Make();
    m_BuffStateValueControl = BuffStateValueControl::Make();
}

void BuffStateSystem::Destroy()
{
}