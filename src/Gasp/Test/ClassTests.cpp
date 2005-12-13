/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "ClassTests.h"
#include "../../GClasses/GString.h"
#include <math.h>
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GBigNumber.h"
#include "../../GClasses/GRayTrace.h"
#include "Class/TestMatrix.h"
#include "Class/TestPrecalcTrigTable.h"
#include "Class/TestSpinLock.h"
#include "Class/TestBigNumber.h"
#include "Class/TestAvlTree.h"
#include "Class/TestDataBase.h"
#include "Class/TestHashTable.h"
#include <wchar.h>
#include "../../GClasses/GBezier.h"
#include "../../GClasses/GMath.h"
#include "../../GClasses/GPolynomial.h"
#include "../../GClasses/GFourier.h"
#include "../../GClasses/GTrie.h"
#include "../../GClasses/GLList.h"
#include "../../GClasses/GStack.h"
#include "../../GClasses/GEZSocket.h"
#include "../../GClasses/GWindows.h"
#include "../../GClasses/GThread.h"
#include "../../GClasses/GCompress.h"
#ifndef WIN32
#include <unistd.h>
#endif // !WIN32

bool TestGFourier(ClassTests* pThis)
{
	struct ComplexNumber cn[4];
	cn[0].dReal = 1;
	cn[0].dImag = 0;
	cn[1].dReal = 1;
	cn[1].dImag = 0;
	cn[2].dReal = 1;
	cn[2].dImag = 0;
	cn[3].dReal = 1;
	cn[3].dImag = 0;
	GFourier::FFT(cn, 4);
	if(cn[0].dReal != 4)
		return false;
	if(cn[0].dImag != 0)
		return false;
	int n;
	for(n = 1; n < 3; n++)
	{
		if(cn[n].dReal != 0)
			return false;
		if(cn[n].dImag != 0)
			return false;
	}
	GFourier::FFT(cn, 4, true);
	for(n = 0; n < 3; n++)
	{
		if(cn[n].dReal != 1)
			return false;
		if(cn[n].dImag != 0)
			return false;
	}
	return true;
}

bool TestGQueue(ClassTests* pThis)
{
	GQueue queue(1025);
	int nIn = 0;
	int nOut = 0;
	char c;
	for(nIn = 0; nIn < 250611; nIn++)
	{
		queue.Push((char)nIn);
		if((rand() * 8 / RAND_MAX) == 0)
		{
			queue.Pop(&c);
			if(c != (char)nOut)
				return false;
			nOut++;
		}
	}
	while(nOut < 250611)
	{
		queue.Pop(&c);
		if(c != (char)nOut)
			return false;
		nOut++;
	}

	return true;
}

bool TestGStack(ClassTests* pThis)
{
	GStack stack(1026);
	int nValue = 0;
	int nTmp;
	int n;
	for(n = 0; n < 274333; n++)
	{
		stack.Push(nValue);
		nValue++;
		if((rand() * 8 / RAND_MAX) == 0)
		{
			if(!stack.Pop(&nTmp))
				return false;
			nValue--;
			if(nTmp != nValue)
				return false;
		}
	}
	while(nValue > 0)
	{
		nValue--;
		if(!stack.Pop(&nTmp))
			return false;
		if(nTmp != nValue)
			return false;
	}

	return true;
}

bool TestGString(ClassTests* pThis)
{
	GString s;
	if(wcscmp(s.GetString(), L"") != 0)
		return false;
	s.Add(L"Gasp");
	s.Add(" SDK");
	if(s.CompareTo(L"Gasp SDK") != 0)
		return false;
	if(s.GetLength() != 8)
		return false;

	return true;
}


