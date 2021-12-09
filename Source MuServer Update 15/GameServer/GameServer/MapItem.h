// MapItem.h: interface for the CMapItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "ItemManager.h"
#include "User.h"

class CMapItem : public CItem
{
public:
	CMapItem();
	virtual ~CMapItem();
	void Init();
	void CreateItem(int index,int level,int x,int y,float dur,BYTE Option1,BYTE Option2,BYTE Option3,BYTE NewOption,BYTE SetOption,DWORD serial,BYTE JewelOfHarmonyOption,BYTE ItemOptionEx,BYTE SocketOption[MAX_SOCKET_OPTION],BYTE SocketOptionBonus,DWORD duration);
	void DropCreateItem(int index,int level,int x,int y,float dur,BYTE Option1,BYTE Option2,BYTE Option3,BYTE NewOption,BYTE SetOption,DWORD serial,int PetLevel,int PetExp,BYTE JewelOfHarmonyOption,BYTE ItemOptionEx,BYTE SocketOption[MAX_SOCKET_OPTION],BYTE SocketOptionBonus,DWORD duration);
public:
	BYTE m_X;
	BYTE m_Y;
	BYTE m_Live;
	BYTE m_Give;
	DWORD m_State;
	DWORD m_Time;
	int m_UserIndex;
	DWORD m_LootTime;
};
