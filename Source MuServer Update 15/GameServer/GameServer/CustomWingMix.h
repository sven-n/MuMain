// ---
#include "stdafx.h"
#include "RandomManager.h"
// ---
#define MAX_WING_MIX 50
// ---
struct CUSTOM_WING_MIX
{
	int  m_Category;
	int  m_Index;
};
// ---

class CCustomWingMix
{
public:
	void Load(char* path);
	CUSTOM_WING_MIX m_Data[MAX_WING_MIX];
	int m_count;
private:

};
extern CCustomWingMix gCustomWingMix;
// ---