bool TestGLList(ClassTests* pThis)
{
	class MyBucket : public GBucket
	{
	public:
		char m_c;
		MyBucket(char c) : GBucket() { m_c = c; }
		virtual ~MyBucket() { }
		virtual int Compare(GBucket* pBucket) { return ((m_c < ((MyBucket*)pBucket)->m_c) ? -1 : (m_c > ((MyBucket*)pBucket)->m_c ? 1 : 0)); }
	};

	GLList llist;
	int n;
	for(n = 'z'; n >= 'a'; n--)
	{
		MyBucket* pNewBucket = new MyBucket(n);
		llist.Link(pNewBucket);
	}
	llist.Sort();
	MyBucket* pBucket;
	n = 'a';
	for(pBucket = (MyBucket*)llist.GetFirst(); pBucket; pBucket = (MyBucket*)llist.GetNext(pBucket))
	{
		if(pBucket->m_c != n)
			return false;
		n++;
	}
	GBucket* pFBucket = llist.Unlink(llist.GetBucket(4));
	n = 'a';
	for(pBucket = (MyBucket*)llist.GetFirst(); pBucket; pBucket = (MyBucket*)llist.GetNext(pBucket))
	{
		if(pBucket->m_c != n)
			return false;
		n++;
		if(n == 'f')
			n++;
	}
	llist.Insert(llist.GetBucket(4), pFBucket);
	n = 'a';
	for(pBucket = (MyBucket*)llist.GetFirst(); pBucket; pBucket = (MyBucket*)llist.GetNext(pBucket))
	{
		if(pBucket->m_c != n)
			return false;
		n++;
	}
	if(llist.GetCount() != 26)
		return false;
	return true;
}

int PointerArrayComparer(void* pThis, void* pA, void* pB)
{
	if((int)pA > (int)pB)
		return 1;
	else if((int)pA < (int)pB)
		return -1;
	else
		return 0;
}

bool TestGSmallArray(ClassTests* pThis)
{
	// Add, insert, and then check
	GSmallArray sa(1, 17);
	int n;
	for(n = 0; n < 1000; n++)
	{
		char cTmp = n + 50;
		sa._AddCellByRef(&cTmp);
	}
	for(n = 0; n < 50; n++)
		sa._InsertCellByRef(0, &n);
	for(n = 0; n < 50; n++)
	{
		if(*(char*)sa._GetCellRef(n) != (char)(49 - n))
			return false;
	}
	for(n = 50; n < 1050; n++)
	{
		if(*(char*)sa._GetCellRef(n) != (char)n)
			return false;
	}
	if(sa.GetSize() != 1050)
		return false;

	// Delete and then check again
	for(n = 0; n < 49; n++)
		sa.DeleteCell(1);
	if(sa.GetSize() != 1001)
		return false;
	sa.DeleteCell(0);
	for(n = 0; n < 1000; n++)
	{
		char cTmp = n + 50;
		if(*(char*)sa._GetCellRef(n) != cTmp)
			return false;
	}

	// Test GPointerArray::Sort
	GPointerArray arr(64);
	for(n = 34; n >= 0; n--)
		arr.AddPointer((void*)n);
	arr.Sort(PointerArrayComparer, NULL);
	for(n = 0; n <= 34; n++)
	{
		if((int)arr.GetPointer(n) != n)
			return false;
	}

	return true;
}

bool TestGArray(ClassTests* pThis)
{
	class IntArray : public GSmallArray
	{
	public:
		IntArray(int nGrowBy) : GSmallArray(sizeof(int), nGrowBy) { }
		virtual ~IntArray() { }

		int GetInt(int nIndex) { return *(int*)_GetCellRef(nIndex); }
		void AddInt(int n) { _AddCellByRef(&n); }
		void SetInt(int nCell, int n) { _SetCellByRef(nCell, &n); }
	};
	
	IntArray arr(64);
	arr.SetAllocSize(7000);
	int n;
	for(n = 0; n < 100000; n++)
		arr.AddInt(n);
	if(arr.GetSize() != 100000)
		return false;
	for(n = 0; n < 100000; n++)
	{
		if(arr.GetInt(n) != n)
			return false;
	}
	for(n = 0; n < 30000; n++)
		arr.SetInt(n, 5);
	for(n = 0; n < 30000; n++)
	{
		if(arr.GetInt(n) != 5)
			return false;
	}
	return true;
}

bool IsInArray(unsigned int* pnArray, unsigned int nVal, int nArraySize)
{
	int n;
	for(n = 0; n < nArraySize; n++)
		if(pnArray[n] == nVal)
			return true;
	return false;
}

