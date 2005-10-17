/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifdef WIN32








// ----------------------
//  WINDOWS VERSION
// ----------------------

#include "GDirList.h"
#include "GFindFile.h"
#include "GQueue.h"

BOOL GFileFinder::Find(LPCTSTR pFile)
{
   if(m_hFind == INVALID_HANDLE_VALUE)
   {
      m_hFind = FindFirstFile(pFile, this);
   }
   else
   {
      if(!FindNextFile(m_hFind, this))
         Close();
   }
   return m_hFind != INVALID_HANDLE_VALUE;
}

BOOL GFileFinder::IsDots()
{
   int i = 0;
   while(cFileName[i])
   {
      if(cFileName[i++] != '.')
         return FALSE;
   }
   return TRUE;
}

////////////////////////////////////////////////////////

GDirList::GDirList(bool bRecurseSubDirs, bool bReportFiles, bool bReportDirs, bool bReportPaths)
{
   m_bReportFiles = bReportFiles;
   m_bReportDirs = bReportDirs;
   m_bRecurseSubDirs = bRecurseSubDirs;
   m_bReportPaths = bReportPaths;
   GetCurrentDirectory(255, m_szOldDir);
   m_pBuffer = new GQueue();
   m_pFinder[0] = new GFileFinder();
   m_nNests = 1;
   m_pTempBuf = NULL;
}

GDirList::~GDirList()
{
   delete(m_pBuffer);
   for( ; m_nNests > 0; m_nNests--)
      delete(m_pFinder[m_nNests - 1]);
   SetCurrentDirectory(m_szOldDir);
   delete(m_pTempBuf);
}

void GDirList::SetTempBuf(char* pNewString)
{
	delete(m_pTempBuf);
	m_pTempBuf = pNewString;
}

const char* GDirList::GetNext()
{
   GFileFinder* pFinder;
   if(m_nNests > 0)
      pFinder = m_pFinder[m_nNests - 1];
   else
      return NULL;
   if(pFinder->Find("*"))
   {
      if(pFinder->IsDir())
      {
         if(pFinder->IsDots())
            return(GetNext());
         if(m_bReportDirs)
         {
            if(m_bReportPaths)
            {
               char szBuff[256];
               GetCurrentDirectory(255, szBuff);
               m_pBuffer->Flush();
			   m_pBuffer->Push(szBuff);
               m_pBuffer->Push("\\");
            }
            else
               m_pBuffer->Flush();
         }
         if(m_bRecurseSubDirs)
         {
            SetCurrentDirectory(pFinder->GetFileName());
            m_pFinder[m_nNests] = new GFileFinder();
            m_nNests++;
         }
         if(m_bReportDirs)
         {
            m_pBuffer->Push(pFinder->GetFileName());
            SetTempBuf(m_pBuffer->DumpToString());
			return m_pTempBuf;;
         }
         else
            return GetNext();
      }
		else
		{
			if(m_bReportFiles)
			{
				if(m_bReportPaths)
				{
					char szBuff[256];
					GetCurrentDirectory(255, szBuff);
					m_pBuffer->Flush();
					m_pBuffer->Push(szBuff);
					m_pBuffer->Push("\\");
				}
				else
					m_pBuffer->Flush();
				m_pBuffer->Push(pFinder->GetFileName());
				SetTempBuf(m_pBuffer->DumpToString());
				return(m_pTempBuf);
			}
			else
			{
				return GetNext();
			}
		}
   }
   else
   {
      if(m_nNests > 0)
      {
         SetCurrentDirectory("..");
         m_nNests--;
         delete(m_pFinder[m_nNests]);
         return(GetNext());
      }
      else
         return NULL;
   }
}















#else








// ----------------------
//  LINUX VERSION
// ----------------------


#include "GDirList.h"
#include "GQueue.h"
#include <unistd.h>


////////////////////////////////////////////////////////

GDirList::GDirList(bool bRecurseSubDirs, bool bReportFiles, bool bReportDirs, bool bReportPaths)
{
	m_bReportFiles = bReportFiles;
	m_bReportDirs = bReportDirs;
	m_bRecurseSubDirs = bRecurseSubDirs;
	m_bReportPaths = bReportPaths;
	getcwd(m_szOldDir, 255);
	m_pBuffer = new GQueue();
	m_nNests = 0;
	m_pTempBuf = NULL;
	m_pCurDir = opendir( "." );
}

GDirList::~GDirList()
{
	delete(m_pBuffer);
	for( ; m_nNests > 0; m_nNests--)
	{
		closedir(m_pDirs[m_nNests]);
		chdir("..");
	}
	delete(m_pTempBuf);
}

void GDirList::SetTempBuf(char* pNewString)
{
	delete(m_pTempBuf);
	m_pTempBuf = pNewString;
}

const char* GDirList::GetNext()
{
	//The current directory isn't opening
	if(m_pCurDir == NULL)
		return NULL;

	struct dirent *pDirent;
	pDirent = readdir(m_pCurDir);
	if(pDirent != NULL)
	{
		if(pDirent->d_type == DT_DIR)
		{
			//skip the . and .. directories
			if(!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, ".."))
				return(GetNext());

			//We need the full path if we want to open the next directory
			if(m_bReportPaths)
			{
				char szBuff[256];
				getcwd(szBuff, 255);
				m_pBuffer->Flush();
							m_pBuffer->Push(szBuff);
				m_pBuffer->Push("/");
			}
			else
				m_pBuffer->Flush();
			
			if(m_bRecurseSubDirs)
			{ 
				//Put the current Dir object on the recursion stack, 
				//change the current dir to the new one in preparation for next query
				m_pDirs[m_nNests] = m_pCurDir;
				m_nNests++;
				chdir(pDirent->d_name);
				m_pCurDir = opendir(".");
			}  
			if(m_bReportDirs)
			{
				m_pBuffer->Push(pDirent->d_name);
				if(m_bReportPaths)
					m_pBuffer->Push("/");
				SetTempBuf(m_pBuffer->DumpToString());
					return m_pTempBuf;
			}
			else
				return GetNext();
		}
		else
		{
			if(m_bReportFiles)
			{
				if(m_bReportPaths)
				{
					char szBuff[256];
					getcwd(szBuff, 255);
					m_pBuffer->Flush();
					m_pBuffer->Push(szBuff);
					m_pBuffer->Push("/");
				}
				else
					m_pBuffer->Flush();

				m_pBuffer->Push(pDirent->d_name);
				SetTempBuf(m_pBuffer->DumpToString());
				return m_pTempBuf;
			}
			else
			{
				return GetNext();
			}
		} 
	} 
	else
	{
		//In here, there are no more files in the current directory
		//Step out of the current nest, recurse up
		if(m_nNests > 0)
		{
			chdir("..");
			closedir(m_pCurDir);
			m_pCurDir = m_pDirs[m_nNests - 1];
			m_nNests--;
			return(GetNext());
		}
		else //all done! No more files.
		{
			closedir(m_pCurDir);
			return NULL;
		}
	}
}




#endif // !WIN32
