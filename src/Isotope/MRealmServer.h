/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MREALMSERVER_H__
#define __MREALMSERVER_H__

#include "Model.h"
#include <stdio.h>

#define SERVER_LOAD_CHECKS 50

class View;
class Controller;
class MScriptEngine;
class IsotopeErrorHandler;
class GPointerArray;
class MGameServer;
struct MClientRecord;
class VarHolder;
class NSendObjectPacket;
class GHttpClient;


// An MRealmServer is a daemon for a single realm
class MRealmServer
{
protected:
	char* m_szUrl;
	char* m_szBase;
	MRealm* m_pRealm;
	double m_dLatestSentUpdates;
	MScriptEngine* m_pScriptEngine;
	IsotopeErrorHandler* m_pErrorHandler;
	VarHolder* m_pRemoteVar;

	MRealmServer(const char* szUrl, MGameServer* pGameServer);
public:
	~MRealmServer();

	static MRealmServer* LoadRealm(const char* szUrl, MGameServer* pGameServer, Controller* pController);
	const char* GetUrl() { return m_szUrl; }
	MScriptEngine* GetScriptEngine() { return m_pScriptEngine; }
	void SendUpdates(int nConnection, NRealmServerConnection* pConnection, MClientRecord* pRecord, double time);
	void UpdateRealmObject(NUpdateRealmObjectPacket* pPacket, double time, MClientRecord* pRecord);
	void RemoveRealmObject(NRemoveRealmObjectPacket* pPacket, MClientRecord* pRecord);
	void ReceiveObject(NSendObjectPacket* pPacket, int nConnection);

protected:
	bool LoadScript(GHttpClient* pHttpClient, const char* szUrl, GXMLTag* pMapTag, Controller* pController, MRealm* pRealm);
};







struct MClientRecord
{
	MRealmServer* pRealmServer;
	double dTimeDelta; // The difference between the client's clock and the server's clock
	double dLastSentUpdatesTime; // This can be used to detect dead clients since it is updated whenever the client requests updates (whether or not there are any updates to send)

	MClientRecord()
	{
		pRealmServer = NULL;
		dLastSentUpdatesTime = 0;
	}
};








// This class represents a collection of MRealmServer objects.  This is the model
// when the application is running in server mode.
class MGameServer : public Model
{
friend class MRealmServer;
protected:
	Controller* m_pController;
	NRealmServerConnection* m_pConnection;
	int m_nLoadChecks[SERVER_LOAD_CHECKS];
	int m_nLoadCheckPos;
	GPointerArray* m_pRealmServers;
	GPointerArray* m_pClients;

public:
	MGameServer(Controller* pController);
	virtual ~MGameServer();

	virtual void Update(double time);
	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew);
	virtual ModelType GetType() { return Server; }
	virtual void SendObject(GObject* pObj, int nConnection);
	float MeasureLoad();
	MRealmServer* GetRealmServer(int nConnection);
	void OnLoseConnection(int nConnection);

protected:
	MClientRecord* GetClientRecord(int n, bool bCreateIfNotFound);
	void ProcessPacket(NRealmPacket* pPacket, double time);
	void ConnectToRealm(NSetPathPacket* pPacketIn);
	MRealmServer* FindOrLoadRealmServer(const char* szUrl);
	void SendUpdates(NSendMeUpdatesPacket* pPacket, double time);
	void UpdateRealmObject(NUpdateRealmObjectPacket* pPacket, double time);
	void ReceiveObject(NSendObjectPacket* pPacket);
	void RemoveRealmObject(NRemoveRealmObjectPacket* pPacket);
	void BootClient(int nConnection);
};

#endif // __MREALMSERVER_H__
