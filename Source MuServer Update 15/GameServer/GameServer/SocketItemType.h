// SocketItemType.h: interface for the CSocketItemType class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"

struct SOCKET_ITEM_TYPE_INFO
{
	int Index;
	int MaxSocket;
};

class CSocketItemType
{
public:
	CSocketItemType();
	virtual ~CSocketItemType();
	void Load(char* path);
	bool CheckSocketItemType(int index);
	long GetSocketItemMaxSocket(int index);
private:
	std::map<int,SOCKET_ITEM_TYPE_INFO> m_SocketItemTypeInfo;
};

extern CSocketItemType gSocketItemType;
