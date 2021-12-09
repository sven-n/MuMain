// ItemMove.h: interface for the CItemMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct ITEM_MOVE_INFO
{
	int Index;
	int AllowDrop;
	int AllowSell;
	int AllowTrade;
	int AllowVault;
};

class CItemMove
{
public:
	CItemMove();
	virtual ~CItemMove();
	void Load(char* path);
	bool CheckItemMoveAllowDrop(int index);
	bool CheckItemMoveAllowSell(int index);
	bool CheckItemMoveAllowTrade(int index);
	bool CheckItemMoveAllowVault(int index);
private:
	std::map<int,ITEM_MOVE_INFO> m_ItemMoveInfo;
};

extern CItemMove gItemMove;
