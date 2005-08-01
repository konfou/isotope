#include <stdio.h>
#include <string.h>
#include "AutoUpdate.h"
#include "GameEngine.h"
#include "../GClasses/GKeyPair.h"
#include "../GClasses/GBigNumber.h"
#include "../GClasses/sha2.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GMacros.h"
#include "../GClasses/GFile.h"
#include "../GClasses/GThread.h"

const char* g_szTrustedPublicKey = "<KeyPair N=\"1b5d56db6338439933613b7dfbd6b687decdc03e1ef0ae02d93c02cd9270406cb6428d0757b552551ebd9426f8810b36d6f6348af1835e4539fb7f2689af46bf\" Public=\"5276ee4b56255a9784fe859cd5d7ae12028a81b983c44ccd9efdefbf4a34aab\" />";const char* GetTrustedPublicKey()
{
	return g_szTrustedPublicKey;
}
void HashBlob(unsigned char* pOutDigest, const unsigned char* pBlob, int nSize)
{
	sha512_ctx ctx;
	memset(&ctx, '\0', sizeof(sha512_ctx));
	sha512_begin(&ctx);
	sha512_hash(pBlob, nSize, &ctx);
	sha512_end(pOutDigest, &ctx);
}

int GetHashOfTagMinusSignature(unsigned char* pOutDigest, int nBytes, GXMLTag* pTag)
{
	// Clear the "Signature" attribute
	Holder<char*> hOldSig(NULL);
	GXMLAttribute* pAttrSignature = pTag->GetAttribute("Signature");
	if(!pAttrSignature)
	{
		pAttrSignature = new GXMLAttribute("Signature", "");
		pTag->AddAttribute(pAttrSignature);
	}
	else
	{
		hOldSig.Set(new char[strlen(pAttrSignature->GetValue()) + 1]);
		strcpy(hOldSig.Get(), pAttrSignature->GetValue());
		pAttrSignature->SetValue("");
	}

	// Hash the XML doc
	Holder<char*> hDoc(pTag->ToString());
	char* pDoc = hDoc.Get();
	unsigned char pDigest[SHA512_DIGEST_LENGTH];
	HashBlob(pDigest, (const unsigned char*)pDoc, strlen(pDoc));

	// Copy as much of the hash as the buffer can hold
	int nOutSize = MIN(nBytes, SHA512_DIGEST_LENGTH);
	memcpy(pOutDigest, pDigest, nOutSize);

	// Restore the "Signature" value
	if(hOldSig.Get())
		pAttrSignature->SetValue(hOldSig.Get());

	return nOutSize;
}

void Sign(GXMLTag* pTag, GKeyPair* pKeyPair)
{
	// Digest the tag
	int nMaxBlockSize = pKeyPair->GetMaxBlockSize();
	Holder<unsigned char*> hDigest(new unsigned char[nMaxBlockSize]);
	unsigned char* pDigest = hDigest.Get();
	int nDigestSize = GetHashOfTagMinusSignature(pDigest, nMaxBlockSize, pTag);

	// Encrypt the digest with the private key
	int nSignatureSize;
	Holder<unsigned char*> hSignature(pKeyPair->PowerMod(pDigest, nDigestSize, false, &nSignatureSize));

	// Make sure it will round-trip
	int nRoundTripSize;
	Holder<unsigned char*> hRoundTrip(pKeyPair->PowerMod(hSignature.Get(), nSignatureSize, true, &nRoundTripSize));
	if(nRoundTripSize != nDigestSize || memcmp(hRoundTrip.Get(), pDigest, nDigestSize) != 0)
	{
		GAssert(false, "Failed to round trip the signature");
		throw "Failed to round trip the signature";
	}

	// Convert to hex and store in the signature
	Holder<char*> hHexSig(new char[nSignatureSize * 2 + 10]);
	char* pHexSig = hHexSig.Get();
	BufferToHex(hSignature.Get(), nSignatureSize, pHexSig);
	GXMLAttribute* pAttrSignature = pTag->GetAttribute("Signature");
	GAssert(pAttrSignature, "Expected a signature attribute");
	pAttrSignature->SetValue(pHexSig);
}

