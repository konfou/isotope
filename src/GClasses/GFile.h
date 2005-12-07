/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GFILE_H__
#define __GFILE_H__

// Contains some useful routines for manipulating files
class GFile
{
public:
	// returns true if the file exists
	static bool DoesFileExist(const char *filename);

	// returns true if the directory exists
	static bool DoesDirExist(const char* szDir);

	// This finds the last slash in szBuff and returns a
	// pointer to the char past that.  (If there are no
	// slashes or back-slashes, it returns szBuff)
	static const char* ClipPath(const char* szBuff);

	// This finds the last slash in szBuff and sets it
	// to '\0' and returns szBuff.
	static char* ClipFilename(char* szBuff);

	// This copies a file.  It doesn't check to see if it is
	// overwriting--it just does the copying.  On success it
	// returns true.  On error it returns false.  It won't
	// work with a file bigger than 2GB.  Both paths must
	// include the filename.
	static bool CpyFile(const char* szSrcPath, const char* szDestPath);

	// Loads a file into memory and returns a pointer to the
	// memory.  You must delete the buffer it returns.
	static char* LoadFileToBuffer(const char* szFilename, int* pnSize);

	// Saves a buffer as a file.  Returns true on success
	static bool SaveBufferToFile(unsigned char* pBuf, int nSize, const char* szFilename);

	// This is a brute force way to make a directory.  It i
	// itterates through each subdir in szDir and calls mkdir
	// until it has created the complete set of nested directories.
	static bool MakeDir(char* szDir);

};

#endif // __GFILE_H__
