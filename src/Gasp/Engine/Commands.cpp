/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#ifdef WIN32
#	include <io.h>
#	include <direct.h>
#endif // WIN32
#include "Error.h"
#include "GCompiler.h"
#include "Disassembler.h"
#include "../Include/GaspEngine.h"
#include "../CodeObjects/File.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/FileSet.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GString.h"
#include "../../GClasses/GXML.h"

char* LoadFileToBuffer(ErrorHandler* pErrorHandler, const char* szFilename)
{
	// Load the script
	GlobalError errorHolder;
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
	{
		errorHolder.SetError(&Error::FILE_NOT_FOUND);
		errorHolder.SetParam1(szFilename);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return NULL;
	}
	int nFileSize = filelength(fileno(pFile));
	Holder<char*> hBuffer(new char[nFileSize + 1]);
	if(!hBuffer.Get())
	{
		errorHolder.SetError(&Error::OUT_OF_MEMORY);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		fclose(pFile);
		return NULL;
	}
	int nBytesRead = fread(hBuffer.Get(), sizeof(char), nFileSize, pFile);
	int err = ferror(pFile);
	if(err != 0 || nBytesRead != nFileSize)
	{
		errorHolder.SetError(&Error::INTERNAL_ERROR);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		fclose(pFile);
		return NULL;
	}
	hBuffer.Get()[nFileSize] = '\0';
	fclose(pFile);
	return hBuffer.Drop();
}

FILETYPES IdentifyFileType(const char* szFile, const char** ppStart, int* pnLine)
{
	*ppStart = szFile;
	const char* pStart = szFile;
	FILETYPES fileType = FT_UNRECOGNIZED;
	while(*pStart != '\0')
	{
		if(*pStart == '\n')
			(*pnLine)++;
		else if(*pStart == '<')
		{
			if(strnicmp(pStart, "<Library", 8) == 0)
			{
				fileType = FT_XLIB;
				break;
			}
			if(strnicmp(pStart, "<Source", 7) == 0)
			{
				fileType = FT_SOURCE;
				break;
			}
			if(strnicmp(pStart, "<File", 5) == 0)
			{
				fileType = FT_XML;
				break;
			}
		}
		else
		{
			if(strnicmp(pStart, "class ", 6) == 0)
			{
				fileType = FT_CLASSIC;
				break;
			}
			if(strnicmp(pStart, "interface ", 10) == 0)
			{
				fileType = FT_CLASSIC;
				break;
			}
			if(strnicmp(pStart, "machine ", 8) == 0)
			{
				fileType = FT_CLASSIC;
				break;
			}
		}

		if(*pStart > ' ')
		{
			while(*pStart != '\0' && *pStart != '\n')
				pStart++;
		}
		else
			pStart++;
	}
	if(fileType == FT_UNRECOGNIZED)
		return FT_UNRECOGNIZED;

	*ppStart = pStart;
	return fileType;
}

char* LoadFile(ErrorHandler* pErrorHandler, const char* szFilename, FILETYPES* peType, const char** ppStart, int* pnStartLine)
{
	char* pBuf = LoadFileToBuffer(pErrorHandler, szFilename);
	if(!pBuf)
		return NULL;
	*peType = IdentifyFileType(pBuf, ppStart, pnStartLine);
	return pBuf;
}

Library* BuildProject(ErrorHandler* pErrorHandler, const char* szSourceFilename, const char* szLibrariesPath)
{
	// Make a project
	Holder<COProject*> hProject(COProject::LoadProject(szLibrariesPath, szSourceFilename, pErrorHandler));
	if(!hProject.Get())
		return NULL;

	// Compile the project to a library
	CompileError errorHolder;
	Library* pLib;
	{
		GCompiler comp(hProject.Drop(), &errorHolder);
		pLib = comp.Compile(true);
	}
	if(!pLib)
	{
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return NULL;
	}
	
	return pLib;
}

