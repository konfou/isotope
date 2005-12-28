/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "GameEngine.h"
#include "MRealmServer.h"
#include "View.h"
#include "VGame.h"
#include "VServer.h"
#include "Controller.h"
#include "MRealm.h"
#include <time.h>
#include "../GClasses/GWindows.h"
#include "../GClasses/GHttp.h"
#include "../GClasses/GFile.h"
#include "../GClasses/GTime.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GBillboardCamera.h"
#include "../GClasses/GThread.h"
#include "../GClasses/GMatrix.h"
#include "../GClasses/sha2.h"
#include "MGameClient.h"
#include "MScriptEngine.h"
#include <wchar.h>
#ifdef WIN32
#	include <io.h>
#	include <direct.h>
#else
#	include <unistd.h>
#	include <signal.h>
#endif // WIN32
#include "MObject.h"
#include <string.h>
#include "AutoUpdate.h"
#include "MStore.h"
#include <stdarg.h>
#include "PuzzleGenerator.h"

/*static*/ const char* GameEngine::s_szAppPath = NULL;
/*static*/ const char* GameEngine::s_szCachePath = NULL;
/*static*/ char* GameEngine::s_szErrorMessage = NULL;
/*static*/ MImageStore* GameEngine::s_pImageStore = NULL;
/*static*/ MAnimationStore* GameEngine::s_pAnimationStore = NULL;
/*static*/ GXMLTag* GameEngine::s_pConfigTag = NULL;

void GameEngine::ThrowError(const char* szFormat, ...)
{
	// Measure the required buffer size--todo: there might be a function like "vscwprintf" that does this, but I couldn't find it so I kludged my own
	int nSize = 0;
	{
		va_list args;
		const char* sz = szFormat;
		va_start(args, szFormat);
		{
			while(*sz != L'\0')
			{
				if(*sz == L'%')
				{
					sz++;
					switch(*sz)
					{
						case 'c':
							nSize += 1;
							va_arg(args, int/*char*/);
							break;
						case 's':
							nSize += strlen(va_arg(args, char*));
							break;
						case 'd':
							nSize += 10;
							va_arg(args, int);
							break;
						case 'l':
							nSize += 20;
							va_arg(args, double);
							break;
						case 'f':
							nSize += 20;
							va_arg(args, double/*float*/);
							break;
						default:
							nSize += 20; // take a guess
							break;
					}
				}
				sz++;
				nSize++;
			}
		}
		va_end(args);
	}
	nSize++;

	// Allocate the buffer
	char* szBuf = GetErrorMessageBuffer(nSize + 1);

	// Format the message
	{
		va_list args;
		va_start(args, szFormat);
		{
#ifdef WIN32
			int res = vsprintf(szBuf, szFormat, args);
#else
			int res = vsnprintf(szBuf, nSize, szFormat, args);
#endif
			GAssert(res >= 0, "Error formatting string");
		}
		va_end(args);
	}

	// Throw the error
	throw (const char*)szBuf;
}

/*static*/ double GameEngine::GetTime()
{
	return GTime::GetTime();
}

/*static*/ int GameEngine::GetUid()
{
	// todo: rewrite this function so it won't produce duplicate uids
	while(true)
	{
		int uid = (rand() << 16) | rand(); // todo: "rand()" isn't reentrant which can cause concurrency issues with the server.  So use some other RNG (if we use an RNG at all)
		if(uid == 0x80000000)
			continue;
		return uid;
	}
}

/*static*/ void GameEngine::MakePasswordHash(char* pOutHash, const char* szPassword)
{
	unsigned char pDigest[SHA512_DIGEST_LENGTH];
	HashBlobSha512(pDigest, (const unsigned char*)szPassword, strlen(szPassword));
	BufferToHex(pDigest, SHA512_DIGEST_LENGTH, pOutHash);
}

/*static*/ void GameEngine::SetAppPath(const char* szPath)
{
	GAssert(!s_szAppPath, "The app path was alread set");
	s_szAppPath = szPath;
}

/*static*/ void GameEngine::SetCachePath(const char* szPath)
{
	GAssert(!s_szCachePath, "The cache path was already set");
	s_szCachePath = szPath;
}

const char* g_szBogusScript = "\
class Bogus(Object)\n\
{\n\
	proc main()\n\
	{\n\
		!Rect:r.new()\n\
		!Float:f.new()\n\
		!Stream:s.new()\n\
		!GImage:i.load(\"foo\")\n\
		!Animation:a.newCopy(null)\n\
		!RealmObject:o.set(null)\n\
	}\n\
}";

