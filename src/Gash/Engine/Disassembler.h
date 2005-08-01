/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __DISASSEMBLER_H__
#define __DISASSEMBLER_H__

class Library;
class Disassembly;
class GXMLTag;
class EInstrArray;

#include "../../GClasses/GQueue.h"

class Disassembler
{
protected:
   	GQueue m_q;
	Library* m_pLibrary;
	int m_nCurrentLine;

public:
	// Disassembles a .xlib file (to a text file)
	static char* DisassembleLibraryToText(Library* pLibrary, int* pnOutLength);

	// Disassembles just the instructions in a method
	static char* DisassembleEInstrArrayToText(EInstrArray* pEInstrArray, int* pnOutLength);

protected:
	Disassembler(Library* pLibrary);
    virtual ~Disassembler();

	void Add(const char* szText, int* pnPos = NULL);
    char* LibraryToText(int* pnOutLength);
	char* QueueToBuffer(int* pnOutLength);
	void MethodToText(int nMethodID);
	void InstructionToText(EInstrArray* pEInstrArray, int nInstr);
};

#endif // __DISASSEMBLER_H__
