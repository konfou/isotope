/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GEZSocket.h"
#include "GMacros.h"
#include "GKeyPair.h"
#include "GBigNumber.h"
#include "GSpinLock.h"
#include "GPointerQueue.h"
#include "GRand.h"
#include "GArray.h"
#include "GCrypto.h"
#include "GXML.h"
#ifdef WIN32
#include <malloc.h>
#include "GWindows.h"
#else
#include <alloca.h>
#include <unistd.h>
#endif // !WIN32


class GSocketMessage
{
protected:
	unsigned char* m_pMessage;
	int m_nMessageSize;
	int m_nConnection;

public:
	GSocketMessage(unsigned char* pMessage, int nMessageSize, int nConnection)
	{
		m_pMessage = new unsigned char[nMessageSize];
		GAssert(m_pMessage, "out of memory");
		memcpy(m_pMessage, pMessage, nMessageSize);
		m_nMessageSize = nMessageSize;
		m_nConnection = nConnection;
	}

	virtual ~GSocketMessage()
	{
		delete(m_pMessage);
	}

	const unsigned char* GetTheMessage() { return m_pMessage; }
	int GetMessageSize() { return m_nMessageSize; }
	int GetConnection() { return m_nConnection; }

	// you must delete the buffer this returns
	unsigned char* TakeBuffer()
	{
		unsigned char* pMessage = m_pMessage;
		m_pMessage = NULL;
		return pMessage;
	}
};

// --------------------------------------------------------------------------

const char GEZSocketTag[] = "GEZS";

struct GEZSocketPacketHeader
{
	char tag[4];
	int nPayloadSize;
};

// --------------------------------------------------------------------------

struct GEZSocketServerBuffer
{
public:
	unsigned char* m_pBuffer;
	int m_nBufferPos;

	GEZSocketServerBuffer(int nMaxPacketSize)
	{
		m_pBuffer = new unsigned char[nMaxPacketSize + sizeof(struct GEZSocketPacketHeader)];
		m_nBufferPos = 0;
	}

	~GEZSocketServerBuffer()
	{
		delete(m_pBuffer);
	}
};

// if nMaxPacketSize = 0, the socket will be compatible with TCP sockets.
// if nMaxPacketSize > 0, it will use it's own protocol that guarantees
//          same-size delivery of packets, but has a maximum packet size.
GEZSocketServer::GEZSocketServer(int nMaxPacketSize) : GSocket()
{
	GAssert(sizeof(struct GEZSocketPacketHeader) == 8, "packing issue");
	m_nMaxPacketSize = nMaxPacketSize;
	if(nMaxPacketSize > 0)
		m_pBuffers = new GPointerArray(16);
	else
		m_pBuffers = NULL;
	m_pMessageQueueLock = new GSpinLock();
	m_pMessageQueue = new GPointerQueue();
}

GEZSocketServer::~GEZSocketServer()
{
	// Join the other threads now so they don't try
	// to queue up a message after we delete the
	// message queue
	m_bKeepAccepting = false;
	m_bKeepListening = false;
	JoinAcceptorThread();
	JoinAllListenThreads();

	if(m_pBuffers)
	{
		int nCount = m_pBuffers->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
			delete((GEZSocketServerBuffer*)m_pBuffers->GetPointer(n));
		delete(m_pBuffers);
	}
	delete(m_pMessageQueue);
	delete(m_pMessageQueueLock);
}