bool VerifySignature(GXMLTag* pTag, GKeyPair* pKeyPair)
{
	// Convert the signature from hex to bytes
	GXMLAttribute* pAttrSignature = pTag->GetAttribute("Signature");
	if(!pAttrSignature)
	{
		GAssert(false, "Expected a signature attribute");
		return false;
	}
	const char* pHexSignature = pAttrSignature->GetValue();
	int nHexSize = strlen(pHexSignature);
	Holder<unsigned char*> hSignature(new unsigned char[nHexSize / 2]);
	unsigned char* pSignature = hSignature.Get();
	HexToBuffer(pHexSignature, nHexSize, pSignature);

	// Decrypt the signature with the public key
	int nDigestSize;
	Holder<unsigned char*> hDigest(pKeyPair->PowerMod(pSignature, nHexSize / 2, true, &nDigestSize));
	unsigned char* pDigest = hDigest.Get();

	// Compute the actual digest of the tag and compare it with the reported digest
	Holder<unsigned char*> hActualDigest(new unsigned char[nDigestSize]);
	unsigned char* pActualDigest = hActualDigest.Get();
	int nActualDigestSize = GetHashOfTagMinusSignature(pActualDigest, nDigestSize, pTag);
	if(nActualDigestSize != nDigestSize || memcmp(pDigest, pActualDigest, nDigestSize) != 0)
	{
		GAssert(false, "Invalid signature!");
		return false;
	}
	return true;
}

void BlessThisApplication(const char* szPrivateKeyFilename, const char* szUrl)
{
	// Load the private key file
	int nSize;
	Holder<char*> hFile(GFile::LoadFileToBuffer(szPrivateKeyFilename, &nSize));
	char* pFile = hFile.Get();
	if(!pFile)
		throw "error loading private key file";
	GKeyPair kp;
	if(!kp.FromXML(pFile, nSize))
		throw "error parsing private key file";

	// Make sure the public key is the trusted public key
	GKeyPair kpTmp;
	const char* szTrustedPublicKey = GetTrustedPublicKey();
	if(!kpTmp.FromXML(szTrustedPublicKey, strlen(szTrustedPublicKey)))
		throw "error deserializing the trusted public key";
	if(kp.GetPublicKey()->CompareTo(kpTmp.GetPublicKey()) != 0)
		throw "That isn't the trusted key pair!";

	// Hash this file
	const char* szAppPath = GameEngine::GetAppPath();
	char* szFilename = (char*)alloca(strlen(szAppPath) + 50);
	strcpy(szFilename, szAppPath);
	strcat(szFilename, "Isotope.exe");
	Holder<char*> hThisApp(GFile::LoadFileToBuffer(szFilename, &nSize));
	char* pThisApp = hThisApp.Get();
	unsigned char pDigest[SHA512_DIGEST_LENGTH];
	HashBlob(pDigest, (const unsigned char*)pThisApp, nSize);
	Holder<char*> hHex(new char[2 * SHA512_DIGEST_LENGTH + 10]);
	char* pHex = hHex.Get();
	BufferToHex(pDigest, SHA512_DIGEST_LENGTH, pHex);

	// Create the update.xml file
	Holder<GXMLTag*> hTag(new GXMLTag("Update"));
	GXMLTag* pTag = hTag.Get();
	char szTmp[64];
	itoa(BUILD_NUMBER, szTmp, 10);
	pTag->AddAttribute(new GXMLAttribute("Build", szTmp));
	pTag->AddAttribute(new GXMLAttribute("File", szUrl));
	pTag->AddAttribute(new GXMLAttribute("Hash", pHex));
	Sign(pTag, &kp);
	pTag->ToFile("update.xml");
}

