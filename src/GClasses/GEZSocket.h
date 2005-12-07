/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GEZSOCKET_H__
#define __GEZSOCKET_H__

#include "GSocket.h"

// This class is designed to make network communication really easy
class GEZSocketServer : public GSocket
{
protected:
	GPointerArray* m_pBuffers;
	int m_nMaxPacketSize;
	GPointerQueue* m_pMessageQueue;
	GSpinLock* m_pMessageQueueLock;

	virtual bool Receive(unsigned char *pBuf, int len, int nConnectionNumber);
	void QueueMessage(unsigned char* pBuf, int nLen, int nConnectionNumber);

	GEZSocketServer(int nMaxPacketSize);

public:
	// Host a TCP socket
	static GEZSocketServer* HostTCPSocket(int nPort);

	// Host a UDP socket
	static GEZSocketServer* HostUDPSocket(int nPort);

	// Host a Gash socket.  (Adds a layer on top of TCP to guarantee same-size packet delivery)
	static GEZSocketServer* HostGashSocket(int nPort, int nMaxPacketSize);

	virtual ~GEZSocketServer();

	// Send some data
	bool Send(const void* pBuf, int nLen, int nConnectionNumber);

	// Returns the number of messages waiting to be received
	int GetMessageCount();

	// Receive the next message. (You are responsible to delete the buffer this returns)
	unsigned char* GetNextMessage(int* pnSize, int* pnOutConnectionNumber);
};

// --------------------------------------------------------------------------

// This class is designed to make network communication really easy
class GEZSocketClient : public GSocket
{
protected:
	unsigned char* m_pBuffer;
	int m_nBufferPos;
	int m_nMaxPacketSize;
	GPointerQueue* m_pMessageQueue;
	GSpinLock* m_pMessageQueueLock;

	virtual bool Receive(unsigned char *pBuf, int len, int nConnectionNumber);
	void QueueMessage(unsigned char* pBuf, int nLen);

	GEZSocketClient(int nMaxPacketSize);

public:
	// Connect to a TCP socket
	static GEZSocketClient* ConnectToTCPSocket(const char* szAddress, int nPort);

	// Connect to a UDP socket
	static GEZSocketClient* ConnectToUDPSocket(const char* szAddress, int nPort);

	// Connect to a Gash socket.  (Adds a layer on top of TCP to guarantee same-size packet delivery)
	static GEZSocketClient* ConnectToGashSocket(const char* szAddress, int nPort, int nMaxPacketSize);

	virtual ~GEZSocketClient();

	// Send some data
	bool Send(const void* pBuf, int nLen);

	// Returns the number of messages waiting to be received
	int GetMessageCount();

	// Receive the next message. (You are responsible to delete the buffer this returns)
	unsigned char* GetNextMessage(int* pnSize);
};

// --------------------------------------------------------------------------

class GSecureSocketServer : public GEZSocketServer
{
protected:
	GKeyPair* m_pKeyPair;
	GRand* m_pRand;
	GPointerArray* m_pPassphrases;
	int m_nPassphraseSize;
	GPointerArray* m_pClientPassphrases;
	GIntArray* m_nClientPassphraseSizes;

	GSecureSocketServer(int nMaxPacketSize, GRand* pRand);

public:
	// nRandomDataSize should be cryptographic random data four times the size you want your keys to be
	static GSecureSocketServer* HostSecureSocket(u_short nPort, int nMaxPacketSize, GRand* pRand);
	virtual ~GSecureSocketServer();

	// Send some data
	bool Send(const void* pBuf, int nLen, int nConnectionNumber);

	// Returns the number of messages waiting to be received
	// Note: you should call this method regularly.  If you don't, client's won't be
	// able to handshake with this socket.
	int GetMessageCount();
	
	// Receive the next message. (You are responsible to delete the buffer this returns)
	unsigned char* GetNextMessage(int* pnSize, int* pnOutConnectionNumber);

protected:
	void Handshake();
	bool SendPublicKey(int nConnection);
	bool SendPassphrase(int nConnection, GKeyPair* pKeyPair);
	bool OnReceivePublicKey(const char* pXML, int nXMLSize, int nConnectionNumber);
	bool OnReceivePassphrase(const unsigned char* pCypher, int nCypherSize, int nConnectionNumber);
};

// --------------------------------------------------------------------------

class GSecureSocketClient : public GEZSocketClient
{
protected:
	GKeyPair* m_pKeyPair;
	GRand* m_pRand;
	unsigned char* m_pPassphrase;
	int m_nPassphraseSize;
	unsigned char* m_pServerPassphrase;
	int m_nServerPassphraseSize;

	GSecureSocketClient(int nMaxPacketSize, GRand* pRand);

public:
	// nRandomDataSize should be cryptographic random data four times the size you want your keys to be
	static GSecureSocketClient* ConnectToSecureSocket(const char* szAddress, u_short nPort, int nMaxPacketSize, GRand* pRand);
	virtual ~GSecureSocketClient();

	// Send some data
	bool Send(const void* pBuf, int nLen);

	// Returns the number of messages waiting to be received
	int GetMessageCount();

	// Receive the next message. (You are responsible to delete the buffer this returns)
	unsigned char* GetNextMessage(int* pnSize);

protected:
	bool HandShake();
	bool SendPublicKey();
	bool SendPassphrase(GKeyPair* pKeyPair);
	bool OnReceivePublicKey(const char* pXML, int nXMLSize);
	bool OnReceivePassphrase(const unsigned char* pCypher, int nCypherSize);
};

#endif // __GEZSOCKET_H__