/*static*/ GEZSocketServer* GEZSocketServer::HostTCPSocket(int nPort)
{
	GEZSocketServer* pSocket = new GEZSocketServer(0);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, true, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

/*static*/ GEZSocketServer* GEZSocketServer::HostUDPSocket(int nPort)
{
	GEZSocketServer* pSocket = new GEZSocketServer(0);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(true, true, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

/*static*/ GEZSocketServer* GEZSocketServer::HostGashSocket(int nPort, int nMaxPacketSize)
{
	GEZSocketServer* pSocket = new GEZSocketServer(nMaxPacketSize);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, true, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

void GEZSocketServer::QueueMessage(unsigned char* pBuf, int nLen, int nConnectionNumber)
{
	m_pMessageQueueLock->Lock("GEZSocketServer::QueueMessage");
	GSocketMessage* pNewMessage = new GSocketMessage(pBuf, nLen, nConnectionNumber);
	m_pMessageQueue->Push(pNewMessage);
	m_pMessageQueueLock->Unlock();
}

int GEZSocketServer::GetMessageCount()
{
/*
	int nSize;
	m_pMessageQueueLock->Lock("GEZSocketServer::GetMessageCount");
	nSize = m_pMessageQueue->GetSize();
	m_pMessageQueueLock->Unlock();
	return nSize;
*/
	return m_pMessageQueue->GetSize();
}

unsigned char* GEZSocketServer::GetNextMessage(int* pnSize, int* pnOutConnectionNumber)
{
	m_pMessageQueueLock->Lock("GEZSocketClient::GetNextMessage");
	if(m_pMessageQueue->GetSize() <= 0)
	{
		m_pMessageQueueLock->Unlock();
		*pnOutConnectionNumber = -1;
		return NULL;
	}
	GSocketMessage* pMessage = (GSocketMessage*)m_pMessageQueue->Pop();
	m_pMessageQueueLock->Unlock();
	*pnSize = pMessage->GetMessageSize();
	*pnOutConnectionNumber = pMessage->GetConnection();
	unsigned char* pBuf = pMessage->TakeBuffer();
	delete(pMessage);
	return pBuf;
}

bool GEZSocketServer::Receive(unsigned char *pBuf, int nLen, int nConnectionNumber)
{
//fprintf(stderr, "Received %d bytes from %d {%c%c...%c%c}\n", nLen, nConnectionNumber, pBuf[0], pBuf[1], pBuf[nLen - 2], pBuf[nLen - 1]);
//fflush(stderr);
	if(m_nMaxPacketSize == 0)
	{
		QueueMessage(pBuf, nLen, nConnectionNumber);
	}
	else
	{
		while(nConnectionNumber > m_pBuffers->GetSize())
			m_pBuffers->AddPointer(new GEZSocketServerBuffer(m_nMaxPacketSize));
		GEZSocketServerBuffer* pBuffer = (GEZSocketServerBuffer*)m_pBuffers->GetPointer(nConnectionNumber - 1);
		while(nLen > 0)
		{
			if(pBuffer->m_nBufferPos == 0 &&
				nLen >= (int)sizeof(struct GEZSocketPacketHeader) &&
				nLen >= (int)sizeof(struct GEZSocketPacketHeader) + ((struct GEZSocketPacketHeader*)pBuf)->nPayloadSize)
			{
				// We've got a whole packet, so just queue it up
				GAssert(*(unsigned int*)pBuf == *(unsigned int*)GEZSocketTag, "Bad Packet");
				int nSize = ((struct GEZSocketPacketHeader*)pBuf)->nPayloadSize;
				pBuf += sizeof(struct GEZSocketPacketHeader);
				nLen -= sizeof(struct GEZSocketPacketHeader);
				QueueMessage(pBuf, nSize, nConnectionNumber);
				pBuf += nSize;
				nLen -= nSize;
			}
			else
			{
				// We've only got a partial packet, so we need to buffer it
				while(pBuffer->m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) && nLen > 0)
				{
					pBuffer->m_pBuffer[pBuffer->m_nBufferPos] = *pBuf;
					if(pBuffer->m_nBufferPos < 4 && *pBuf != GEZSocketTag[pBuffer->m_nBufferPos])
					{
						GAssert(false, "bad packet");
						pBuffer->m_nBufferPos = -1;
					}
					pBuffer->m_nBufferPos++;
					pBuf++;
					nLen--;
				}
				if(pBuffer->m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader))
					return true;
				struct GEZSocketPacketHeader* pHeader = (struct GEZSocketPacketHeader*)pBuffer->m_pBuffer;
				if(pHeader->nPayloadSize > m_nMaxPacketSize)
				{
					GAssert(false, "Received a packet that was too big");
					pHeader->nPayloadSize = m_nMaxPacketSize;
				}
				while(pBuffer->m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) + pHeader->nPayloadSize && nLen > 0)
				{
					pBuffer->m_pBuffer[pBuffer->m_nBufferPos] = *pBuf;
					pBuffer->m_nBufferPos++;
					pBuf++;
					nLen--;
				}
				if(pBuffer->m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) + pHeader->nPayloadSize)
					return true;
				QueueMessage(pBuffer->m_pBuffer + sizeof(struct GEZSocketPacketHeader), pHeader->nPayloadSize, nConnectionNumber);
				pBuffer->m_nBufferPos = 0;
			}
		}
	}
	return true;
}

