/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GRIDDLE_H__
#define __GRIDDLE_H__

class GRand;
class GXMLTag;
class GKeyPair;

class GRiddle
{
protected:
	int m_nTotalBytes;
	int m_nMysteryBits;
	unsigned char* m_pRiddle;
	unsigned char* m_pDigest;
	char* m_pSignature;

public:
	GRiddle();
	virtual ~GRiddle();

	unsigned char* GetDigest() { return m_pDigest; }
	int GetDigestByteCount();

	unsigned char* GetRiddle() { return m_pRiddle; }
	int GetRiddleTotalByteCount() { return m_nTotalBytes; }
	int GetUnknownBitCount() { return m_nMysteryBits; }

	bool GenerateRiddle(GRand* pRand, int nTotalBytes, int nMysteryBits);
	bool Solve();
	bool CheckAnswer();

	GXMLTag* ToXML(bool IncludeAnswer);
	bool FromXML(GXMLTag* pTag);

	void Sign(GKeyPair* pKeyPair);
	bool CheckSignature(GKeyPair* pKeyPair);

protected:
	void DigestRiddle(unsigned char* pOutput);
	void SetSignature(const char* szSig);
};

#endif // __GRIDDLE_H__
