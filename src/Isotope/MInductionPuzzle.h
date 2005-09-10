#ifndef __MINDUCTIONPUZZLE_H__
#define __MINDUCTIONPUZZLE_H__

#include "../Gash/Include/GashSdl.h"

class MInductionPuzzle : public WrapperObject
{
protected:
	int m_nDimensions;
	int* m_nDimensionSizes;
	int m_nElements;

public:
	MInductionPuzzle(Engine* pEngine, GXMLTag* pSpecTag);
	virtual ~MInductionPuzzle();

	void ParseSpec(GXMLTag* pSpecTag);

	void allocate(Engine* pEngine, EVar* pSpecification);

	// This method is called from Gash when the object needs to be serialized
	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs);

	// This method is called from Gash as part of the deserialization process
	void fromStream(Engine* pEngine, EVar* pStream);

	// This method is called from Gash as part of the deserialization process
	void setRefs(Engine* pEngine, EVar* pRefs)	{}

	// This method is called from Gash when debugging
	virtual void GetDisplayValue(wchar_t* pBuf, int nSize);

};

#endif // __MINDUCTIONPUZZLE_H__