Library* BuildFileRefs(ErrorHandler* pErrorHandler, const char** ppFiles, const char** pszFilenames, int nFileCount, const char* szLibrariesPath, bool bSaveProjectFile)
{
	// Make a project
	Holder<COProject*> hProject(new COProject("Untitled.proj"));
	if(!hProject.Get()->LoadLibraries(szLibrariesPath, pErrorHandler))
		return NULL;
	if(!hProject.Get()->LoadSources(ppFiles, pszFilenames, nFileCount, pErrorHandler))
		return NULL;

	// Save project file
	if(bSaveProjectFile)
	{
		hProject.Get()->GetSource()->SetModified(true);
		hProject.Get()->SaveChanges();
	}

	// Compile the project to a library
	CompileError errorHolder;
	Library* pLib;
	{
		GCompiler comp(hProject.Drop(), &errorHolder);
		pLib = comp.Compile(true);
	}
	if(!pLib)
	{
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return NULL;
	}
	
	return pLib;
}

class VarHolderArray
{
public:
	int m_nSize;
	VarHolder** m_pParams;

	VarHolderArray(int nSize)
	{
		m_nSize = nSize;
		if(nSize > 0)
		{
			m_pParams = new VarHolder*[nSize];
			memset(m_pParams, '\0', nSize * sizeof(VarHolder*));
		}
		else
			m_pParams = NULL;
	}

	~VarHolderArray()
	{
		int n;
		for(n = 0; n < m_nSize; n++)
			delete(m_pParams[n]);
		delete(m_pParams);
	}
};

void RunLibraryFromMemory(ErrorHandler* pErrorHandler, Library* pLibrary, struct MethodRef* pMethodRef, Primitive** ppParams, int nParamCount, CallBackGetter* pCallBackGetter, const char* szChrootJail, bool bExpectAnException)
{
	// Set up engine
	int n;
	GVM vm(pLibrary, pCallBackGetter, pErrorHandler, szChrootJail);
	vm.SetExpectAnException(bExpectAnException);

	// Set up params
	VarHolderArray params(nParamCount);
	for(n = 0; n < nParamCount; n++)
		params.m_pParams[n] = new VarHolder(&vm, ppParams[n]);

	// Run it
	vm.Call(pMethodRef, params.m_pParams, nParamCount);
}

void RunMainProcInLibrary(ErrorHandler* pErrorHandler, Library* pLibrary, Primitive** ppParams, int nParamCount, CallBackGetter* pCallBackGetter, const char* szChrootJail, bool bExpectAnException = false)
{
	// Find "main" proc
	struct MethodRef mr;
	if(!pLibrary->FindMainProc(&mr))
	{
		GlobalError errorHolder;
		errorHolder.SetError(&Error::NO_MAIN_PROC);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return;
	}

	// Run the library
	RunLibraryFromMemory(pErrorHandler, pLibrary, &mr, ppParams, nParamCount, pCallBackGetter, szChrootJail, bExpectAnException);
}

Library* BufferToLibrary(ErrorHandler* pErrorHandler, const char* szLibrary)
{
	Library* pLibrary = Library::LoadFromBuffer(szLibrary, strlen(szLibrary), NULL, false);
	if(!pLibrary)
	{
		GlobalError errorHolder;
		errorHolder.SetError(&Error::BAD_LIBRARY);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return NULL;
	}
	return pLibrary;
}

Library* BuildFile(ErrorHandler* pErrorHandler, const char* szFilename, const char* szLibrariesPath)
{
	// Load the file
	const char* pStart;
	FILETYPES ft;
	int nStartLine = 1;
	Holder<char*> hBuffer(LoadFile(pErrorHandler, szFilename, &ft, &pStart, &nStartLine));
	if(!hBuffer.Get())
		return false;

	// Build it
	Library* pLibrary = NULL;
	bool bRet = false;
	switch(ft)
	{
	case FT_UNRECOGNIZED:
		{
			GlobalError errorHolder;
			errorHolder.SetError(&Error::UNRECOGNIZED_FILE_FORMAT);
			errorHolder.SetParam1(szFilename);
			if(pErrorHandler)
				pErrorHandler->OnError(&errorHolder);
		}
		break;

	case FT_CLASSIC:
		{
			const char* pFile = hBuffer.Get();
			pLibrary = BuildFileRefs(pErrorHandler, &pFile, &szFilename, 1, szLibrariesPath, false);
		}
		break;

	case FT_XML:
		{
			const char* pFile = hBuffer.Get();
			pLibrary = BuildFileRefs(pErrorHandler, &pFile, &szFilename, 1, szLibrariesPath, false);
		}
		break;

	case FT_XLIB:
		pLibrary = BufferToLibrary(pErrorHandler, pStart);
		break;		

	case FT_SOURCE:
		pLibrary = BuildProject(pErrorHandler, szFilename, szLibrariesPath);
		break;

	default:
		GAssert(false, "unexpected type");
		break;		
	}
	
	return pLibrary;
}