bool TestGTrie(ClassTests* pThis)
{
	const int nTestSize = 1024;
	unsigned int nArray[nTestSize];
	int n;
	GTrie trie;
	for(n = 0; n < nTestSize; n++)
	{
		unsigned int nRand = (rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24);
		nArray[n] = nRand;
		trie.Add(nRand);
	}
	if(trie.GetCount() != nTestSize)
		return false;
	for(n = 0; n < nTestSize; n++)
	{
		if(!trie.Check(nArray[n]))
			return false;
	}
	for(n = 0; n < nTestSize; n++)
	{
		unsigned int nRand = (rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24);
		if(trie.Check(nRand) != IsInArray(nArray, nRand, nTestSize))
			return false;
	}
	while(trie.GetCount() > 0)
	{
		if(rand() & 1)
		{
			if(!trie.Remove(trie.GetSmallest()))
				return false;
		}
		else
		{
			if(!trie.Remove(trie.GetBiggest()))
				return false;
		}
	}
	GTrie trie2;
	trie2.Add(237);
	trie2.Add(143);
	trie2.Add(331);
	if(trie2.GetSmallest() != 143)
		return false;
	if(trie2.GetBiggest() != 331)
		return false;
	return true;
}

bool IsPrettyClose(double a, double b)
{
	a -= b;
	if(a > .0000001)
		return false;
	if(a < -.0000001)
		return false;
	return true;
}

bool TestNewtonPolynomial(ClassTests* pThis)
{
	double t0 = 23;
	double t1 = 17;
	double t2 = 37;
	double t3 = 83;
	double x0 = 11;
	double x1 = 53;
	double x2 = 83;
	double x3 = 7;
	double t[4];
	double x[4];
	t[0] = t0;
	t[1] = t1;
	t[2] = t2;
	t[3] = t3;
	x[0] = x0;
	x[1] = x1;
	x[2] = x2;
	x[3] = x3;
	GMath::NewtonPolynomial(t, x, 4);
	if(!IsPrettyClose(x[0] + x[1] * t0 + x[2] * t0 * t0 + x[3] * t0 * t0 * t0, x0))
		return false;
	if(!IsPrettyClose(x[0] + x[1] * t1 + x[2] * t1 * t1 + x[3] * t1 * t1 * t1, x1))
		return false;
	if(!IsPrettyClose(x[0] + x[1] * t2 + x[2] * t2 * t2 + x[3] * t2 * t2 * t2, x2))
		return false;
	if(!IsPrettyClose(x[0] + x[1] * t3 + x[2] * t3 * t3 + x[3] * t3 * t3 * t3, x3))
		return false;
	return true;
}

bool TestBezier(ClassTests* pThis)
{
	srand(1234);
	Point3D p1, p2, p3;
	GBezier* pOrig = new GBezier(7);
	int n;
	for(n = 0; n < pOrig->GetControlPointCount(); n++)
	{
		p1.x = rand() % 1000000;
		p1.y = rand() % 1000000;
		p1.z = rand() % 1000000;
		pOrig->SetControlPoint(n, &p1, rand());
	}
	GBezier* pCopy = pOrig->Copy();
	pCopy->ElevateDegree();
	if(pCopy->GetControlPointCount() != pOrig->GetControlPointCount() + 1)
		return false;
	for(n = 0; n < 10; n++)
	{
		pOrig->GetPoint((double)n / 10, &p1);
		pCopy->GetPoint((double)n / 10, &p2);
		if(p1.GetDistance(&p2) > .00001)
			return false;
	}
	GBezier* pCopy2 = pCopy->Copy();
	pCopy->GetSegment(.3, false);
	pCopy2->GetSegment(.3, true);
	for(n = 0; n < 10; n++)
	{
		pOrig->GetPoint((double)n / 10, &p1);
		pCopy->GetPoint(((double)n / 10) / .3, &p2);
		pCopy2->GetPoint(((double)n / 10) / .7 - .3 / .7, &p3);
		if(p1.GetDistance(&p2) > .00001)
			return false;
		if(p1.GetDistance(&p3) > .00001)
			return false;
	}
	Point3D coeff[7];
	pOrig->ToPolynomial(coeff);
	GBezier* pAnother = GBezier::FromPolynomial(coeff, 7);
	for(n = 0; n < 10; n++)
	{
		pOrig->GetPoint((double)n / 10, &p1);
		pAnother->GetPoint((double)n / 10, &p2);
		if(p1.GetDistance(&p2) > .00001)
			return false;
	}
	delete(pOrig);
	delete(pCopy);
	delete(pCopy2);
	delete(pAnother);
	return true;
}