bool GEZSocketServer::Send(const void* pBuf, int nLen, int nConnectionNumber)
{
	if(m_nMaxPacketSize > 0)
	{
		GAssert(nLen <= m_nMaxPacketSize, "packet too big");
		struct GEZSocketPacketHeader header;
		header.tag[0] = GEZSocketTag[0];
		header.tag[1] = GEZSocketTag[1];
		header.tag[2] = GEZSocketTag[2];
		header.tag[3] = GEZSocketTag[3];
		header.nPayloadSize = nLen;
		if(!GSocket::Send((const unsigned char*)&header, sizeof(struct GEZSocketPacketHeader), nConnectionNumber))
			return false;
	}
	bool bRet = GSocket::Send((const unsigned char*)pBuf, nLen, nConnectionNumber);
	return bRet;
}

// --------------------------------------------------------------------------

// if nMaxPacketSize = 0, the socket will be compatible with TCP sockets.
// if nMaxPacketSize > 0, it will use it's own protocol that guarantees
//          same-size delivery of packets, but has a maximum packet size.
GEZSocketClient::GEZSocketClient(int nMaxPacketSize) : GSocket()
{
	m_nMaxPacketSize = nMaxPacketSize;
	if(nMaxPacketSize > 0)
		m_pBuffer = new unsigned char[nMaxPacketSize + sizeof(struct GEZSocketPacketHeader)];
	else
		m_pBuffer = NULL;
	m_nBufferPos = 0;
	m_pMessageQueueLock = new GSpinLock();
	m_pMessageQueue = new GPointerQueue();
}

GEZSocketClient::~GEZSocketClient()
{
	// Join the other threads now so they don't try
	// to queue up a message after we delete the
	// message queue
	m_bKeepAccepting = false;
	m_bKeepListening = false;
	JoinAcceptorThread();
	JoinAllListenThreads();

	delete(m_pBuffer);
	delete(m_pMessageQueue);
	delete(m_pMessageQueueLock);
}

