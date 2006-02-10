/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GMACROS_H__
#define __GMACROS_H__

#include <string.h>
#include <stdio.h>
#ifdef WIN32
#	include <malloc.h>
#else // WIN32
#	include <alloca.h>
#endif // !WIN32
#ifndef WIN32
#	include <sys/stat.h>
#	include <cassert>
#endif // !WIN32

// ********************************************
// *************  Useful Macros  **************
// ********************************************

inline int MIN(int a, int b)
{
	return a < b ? a : b;
}

inline unsigned int MIN(unsigned int a, unsigned int b)
{
	return a < b ? a : b;
}

inline float MIN(float a, float b)
{
	return a < b ? a : b;
}

inline double MIN(double a, double b)
{
	return a < b ? a : b;
}

inline int MAX(int a, int b)
{
	return a > b ? a : b;
}

inline unsigned int MAX(unsigned int a, unsigned int b)
{
	return a > b ? a : b;
}

inline float MAX(float a, float b)
{
	return a > b ? a : b;
}

inline double MAX(double a, double b)
{
	return a > b ? a : b;
}

#ifdef WIN32
#define BAD_HANDLE (void*)0xffffffff
#else // WIN32
#define HANDLE unsigned long
#define SOCKET int
#define BAD_HANDLE 0xffffffff
#define INVALID_SOCKET -1
#endif // else WIN32




#ifndef UCHAR
#define UCHAR(c) ((c) & (~32))
#endif // UCHAR

inline float ABS(float f)
{
	return (f >= 0 ? f : -f);
}

inline double ABS(double d)
{
	return (d >= 0 ? d : -d);
}

inline int ABS(int n)
{
	return (int)((unsigned int)n & 0x7fffffff);
}

#ifndef ABS
#define ABS(n) ((n) >= 0 ? (n) : (-(n)))
#endif // ABS

#ifndef PI
#define PI (3.14159265358979323846)
#endif // PI

#define MEMBEROFFSET(type, member) ((int)&(((type*)0)->member))


#ifndef GAssert
#ifdef _DEBUG
#ifdef WIN32
#define GAssert(x,y)				\
				{					\
					if(!(x))		\
					{				\
						__asm int 3	\
					}				\
				}
#else // WIN32
#define GAssert(x,y)                        \
                {                           \
				    if(!(x))                \
					{                       \
						fprintf(stderr, "Debug Assert Failed: %s\n", y); \
						assert(x);          \
				    }                       \
				}
#endif // !WIN32
#else // _DEBUG
#define GAssert(x,y)	((void)0)
#endif // else _DEBUG
#endif // GAssert

void ThrowError(const wchar_t* wszFormat, ...);


// Macro for allocating a temporary buffer
#define MAX_STACK_TEMP_BUFFER 1024
class GTempBufHelper
{
public:
	char* m_pBuf;
	GTempBufHelper(int nSize)
	{
		m_pBuf = ((nSize > MAX_STACK_TEMP_BUFFER) ? new char[nSize] : NULL);
	}

	~GTempBufHelper()
	{
		if(m_pBuf) delete(m_pBuf);
	}
};

