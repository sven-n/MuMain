// MoveCommandData.cpp: implementation of the CMoveCommandData class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoveCommandData.h"

using namespace SEASON3B;

CMoveCommandData::CMoveCommandData()
{
}

CMoveCommandData::~CMoveCommandData()
{
	Release();
}

CMoveCommandData* CMoveCommandData::GetInstance()
{
	static CMoveCommandData s_Instance;
	return &s_Instance;
}

bool CMoveCommandData::Create(const std::string& filename)
{
	FILE* fp = fopen ( filename.c_str(), "rb" );
	if(fp == NULL) return false;

	int count = 0;
	fread(&count, sizeof(int), 1, fp);
	
	for(int i=0; i<count; i++)
	{		
		MOVEINFODATA* pMoveInfoData = new MOVEINFODATA;
		fread(&(pMoveInfoData->_ReqInfo), sizeof(MOVEREQINFO), 1, fp);
		BuxConvert((BYTE*)&(pMoveInfoData->_ReqInfo), sizeof(MOVEREQINFO));	
		m_listMoveInfoData.push_back(pMoveInfoData);
	}
	fclose(fp);

	return true;
}

void CMoveCommandData::Release()
{
	std::list<MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	for(; li != m_listMoveInfoData.end(); li++)
		delete (*li);
	m_listMoveInfoData.clear();
}

void CMoveCommandData::BuxConvert(BYTE* pBuffer, int size)
{
	BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
	for(int i=0;i<size;i++)
		pBuffer[i] ^= bBuxCode[i%3];
}

bool CMoveCommandData::OpenMoveReqScript(const std::string& filename) 
{
	return CMoveCommandData::GetInstance()->Create(filename);
}

int CMoveCommandData::GetNumMoveMap()
{	
	if( m_listMoveInfoData.size() > 0 )
		return m_listMoveInfoData.size();

	return -1;
}

const CMoveCommandData::MOVEINFODATA* CMoveCommandData::GetMoveCommandDataByIndex( int iIndex )
{
	std::list<MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	for(; li != m_listMoveInfoData.end(); li++)
	{
		if( (*li)->_ReqInfo.index == iIndex )
		{
			return (*li);
		}
	}
	return 0;
}

const std::list<CMoveCommandData::MOVEINFODATA*>& CMoveCommandData::GetMoveCommandDatalist()
{
	return m_listMoveInfoData;
}