/*static*/ GEZSocketClient* GEZSocketClient::ConnectToTCPSocket(const char* szAddress, int nPort)
{
	GEZSocketClient* pSocket = new GEZSocketClient(0);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, false))
	{
		delete(pSocket);
		return NULL;
	}
	if(!pSocket->Connect(szAddress, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

/*static*/ GEZSocketClient* GEZSocketClient::ConnectToUDPSocket(const char* szAddress, int nPort)
{
	GEZSocketClient* pSocket = new GEZSocketClient(0);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(true, false))
	{
		delete(pSocket);
		return NULL;
	}
	if(!pSocket->Connect(szAddress, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

/*static*/ GEZSocketClient* GEZSocketClient::ConnectToGashSocket(const char* szAddress, int nPort, int nMaxPacketSize)
{
	GEZSocketClient* pSocket = new GEZSocketClient(nMaxPacketSize);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, false))
	{
		delete(pSocket);
		return NULL;
	}
	if(!pSocket->Connect(szAddress, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

void GEZSocketClient::QueueMessage(unsigned char* pBuf, int nLen)
{
	m_pMessageQueueLock->Lock("GEZSocketClient::QueueMessage");
	m_pMessageQueue->Push(new GSocketMessage(pBuf, nLen, 0));
	m_pMessageQueueLock->Unlock();
}

int GEZSocketClient::GetMessageCount()
{
	return m_pMessageQueue->GetSize();
}

unsigned char* GEZSocketClient::GetNextMessage(int* pnSize)
{
	if(m_pMessageQueue->GetSize() <= 0)
	{
		*pnSize = 0;
		return NULL;
	}
	m_pMessageQueueLock->Lock("GEZSocketClient::GetNextMessage");
	GSocketMessage* pMessage = (GSocketMessage*)m_pMessageQueue->Pop();
	m_pMessageQueueLock->Unlock();
	*pnSize = pMessage->GetMessageSize();
	unsigned char* pBuf = pMessage->TakeBuffer();
	delete(pMessage);
	return pBuf;
}

bool GEZSocketClient::Receive(unsigned char *pBuf, int nLen, int nConnectionNumber)
{
	if(m_nMaxPacketSize == 0)
	{
		GAssert(nConnectionNumber == 0, "unexpected connection number");
		QueueMessage(pBuf, nLen);
	}
	else
	{
		while(nLen > 0)
		{
			if(m_nBufferPos == 0 &&
				nLen >= (int)sizeof(struct GEZSocketPacketHeader) &&
				nLen >= (int)sizeof(struct GEZSocketPacketHeader) + ((struct GEZSocketPacketHeader*)pBuf)->nPayloadSize)
			{
				// We've got a whole packet, so just queue it up
				GAssert(*(unsigned int*)pBuf == *(unsigned int*)GEZSocketTag, "Bad Packet");
				int nSize = ((struct GEZSocketPacketHeader*)pBuf)->nPayloadSize;
				pBuf += sizeof(struct GEZSocketPacketHeader);
				nLen -= sizeof(struct GEZSocketPacketHeader);
				QueueMessage(pBuf, nSize);
				pBuf += nSize;
				nLen -= nSize;
			}
			else
			{
				// We've only got a partial packet, so we need to buffer it
				while(m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) && nLen > 0)
				{
					m_pBuffer[m_nBufferPos] = *pBuf;
					if(m_nBufferPos < 4 && *pBuf != GEZSocketTag[m_nBufferPos])
						m_nBufferPos = -1;
					m_nBufferPos++;
					pBuf++;
					nLen--;
				}
				if(m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader))
					return true;
				struct GEZSocketPacketHeader* pHeader = (struct GEZSocketPacketHeader*)m_pBuffer;
				if(pHeader->nPayloadSize > m_nMaxPacketSize)
				{
					GAssert(false, "Received a packet that was too big");
					pHeader->nPayloadSize = m_nMaxPacketSize;
				}
				while(m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) + pHeader->nPayloadSize && nLen > 0)
				{
					m_pBuffer[m_nBufferPos] = *pBuf;
					m_nBufferPos++;
					pBuf++;
					nLen--;
				}
				if(m_nBufferPos < (int)sizeof(struct GEZSocketPacketHeader) + pHeader->nPayloadSize)
					return true;
				GAssert(nConnectionNumber == 0, "unexpected connection number");
				QueueMessage(m_pBuffer + sizeof(struct GEZSocketPacketHeader), pHeader->nPayloadSize);
				m_nBufferPos = 0;
			}
		}
	}
	return true;
}

bool GEZSocketClient::Send(const void* pBuf, int nLen)
{
	if(m_nMaxPacketSize > 0)
	{
		GAssert(nLen <= m_nMaxPacketSize, "packet too big");
		struct GEZSocketPacketHeader header;
		header.tag[0] = GEZSocketTag[0];
		header.tag[1] = GEZSocketTag[1];
		header.tag[2] = GEZSocketTag[2];
		header.tag[3] = GEZSocketTag[3];
		header.nPayloadSize = nLen;
		if(!GSocket::Send((const unsigned char*)&header, sizeof(struct GEZSocketPacketHeader), 0))
			return false;
	}
//fprintf(stderr, "Sending %d bytes {%c%c...%c%c}\n", nLen, ((char*)pBuf)[0], ((char*)pBuf)[1], ((char*)pBuf)[nLen - 2], ((char*)pBuf)[nLen - 1]);
//fflush(stderr);
	return GSocket::Send((const unsigned char*)pBuf, nLen, 0);
}

// --------------------------------------------------------------------------
















// --------------------------------------------------------------------------

// Handshake Protocol
// ------------------
// 1- Client: (unencrypted) My public key is ____.
// 2- Server: (unencrypted) My public key is ____.
// 3- Server: (enc w/ client public key) My passphrase is ____.
// 4- Client: (enc w/ server public key) My passphrase is ____.
// 5- Anyone: (enc w/ other passphrase) ...

#define HANDSHAKE_MAGIC 0x5ec50ce7

struct HandshakeHeader
{
	unsigned long nMagic; // HANDSHAKE_MAGIC
	unsigned short nMessage; // 0 = My public key is..., 1 = My passphrase is...
	// Data
};

GSecureSocketServer::GSecureSocketServer(int nMaxPacketSize, GRand* pRand) : GEZSocketServer(nMaxPacketSize)
{
	m_pRand = pRand;
	m_pKeyPair = new GKeyPair();
	m_pKeyPair->GenerateKeyPair(pRand);
	m_nPassphraseSize = pRand->GetRandByteCount();
	m_pPassphrases = new GPointerArray(16);
	m_pClientPassphrases = new GPointerArray(16);
	m_nClientPassphraseSizes = new GIntArray(16);
}

GSecureSocketServer::~GSecureSocketServer()
{
	int n;
	for(n = 0; n < m_pPassphrases->GetSize(); n++)
		delete((unsigned char*)m_pPassphrases->GetPointer(n));
	delete(m_pPassphrases);

	for(n = 0; n < m_pClientPassphrases->GetSize(); n++)
		delete((unsigned char*)m_pClientPassphrases->GetPointer(n));
	delete(m_pClientPassphrases);
	delete(m_nClientPassphraseSizes);
	delete(m_pKeyPair);
}

/*static*/ GSecureSocketServer* GSecureSocketServer::HostSecureSocket(u_short nPort, int nMaxPacketSize, GRand* pRand)
{
	// Check input
	int nPassphraseSize = pRand->GetRandByteCount();
	GAssert(nPassphraseSize >= (int)sizeof(int), "pRand not seeded big enough");
	GAssert(nMaxPacketSize > 0, "Bad max packet size");

	// Make the socket
	GSecureSocketServer* pSocket = new GSecureSocketServer(nMaxPacketSize, pRand);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, true, nPort))
	{
		delete(pSocket);
		return NULL;
	}
	return pSocket;
}