#ifdef WIN32
#define GTEMPBUF(pBuf, nSize)								\
	GTempBufHelper pBuf##__(nSize);							\
	char* pBuf = (((nSize) <= MAX_STACK_TEMP_BUFFER) ? (char*)alloca(nSize) : pBuf##__.m_pBuf);
#else
#define GTEMPBUF(pBuf, nSize)								\
	GTempBufHelper pBuf##__(nSize);							\
	char* pBuf = (((nSize) <= MAX_STACK_TEMP_BUFFER) ? (char*)alloca(nSize) : pBuf##__.m_pBuf);
#endif // !WIN32

// Macro for converting Unicode to Ansi
#define ConvertUnicodeToAnsi(wszUnicode, szAnsi)            \
    char* szAnsi = (char*)alloca(wcslen(wszUnicode) + 1);   \
    do                                                      \
    {                                                       \
        int n;                                              \
        for(n = 0; wszUnicode[n] != L'\0'; n++)             \
            szAnsi[n] = (char)wszUnicode[n];                \
        szAnsi[n] = '\0';                                   \
    } while(false);

// Macro for converting Ansi to Unicode
#define ConvertAnsiToUnicode(szAnsi, wszUnicode)                                \
    wchar_t* wszUnicode = (wchar_t*)alloca((strlen(szAnsi) + 1) * sizeof(wchar_t));  \
    do                                                                          \
    {                                                                           \
        int n;                                                                  \
        for(n = 0; szAnsi[n] != '\0'; n++)                                      \
            wszUnicode[n] = (wchar_t)szAnsi[n];                                 \
        wszUnicode[n] = L'\0';                                                  \
    } while(false);


// A Holder will hold a pointer to allocated memory and will guarantee to
// delete it when the holder is deleted.  (Usually you will put a holder on
// the stack so it will free up the memory it holds a pointer to even if
// an exception is thrown.
template <class T>
class Holder
{
private:
	T m_p;

public:
	Holder(T p)
	{
		m_p = p;
	}

	~Holder()
	{
		delete(m_p);
	}

	void Set(T p)
	{
		delete(m_p);
		m_p = p;
	}

	T Get()
	{
		return m_p;
	}

	T Drop()
	{
		T pTmp = m_p;
		m_p = NULL;
		return pTmp;
	}
};

template <class T>
class ArrayHolder
{
private:
	T m_p;

public:
	ArrayHolder(T p)
	{
		m_p = p;
	}

	~ArrayHolder()
	{
		delete [] m_p;
	}

	void Set(T p)
	{
		delete [] m_p;
		m_p = p;
	}

	T Get()
	{
		return m_p;
	}

	T Drop()
	{
		T pTmp = m_p;
		m_p = NULL;
		return pTmp;
	}
};

class FileHolder
{
private:
	FILE* m_pFile;

public:
	FileHolder(FILE* pFile)
	{
		m_pFile = pFile;
	}

	~FileHolder()
	{
		if(m_pFile)
			fclose(m_pFile);
	}

	void Set(FILE* pFile)
	{
		if(m_pFile)
			fclose(m_pFile);
		m_pFile = pFile;
	}

	FILE* Get()
	{
		return m_pFile;
	}

	FILE* Drop()
	{
		FILE* pFile = m_pFile;
		m_pFile = NULL;
		return pFile;
	}
};


class StringHolderArray
{
public:
	int m_nSize;
	char** m_pData;

	StringHolderArray(int nSize)
	{
		m_nSize = nSize;
		if(nSize > 0)
		{
			m_pData = new char*[nSize];
			memset(m_pData, '\0', nSize * sizeof(char*));
		}
		else
			m_pData = NULL;
	}

	~StringHolderArray()
	{
		int n;
		for(n = 0; n < m_nSize; n++)
			delete(m_pData[n]);
		delete(m_pData);
	}
};



inline unsigned char HexToByte(char h1, char h2)
{
	char v1, v2;
	if(h1 <= '9')
		v1 = h1 - '0';
	else if(h1 <= 'Z')
		v1 = h1 - 'A' + 10;
	else
		v1 = h1 - 'a' + 10;
	if(h2 <= '9')
		v2 = h2 - '0';
	else if(h2 <= 'Z')
		v2 = h2 - 'A' + 10;
	else
		v2 = h2 - 'a' + 10;
	GAssert(v1 >= 0 && v1 <= 15 && v2 >= 0 && v2 <= 15, "bad hex character");
	return(v1 | (v2 << 4));
}

inline void ByteToHex(unsigned char byte, char* pHex)
{
	pHex[0] = (byte & 15) + '0';
	if(pHex[0] > '9')
		pHex[0] += ('a' - '0' - 10);
	pHex[1] = (byte >> 4) + '0';
	if(pHex[1] > '9')
		pHex[1] += ('a' - '0' - 10);
}

// pHex should point to a buffer that is at 2 * nBufferSize + 1
inline void BufferToHex(const unsigned char* pBuffer, int nBufferSize, char* pHex)
{
	int n;
	for(n = 0; n < nBufferSize; n++)
		ByteToHex(pBuffer[n], &pHex[n << 1]);
	pHex[2 * n] = '\0';
}

// pBuffer should be half the size of nHexSize
inline void HexToBuffer(const char* pHex, int nHexSize, unsigned char* pBuffer)
{
	GAssert(nHexSize % 2 == 0, "not a multiple of 2");
	nHexSize /= 2;
	int n;
	for(n = 0; n < nHexSize; n++)
		pBuffer[n] = HexToByte(pHex[2 * n], pHex[2 * n + 1]);
}






// ----------------------------
// Platform Compatability Stuff
// ----------------------------

#ifdef WIN32
// The Windows version of swprintf doesn't conform to the prototype required by the ISO C Standard,
// but _snwprintf is provided to replace it with a safe version that does conform to the standard.
#define swprintf _snwprintf

#else // WIN32
inline char* itoa(int n, char* szBuf, int b)
{
	sprintf(szBuf, "%d", n);
	return szBuf;
}

inline int stricmp(const char* szA, const char* szB)
{
	while(*szA)
	{
		if((*szA | 32) < (*szB | 32))
			return -1;
		if((*szA | 32) > (*szB | 32))
			return 1;
		szA++;
		szB++;
	}
	if(*szB)
		return -1;
	return 0;
}

inline int wcsicmp(const wchar_t* wszA, const wchar_t* wszB)
{
	while(*wszA)
	{
		if((*wszA | 32) < (*wszB | 32))
			return -1;
		if((*wszA | 32) > (*wszB | 32))
			return 1;
		wszA++;
		wszB++;
	}
	if(*wszB)
		return -1;
	return 0;
}

inline int strnicmp(const char* szA, const char* szB, int len)
{
	int n;
	for(n = 0; n < len; n++)
	{
		if((*szA | 32) < (*szB | 32))
			return -1;
		if((*szA | 32) > (*szB | 32))
			return 1;
		szA++;
		szB++;
	}
	return 0;
}

inline long filelength(int filedes)
{
    struct stat s;
    if(fstat(filedes, &s) == -1)
    	return 0;
    return s.st_size;
}

inline void _splitpath(const char* szFull, char* szDrive, char* szDir, char* szFile, char* szExt)
{
	if(szDrive)
		strcpy(szDrive, "");
	int n;
	int lastSlash = -1;
	for(n = 0; szFull[n] != '\0'; n++)
	{
		if(szFull[n] == '/')
			lastSlash = n;
	}
	if(szDir)
	{
		if(lastSlash >= 0)
		{
			memcpy(szDir, szFull, lastSlash + 1);
			szDir[lastSlash + 1] = '\0';
		}
		else
			strcpy(szDir, "");
	}
	int lastPeriod = -1;
	for(n = lastSlash + 1; szFull[n] != '\0'; n++)
	{
		if(szFull[n] == '.')
			lastPeriod = n;
	}
	if(szFile)
	{
		if(lastPeriod >= 0)
		{
			memcpy(szFile, szFull + lastSlash + 1, lastPeriod - lastSlash - 1);
			szFile[lastPeriod - lastSlash - 1] = '\0';
		}
		else
			strcpy(szFile, szFull + lastSlash + 1);
	}
	if(szExt)
	{
		if(lastPeriod >= 0)
			strcpy(szExt, szFull + lastPeriod);
		else
			strcpy(szExt, "");
	}
}

inline void _makepath(char* szFull, const char* szDrive, const char* szDir, const char* szFile, const char* szExt)
{
	szFull[0] = '\0';
	if(szDir)
		strcpy(szFull, szDir);
	if(szFile)
		strcat(szFull, szFile);
	if(szExt)
		strcat(szFull, szExt);
}

#endif // !WIN32

#endif // __GMACROS_H__
