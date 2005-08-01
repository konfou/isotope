/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

class Library;
class COProject;
class GIntArray;
class GQueue;
class EInstrArray;

class Optimizer
{
protected:
	Library* m_pLibrary;
	COProject* m_pProject;
	int m_nMethodCount;
	int** m_pNewOffsets;
	GIntArray** m_pCallerIDs;
	GQueue* m_pToDoQueue;
	EInstrArray** m_pEInstrArrays;

public:
	// pLibrary is the library to optimize
	// pProject is the source code.  (It can be NULL if not available).
	static void Optimize(Library* pLibrary, COProject* pProject);

protected:
	Optimizer(Library* pLibrary, COProject* pProject);
	virtual ~Optimizer();

	void InitializeNewOffsets();
	void InitializeCallerIDLists();
	void OptimizeLibrary();
	void UpdateSourceOffsets();
	void OptimizeMethod(int n);

};

#endif // __OPTIMIZER_H__