void BuildAndRunFile(ErrorHandler* pErrorHandler, const char* szFilename, const char* szLibrariesPath, Primitive** ppParams, int nParamCount, CallBackGetter* pCallBackGetter, bool bExpectAnException = false)
{
	Holder<Library*> hLibrary(BuildFile(pErrorHandler, szFilename, szLibrariesPath));
//hLibrary.Get()->GetLibraryTag()->ToFile("c:\\tmp.xml");
	if(!hLibrary.Get())
		return;
	char szDrive[10];
	char szDir[512];
	_splitpath(szFilename, szDrive, szDir, NULL, NULL);
	char szPath[512];
	_makepath(szPath, szDrive, szDir, NULL, NULL);
	RunMainProcInLibrary(pErrorHandler, hLibrary.Get(), ppParams, nParamCount, pCallBackGetter, szPath, bExpectAnException);
}

void GetLibrariesPath(char* szLibrariesPath, const char* szAppPath)
{
	strcpy(szLibrariesPath, szAppPath);
#ifdef WIN32
	int i;
	for(i = 0; szLibrariesPath[i] != '\0'; i++)
	{
		if(szLibrariesPath[i] == '/')
			szLibrariesPath[i] = '\\';
	}
#endif
}

void CommandRun(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, CallBackGetter* pCBG, bool bExpectAnException)
{
	// Make a Libraries path
	char szLibrariesPath[512];
	GetLibrariesPath(szLibrariesPath, szAppPath);

	// Convert parameters to primitives
	GAssert(argc >= 3, "not enough args");
	const char* szFilename = argv[2];
	ArrayHolder<PrimitiveString*> hParams(NULL);
	Primitive** ppParams = NULL;
	if(argc > 3)
	{
		ppParams = (Primitive**)alloca((argc - 3) * sizeof(Primitive**));
		hParams.Set(new PrimitiveString[argc - 3]);
		int n;
		for(n = 0; n < argc - 3; n++)
		{
			GString s;
			s.Copy(argv[n + 3]);
			hParams.Get()[n].SetValue(s.GetString());
			ppParams[n] = &hParams.Get()[n];
		}
	}

	// Run the file	
	BuildAndRunFile(pErrorHandler, szFilename, szLibrariesPath, ppParams, argc - 3, pCBG, bExpectAnException);
}

bool CommandDisassemble(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv)
{
	// Make a Libraries path
	char szLibrariesPath[512];
	GetLibrariesPath(szLibrariesPath, szAppPath);

	// Build the file
	const char* szFilename = argv[2];
	Holder<Library*> hLibrary(BuildFile(pErrorHandler, szFilename, szLibrariesPath));
	if(!hLibrary.Get())
		return false;

	// Disassemble it
	int nLen;
	Holder<char*> hDisassembly(Disassembler::DisassembleLibraryToText(hLibrary.Get(), &nLen));
	FILE* pFile = fopen("disassembly.txt", "w");
	fwrite(hDisassembly.Get(), nLen, 1, pFile);
	fclose(pFile);

#ifdef WIN32
	// Open the file with the default text editor
	ShellExecute(NULL, NULL, "disassembly.txt", NULL, NULL, SW_SHOW);
#endif // WIN32

	return true;
}

