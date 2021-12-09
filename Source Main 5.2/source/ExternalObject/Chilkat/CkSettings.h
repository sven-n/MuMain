// CkSettings.h: interface for the CkSettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CKSETTINGS_H__62E2D44A_4858_429D_994D_A6EAAD2FFEA1__INCLUDED_)
#define AFX_CKSETTINGS_H__62E2D44A_4858_429D_994D_A6EAAD2FFEA1__INCLUDED_

#pragma once

#pragma pack (push, 8) 

class CkSettings  
{
public:
	CkSettings();
	virtual ~CkSettings();

	// CLASS: CkSettings

	// BEGIN PUBLIC INTERFACE

	// Call this once at the beginning of your program
	// if your program is multithreaded.
	static void initializeMultithreaded(void);

	// Call this once at the end of your program.
	// It is only necessary if you are testing for memory leaks.
	static void cleanupMemory(void);


	// Get the sum of the sizes of all the process heaps.
	static unsigned long getTotalSizeProcessHeaps(void);



	// END PUBLIC INTERFACE
};
#pragma pack (pop)

#endif // !defined(AFX_CKSETTINGS_H__62E2D44A_4858_429D_994D_A6EAAD2FFEA1__INCLUDED_)