/*static*/ void GameEngine::LoadGlobalMedia()
{
	// Load the global media XML file
	const char* szAppPath = GetAppPath();
	char* szMediaListFile = (char*)alloca(strlen(szAppPath) + 50);
	strcpy(szMediaListFile, szAppPath);
	strcat(szMediaListFile, "media/media.xml");
	const char* szErrorMessage = NULL;
	int nErrorLine = 0;
	Holder<GXMLTag*> hRootTag(GXMLTag::FromFile(szMediaListFile, &szErrorMessage, NULL, &nErrorLine, NULL));
	GXMLTag* pRootTag = hRootTag.Get();
	if(!pRootTag)
		ThrowError("Error loading XML file \"%s\" at line %d.\n%s\n", szMediaListFile, nErrorLine, szErrorMessage);

	// Make a bogus script engine
	MScriptEngine* pBogusScriptEngine = new MScriptEngine("*Global Script*", g_szBogusScript, strlen(g_szBogusScript), new IsotopeErrorHandler(), NULL, NULL, NULL, NULL);

	// Load the image store
	GXMLTag* pImages = pRootTag->GetChildTag("Images");
	if(!pImages)
		GameEngine::ThrowError("Expected an 'Images' tag");
	s_pImageStore = new MImageStore();
	s_pImageStore->FromXml(NULL, pImages, pBogusScriptEngine);

	// Load the animation store
	GXMLTag* pAnimations = pRootTag->GetChildTag("Animations");
	if(!pAnimations)
		GameEngine::ThrowError("Expected an 'Animations' tag");
	s_pAnimationStore = new MAnimationStore(pBogusScriptEngine);
	s_pAnimationStore->FromXml(pAnimations, s_pImageStore);
}

/*static*/ MImageStore* GameEngine::GetGlobalImageStore()
{
	if(!s_pImageStore)
		LoadGlobalMedia();
	return s_pImageStore;
}

/*static*/ MAnimationStore* GameEngine::GetGlobalAnimationStore()
{
	if(!s_pAnimationStore)
		LoadGlobalMedia();
	return s_pAnimationStore;
}

/*static*/ GXMLTag* GameEngine::GetConfig()
{
	if(s_pConfigTag)
		return s_pConfigTag;
	const char* szAppPath = GetAppPath();
	char* szConfigFileName = (char*)alloca(strlen(szAppPath) + 50);
	strcpy(szConfigFileName, szAppPath);
	strcat(szConfigFileName, "config.xml");
	const char* szErrorMessage;
	int nErrorLine;
	s_pConfigTag = GXMLTag::FromFile(szConfigFileName, &szErrorMessage, NULL, &nErrorLine, NULL);
	if(!s_pConfigTag)
		ThrowError("Failed to load config file \"%s\" at line %d.\n%s", szConfigFileName, nErrorLine, szErrorMessage);
	return s_pConfigTag;
}

/*static*/ const char* GameEngine::GetStartUrl()
{
	GXMLTag* pConfigTag = GetConfig();
	GXMLTag* pStartTag = pConfigTag->GetChildTag("Start");
	if(!pStartTag)
		GameEngine::ThrowError("Expected a <Start> tag in the config.xml file");
	GXMLAttribute* pUrlAttr = pStartTag->GetAttribute("url");
	if(!pUrlAttr)
		GameEngine::ThrowError("Expected a \"url\" attribute in the start tag in the config.xml file");
	return pUrlAttr->GetValue();
}

/*static*/ void GameEngine::SaveConfig()
{
	const char* szAppPath = GetAppPath();
	char* szConfigFileName = (char*)alloca(strlen(szAppPath) + 50);
	strcpy(szConfigFileName, szAppPath);
	strcat(szConfigFileName, "config.xml");
	GetConfig()->ToFile(szConfigFileName);
}

/*static*/ char* GameEngine::GetErrorMessageBuffer(int nSize)
{
	delete(s_szErrorMessage);
	s_szErrorMessage = new char[nSize];
	return s_szErrorMessage;
}