bool TestPolynomial(ClassTests* pThis)
{
	GPolynomial gp(2, 3);
	int degrees[2];
	degrees[0] = 0;
	degrees[1] = 0;
	gp.SetCoefficient(degrees, 1);
	degrees[0] = 1;
	degrees[1] = 0;
	gp.SetCoefficient(degrees, 2);
	degrees[0] = 2;
	degrees[1] = 0;
	gp.SetCoefficient(degrees, 3);
	degrees[0] = 0;
	degrees[1] = 1;
	gp.SetCoefficient(degrees, 4);
	degrees[0] = 1;
	degrees[1] = 1;
	gp.SetCoefficient(degrees, 5);
	degrees[0] = 2;
	degrees[1] = 1;
	gp.SetCoefficient(degrees, 6);
	degrees[0] = 0;
	degrees[1] = 2;
	gp.SetCoefficient(degrees, 7);
	degrees[0] = 1;
	degrees[1] = 2;
	gp.SetCoefficient(degrees, 8);
	degrees[0] = 2;
	degrees[1] = 2;
	gp.SetCoefficient(degrees, 9);
	double vars[2];
	vars[0] = 7;
	vars[1] = 11;
	double val = gp.Eval(vars);
	// 1 + 2 * (7) + 3 * (7 * 7) +
	// 4 * (11) + 5 * (11 * 7) + 6 * (11 * 7 * 7) +
	// 7 * (11 * 11) + 8 * (11 * 11 * 7) + 9 * (11 * 11 * 7 * 7)
	// = 64809
	if(val != 64809)
		return false;
	return true;
}

#define TEST_SOCKET_PORT 4464

