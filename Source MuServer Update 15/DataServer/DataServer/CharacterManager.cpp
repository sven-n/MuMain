// CharacterManager.cpp: implementation of the CCharacterManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterManager.h"
#include "ServerManager.h"
#include "Util.h"

CCharacterManager gCharacterManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharacterManager::CCharacterManager() // OK
{

}

CCharacterManager::~CCharacterManager() // OK
{

}

void CCharacterManager::ClearServerCharacterInfo(WORD ServerCode) // OK
{
	this->m_critical.lock();

	for(std::map<std::string,CHARACTER_INFO>::iterator it=this->m_CharacterInfo.begin();it != this->m_CharacterInfo.end();)
	{
		if(it->second.GameServerCode != ServerCode)
		{
			it++;
			continue;
		}

		it = this->m_CharacterInfo.erase(it);
	}

	this->m_critical.unlock();
}

bool CCharacterManager::GetCharacterInfo(CHARACTER_INFO* lpCharacterInfo,char* name) // OK
{
	this->m_critical.lock();

	std::string nme(name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,CHARACTER_INFO>::iterator it = this->m_CharacterInfo.find(nme);

	if(it != this->m_CharacterInfo.end())
	{
		(*lpCharacterInfo) = it->second;
		this->m_critical.unlock();
		return 1;
	}

	this->m_critical.unlock();
	return 0;
}

void CCharacterManager::InsertCharacterInfo(CHARACTER_INFO CharacterInfo) // OK
{
	this->m_critical.lock();

	std::string nme(CharacterInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,CHARACTER_INFO>::iterator it = this->m_CharacterInfo.find(nme);

	if(it == this->m_CharacterInfo.end())
	{
		this->m_CharacterInfo.insert(std::pair<std::string,CHARACTER_INFO>(nme,CharacterInfo));
	}
	else
	{
		it->second = CharacterInfo;
	}

	this->m_critical.unlock();
}

void CCharacterManager::RemoveCharacterInfo(CHARACTER_INFO CharacterInfo) // OK
{
	this->m_critical.lock();

	std::string nme(CharacterInfo.Name);

	std::transform(nme.begin(),nme.end(),nme.begin(),tolower);

	std::map<std::string,CHARACTER_INFO>::iterator it = this->m_CharacterInfo.find(nme);

	if(it != this->m_CharacterInfo.end())
	{
		this->m_CharacterInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

long CCharacterManager::GetCharacterCount() // OK
{
	long CharacterCount = 0;

	this->m_critical.lock();

	CharacterCount = this->m_CharacterInfo.size();

	this->m_critical.unlock();

	return CharacterCount;
}
