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

#define MAX_PACKET_SIZE 16384
#define DEFAULT_PORT 4748
#define UPDATE_REQUEST_RATE .2 // the rate (in seconds) at which clients are allowed to request updates

class GEZSocketServer;
class GEZSocketClient;
class GPointerQueue;
class MObject;
class MGameClient;
class MGameServer;
class MScriptEngine;
class VarHolder;
class Engine;
class GObject;
class NRealmClientSocket;
class NRealmServerSocket;

// This is the base class of all communication packets for this protocol. WARNING: be very careful
// when writing the deserialize method for your new packet or else you may open a buffer overrun
// security hole on the server!
class NRealmPacket
{
public:
	enum RealmPacketType
	{
		SET_PATH,
		SEND_ME_UPDATES,
		UPDATE_REALM_OBJECT,
		SEND_OBJECT,
		REMOVE_REALM_OBJECT,
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

public:
	NSendMeUpdatesPacket();
	virtual ~NSendMeUpdatesPacket();

	virtual RealmPacketType GetPacketType() { return SEND_ME_UPDATES; }

protected:
	static NSendMeUpdatesPacket* Deserialize(const unsigned char* pData, int nSize);
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
};




// This packet tells the server to remove a particular object from the realm
class NRemoveRealmObjectPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	unsigned int m_uid;

public:
	NRemoveRealmObjectPacket();
	virtual ~NRemoveRealmObjectPacket();

	virtual RealmPacketType GetPacketType() { return REMOVE_REALM_OBJECT; }
	void SetUid(unsigned int uid) { m_uid = uid; }
	unsigned int GetUid() { return m_uid; }

protected:
	static NRemoveRealmObjectPacket* Deserialize(const unsigned char* pData, int nSize);
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
};




// This packet basically contains a serialized MObject, and it says "Here's the latest
// info I've got about this object".  The client should send these to the server whenever
// it (the client) makes changes to an object (like if the avatar changes directions).
// The server will also send these to the clients on request.
class NUpdateRealmObjectPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	MObject* m_pObject;

public:
	NUpdateRealmObjectPacket();
	virtual ~NUpdateRealmObjectPacket();

	virtual RealmPacketType GetPacketType() { return UPDATE_REALM_OBJECT; }
	MObject* GetMObject() { return m_pObject; }
	void SetObject(MObject* pObject) { m_pObject = pObject; }

protected:
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
	static NUpdateRealmObjectPacket* Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize);
};



// This packet is used when either the client or server script wants to send a GObject to
// the other one
class NSendObjectPacket : public NRealmPacket
{
friend class NRealmPacket;
protected:
	VarHolder* m_pVH;

public:
	NSendObjectPacket(Engine* pEngine);
	virtual ~NSendObjectPacket();

	virtual RealmPacketType GetPacketType() { return SEND_OBJECT; }
	GObject* GetGObject();
	void SetObject(GObject* pObject);

protected:
	virtual int Serialize(MScriptEngine* pScriptEngine, unsigned char* pBuffer, int nBufferSize);
	static NSendObjectPacket* Deserialize(MScriptEngine* pScriptEngine, const unsigned char* pData, int nSize);
};




// This class wraps the server's socket connection.  It provides a channel through which the server
// waits for new connections from new clients, and communicates with them when they connect.
class NRealmServerConnection
{
protected:
	NRealmServerSocket* m_pSocket;
	GPointerQueue* m_pQueue;
	unsigned char m_pBuf[MAX_PACKET_SIZE];
	MGameServer* m_pServer;

public:
	NRealmServerConnection(MGameServer* pServer);
	~NRealmServerConnection();

	NRealmPacket* GetNextPacket();
	void SendPacket(NRealmPacket* pPacket, int nConnection);
	void ReportBadPacket(int nConnection);
	void OnCloseConnection(int nConnection);
	void Disconnect(int nConnection);
};







// This class wraps the client's socket connection.  It provides a channel through which the
// client connects with the server and communicates with it.
class NRealmClientConnection
{
protected:
	NRealmClientSocket* m_pSocket;
	GPointerQueue* m_pQueue;
	unsigned char m_pBuf[MAX_PACKET_SIZE];
	MGameClient* m_pGameClient;

public:
	NRealmClientConnection(MGameClient* pGameClient, const char* szServerName);
	~NRealmClientConnection();

	NRealmPacket* GetNextPacket();
	void SendPacket(NRealmPacket* pPacket);
	void OnCloseConnection();
};

#endif // __NREALMPROTOCOL_H__