bool TestGEZSocketSerial(ClassTests* pThis, bool bGash)
{
	// Establish a socket connection
	Holder<GEZSocketServer*> hServer(bGash ? GEZSocketServer::HostGashSocket(TEST_SOCKET_PORT, 5000) : GEZSocketServer::HostTCPSocket(TEST_SOCKET_PORT));
	GEZSocketServer* pServer = hServer.Get();
	if(!pServer)
		return false;
	Holder<GEZSocketClient*> hClient(bGash ? GEZSocketClient::ConnectToGashSocket("localhost", TEST_SOCKET_PORT, 5000) : GEZSocketClient::ConnectToTCPSocket("localhost", TEST_SOCKET_PORT));
	GEZSocketClient* pClient = hClient.Get();
	if(!pClient)
		return false;

	// Send a bunch of data
	int i;
	char szBuf[5000];
	for(i = 0; i < 5000; i++)
		szBuf[i] = (char)i;
	pClient->Send(szBuf, 5000);
#ifdef WIN32
	GWindows::YieldToWindows();
#endif // WIN32
	for(i = 10; i < 60; i++)
	{
		if(!pClient->Send(szBuf + i, i))
			return false;
#ifdef WIN32
		GWindows::YieldToWindows();
#endif // WIN32
	}
	pClient->Send(szBuf, 5000);
#ifdef WIN32
	GWindows::YieldToWindows();
#endif // WIN32

	// Wait for the data to arrive
	int nTimeout;
	for(nTimeout = 500; nTimeout > 0; nTimeout--)
	{
		if(pServer->GetMessageCount() == 52)
			break;
#ifdef WIN32			
		GWindows::YieldToWindows();
		Sleep(50);
#else // WIN32
		usleep(50);
#endif // else WIN32
	}
	if(pServer->GetMessageCount() != 52)
		return false;

	// Check the data and send some of it back to the client
	int nSize, nConnection;
	{
		Holder<unsigned char*> hData = pServer->GetNextMessage(&nSize, &nConnection);
		unsigned char* pData = hData.Get();
		if(!pData || pData == (unsigned char*)szBuf)
			return false;
		if(nSize != 5000)
			return false;
		if(pData[122] != (char)122)
			return false;
	}
	for(i = 10; i < 60; i++)
	{
		Holder<unsigned char*> hData = pServer->GetNextMessage(&nSize, &nConnection);
		unsigned char* pData = hData.Get();
		if(nSize != i || !pData)
			return false;
		if(pData[0] != (char)i)
			return false;
		if(pData[i - 1] != (char)(i + i - 1))
			return false;
		if(!pServer->Send(pData, 10, nConnection))
			return false;
#ifdef WIN32
		GWindows::YieldToWindows();
#endif // WIN32
	}
	{
		Holder<unsigned char*> hData = pServer->GetNextMessage(&nSize, &nConnection);
		unsigned char* pData = hData.Get();
		if(!pData || pData == (unsigned char*)szBuf)
			return false;
		if(nSize != 5000)
			return false;
		if(pData[122] != (char)122)
			return false;
	}

	// Wait for the data to arrive
	for(nTimeout = 500; nTimeout > 0; nTimeout--)
	{
		if(pClient->GetMessageCount() == 50)
			break;
#ifdef WIN32			
		GWindows::YieldToWindows();
		Sleep(50);
#else // WIN32
		usleep(50);
#endif // else WIN32
	}
	if(pClient->GetMessageCount() != 50)
		return false;

	// Check the data
	for(i = 10; i < 60; i++)
	{
		Holder<unsigned char*> hData = pClient->GetNextMessage(&nSize);
		unsigned char* pData = hData.Get();
		if(nSize != 10 || !pData)
			return false;
		if(pData[0] != (char)i)
			return false;
		if(pData[9] != (char)(i + 9))
			return false;
	}

	return true;
}

struct TestGEZSocketParallelData
{
};

unsigned int TestGEZSocketParallelClientThread(void* pParam)
{
	TestGEZSocketParallelData* pData = (TestGEZSocketParallelData*)pParam;
	Holder<GEZSocketClient*> hClient(GEZSocketClient::ConnectToGashSocket("localhost", TEST_SOCKET_PORT, 5000));
	GEZSocketClient* pClient = hClient.Get();
	char szBuf[1025];
	memset(szBuf, 'x', 1025);
	int n;
	for(n = 0; n < 100; n++)
		pClient->Send(szBuf, 1025);
	return 0;
}

bool TestGEZSocketParallel(ClassTests* pThis)
{
	// Establish a socket connection
	Holder<GEZSocketServer*> hServer(GEZSocketServer::HostGashSocket(TEST_SOCKET_PORT, 5000));
	GEZSocketServer* pServer = hServer.Get();
	TestGEZSocketParallelData sData;
	if(!pServer)
		return false;
	GThread::SpawnThread(TestGEZSocketParallelClientThread, &sData);
	GThread::SpawnThread(TestGEZSocketParallelClientThread, &sData);
	GThread::SpawnThread(TestGEZSocketParallelClientThread, &sData);
	GThread::SpawnThread(TestGEZSocketParallelClientThread, &sData);
	GThread::SpawnThread(TestGEZSocketParallelClientThread, &sData);
	int i;
	int nSize, nConnection;
	for(i = 0; i < 500; i++)
	{
		Holder<unsigned char*> hData = pServer->GetNextMessage(&nSize, &nConnection);
		if(!hData.Get())
		{
			i--;
#ifdef WIN32
			Sleep(0);
#else // WIN32
			usleep(0);
#endif // !WIN32
			continue;
		}
		if(nSize != 1025)
			return false;
	}
	return true;
}

bool TestGEZSocket(ClassTests* pThis)
{
	if(!TestGEZSocketSerial(pThis, true))
		return false;
	if(!TestGEZSocketParallel(pThis))
		return false;
	return true;
}