Library* BuildFiles(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv, char* szTargetBuf)
{
	// Make a Libraries path
	char szLibrariesPath[512];
	GetLibrariesPath(szLibrariesPath, szAppPath);

	// Load the files to build
	GAssert(argc > 2, "not enough args");
	StringHolderArray files(argc - 2);
	Holder<const char**> hFiles(new const char*[argc - 2]);
	bool bOK = true;
	int n;
	bool bGotSourceFile = false;
	for(n = 0; n < argc - 2; n++)
	{
		// Load the file
		const char* szFilename = argv[n + 2];
		FILETYPES ft;
		const char* pStart;
		int nStartLine = 1;
		const char* pFile = LoadFile(pErrorHandler, szFilename, &ft, &pStart, &nStartLine);
		if(!pFile)
		{
			bOK = false;
			break;
		}
		files.m_pData[n] = (char*)pFile;

		// Determine the file type
		hFiles.Get()[n] = pStart;
		switch(ft)
		{
			case FT_CLASSIC:
				break;
			case FT_XML:
				break;
			case FT_SOURCE:
				bGotSourceFile = true;
				break;
			default:
				{
					GlobalError errorHolder;
					errorHolder.SetError(&Error::UNRECOGNIZED_FILE_FORMAT);
					errorHolder.SetParam1(szFilename);
					if(pErrorHandler)
						pErrorHandler->OnError(&errorHolder);
					bOK = false;
				}
		}
		if(!bOK)
			break;

	}

	// Build to a library
	Library* pLibrary = NULL;
	if(bOK)
	{
		// Build the library
		if(bGotSourceFile)
		{
			if(argc - 2 == 1)
			{
				char szDrive[256];
				char szDir[256];
				char szName[256];
				_splitpath(argv[2], szDrive, szDir, szName, NULL);
				_makepath(szTargetBuf, szDrive, szDir, szName, ".xlib"); // todo: unmagic
				pLibrary = BuildProject(pErrorHandler, argv[2], szLibrariesPath);
			}
			else
			{
				GlobalError errorHolder;
				errorHolder.SetError(&Error::CANT_BUILD_MULTIPLE_PROJECTS);
				if(pErrorHandler)
					pErrorHandler->OnError(&errorHolder);
			}
		}
		else
		{
			strcpy(szTargetBuf, "Untitled.xlib");
			pLibrary = BuildFileRefs(pErrorHandler, hFiles.Get(), (const char**)&argv[2], argc - 2, szLibrariesPath, false);
		}
	}

	// Clean up
	return pLibrary;
}

bool CommandBuild(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv)
{
	char szTargetBuf[256];
	Holder<Library*> hLibrary(BuildFiles(pErrorHandler, szAppPath, argc, argv, szTargetBuf));
	if(!hLibrary.Get())
		return false;
	GXMLTag* pLibTag = hLibrary.Get()->GetLibraryTag();
	if(!pLibTag->ToFile(szTargetBuf))
	{
		GlobalError errorHolder;
		errorHolder.SetError(&Error::ERROR_SAVING);
		errorHolder.SetParam1(szTargetBuf);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return false;
	}
	return true;
}

bool CommandCpp(ErrorHandler* pErrorHandler, const char* szAppPath, int argc, char** argv)
{
	const char* szHeader = 
		"// ------------------------------------------------------------------\n"
		"//  This is a generated file (so don't waste your time modifying it) \n"
		"// ------------------------------------------------------------------\n\n";
	char szTargetBuf[256];
	Holder<Library*> hLibrary(BuildFiles(pErrorHandler, szAppPath, argc, argv, szTargetBuf));
	if(hLibrary.Get())
		return false;
	char szDrive[256];
	char szDir[256];
	char szName[256];
	char szTarget[256];
	_splitpath(szTargetBuf, szDrive, szDir, szName, NULL);
	_makepath(szTarget, szDrive, szDir, szName, ".cpp");
	GXMLTag* pLibTag = hLibrary.Get()->GetLibraryTag();
	strcat(szName, "_xlib_string");
	if(!pLibTag->ToCppFile(szTarget, szName, szHeader))
	{
		GlobalError errorHolder;
		errorHolder.SetError(&Error::ERROR_SAVING);
		errorHolder.SetParam1(szTarget);
		if(pErrorHandler)
			pErrorHandler->OnError(&errorHolder);
		return false;
	}		
	return true;
}
