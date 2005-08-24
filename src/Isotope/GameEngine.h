/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __GAMEENGINE_H__
#define __GAMEENGINE_H__

class MImageStore;
class MAnimationStore;
class GXMLTag;

// This is the main class that represents the entire application
class GameEngine
{
protected:
	static const char* s_szAppPath;
	static const char* s_szCachePath;
	static char* s_szErrorMessage;
	static const char* s_szEdumetricsPublicKey;
	static MImageStore* s_pImageStore;
	static MAnimationStore* s_pAnimationStore;
	static GXMLTag* s_pConfigTag;
	static int s_nNextUid;

public:
	static void ThrowError(const char* szMessage);
	static void ThrowError(const char* szMessage, const char* szParam1);
	static void ThrowError(const char* szMessage, const char* szParam1, const char* szParam2);
	static void ThrowError(const char* szMessage, const char* szParam1, const char* szParam2, const char* szParam3);

	// Returns the path to where the running application is found
	static const char* GetAppPath() { return s_szAppPath; }

	// Sets the app path
	static void SetAppPath(const char* szPath);

	// Returns the path to where the cache is stored
	static const char* GetCachePath() { return s_szCachePath; }

	// Sets the cache path
	static void SetCachePath(const char* szPath);

	// Returns a random number.  For convenience of working with GHashTable, it guarantees
	// that the ID will not be 0x80000000 because GHashTable won't accept that value as
	// a key.  todo: find a better algorithm to guarantee unique ID's
	static int GetUid();

	// Returns a double that represents the number of seconds (precise to the milisecond
	// that have elapsed since midnight.  todo: does the game choke if you play it across midnight?
	static double GetTime();

	// This downloads a file from a URL or loads it from disk.  You must delete the
	// buffer that it returns.  It will call ThrowError if there are any problems.
	// (If pnSize is NULL, then instead of returning a pointer to the file loaded into memory,
	// it will just return the cached filename.  You must still delete the buffer it returns,
	// and it will still download the file if it doesn't exist in the cache yet.)
	static char* LoadFileFromUrl(const char* szRemotePath, const char* szUrl, int* pnSize);

	// Downloads the file at the specified URL.  If it fails and bThrow is true,
	// it will throw an execption, otherwise just return NULL.
	static char* DownloadFile(const char* szUrl, int* pnSize, bool bThrow);

	// pOutHash should be a buffer of at least size 2 * SHA512_DIGEST_LENGTH + 1
	static void MakePasswordHash(char* pOutHash, const char* szPassword);

	static const char* SetErrorMessage(const char* szMessage);
	static MImageStore* GetGlobalImageStore();
	static MAnimationStore* GetGlobalAnimationStore();
	static GXMLTag* GetConfig();
	static void SaveConfig();
	static const char* GetStartUrl();

protected:
	// Helper used by LoadFileFromUrl
	static char* DownloadAndCacheFile(const char* szUrl, int* pnSize, char* szCacheName);

	static void LoadGlobalMedia();
};

#endif // __GAMEENGINE_H__
