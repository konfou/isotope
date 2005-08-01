/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GKEYPAIR_H__
#define __GKEYPAIR_H__

class GBigNumber;
class GXMLTag;
class GRand;

class GKeyPair
{
private:
	GBigNumber* m_pPublicKey;
	GBigNumber* m_pPrivateKey;
	GBigNumber* m_pN;

public:
	GKeyPair();
	virtual ~GKeyPair();

	// Input:  The size that pRand was seeded with will determine the size of the keys
	// Output: "this" will hold both a public and private key when it returns
	void GenerateKeyPair(GRand* pRand);

	// Takes ownership of the GBigNumber you pass in
	void SetPublicKey(GBigNumber* pPublicKey);
	void SetPrivateKey(GBigNumber* pPrivateKey);
	void SetN(GBigNumber* pN);

	// Copies the GBigNumber you pass in
	void CopyPublicKey(GBigNumber* pPublicKey);
	void CopyPrivateKey(GBigNumber* pPrivateKey);
	void CopyN(GBigNumber* pN);

	// Getters
	GBigNumber* GetPublicKey();
	GBigNumber* GetPrivateKey();
	GBigNumber* GetN();

	// Note: you must delete the GXMLTag this returns
	GXMLTag* ToXML(bool bIncludePrivateKey);

	// FromXML
	bool FromXML(GXMLTag* pTag);
	bool FromXML(const char* pBuf, int nBufSize);

	// Returns the maximum number of bytes that you can encrypt using the PowerMod method
	int GetMaxBlockSize();

	// This is the method that encrypts/decrypts your message.
	// If bPublicKey is true, it uses the public key.  If false it uses the private key.
	// Note: you must delete the buffer this returns
	unsigned char* PowerMod(const unsigned char* pInput, int nInputSize, bool bPublicKey, int* pnOutputSize);
};

#endif // __GKEYPAIR_H__
