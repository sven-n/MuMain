// KanturuEntranceNPC.h: interface for the CKanturuEntranceNPC class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CKanturuEntranceNPC
{
public:
	CKanturuEntranceNPC();
	virtual ~CKanturuEntranceNPC();
	void CGKanturuEnterInfoRecv(int aIndex);
	void CGKanturuEnterRecv(int aIndex);
};

extern CKanturuEntranceNPC gKanturuEntranceNPC;