int GSecureSocketServer::GetMessageCount()
{
	while(true)
	{
		int nMessageCount = m_pMessageQueue->GetSize();
		if(nMessageCount > 0)
		{
			// See if we are done handshaking for this client
			GSocketMessage* pMessage = (GSocketMessage*)m_pMessageQueue->Peek();
			int nConnection = pMessage->GetConnection();

			// Make sure we have slots for the passphrases
			while(m_pPassphrases->GetSize() <= nConnection)
				m_pPassphrases->AddPointer(NULL);
			while(m_pClientPassphrases->GetSize() <= nConnection)
				m_pClientPassphrases->AddPointer(NULL);
			while(m_nClientPassphraseSizes->GetSize() <= nConnection)
				m_nClientPassphraseSizes->AddInt(0);

			// See if it's a handshake message or a real message
			const unsigned char* pClientPassphrase = (const unsigned char*)m_pClientPassphrases->GetPointer(nConnection);
			if(pClientPassphrase)
				return nMessageCount;
			else
				Handshake();
		}
		else
			return 0;
	}
}

void GSecureSocketServer::Handshake()
{
	// Check the header
	int nSize;
	int nConnection;
	unsigned char* pBuf = GEZSocketServer::GetNextMessage(&nSize, &nConnection);
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)pBuf;
	if(nSize <= (int)sizeof(struct HandshakeHeader) || pHeader->nMagic != HANDSHAKE_MAGIC)
	{
		GAssert(false, "Bad header packet");
		memset(pBuf, '\0', nSize);
		delete(pBuf);
		return;
	}

	// Process the payload
	unsigned char* pPayload = pBuf + sizeof(struct HandshakeHeader);
	int nPayloadSize = nSize - sizeof(struct HandshakeHeader);
	if(pHeader->nMessage == 0)
	{
		bool bRet = OnReceivePublicKey((const char*)pPayload, nPayloadSize, nConnection);
		GAssert(bRet, "error processing public key");
	}
	else
	{
		GAssert(pHeader->nMessage == 1, "unexpected handshake packet");
		bool bRet = OnReceivePassphrase((const unsigned char*)pPayload, nPayloadSize, nConnection);
		GAssert(bRet, "error processing passphrase");
	}
	memset(pBuf, '\0',nSize);
	delete(pBuf);
}

