/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MKeyPair.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GKeyPair.h"
#include "../GClasses/GRand.h"
#include "../GClasses/GXML.h"


MKeyPair::MKeyPair(Controller* pController, const char* szFilename)
: Model()
{
	m_szFilename = szFilename;
	m_pEntropy = new unsigned char[ENTROPY_BYTES];
	GAssert(ENTROPY_BYTES > 2048, "not enough entropy to be safe");
	m_nPos = 4; // leave the first 4 bytes uninitialized just for fun
}

/*virtual*/ MKeyPair::~MKeyPair()
{
	delete(m_pEntropy);
}

void MKeyPair::AddEntropy(int n)
{
	unsigned char* pBytes = (unsigned char*)&n;
	int i;
	for(i = 0; i < sizeof(int); i++)
	{
		if(m_nPos < ENTROPY_BYTES)
			m_pEntropy[m_nPos++] = pBytes[i];
	}
}

double MKeyPair::GetPercent()
{
	return (double)m_nPos / ENTROPY_BYTES;
}

void MKeyPair::SaveKeyPair()
{
	if(m_nPos < ENTROPY_BYTES)
		throw "Not enough entropy to produce the key pair yet";
	Holder<unsigned char*> hSeed(new unsigned char[KEY_BIT_SIZE / 8]);
	unsigned char* pSeed = hSeed.Get();
	GRand::ShaDigestEntropy(m_pEntropy, ENTROPY_BYTES, ENTROPY_BYTES * 8 / KEY_BIT_SIZE, (unsigned int*)pSeed);
	GRand r(pSeed, KEY_BIT_SIZE / 8);
	GKeyPair kp;
	kp.GenerateKeyPair(&r);
	Holder<GXMLTag*> hTag(kp.ToXML(true));
	GXMLTag* pTag = hTag.Get();
	pTag->ToFile(m_szFilename);
}

