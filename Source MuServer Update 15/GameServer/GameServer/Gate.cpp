// Gate.cpp: implementation of the CGate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Gate.h"
#include "DefaultClassInfo.h"
#include "Map.h"
#include "MemScript.h"
#include "Util.h"

CGate gGate;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGate::CGate() // OK
{
	this->m_GateInfo.clear();
}

CGate::~CGate() // OK
{

}

void CGate::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_GateInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			GATE_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Flag = lpMemScript->GetAsNumber();

			info.Map = lpMemScript->GetAsNumber();

			info.X = lpMemScript->GetAsNumber();

			info.Y = lpMemScript->GetAsNumber();

			info.TX = lpMemScript->GetAsNumber();

			info.TY = lpMemScript->GetAsNumber();

			info.TargetGate = lpMemScript->GetAsNumber();

			info.Dir = lpMemScript->GetAsNumber();

			info.MinLevel = lpMemScript->GetAsNumber();

			info.MaxLevel = lpMemScript->GetAsNumber();

			info.MinReset = lpMemScript->GetAsNumber();

			info.MaxReset = lpMemScript->GetAsNumber();

			info.AccountLevel = lpMemScript->GetAsNumber();

			this->m_GateInfo.insert(std::pair<int,GATE_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CGate::GetInfo(int index,GATE_INFO* lpInfo) // OK
{
	std::map<int,GATE_INFO>::iterator it = this->m_GateInfo.find(index);

	if(it == this->m_GateInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

int CGate::GetGateMap(int index) // OK
{
	GATE_INFO info;

	if(this->GetInfo(index,&info) == 0)
	{
		return -1;
	}

	return info.Map;
}

int CGate::GetMoveLevel(LPOBJ lpObj,int map,int level) // OK
{
	if(map != MAP_SWAMP_OF_CALMNESS && (lpObj->Class == CLASS_MG || lpObj->Class == CLASS_DL || lpObj->Class == CLASS_RF))
	{
		return ((level*2)/3);
	}

	return level;
}

bool CGate::IsGate(int index) // OK
{
	GATE_INFO info;

	if(this->GetInfo(index,&info) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool CGate::IsInGate(LPOBJ lpObj,int index) // OK
{
	GATE_INFO info;

	if(this->GetInfo(index,&info) == 0)
	{
		return 0;
	}

	if(lpObj->Map != info.Map)
	{
		return 0;
	}

	if(lpObj->X < (info.TX-5) || lpObj->X > (info.TX+5) || lpObj->Y < (info.TY-5) || lpObj->Y > (info.TY+5))
	{
		return 0;
	}

	if(info.MinLevel != -1 && lpObj->Level < this->GetMoveLevel(lpObj,info.Map,info.MinLevel))
	{
		return 0;
	}

	if(info.MaxLevel != -1 && lpObj->Level > info.MaxLevel)
	{
		return 0;
	}

	if(info.MinReset != -1 && lpObj->Reset < info.MinReset)
	{
		return 0;
	}

	if(info.MaxReset != -1 && lpObj->Reset > info.MaxReset)
	{
		return 0;
	}

	if(lpObj->AccountLevel < info.AccountLevel)
	{
		return 0;
	}

	return 1;
}

bool CGate::GetGate(int index,int* gate,int* map,int* x,int* y,int* dir,int* level) // OK
{
	GATE_INFO info;

	if(this->GetInfo(index,&info) == 0)
	{
		return 0;
	}

	if(info.TargetGate != 0 && this->GetInfo(info.TargetGate,&info) == 0)
	{
		return 0;
	}

	int px,py;

	for(int n=0;n < 50;n++)
	{
		if((info.TX-info.X) > 0)
		{
			px = info.X+(GetLargeRand()%(info.TX-info.X));
		}
		else
		{
			px = info.X;
		}

		if((info.TY-info.Y) > 0)
		{
			py = info.Y+(GetLargeRand()%(info.TY-info.Y));
		}
		else
		{
			py = info.Y;
		}

		if(gMap[info.Map].CheckAttr(px,py,4) == 0 && gMap[info.Map].CheckAttr(px,py,8) == 0)
		{
			(*gate) = info.Index;
			(*map) = info.Map;
			(*x) = px;
			(*y) = py;
			(*dir) = info.Dir;
			(*level) = info.MinLevel;
			return 1;
		}
	}

	return 0;
}