unsigned char* GSecureSocketServer::GetNextMessage(int* pnSize, int* pnOutConnectionNumber)
{
	// Get the encrypted message
	if(GetMessageCount() <= 0)
	{
		*pnOutConnectionNumber = -1;
		*pnSize = 0;
		return NULL;
	}
	unsigned char* pBuf = GEZSocketServer::GetNextMessage(pnSize, pnOutConnectionNumber);
	const unsigned char* pPassphrase = (const unsigned char*)m_pPassphrases->GetPointer(*pnOutConnectionNumber);
	GAssert(pPassphrase, "should have passphrase by now");

	// Decrypt the message
	GCrypto::Decrypt(pBuf, *pnSize, pPassphrase, m_nPassphraseSize);

	return pBuf;
}

bool GSecureSocketServer::SendPublicKey(int nConnection)
{
	bool bRet = true;
	GXMLTag* pTag = m_pKeyPair->ToXML(false);
	char* pMyPublicKey = pTag->ToString();
	int nPublicKeyBlobSize = strlen(pMyPublicKey);
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)alloca(sizeof(struct HandshakeHeader) + nPublicKeyBlobSize);
	memcpy(((unsigned char*)pHeader) + sizeof(struct HandshakeHeader), pMyPublicKey, nPublicKeyBlobSize);
	pHeader->nMagic = HANDSHAKE_MAGIC;
	pHeader->nMessage = 0;
	if(!GEZSocketServer::Send(pHeader, sizeof(struct HandshakeHeader) + nPublicKeyBlobSize, nConnection))
		bRet = false;
	delete(pMyPublicKey);
	delete(pTag);
	return bRet;
}

bool GSecureSocketServer::SendPassphrase(int nConnection, GKeyPair* pKeyPair)
{
	bool bRet = true;

	// Make a passphrase for this client
	unsigned char* pPassphrase = new unsigned char[m_nPassphraseSize];
	memcpy(pPassphrase, m_pRand->GetRand(), m_nPassphraseSize);
	if(pPassphrase[m_nPassphraseSize - 1] == 0)
		pPassphrase[m_nPassphraseSize - 1] = 1;
	m_pPassphrases->SetPointer(nConnection, pPassphrase);

	// Encrypt my passphrase with client's public key
	int nCypherSize;
	unsigned char* pCypher = pKeyPair->PowerMod(pPassphrase, m_nPassphraseSize, true, &nCypherSize);

	// Send it to the client
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)alloca(sizeof(struct HandshakeHeader) + nCypherSize);
	memcpy(((unsigned char*)pHeader) + sizeof(struct HandshakeHeader), pCypher, nCypherSize);
	pHeader->nMagic = HANDSHAKE_MAGIC;
	pHeader->nMessage = 1;
	if(!GEZSocketServer::Send(pHeader, sizeof(struct HandshakeHeader) + nCypherSize, nConnection))
	{
		GAssert(false, "Error sending passphrase");
		bRet = false;
	}
	delete(pCypher);
	return bRet;
}

bool GSecureSocketServer::OnReceivePublicKey(const char* pXML, int nXMLSize, int nConnectionNumber)
{
	// Decode client's public key
	GKeyPair kp;
	if(!kp.FromXML(pXML, nXMLSize))
	{
		GAssert(false, "Bad keypair");
		return false;
	}

	// Send my public key
	if(!SendPublicKey(nConnectionNumber))
	{
		GAssert(false, "Error sending my public key");
	}

	// Send my passphrase for this client
	if(!SendPassphrase(nConnectionNumber, &kp))
	{
		GAssert(false, "Error sending passphrase");
	}
	
	return true;
}

bool GSecureSocketServer::OnReceivePassphrase(const unsigned char* pCypher, int nCypherSize, int nConnectionNumber)
{
	// Decrypt it with my private key
	int nPassphraseSize;
	unsigned char* pPassphrase = m_pKeyPair->PowerMod(pCypher, nCypherSize, false, &nPassphraseSize);
	m_pClientPassphrases->SetPointer(nConnectionNumber, pPassphrase);
	m_nClientPassphraseSizes->SetInt(nConnectionNumber, nPassphraseSize);
	return true;
}

