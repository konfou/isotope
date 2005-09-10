#include "MInductionPuzzle.h"
#include "../Gash/BuiltIns/GashString.h"
#include "../GClasses/GXML.h"
#include <wchar.h>
#include "GameEngine.h"

MInductionPuzzle::MInductionPuzzle(Engine* pEngine, GXMLTag* pSpecTag)
 : WrapperObject(pEngine, "InductionPuzzle")
{
	ParseSpec(pSpecTag);
}

MInductionPuzzle::~MInductionPuzzle()
{
	delete(m_nDimensionSizes);
}

void MInductionPuzzle::allocate(Engine* pEngine, EVar* pSpecification)
{
	// Parse the string into an XML blob
	GString* pSpecString = &pSpecification->pStringObject->m_value;
	Holder<char*> hSpecString(new char[pSpecString->GetLength() + 1]);
	char* szSpecString = hSpecString.Get();
	pSpecString->GetAnsi(szSpecString);
	const char* szErrorMessage;
	int nErrorLine;
	Holder<GXMLTag*> hTag(GXMLTag::FromString(szSpecString, pSpecString->GetLength(), &szErrorMessage, NULL, &nErrorLine, NULL));
	GXMLTag* pTag = hTag.Get();
	if(!pTag)
		GameEngine::ThrowError("Failed to parse induction puzzle spec xml string: %s", szErrorMessage);

	// Construct the puzzle
	pEngine->SetThis(new MInductionPuzzle(pEngine, pTag));
}

/*virtual*/ void MInductionPuzzle::GetDisplayValue(wchar_t* pBuf, int nSize)
{
	GAssert(nSize > 32, "Buffer too small");
	wcscpy(pBuf, L"InductionPuzzle");
}

void MInductionPuzzle::ParseSpec(GXMLTag* pSpecTag)
{
	// Count dimensions
	m_nDimensions = 0;
	GXMLTag* pTag;
	for(pTag = pSpecTag->GetFirstChildTag(); pTag; pTag = pSpecTag->GetNextChildTag(pTag))
	{
		if(stricmp(pTag->GetName(), "Dimension") == 0)
			m_nDimensions++;
	}

	// Make array of dimension sizes
	m_nDimensionSizes = new int[m_nDimensions];
	m_nElements = 1;
	int nDim = 0;
	for(pTag = pSpecTag->GetFirstChildTag(); pTag; pTag = pSpecTag->GetNextChildTag(pTag))
	{
		if(stricmp(pTag->GetName(), "Dimension") == 0)
		{
			GXMLAttribute* pSizeAttr = pTag->GetAttribute("Size");
			if(!pSizeAttr)
				GameEngine::ThrowError("Expected a Size attribute in puzzle spec");
			int nSize = atoi(pSizeAttr->GetValue());
			m_nDimensionSizes[nDim] = nSize;
			m_nElements *= nSize;
			nDim++;
		}
	}

	
}

void MInductionPuzzle::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	GAssert(false, "who referenced this object?");
}

void MInductionPuzzle::fromStream(Engine* pEngine, EVar* pStream)
{
	GAssert(false, "who referenced this object?");
}

