/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "NRealmProtocol.h"
#include "../GClasses/GEZSocket.h"
#include "../GClasses/GPointerQueue.h"
#include "GameEngine.h"
#include "MObject.h"
#include "MScriptEngine.h"
#include "MGameClient.h"
#include "MRealmServer.h"

NRealmPacket::NRealmPacket()
{
}

NRealmPacket::~NRealmPacket()
{
}

int NRealmPacket::SerializePacket(MScriptEngine* pScriptEngine, unsigned char* pBuf, int nBufSize)
{
	unsigned char cPacketType = (unsigned char)GetPacketType();
	*pBuf = cPacketType;
	return Serialize(pScriptEngine, pBuf + 1, nBufSize - 1) + 1;
}

/*static*/ NRealmPacket* NRealmPacket::Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize)
{
	const unsigned char cPacketType = *pData;
	pData++;
	nSize--;
	NRealmPacket* pPacket = NULL;
	switch(cPacketType)
	{
		case SET_PATH:
			pPacket = NSetPathPacket::Deserialize(pData, nSize);
			break;

		case SEND_ME_UPDATES:
			pPacket = NSendMeUpdatesPacket::Deserialize(pData, nSize);
			break;

		case UPDATE_OBJECT:
			pPacket = NUpdateObjectPacket::Deserialize(pScriptEngine, pData, nSize);
			break;

		case SEND_OBJECT:
			pPacket = NSendObjectPacket::Deserialize(pScriptEngine, pData, nSize);
			break;

		default:
			GAssert(false, "Unexpected packet type");
	}
	return pPacket;
}

// -----------------------------------------------------------

NSetPathPacket::NSetPathPacket(const char* szPath)
: NRealmPacket()
{
	m_szPath = new char[strlen(szPath) + 1];
	strcpy(m_szPath, szPath);
}

NSetPathPacket::NSetPathPacket(const char* pPath, int nLen)
: NRealmPacket()
{
	m_szPath = new char[nLen + 1];
	memcpy(m_szPath, pPath, nLen);
	m_szPath[nLen] = '\0';
}

/*virtual*/ NSetPathPacket::~NSetPathPacket()
{
	delete(m_szPath);
}

/*virtual*/ int NSetPathPacket::Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize)
{
	int nLen = strlen(m_szPath);
	if(nLen > nBufferSize)
		GameEngine::ThrowError("Buffer too small");
	memcpy(pBuffer, m_szPath, nLen);
	return nLen;
}

/*static*/ NSetPathPacket* NSetPathPacket::Deserialize(const unsigned char* pData, int nSize)
{
	return new NSetPathPacket((const char*)pData, nSize);
}

// -----------------------------------------------------------

NSendMeUpdatesPacket::NSendMeUpdatesPacket()
: NRealmPacket()
{
	m_time = 0;
}

/*virtual*/ NSendMeUpdatesPacket::~NSendMeUpdatesPacket()
{
}

/*virtual*/ int NSendMeUpdatesPacket::Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuf, int nBufSize)
{
	*(double*)pBuf = m_time;
	return sizeof(double);
}

/*static*/ NSendMeUpdatesPacket* NSendMeUpdatesPacket::Deserialize(const unsigned char* pData, int nSize)
{
	if(nSize != sizeof(double))
	{
		GAssert(false, "bad packet");
		return NULL;
	}
	NSendMeUpdatesPacket* pNewPacket = new NSendMeUpdatesPacket();
	pNewPacket->m_time = *(double*)pData;
	return pNewPacket;
}

// -----------------------------------------------------------

NUpdateObjectPacket::NUpdateObjectPacket()
: NRealmPacket()
{
	m_pObject = NULL;
}

/*virtual*/ NUpdateObjectPacket::~NUpdateObjectPacket()
{
	delete(m_pObject);
}

/*virtual*/ int NUpdateObjectPacket::Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuf, int nBufSize)
{
	if(!pScriptEngine)
		GameEngine::ThrowError("No realm loaded yet");
	GAssert(m_pObject, "You must set the object before you serialize");
	return pScriptEngine->SerializeObject(m_pObject->GetGObject(), pBuf, nBufSize);
}

/*static*/ NUpdateObjectPacket* NUpdateObjectPacket::Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize)
{
	if(!pScriptEngine)
		GameEngine::ThrowError("No realm loaded yet");
	GObject* pObj = pScriptEngine->DeserializeObject(pData, nSize);
	if(!pObj)
		return NULL;
	MObject* pObject = new MObject(pScriptEngine);
	pObject->SetGObject(pObj);
	NUpdateObjectPacket* pNewPacket = new NUpdateObjectPacket();
	pNewPacket->SetObject(pObject);
	return pNewPacket;
}

// -----------------------------------------------------------

NSendObjectPacket::NSendObjectPacket(Engine* pEngine)
: NRealmPacket()
{
	m_pVH = new VarHolder(pEngine);
}

/*virtual*/ NSendObjectPacket::~NSendObjectPacket()
{
	delete(m_pVH);
}

GObject* NSendObjectPacket::GetGObject()
{
	return m_pVH->GetGObject();
}

void NSendObjectPacket::SetObject(GObject* pObject)
{
	m_pVH->SetGObject(pObject);
}

/*virtual*/ int NSendObjectPacket::Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuf, int nBufSize)
{
	if(!pScriptEngine)
		GameEngine::ThrowError("No realm loaded yet");
	GObject* pOb = GetGObject();
	GAssert(pOb, "You must set the object before you serialize");
	return pScriptEngine->SerializeObject(pOb, pBuf, nBufSize);
}

