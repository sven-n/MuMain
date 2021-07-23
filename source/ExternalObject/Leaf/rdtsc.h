
//. Real Date Time Stamp Counter

#ifndef _RDTSC_H_
#define _RDTSC_H_

#include <windows.h>

namespace leaf {

	inline bool IsSupportRdtsc()
	{
		// First we get the CPUID standard level 0x00000001
		DWORD reg;
		__asm
		{
			mov eax, 1
			cpuid
			mov reg, edx
		}
		
		// Then we check, if the RDTSC (Real Date Time Stamp Counter) is available.
		// This function is necessary for our measure process.
		if (reg & ( 1 << 4)) {
			return true;
		}
		return false;
	}
	
	inline __int64 GetClockCount()
	{
		__int64 i64StartClock;	//. can't use member variable
		__asm 
		{
			rdtsc
			mov dword ptr [i64StartClock+4], edx
			mov dword ptr [i64StartClock], eax
		}
		return i64StartClock;
	}
	
	inline __int64 GetCPUFrequency(UINT uiMeasureMSecs)
	{
		// First we get the CPUID standard level 0x00000001
		if(false == IsSupportRdtsc())
			return ( 0);
		
		// After that we declare some vars and check the frequency of the high
		// resolution timer for the measure process.
		// If there's no high-res timer, we exit.
		__int64 llFreq;
		if ( !QueryPerformanceFrequency( ( LARGE_INTEGER*) &llFreq))
		{
			return ( 0);
		}
		
		// Now we can init the measure process. We set the process and thread priority
		// to the highest available level (Realtime priority). Also we focus the
		// first processor in the multiprocessor system.
		HANDLE hProcess = GetCurrentProcess();
		HANDLE hThread = GetCurrentThread();
		DWORD dwCurPriorityClass = GetPriorityClass(hProcess);
		int iCurThreadPriority = GetThreadPriority(hThread);
		DWORD dwProcessMask, dwSystemMask, dwNewMask = 1;
		GetProcessAffinityMask(hProcess, &dwProcessMask, &dwSystemMask);
		
		SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		SetProcessAffinityMask(hProcess, dwNewMask);
		
		// Now we call a CPUID to ensure, that all other prior called functions are
		// completed now (serialization)
		__asm { cpuid }
		
		__int64 llStartTime, llEndTime;
		__int64 llStart, llEnd;
		// We ask the high-res timer for the start time
		QueryPerformanceCounter((LARGE_INTEGER *) &llStartTime);
		// Then we get the current cpu clock and store it
		__asm 
		{
			rdtsc
			mov dword ptr [llStart+4], edx
			mov dword ptr [llStart], eax
		}
		
		// Now we wart for some msecs
		Sleep(uiMeasureMSecs);
		
		// We ask for the end time
		QueryPerformanceCounter((LARGE_INTEGER *) &llEndTime);
		// And also for the end cpu clock
		__asm 
		{
			rdtsc
			mov dword ptr [llEnd+4], edx
			mov dword ptr [llEnd], eax
		}
		
		// Now we can restore the default process and thread priorities
		SetProcessAffinityMask(hProcess, dwProcessMask);
		SetThreadPriority(hThread, iCurThreadPriority);
		SetPriorityClass(hProcess, dwCurPriorityClass);
		
		// Then we calculate the time and clock differences
		__int64 llDif = llEnd - llStart;
		__int64 llTimeDif = llEndTime - llStartTime;
		
		// And finally the frequency is the clock difference divided by the time
		// difference. 
		__int64 llFrequency = (__int64)((( double)llDif) / ((( double)llTimeDif) / llFreq));
		// At last we just return the frequency that is also stored in the call
		// member var uqwFrequency
		return llFrequency;
	}
}

#endif // _RDTSC_H_