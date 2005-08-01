#include <stdio.h>
#include "../../../Include/GashEngine.h"
#include "../../../Include/GashQt.h"
#include "../../../CodeObjects/Method.h"
#include "../../../../GClasses/GMacros.h"
#include "../../../../../SDL/Include/Sdl.h"

class MyErrorHandler : public ErrorHandler
{
public:
	virtual void OnError(ErrorHolder* pErrorHolder)
	{
		printf("There was an error\n");
	}
};

void doit()
{
	Holder<Library*> hLib(Library::LoadFromFile("test.xlib"));
	if(!hLib.Get())
	{
		printf("error loading library\n");
		return;
	}
	GashQtCallBackGetter cbg;
	MyErrorHandler eh;
	GVM vm(hLib.Get(), &cbg, &eh);
	EType* pType = hLib.Get()->FindType("test");
	if(!pType)
	{
		printf("class not found\n");
		return;
	}
	EMethodSignature sig("method foo()");
	int nMethodID = hLib.Get()->FindMethodID((EClass*)pType, &sig);
	if(nMethodID < 0)
	{
		printf("method not found\n");
		return;
	}
	if(!vm.Call(nMethodID, NULL, 0))
	{
		printf("call failed\n");
		return;
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

	doit();
	char szBuf[256];
	gets(szBuf);

	SDL_Quit();

	return 0;
}
