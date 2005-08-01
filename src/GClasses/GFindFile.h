#ifndef __GFINDFILE_H__
#define __GFINDFILE_H__

#ifdef WIN32
#include <winsock2.h>
#endif // WIN32

class GFileFinder : public WIN32_FIND_DATA
{
protected:
   HANDLE m_hFind;

public:
   GFileFinder()                        { m_hFind = INVALID_HANDLE_VALUE; }
   GFileFinder(LPCTSTR pFile)           { m_hFind = INVALID_HANDLE_VALUE; GetFileInfo(pFile); }
   virtual ~GFileFinder()               { Close(); }

   void Close()                        { if(m_hFind != INVALID_HANDLE_VALUE){FindClose(m_hFind); m_hFind = INVALID_HANDLE_VALUE;} }

   BOOL FindFirst(LPCTSTR pFile)       { Close(); m_hFind = FindFirstFile(pFile, this); return m_hFind != INVALID_HANDLE_VALUE; }
   BOOL FindNext()                     { return m_hFind == INVALID_HANDLE_VALUE ? FALSE : FindNextFile(m_hFind, this); }
   BOOL Find(LPCTSTR pFile);
   BOOL Find()                         { return m_hFind == INVALID_HANDLE_VALUE ? FALSE : Find(NULL); }

   BOOL IsValid()                      { return m_hFind != INVALID_HANDLE_VALUE; }
   BOOL IsDots();
   BOOL IsNotDots()                    { return IsDots() ? FALSE : TRUE; }
   BOOL IsFile()                       { return IsNotDots(); }
   BOOL IsFile(BOOL excludeDirs)       { return (excludeDirs && IsDir()) ? FALSE : IsNotDots(); }
   BOOL IsDir()                        { return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE; }

   BOOL GetFileInfo(LPCTSTR pFile)     { BOOL rval = FindFirst(pFile); Close(); return rval; }

   DWORD    GetAttributes()            { return dwFileAttributes; }
   FILETIME GetCreationTime()          { return ftCreationTime; }
   FILETIME GetLastAccessTime()        { return ftLastAccessTime; }
   FILETIME GetLastWriteTime()         { return ftLastWriteTime; }
   DWORD    GetFileSizeHigh()          { return nFileSizeHigh; }
   DWORD    GetFileSizeLow()           { return nFileSizeLow; }
   LPSTR    GetFileName()              { return cFileName; }
   LPSTR    GetDOSName()               { return cAlternateFileName; }
};

#endif // __GFINDFILE_H__