/*static*/ NSendObjectPacket* NSendObjectPacket::Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize)
{
	if(!pScriptEngine)
		GameEngine::ThrowError("No realm loaded yet");
	GObject* pObj = pScriptEngine->DeserializeObject(pData, nSize);
	if(!pObj)
	{
		GAssert(false, "failed to deserialize sent object");
		return NULL;
	}
	NSendObjectPacket* pNewPacket = new NSendObjectPacket(pScriptEngine->GetEngine());
	pNewPacket->SetObject(pObj);
	return pNewPacket;
}

// -----------------------------------------------------------

NRealmServerConnection::NRealmServerConnection(MGameServer* pServer)
{
	m_pSocket = GEZSocketServer::HostGashSocket(DEFAULT_PORT, MAX_PACKET_SIZE);
	if(!m_pSocket)
		GameEngine::ThrowError("Failed to create server socket");
	m_pQueue = new GPointerQueue();
	m_pServer = pServer;
}

NRealmServerConnection::~NRealmServerConnection()
{
	delete(m_pSocket);
	while(m_pQueue->GetSize() > 0)
		delete((NRealmPacket*)m_pQueue->Pop());
	delete(m_pQueue);
}

NRealmPacket* NRealmServerConnection::GetNextPacket()
{
	while(true)
	{
		int nSize;
		int nConnection;
		unsigned char* pData = m_pSocket->GetNextMessage(&nSize, &nConnection);
		if(!pData)
			break;
		NRealmPacket* pNewPacket = NULL;
		MRealmServer* pRS = m_pServer->GetRealmServer(nConnection);
		MScriptEngine* pScriptEngine = pRS ? pRS->GetScriptEngine() : NULL;

		// In RETAIL mode we want to catch any exceptions here to prevent bad/malicious packets
		// from taking down the server.  In DEBUG mode we don't want a try/catch block to hide
		// errors because no exceptions are expected while deserializing a packet (as long as
		// the packet is properly formed, which it should always be).
#ifndef _DEBUG
		try
		{
#endif // !_DEBUG
			pNewPacket = NRealmPacket::Deserialize(pScriptEngine, pData, nSize);
#ifndef _DEBUG
		}
		catch(...)
		{
			// there was a bad packet
		}
#endif // !_DEBUG

		if(pNewPacket)
		{
			pNewPacket->SetConnection(nConnection);
			m_pQueue->Push(pNewPacket);
		}
		else
			ReportBadPacket(nConnection);
		delete(pData);
	}
	if(m_pQueue->GetSize() <= 0)
		return NULL;
	return (NRealmPacket*)m_pQueue->Pop();
}

void NRealmServerConnection::ReportBadPacket(int nConnection)
{
	fprintf(stderr, "Error! Received a bad packet!\n");
	GAssert(false, "bad packet");
	// todo: disconnect the client as punishment for sending a bad packet
}

void NRealmServerConnection::SendPacket(NRealmPacket* pPacket, int nConnection)
{
	MRealmServer* pRS = m_pServer->GetRealmServer(nConnection);
	MScriptEngine* pScriptEngine = pRS ? pRS->GetScriptEngine() : NULL;
	int nLen;
	// todo: catch exceptions in next line
#ifndef _DEBUG
	try
	{
#endif // !_DEBUG
		nLen = pPacket->SerializePacket(pScriptEngine, m_pBuf, MAX_PACKET_SIZE);
#ifndef _DEBUG
	}
	catch(...)
	{
		// there was a bad packet
		// todo: log this or print some message
		return;
	}
#endif // !_DEBUG
	m_pSocket->Send(m_pBuf, nLen, nConnection);
}

// -----------------------------------------------------------

NRealmClientConnection::NRealmClientConnection(MGameClient* pGameClient, const char* szServerName)
{
	m_pGameClient = pGameClient;
	m_pSocket = GEZSocketClient::ConnectToGashSocket(szServerName, DEFAULT_PORT, MAX_PACKET_SIZE);
	if(!m_pSocket)
		GameEngine::ThrowError("Failed to connect to server: %s", szServerName);
	m_pQueue = new GPointerQueue();

}

NRealmClientConnection::~NRealmClientConnection()
{
	delete(m_pSocket);
	while(m_pQueue->GetSize() > 0)
		delete((NRealmPacket*)m_pQueue->Pop());
	delete(m_pQueue);
}

NRealmPacket* NRealmClientConnection::GetNextPacket()
{
	while(true)
	{
		int nSize;
		unsigned char* pData = m_pSocket->GetNextMessage(&nSize);
		if(!pData)
			break;
		NRealmPacket* pNewPacket = NRealmPacket::Deserialize(m_pGameClient->GetScriptEngine(), pData, nSize);
		delete(pData);
		if(pNewPacket)
			m_pQueue->Push(pNewPacket);
		else
			GameEngine::ThrowError("Got a bad packet from the server!");
	}
	if(m_pQueue->GetSize() <= 0)
		return NULL;
	return (NRealmPacket*)m_pQueue->Pop();
}

void NRealmClientConnection::SendPacket(NRealmPacket* pPacket)
{
	int nLen = pPacket->SerializePacket(m_pGameClient->GetScriptEngine(), m_pBuf, MAX_PACKET_SIZE);
	GAssert(nLen > 0, "Looks like a bad packet");
	m_pSocket->Send(m_pBuf, nLen);
}
