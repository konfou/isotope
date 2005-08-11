/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Optimizer.h"
#include "EInstrArray.h"
#include "EMethod.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GArray.h"

/*static*/ void Optimizer::Optimize(Library* pLibrary, COProject* pProject)
{
	Optimizer tmp(pLibrary, pProject);
	tmp.OptimizeLibrary();
}

Optimizer::Optimizer(Library* pLibrary, COProject* pProject)
{
	m_pLibrary = pLibrary;
	m_pProject = pProject;
	m_nMethodCount = m_pLibrary->GetMethodCount();

	// Method Binaries
	m_pEInstrArrays = new EInstrArray*[m_nMethodCount];
	int n;
	for(n = 0; n < m_nMethodCount; n++)
		m_pEInstrArrays[n] = new EInstrArray(m_pLibrary->GetEMethod(n)->GetTag(), NULL);

	// Maps from old to new source code offsets
	m_pNewOffsets = new int*[m_nMethodCount];
	InitializeNewOffsets();

	// List of every method that calls each method
	m_pCallerIDs = new GIntArray*[m_nMethodCount];
	InitializeCallerIDLists();

	// Queue that lists all the methods that need to be optimized
	m_pToDoQueue = new GQueue();
	for(n = m_nMethodCount - 1; n >= 0; n--)
		m_pToDoQueue->Push(n);
}

Optimizer::~Optimizer()
{
	delete(m_pNewOffsets);
	int n;
	for(n = 0; n < m_nMethodCount; n++)
		delete(m_pCallerIDs[n]);
	delete(m_pCallerIDs);
	delete(m_pToDoQueue);
}

void Optimizer::InitializeNewOffsets()
{
	
}

void Optimizer::InitializeCallerIDLists()
{
	
}

void Optimizer::OptimizeLibrary()
{
	while(m_pToDoQueue->GetSize() > 0)
	{
		int n;
		bool b = m_pToDoQueue->Pop(&n);
		GAssert(b, "Failed to pop from queue");
		OptimizeMethod(n);		
	}
	if(m_pProject)
		UpdateSourceOffsets();
}

void Optimizer::UpdateSourceOffsets()
{
	// todo: write this method
}

void Optimizer::OptimizeMethod(int n)
{
	// todo: write this method
}
