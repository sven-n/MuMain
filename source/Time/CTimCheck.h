
#include <VECTOR>
#include "Singleton.h"

using namespace std;

struct TimeCheck
{
	int		iIndex;			// 시간 관련 번호
	int		iBackupTime;	// 시간 관련 백업
	bool	bTimeCheck;		// 시간 관련 체크	
};

class CTimeCheck : public Singleton <CTimeCheck>
{
public:
	vector<TimeCheck> stl_Time;
	vector<TimeCheck>::iterator stl_Time_I;

	CTimeCheck();
	virtual ~CTimeCheck();

	int	 CheckIndex(int index);
	bool GetTimeCheck(int index, int DelayTime);
	void DeleteTimeIndex(int index);
};

#define g_Time CTimeCheck::GetSingleton()