bool GSecureSocketServer::Send(const void* pBuf, int nLen, int nConnectionNumber)
{
	if(m_pClientPassphrases->GetSize() <= nConnectionNumber)
	{
		GAssert(false, "not done handshaking yet");
		return false;
	}

	// Encrypt the message with the client's passphrase
	unsigned char* pClientPassphrase = (unsigned char*)m_pClientPassphrases->GetPointer(nConnectionNumber);
	if(!pClientPassphrase)
	{
		GAssert(false, "not done handshaking yet");
		return false;
	}
	int nClientPassphraseSize = m_nClientPassphraseSizes->GetInt(nConnectionNumber);
	unsigned char* pCypher = (unsigned char*)alloca(nLen);
	memcpy(pCypher, pBuf, nLen);
	GCrypto::Encrypt(pCypher, nLen, pClientPassphrase, nClientPassphraseSize);
	
	// Send it
	return GEZSocketServer::Send(pCypher, nLen, nConnectionNumber);
}

// --------------------------------------------------------------------------

GSecureSocketClient::GSecureSocketClient(int nMaxPacketSize, GRand* pRand) : GEZSocketClient(nMaxPacketSize)
{
	m_pRand = pRand;
	m_pKeyPair = new GKeyPair();
	m_pKeyPair->GenerateKeyPair(pRand);
	m_nPassphraseSize = pRand->GetRandByteCount();
	m_pPassphrase = new unsigned char[m_nPassphraseSize];
	memcpy(m_pPassphrase, pRand->GetRand(), m_nPassphraseSize);
	if(m_pPassphrase[m_nPassphraseSize - 1] == 0)
		m_pPassphrase[m_nPassphraseSize - 1] = 1;
	m_pServerPassphrase = NULL;
	m_nServerPassphraseSize = 0;
}

GSecureSocketClient::~GSecureSocketClient()
{
	delete(m_pPassphrase);
	delete(m_pServerPassphrase);
	delete(m_pKeyPair);
}

/*static*/ GSecureSocketClient* GSecureSocketClient::ConnectToSecureSocket(const char* szAddress, u_short nPort, int nMaxPacketSize, GRand* pRand)
{
	// Check input
	int nPassphraseSize = pRand->GetRandByteCount();
	GAssert(nPassphraseSize >= (int)sizeof(int), "pRand not seeded big enough");
	GAssert(nMaxPacketSize > 0, "Bad max packet size");

	// Make the socket
	GSecureSocketClient* pSocket = new GSecureSocketClient(nMaxPacketSize, pRand);
	if(!pSocket)
		return NULL;
	if(!pSocket->Init(false, false))
	{
		GAssert(false, "error initializing socket");
		delete(pSocket);
		return NULL;
	}
	if(!pSocket->Connect(szAddress, nPort))
	{
		delete(pSocket);
		return NULL;
	}

	// Handshake
	if(!pSocket->SendPublicKey())
	{
		GAssert(false, "error sending public key");
		delete(pSocket);
		return NULL;
	}
	int nTimeout = 200;
	while(!pSocket->HandShake())
	{
		if(--nTimeout <= 0)
		{
			delete(pSocket);
			return NULL;
		}
#ifdef WIN32
		GWindows::YieldToWindows();
		Sleep(50);
#else
		usleep(50);
#endif // !WIN32
	}

	return pSocket;
}

bool GSecureSocketClient::HandShake()
{
	// See if we already did handshaking
	if(m_pServerPassphrase)
		return true;

	// Make sure there's a message to process
	if(m_pMessageQueue->GetSize() < 1)
		return false;

	// Check the header
	int nSize;
	unsigned char* pBuf = GEZSocketClient::GetNextMessage(&nSize);
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)pBuf;
	if(nSize <= (int)sizeof(struct HandshakeHeader) || pHeader->nMagic != HANDSHAKE_MAGIC)
	{
		GAssert(false, "Bad header packet");
		memset(pBuf, '\0', nSize);
		delete(pBuf);
		return false;
	}

	// Process the payload
	unsigned char* pPayload = pBuf + sizeof(struct HandshakeHeader);
	int nPayloadSize = nSize - sizeof(struct HandshakeHeader);
	if(pHeader->nMessage == 0)
	{
		bool bRet = OnReceivePublicKey((const char*)pPayload, nPayloadSize);
		GAssert(bRet, "error processing public key");
	}
	else
	{
		GAssert(pHeader->nMessage == 1, "unexpected handshake packet");
		bool bRet = OnReceivePassphrase((const unsigned char*)pPayload, nPayloadSize);
		GAssert(bRet, "error processing passphrase");
	}
	memset(pBuf, '\0', nSize);
	delete(pBuf);
	if(m_pServerPassphrase)
		return true;
	else
		return false;
}

