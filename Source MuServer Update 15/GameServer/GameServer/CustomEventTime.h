// ---
#include "Protocol.h"
// ---
#define MAX_EVENTTIME  42
// ---

struct CUSTOM_EVENTTIME_DATA
{
	int index;
	int time;
};

//**********************************************//
//********** GameServer -> Cliente    **********//
//**********************************************//

struct PMSG_CUSTOM_EVENTTIME_SEND
{
	PSWMSG_HEAD header; 
	int count;
};

//**********************************************//
//********** Cliente -> GameServer    **********//
//**********************************************//

struct PMSG_CUSTOM_EVENTTIME_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
};

// ---
class CCustomEventTime
{
public:
	void GCReqEventTime(int Index,PMSG_CUSTOM_EVENTTIME_RECV* pMsg);
	// ---

private:
	int m_count;
	// ---
	CUSTOM_EVENTTIME_DATA r_Data[MAX_EVENTTIME];
};
extern CCustomEventTime gCustomEventTime;
// ---