bool TestGCompress(ClassTests* pThis)
{
	srand(0);
	int nSize = 4096;
	int nRands = 2;
	Holder<unsigned char*> hOrig(new unsigned char[nSize]);
	unsigned char* pOrig = hOrig.Get();
	int n;
	for(n = 0; n < nSize; n++)
		pOrig[n] = (unsigned char)(rand() % nRands + 'A');
	int nCompressedSize, nFinalSize;
	Holder<unsigned char*> hCompressed(GCompress::Compress(pOrig, nSize, &nCompressedSize));
	Holder<unsigned char*> hFinal(GCompress::Decompress(hCompressed.Get(), nCompressedSize, &nFinalSize));
	unsigned char* pFinal = hFinal.Get();
	if(nFinalSize != nSize)
		return false;
	if(memcmp(pFinal, pOrig, nSize) != 0)
		return false;
	return true;	
}

// *******************************************************************************
// *******************************************************************************
// *******************************************************************************
/*
void RunTest(char* szClassName, bool(*pFunc)())
{
	// Get the memory status
	MEMORYSTATUS ms1;
	GlobalMemoryStatus(&ms1);

	// Run the test
	bool bPassed = pFunc();

	// If memory was lost, run the test again.  It was probably due to a new DLL
	// that got loaded.  If memory is lost again, there is a problem.
	MEMORYSTATUS ms2;
	GlobalMemoryStatus(&ms2);
	if(ms2.dwAvailVirtual < ms1.dwAvailVirtual)
	{
		ms1 = ms2;
		bPassed = pFunc();
		GlobalMemoryStatus(&ms2);
	}

	// Display results
	if(ms2.dwAvailVirtual < ms1.dwAvailVirtual)
	{
		cout << "*** FAILED (Mem Leak: " << ms1.dwAvailVirtual - ms2.dwAvailVirtual << " bytes)\n";
	}
	else
	{
		if(!bPassed)
		{
			cout << "****** FAILED !!!!!!\n";
		}
		else
		{
			gnSucceedCount++;
			cout << "Passed\n";
		}
	}
}
*/

typedef bool (*ClassTestFunc)(ClassTests* pThis);

struct ClassTest
{
	const char* szName;
	ClassTestFunc pTest;
};

static struct ClassTest testTable[] = 
{
	{"GArray", TestGArray},
	{"GAVLTree", TestGAvlTree},
	{"GBezier", TestBezier},
	{"GBigNumber", TestGBigNumber},
	{"GCompress", TestGCompress},
	{"GDataBase", TestDataBase},
	{"GEZSocket", TestGEZSocket},
	{"GHashTable", TestGHashTable},
	{"GLList", TestGLList},
	{"GFourier", TestGFourier},
//	{"GMatrix", TestMatrix},
	{"GPolynomial", TestPolynomial},
	{"GPrecalculatedTrigTable", TestGPrecalculatedTrigTable},
	{"GQueue", TestGQueue},
	{"GSmallArray", TestGSmallArray},
	{"GSpinLock", TestSpinLock},
	{"GStack", TestGStack},
	{"GString", TestGString},
	{"GTrie", TestGTrie},
	{"NewtonPolynomial", TestNewtonPolynomial},
};

void ClassTests::GetCategoryName(char* szBuffer, int nBufferSize)
{
	strcpy(szBuffer, "Class Tests");
}

int ClassTests::GetTestCount()
{
	return (sizeof(testTable) / sizeof(struct ClassTest));
}

void ClassTests::GetTestName(int nTest, char* szBuffer, int nBufferSize)
{
	strcpy(szBuffer, testTable[nTest].szName);
}

bool ClassTests::RunTest(int nTest)
{
	ClassTestFunc pTest = testTable[nTest].pTest;
	return pTest(this);
}

void ClassTests::DebugTest(int nTest)
{
	ClassTestFunc pTest = testTable[nTest].pTest;
#ifdef WIN32
	__asm int 3;
#else
	// todo: what's the Linux equivalent of int 3?
#endif // !WIN32
	pTest(this);
}

