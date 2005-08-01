/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __NREALMPROTOCOL_H__
#define __NREALMPROTOCOL_H__

// ------------------------------------
//
//  The classes in this file specify the protocol that the client and server
//  use to communicate with each other
//
// ------------------------------------

#define MAX_PACKET_SIZE 1024
#define DEFAULT_PORT 4748

class GEZSocketServer;
class GEZSocketClient;
class GPointerQueue;
class MObject;
class MGameClient;
class MGameServer;
class MScriptEngine;


// This is the base class of all communication packets for this protocol
class NRealmPacket
{
public:
	enum RealmPacketType
	{
		SET_PATH,
		SEND_ME_UPDATES,
		UPDATE_OBJECT,
	};

protected:
	int m_nConnection;

public:
	NRealmPacket();
	virtual ~NRealmPacket();

	static NRealmPacket* Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize);
	int SerializePacket(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
	virtual RealmPacketType GetPacketType() = 0;
	void SetConnection(int n) { m_nConnection = n; }
	int GetConnection() { return m_nConnection; }

protected:
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize) = 0;
};



// This packet specifies the path to the .realm file to which you want to connect
class NSetPathPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	char* m_szPath;

public:
	NSetPathPacket(const char* szPath);
	NSetPathPacket(const char* pPath, int nLen);
	virtual ~NSetPathPacket();

	virtual RealmPacketType GetPacketType() { return SET_PATH; }
	const char* GetPath() { return m_szPath; }

protected:
	static NSetPathPacket* Deserialize(const unsigned char* pData, int nSize);
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
};



// This packet essentially says "Please send me updated objects for anything that has
// changed since some timestamp".  The client sends this packet periodically to the
// server in order to request the latest info.
class NSendMeUpdatesPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	double m_time;

public:
	NSendMeUpdatesPacket();
	virtual ~NSendMeUpdatesPacket();

	virtual RealmPacketType GetPacketType() { return SEND_ME_UPDATES; }
	void SetTime(double time) { m_time = time; }
	double GetTime() { return m_time; }

protected:
	static NSendMeUpdatesPacket* Deserialize(const unsigned char* pData, int nSize);
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
};





// This packet basically contains a serialized MObject, and it says "Here's the latest
// info I've got about this object".  The client should send these to the server whenever
// it (the client) makes changes to an object (like if the avatar changes directions).
// The server will also send these to the clients on request.
class NUpdateObjectPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	MObject* m_pObject;

public:
	NUpdateObjectPacket();
	virtual ~NUpdateObjectPacket();

	virtual RealmPacketType GetPacketType() { return UPDATE_OBJECT; }
	MObject* GetMObject() { return m_pObject; }
	void SetObject(MObject* pObject) { m_pObject = pObject; }

protected:
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
	static NUpdateObjectPacket* Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize);
};





// This class wraps the server's socket connection.  It provides a channel through which the server
// waits for new connections from new clients, and communicates with them when the connect.
class NRealmServerConnection
{
protected:
	GEZSocketServer* m_pSocket;
	GPointerQueue* m_pQueue;
	unsigned char m_pBuf[MAX_PACKET_SIZE];
	MGameServer* m_pServer;

public:
	NRealmServerConnection(MGameServer* pServer);
	~NRealmServerConnection();

	NRealmPacket* GetNextPacket();
	void SendPacket(NRealmPacket* pPacket, int nConnection);
	void ReportBadPacket(int nConnection);
};





// This class wraps the client's socket connection.  It provides a channel through which the
// client connects with the server and communicates with it.
class NRealmClientConnection
{
protected:
	GEZSocketClient* m_pSocket;
	GPointerQueue* m_pQueue;
	unsigned char m_pBuf[MAX_PACKET_SIZE];
	MGameClient* m_pGameClient;

public:
	NRealmClientConnection(MGameClient* pGameClient, const char* szServerName);
	~NRealmClientConnection();

	NRealmPacket* GetNextPacket();
	void SendPacket(NRealmPacket* pPacket);
};

#endif // __NREALMPROTOCOL_H__