char* GetApplicationPath(const char* szArg0)
{
	// Make sure the app name includes path info
	char szFullNameBuf[512];
#ifdef WIN32
	GetModuleFileName(NULL/*GetModuleHandle(szArg0)*/, szFullNameBuf, 512);
#else // WIN32
	strcpy(szFullNameBuf, szArg0);
#endif // !WIN32
	int nFullLen = strlen(szFullNameBuf);
	const char* szFilename;
	if(nFullLen > 0)
		szFilename = szFullNameBuf;
	else
	{
		GAssert(false, "failed to get full name of executing assembly");
		szFilename = szArg0;
	}

	// Find the last slash in szFilename
	int n = strlen(szFilename);
	for(n--; n >= 0; n--)
	{
		if(szFilename[n] == '/' || szFilename[n] == '\\')
			break;
	}
	const char* szFilePart = szFilename + n + 1;
	char szAppPath[512];
	if(n >= 0)
	{
		memcpy(szAppPath, szFilename, n);
		szAppPath[n] = '\0';
	}
	else
		getcwd(szAppPath, 512);

	// Copy to an allocated buffer and append a slash if necessary
	int nLen = strlen(szAppPath);
	bool bAddSlash = true;
	if(szAppPath[nLen - 1] == '/' || szAppPath[nLen - 1] == '\\')
		bAddSlash = false;
	char* szApplicationPath = new char[nLen + 1 + (bAddSlash ? 1 : 0)];
	strcpy(szApplicationPath, szAppPath);
	if(bAddSlash)
	{
#ifdef WIN32
		szApplicationPath[nLen] = '\\';
#else
		szApplicationPath[nLen] = '/';
#endif
		szApplicationPath[nLen + 1] = '\0';
	}
	return szApplicationPath;
}

void DaemonMain(void* pArg)
{
	Controller c(Controller::SERVER, (const char*)pArg);
	c.Run();
}

typedef void (*DaemonMainFunc)(void* pArg);

void LaunchDaemon(DaemonMainFunc pDaemonMain, void* pArg)
{
#ifdef WIN32
	// Windows isn't POSIX compliant and it has its own process system that
	// isn't really friendly to launching daemons.  You're supposed to create
	// a "service", but I don't know how to do that (and I'm too lazy to learn
	// something that can't generalize off a proprietary platform) so let's
	// just launch it like a normal app and be happy with that.
	pDaemonMain(pArg);
#else // WIN32
	// Fork the process
	int pid = fork();
	if(pid < 0)
		GameEngine::ThrowError("Error forking the daemon process\n");
	if(pid == 0)
	{
		// Drop my process group leader and become my own process group leader
		// (so the process isn't terminated when the group leader is killed)
		setsid();

		// Get off any mounted drives so that they can be unmounted without
		// killing the daemon
		chdir("/");

		// Launch the daemon
		pDaemonMain(pArg);
	}
#endif // !WIN32
}

void PuzzleGenerator()
{
	time_t t;
	srand(time(&t));
	GPointerArray* pOrdered = PuzzleGenerator::LoadPieces("media\\puzzlegen\\ordered");
	GPointerArray* pUnordered = PuzzleGenerator::LoadPieces("media\\puzzlegen\\unordered");
	while(true)
	{
		GImage* pImage = PuzzleGenerator::MakePuzzle(pOrdered, pUnordered);
		pImage->SaveBMPFile("puzgen.bmp");
#ifdef WIN32
		ShellExecute(NULL, NULL, "puzgen.bmp", NULL, NULL, SW_SHOW);
#else // WIN32
		system("konqueror puzgen.bmp");
#endif // !WIN32
		delete(pImage);
		printf("\nPress Enter for another puzzle\n");
		getchar();
	}
	// todo: don't leak the piece sets here
}

void LaunchProgram(int argc, char *argv[])
{
	// Parse the runmode
	bool bOK = false;
	Controller::RunModes eRunMode = Controller::SERVER;
	const char* szArg = NULL;
	if(argc >= 3)
	{
		bOK = true;
		if(stricmp(argv[1], "server") == 0)
			eRunMode = Controller::SERVER;
		else if(stricmp(argv[1], "keypair") == 0)
			eRunMode = Controller::KEYPAIR;
		else if(stricmp(argv[1], "bless") == 0 && argc >= 4)
		{
			BlessThisApplication(argv[2], argv[3]);
			return;
		}
		else
			bOK = false;
		szArg = argv[2];
	}
	else if(argc >= 2)
	{
		bOK = true;
		if(stricmp(argv[1], "client") == 0)
			eRunMode = Controller::CLIENT;
		else if(stricmp(argv[1], "puzgen") == 0)
			eRunMode = Controller::PUZGEN;
		else
			bOK = false;
	}
	else
	{
		bOK = true;
		eRunMode = Controller::CLIENT;
	}

	// Show usage if we couldn't parse the runmode
	if(!bOK)
	{
		printf("\n");
		printf("Usage: isotope.exe [command] [parameters]\n\n");
		printf("Example: isotope.exe client http://edumetrics.org/isotope/start.realm\n");
		printf("\n");
		printf("Valid values for [parameters] depend on the value for [command].\n");
		printf("Possible values for [command]:\n");
		printf("\n");
		printf("client                  Run as a client (the default).\n");
		printf("\n");
		printf("server [WWW-Root-Path]  Run as a server.  The value of [WWW-Root-Path]\n");
		printf("                        should be the web root used by your HTTP server\n");
		printf("                        and all your .realm files and content should be\n");
		printf("                        somewhere within that folder.\n");
		printf("\n");
		printf("keypair [Output File]   Generate a key pair.\n");
		printf("\n");
		printf("bless [Key File] [Url]  This command can only be used by the\n");
		printf("                        owner of the trusted key file.  It generates an\n");
		printf("                        update.xml file that that is used to tell the\n");
		printf("                        auto-update feature about updated versions.\n");
		return;
	}

	// Check for updates
	//if(!DoAutoUpdate())
	//	return;

	if(eRunMode == Controller::PUZGEN)
		PuzzleGenerator();
	else if(eRunMode == Controller::SERVER)
		LaunchDaemon(DaemonMain, (void*)szArg);
	else
	{
		// Run the main loop
		Controller c(eRunMode, szArg);
		c.Run();
	}
}