bool GSecureSocketClient::SendPublicKey()
{
	bool bRet = true;
	GXMLTag* pTag = m_pKeyPair->ToXML(false);
	char* pMyPublicKey = pTag->ToString();
	int nPublicKeyBlobSize = strlen(pMyPublicKey);
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)alloca(sizeof(struct HandshakeHeader) + nPublicKeyBlobSize);
	memcpy(((unsigned char*)pHeader) + sizeof(struct HandshakeHeader), pMyPublicKey, nPublicKeyBlobSize);
	pHeader->nMagic = HANDSHAKE_MAGIC;
	pHeader->nMessage = 0;
	if(!GEZSocketClient::Send(pHeader, sizeof(struct HandshakeHeader) + nPublicKeyBlobSize))
		bRet = false;
	delete(pMyPublicKey);
	delete(pTag);
	return bRet;
}

bool GSecureSocketClient::SendPassphrase(GKeyPair* pKeyPair)
{
	bool bRet = true;

	// Encrypt my passphrase with client's public key
	int nCypherSize;
	unsigned char* pCypher = pKeyPair->PowerMod(m_pPassphrase, m_nPassphraseSize, true, &nCypherSize);

	// Send it to the client
	struct HandshakeHeader* pHeader = (struct HandshakeHeader*)alloca(sizeof(struct HandshakeHeader) + nCypherSize);
	memcpy(((unsigned char*)pHeader) + sizeof(struct HandshakeHeader), pCypher, nCypherSize);
	pHeader->nMagic = HANDSHAKE_MAGIC;
	pHeader->nMessage = 1;
	if(!GEZSocketClient::Send(pHeader, sizeof(struct HandshakeHeader) + nCypherSize))
	{
		GAssert(false, "Error sending passphrase");
		bRet = false;
	}
	delete(pCypher);
	return bRet;
}

bool GSecureSocketClient::OnReceivePublicKey(const char* pXML, int nXMLSize)
{
	// Decode the server's public key
	GKeyPair kp;
	if(!kp.FromXML(pXML, nXMLSize))
	{
		GAssert(false, "Bad keypair");
		return false;
	}

	// Send my passphrase
	if(!SendPassphrase(&kp))
	{
		GAssert(false, "Error sending passphrase");
	}
	return true;
}

bool GSecureSocketClient::OnReceivePassphrase(const unsigned char* pCypher, int nCypherSize)
{
	// Decrypt it with my private key
	m_pServerPassphrase = m_pKeyPair->PowerMod(pCypher, nCypherSize, false, &m_nServerPassphraseSize);
	return true;
}

bool GSecureSocketClient::Send(const void* pBuf, int nLen)
{
	if(!m_pServerPassphrase)
	{
		GAssert(false, "not done handshaking yet");
		return false;
	}

	// Encrypt the message with the server's passphrase
	unsigned char* pCypher = (unsigned char*)alloca(nLen);
	memcpy(pCypher, pBuf, nLen);
	GCrypto::Encrypt(pCypher, nLen, m_pServerPassphrase, m_nServerPassphraseSize);

	// Send it
	return GEZSocketClient::Send(pCypher, nLen);
}

int GSecureSocketClient::GetMessageCount()
{
	return GEZSocketClient::GetMessageCount();
}

unsigned char* GSecureSocketClient::GetNextMessage(int* pnSize)
{
	// Get the encrypted message
	unsigned char* pBuf = GEZSocketClient::GetNextMessage(pnSize);
	if(!pBuf)
		return NULL;

	// Decrypt the message
	GCrypto::Decrypt(pBuf, *pnSize, m_pPassphrase, m_nPassphraseSize);

	return pBuf;
}
