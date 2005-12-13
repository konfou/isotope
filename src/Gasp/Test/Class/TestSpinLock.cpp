/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "../../../GClasses/GMacros.h"
#include "../../../GClasses/GThread.h"
#include "TestSpinLock.h"
#include "../../../GClasses/GSpinLock.h"
#include "../../../GClasses/GMacros.h"
#include "../ClassTests.h"
#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32

#define THREAD_COUNT 3 // 100
#define THREAD_ITTERATIONS 50 // 2000

struct TestSpinLockThreadStruct
{
	int* pBalance;
	bool* pExitFlag;
	GSpinLock* pSpinLock;
	int nOne;
};

// This thread increments the balance a bunch of times.  We use a dilly-dally loop
// instead of just calling Sleep because we don't want our results to reflect
// random context-switches that can happen at any point.
unsigned int TestSpinLockThread(void* pParameter)
{
	struct TestSpinLockThreadStruct* pThreadStruct = (struct TestSpinLockThreadStruct*)pParameter;
	int n;
	for(n = 0; n < THREAD_ITTERATIONS; n++)
	{
		// Take the lock
		pThreadStruct->pSpinLock->Lock("TestSpinLockThread");

		// read the balance
		int nBalance = *pThreadStruct->pBalance;

		// We increment nBalance in this funny way so that a smart optimizer won't
		// figure out that it can remove the nBalance variable from this logic.
		nBalance += pThreadStruct->nOne;

		// update the balance
		*pThreadStruct->pBalance = nBalance;

		// Release the lock
		pThreadStruct->pSpinLock->Unlock();
	}

	// Clean up and exit
	GAssert(*pThreadStruct->pExitFlag == false, "expected this to be false");
	*pThreadStruct->pExitFlag = true;
	delete(pThreadStruct);
	return 1;
}

bool TestSpinLock(ClassTests* pThis)
{
	bool exitFlags[THREAD_COUNT];
	int n;
	for(n = 0; n < THREAD_COUNT; n++)
		exitFlags[n] = false;
	int nBalance = 0;
	GSpinLock sl;

	// spawn a bunch of threads
	for(n = 0; n < THREAD_COUNT; n++)
	{
		TestSpinLockThreadStruct* pThreadStruct = new struct TestSpinLockThreadStruct;
		pThreadStruct->pBalance = &nBalance;
		pThreadStruct->pExitFlag = &exitFlags[n];
		pThreadStruct->pSpinLock = &sl;
		pThreadStruct->nOne = 1;
		HANDLE hThread = GThread::SpawnThread(TestSpinLockThread, pThreadStruct);
		if(hThread == BAD_HANDLE)
			return false;
	}

	// wait until all the threads are done
	while(true)
	{
		bool bDone = true;
		for(n = 0; n < THREAD_COUNT; n++)
		{
			if(!exitFlags[n])
			{
				bDone = false;
#ifdef WIN32
				Sleep(0);
#else
				usleep(0);
#endif // !WIN32
				break;
			}
		}
		if(bDone)
			break;
	}

	// Check the final balance
	if(nBalance != THREAD_COUNT * THREAD_ITTERATIONS)
		return false;

	return true;
}