int FooComparer(void* pA, void* pB)
{
	if((int)pA > (int)pB)
		return 1;
	else if((int)pA < (int)pB)
		return -1;
	else
		return 0;
}


class TestHttpDaemon : public GHttpServer
{
public:
	TestHttpDaemon() : GHttpServer(8989) {}
	~TestHttpDaemon() {}

protected:
	virtual void DoGet(const char* szUrl, const char* szParams, GQueue* pResponse)
	{
	}

	virtual void OnProcessLine(int nConnection, const char* szLine)
	{
		fprintf(stderr, szLine);
	}
};


void MungeImages()
{
	// Munge Image
	GImage image;
	image.SetSize(1280, 512);
	GImage imageIn;
	imageIn.LoadBMPFile("mungeme.bmp");
	int i, j;
	GRect r;
	r.Set(0, 0, 128, 128);
	for(j = 0; j < 4; j++)
	{
		for(i = 0; i < 10; i++)
		{
			GImage* pImageOut = imageIn.Munge(j, ((float)1 - (float)i / 9) / 1);
			image.Blit(i * 128, j * 128, pImageOut, &r);
			delete(pImageOut);
		}
	}
	image.SaveBMPFile("munged.bmp");
}


void test()
{
/*
	// Test GHttpClient
	GHttpClient socket;
	socket.Get("http://www.gashler.com", 80);
	while(socket.CheckStatus() == GHttpClient::Downloading)
		GWindows::YieldToWindows();
	char* szData = (char*)socket.GetData();
	printf(szData);*/
/*
	// Test LoadPng
	GImage gi;
	int nSize;
	Holder<char*> hBuf(GFile::LoadFileToBuffer("c:\\test.png", &nSize));
	const char* pBuf = hBuf.Get();
	if(!LoadPng(&gi, (unsigned char*)pBuf, nSize))
		GAssert(false, "Failed to load PNG");
	gi.SaveBMPFile("c:\\test.bmp");*/
/*
	// Test GPointerArray::Sort
	GPointerArray arr(64);
	int n;
	for(n = 34; n >= 0; n--)
		arr.AddPointer((void*)n);
	arr.Sort(FooComparer);
	for(n = 0; n <= 34; n++)
		GAssert((int)arr.GetPointer(n) == n, "broken");*/
/*
	// Test collision map
	FRect r;
	MCollisionMap map;
	r.x = 0;
	r.y = 0;
	r.w = 10;
	r.h = 10;
	map.AddSolidRect(&r);
	r.x = 20;
	r.y = 0;
	r.w = 10;
	r.h = 10;
	map.AddSolidRect(&r);
	r.x = 25;
	r.y = 5;
	r.w = 10;
	r.h = 10;
	map.AddSolidRect(&r);
	map.Compile();
	bool b;
	b = map.Check(22, 12);
	b = map.Check(22, 5);
	b = map.Check(40, 4);*/
/*
	// Test glowing edges
	GImage img;
	img.LoadBMPFile("c:\\input.bmp");
	img.MakeEdgesGlow((float)0.2, 5, 32, 0x4488ffff);
	img.SaveBMPFile("c:\\output.bmp");*/
/*
	try
	{
		ThrowErrorX(L"This %ls a test %d\nhahaha\n", L"is", 5);
	}
	catch(const wchar_t* wszMessage)
	{
		wprintf(wszMessage);
	}
*/
/*
	TestHttpDaemon daemon;
	GHttpClient socket;
	socket.Get("http://proton.i.edumetrics.org/foo.realm", 8989);
	while(true)
	{
		daemon.Process();
		Sleep(0);
	}
*/
/*
	GHttpClient socket;
//	if(!socket.Get("http://muon.i.edumetrics.org/~jdpf/media/trunk/puzzles/addition/addition.realm", 80))
	if(!socket.Get("http://ion.i.edumetrics.org/~jdpf/media_final/media/trunk/puzzles/addition/addition.realm", 80))
		GameEngine::ThrowError("Failed to connect");
	while(socket.CheckStatus(NULL) == GHttpClient::Downloading)
	{
		Sleep(0);
	}
	int nSize;
	unsigned char* pData = socket.GetData(&nSize);
	GAssert(false, "break");
*/

	// redirect output
	//freopen("stdout.txt", "w", stdout);
	//freopen("stderr.txt", "w", stderr);
/*
	// test matrix
	GMatrix m(2, 2);
	m.Set(0, 0, 2);
	m.Set(0, 1, 3);
	m.Set(1, 0, 5);
	m.Set(1, 1, 1);
	double vec[2];
	vec[0] = 1;
	vec[1] = 1;
	m.Solve(vec);
	printf("[%f, %f]\n", vec[0], vec[1]);
*/
//	LaunchOgre();
}

