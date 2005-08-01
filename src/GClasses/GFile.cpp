/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GFile.h"
#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif // WIN32
#include <stdio.h>
#include "GMacros.h"

bool GFile::DoesFileExist(const char *szFilename)
{
   return(access(szFilename, 0) == 0);
}

bool GFile::DoesDirExist(const char *szDir)
{
	char szBuff[256];
	char* pCurDir = getcwd(szBuff, 255);
	int nVal = chdir(szDir);
	chdir(pCurDir);
	return(nVal == 0);
}

bool GFile::MakeDir(char* szDir)
{
	bool bOK = false;
	int n;
	for(n = 0; ; n++)
	{
		if(szDir[n] == '/' || szDir[n] == '\\' || szDir[n] == '\0')
		{
			char cTmp = szDir[n];
			szDir[n] = '\0';
#ifdef WIN32
			bOK = (mkdir(szDir) == 0);
#else // WIN32
			bOK = (mkdir(szDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0); // read/write/search permissions for owner and group, and with read/search permissions for others
#endif // !WIN32
			szDir[n] = cTmp;
			if(cTmp == '\0')
				break;
		}
	}
	return bOK;
}

// Find the lash slash and return what's past that
const char* GFile::ClipPath(const char* szBuff)
{
	int n = 0;
	int i = 0;
	while(szBuff[i] != 0)
	{
		if(szBuff[i] == '\\' || szBuff[i] == '/')
			n = i + 1;
		i++;
	}
	return(szBuff + n);
}

// Find the lash slash and set it to '\0'
char* GFile::ClipFilename(char* szBuff)
{
	int n = -1;
	int i = 0;
	while(szBuff[i] != 0)
	{
		if(szBuff[i] == '\\' || szBuff[i] == '/')
			n = i;
		i++;
	}
   if(n > -1)
      szBuff[n + 1] = '\0';
	return szBuff;
}

bool GFile::CpyFile(const char* szSrcPath, const char* szDestPath)
{
	FILE* pSrc = fopen(szSrcPath, "rb");
	if(!pSrc)
		return false;
	FILE* pDest = fopen(szDestPath, "wb");
	if(!pDest)
	{
		fclose(pSrc);
		return false;
	}
	char szBuf[1024];
	int nFileSize = filelength(fileno(pSrc));
	int nChunkSize;
	while(nFileSize > 0)
	{
		nChunkSize = MIN(nFileSize, 1024);
		fread(szBuf, nChunkSize, 1, pSrc);
		fwrite(szBuf, nChunkSize, 1, pDest);
		nFileSize -= nChunkSize;
	}
	fclose(pSrc);
	fclose(pDest);
	return true;
}

/*static*/ char* GFile::LoadFileToBuffer(const char* szFilename, int* pnSize)
{
	// Load the script
	FileHolder fh(fopen(szFilename, "rb"));
	FILE* pFile = fh.Get();
	if(!pFile)
		return NULL;
	int nFileSize = filelength(fileno(pFile));
	*pnSize = nFileSize;
	Holder<char*> hBuffer(new char[nFileSize + 1]);
	GAssert(hBuffer.Get(), "out of memory");
	int nBytesRead = fread(hBuffer.Get(), sizeof(char), nFileSize, pFile);
	int err = ferror(pFile);
	if(err != 0 || nBytesRead != nFileSize)
	{
		GAssert(false, "Error reading file");
		return NULL;
	}
	hBuffer.Get()[nFileSize] = '\0';
	return hBuffer.Drop();
}

/*static*/ bool GFile::SaveBufferToFile(unsigned char* pBuf, int nSize, const char* szFilename)
{
	FileHolder fh(fopen(szFilename, "wb"));
	FILE* pFile = fh.Get();
	if(!pFile)
		return false;
	if(fwrite(pBuf, nSize, 1, pFile) != 1)
		return false;
	return true;
}
