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

public:
	static void ThrowError(const char* szFormat, ...);
/*	static void ThrowError(const char* szMessage);
	static void ThrowError(const char* szMessage, const char* szParam1);
	static void ThrowError(const char* szMessage, const char* szParam1, const char* szParam2);
	static void ThrowError(const char* szMessage, const char* szParam1, const char* szParam2, const char* szParam3);*/

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

	// pOutHash should be a buffer of at least size 2 * SHA512_DIGEST_LENGTH + 1
	static void MakePasswordHash(char* pOutHash, const char* szPassword);

	static char* GetErrorMessageBuffer(int nSize);
	static MImageStore* GetGlobalImageStore();
	static MAnimationStore* GetGlobalAnimationStore();
	static GXMLTag* GetConfig();
	static void SaveConfig();
	static const char* GetStartUrl();

protected:
	static void LoadGlobalMedia();
};

#endif // __GAMEENGINE_H__