#ifndef WIN32
void onSigSegV(int n)
{
	throw "A memory access violation occurred.  The most common cause is an attempt to dereference null";
}

void onSigInt(int n)
{
	throw "The program was interrupted when the user pressed Ctrl-C";
}

void onSigQuit(int n)
{
	throw "The program was interrupted with SIGQUIT";
}

void onSigTstp(int n)
{
	throw "The program was interrupted with SIGTSTP";
}

void onSigAbrt(int n)
{
	throw "An unhandled exception was thrown";
}

#endif // !WIN32

int oldmain(int argc, char *argv[])
{
#ifndef WIN32
	signal(SIGSEGV, onSigSegV);
	signal(SIGINT, onSigInt);
	signal(SIGQUIT, onSigQuit);
	signal(SIGTSTP, onSigTstp);
	signal(SIGABRT, onSigAbrt);
#endif // !WIN32

	// Seed the random number generator
	srand((unsigned int)(GameEngine::GetTime() * 10000));

	// Determine app and cache paths
	Holder<char*> hAppPath(GetApplicationPath(argv[0]));
	const char* szAppPath = hAppPath.Get();
	GameEngine::SetAppPath(szAppPath);
	Holder<char*> hCachePath(new char[strlen(szAppPath) + 12]);
	char* szCachePath = hCachePath.Get();
	strcpy(szCachePath, szAppPath);
	strcat(szCachePath, "../cache/");
#ifdef WIN32
	mkdir(szCachePath);
#else // WIN32
	mkdir(szCachePath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // read/write/search permissions for owner and group, and with read/search permissions for others
#endif // !WIN32
	GameEngine::SetCachePath(szCachePath);

	// Run any experimental tests
	test();

	// Run the program
	try
	{
		LaunchProgram(argc, argv);
	}
	catch(const char* szErrorMessage)
	{
		fprintf(stderr, szErrorMessage);
		fprintf(stderr, "\n");
#ifdef WIN32
		printf("\nPress enter to quit\n");
		getchar();
#endif // WIN32
	}
	catch(wchar_t* wszErrorMessage)
	{
		fwprintf(stderr, wszErrorMessage);
		fwprintf(stderr, L"\n");
#ifdef WIN32
		printf("\nPress enter to quit\n");
		getchar();
#endif // WIN32
	}

	// Check for memory leaks
	GAssert(AllocCounter::s_allocs == AllocCounter::s_deallocs, "memory leak");

	return 0;
}

#ifdef OGRE

#	ifdef WIN32

// OGRE on Win32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
	AllocConsole();

	printf("Running...\n");
	fprintf(stderr, "Running...\n");
	char* szArgs = "";
	return oldmain(1, &szArgs);
}
#	else

// OGRE on Linux
int main(int argc, char *argv[])
{
	return oldmain(argc, argv);
}
#	endif
#else // OGRE




int playOgg(); // todo: remove this line


// No Ogre
int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) 
		GameEngine::ThrowError("Unable to initialize SDL: %s", SDL_GetError());
	int nRet = oldmain(argc, argv);
	SDL_Quit();
	return nRet;
}
#endif // !OGRE
