// Message.h: interface for the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct MESSAGE_INFO
{
	int Index;
	char Message[128];
};

class CMessage
{
public:
	CMessage();
	virtual ~CMessage();
	void Load(char* path);
	char* GetMessage(int index);
private:
	char m_DefaultMessage[128];
	std::map<int,MESSAGE_INFO> m_MessageInfo;
};

extern CMessage gMessage;