unsigned int CheckForUpdates(void* pData)
{
	// Download the update.xml file
	int nSize;
	Holder<char*> hUpdateFile(GameEngine::DownloadFile(UPDATE_DESCRIPTOR, &nSize, false));
	char* pUpdateFile = hUpdateFile.Get();
	if(!pUpdateFile)
	{
		// todo: assert cuz the update server is down or inaccessible
		return 0;
	}

	// Parse the xml file
	const char* szErrorMessage;
	int nErrorLine;
	Holder<GXMLTag*> hTag(GXMLTag::FromString(pUpdateFile, nSize, &szErrorMessage, NULL, &nErrorLine));
	GXMLTag* pTag = hTag.Get();
	if(!pTag)
	{
		GAssert(false, "Bad update.xml file");
		return 0;
	}

	// See if there's a newer build available
	GXMLAttribute* pAttrBuild = pTag->GetAttribute("Build");
	if(!pAttrBuild)
	{
		GAssert(false, "Expected a 'Build' attribute");
		return 0;
	}
	int nLatestBuild = atoi(pAttrBuild->GetValue());
	GAssert(nLatestBuild >= BUILD_NUMBER, "How'd we get a build that's newer than the latest build?");
	if(nLatestBuild <= BUILD_NUMBER)
		return 0;

	// Make sure update.xml was signed with the trusted private key
	GKeyPair kp;
	const char* szTrustedPublicKey = GetTrustedPublicKey();
	if(!kp.FromXML(szTrustedPublicKey, strlen(szTrustedPublicKey)))
	{
		GAssert(false, "error deserializing the trusted public key");
		return 0;
	}
	if(!VerifySignature(pTag, &kp))
	{
		GAssert(false, "bad signature");
		return 0;
	}

	// Download the updated application file
	GXMLAttribute* pFileAttr = pTag->GetAttribute("File");
	if(!pFileAttr)
	{
		GAssert(false, "Bad update.xml file");
		return 0;
	}
	Holder<char*> hNewAppFile(GameEngine::DownloadFile(pFileAttr->GetValue(), &nSize, false));
	char* pNewAppFile = hNewAppFile.Get();
	if(!pNewAppFile)
	{
		GAssert(false, "Failed to download new app file");
		return 0;
	}

	// Check the hash of the updated application file.  This makes sure the file didn't get corrupted
	// during transfer and also prevents a man-in-the-middle attacker from sending some other file.
	GXMLAttribute* pHashAttr = pTag->GetAttribute("Hash");
	if(!pHashAttr)
	{
		GAssert(false, "Expected a 'Hash' attribute");
		return 0;
	}
	unsigned char pHash[SHA512_DIGEST_LENGTH];
	HashBlob(pHash, (const unsigned char*)pNewAppFile, nSize);
	Holder<char*> hHex(new char[2 * SHA512_DIGEST_LENGTH + 10]);
	char* pHex = hHex.Get();
	BufferToHex(pHash, SHA512_DIGEST_LENGTH, pHex);
	if(stricmp(pHex, pHashAttr->GetValue()) != 0)
	{
		GAssert(false, "Hashes don't match!");
		return 0;
	}

	// Save the file
	FileHolder hNewFile(fopen("update.dat", "wb"));
	FILE* pNewFile = hNewFile.Get();
	if(!pNewFile)
	{
		GAssert(false, "Failed to save new app file");
		return 0;
	}
	if(fwrite(pNewAppFile, nSize, 1, pNewFile) != 1)
	{
		GAssert(false, "Failed to write new app file");
		return 0;
	}
	return 0;
}

bool DoAutoUpdate()
{
	// Check if an update is ready to go
	const char* szAppPath = GameEngine::GetAppPath();
	char* szBuf = (char*)alloca(strlen(szAppPath) + 50);
	strcpy(szBuf, szAppPath);
	strcat(szBuf, "update.dat");
	if(!GFile::DoesFileExist(szBuf))
	{
		GThread::SpawnThread(CheckForUpdates, NULL);
		return true; // true = just run the app as normal
	}

#ifdef WIN32
	// Run a batch file to swap in the new app
	{
		strcpy(szBuf, szAppPath);
		strcat(szBuf, "update.bat");
		FileHolder hFile(fopen(szBuf, "w"));
		FILE* pFile = hFile.Get();
		if(!pFile)
		{
			GAssert(false, "Error creating the update batch file");
			return true; // true = just run the app as normal
		}
		fputs("call wait.exe 1\r\n", pFile);
		fputs("del /f /q Isotopeold.exe\r\n", pFile);
		fputs("ren Isotope.exe Isotopeold.exe\r\n", pFile);
		fputs("ren update.dat Isotope.exe\r\n", pFile);
		fputs("start Isotope.exe\r\n", pFile);
	}
	ShellExecute(NULL, NULL, szBuf, NULL, NULL, SW_HIDE);
#else // WIN32
	GAssert(false, "todo: Auto update not implemented for Linux yet");
#endif // !WIN32
	return false; // false = exit the application immediately
}

