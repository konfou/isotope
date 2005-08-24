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

#define SERVER_LOAD_CHECKS 50

class View;
class Controller;
class MScriptEngine;
class IsotopeErrorHandler;
class GPointerArray;
class MGameServer;
struct MClientRecord;

// An MRealmServer is a daemon for a single realm
class MRealmServer
{
protected:
	char* m_szPath;
	char* m_szBase;
	MRealm* m_pRealm;
	double m_dLatestSentUpdates;
	MScriptEngine* m_pScriptEngine;
	IsotopeErrorHandler* m_pErrorHandler;

	MRealmServer(const char* szPath, MGameServer* pGameServer);
public:
	~MRealmServer();

	static MRealmServer* LoadRealm(const char* szFilename, MGameServer* pGameServer);
	const char* GetPath() { return m_szPath; }
	void SendUpdates(NSendMeUpdatesPacket* pPacketIn, NRealmServerConnection* pConnection, double time);
	void UpdateObject(NUpdateObjectPacket* pPacket, double time, MClientRecord* pRecord);
	MScriptEngine* GetScriptEngine() { return m_pScriptEngine; }

protected:
	void LoadScript(const char* szFilename, GXMLTag* pObjectsTag);
};

struct MClientRecord
{
	MRealmServer* pRealmServer;
	double dTimeDelta;
};

// This class represents a collection of MRealmServer objects.  This is the model when the application
// is running in server mode.
class MGameServer : public Model
{
friend class MRealmServer;
protected:
	NRealmServerConnection* m_pConnection;
	int m_nLoadChecks[SERVER_LOAD_CHECKS];
	int m_nLoadCheckPos;
	GPointerArray* m_pRealmServers;
	GPointerArray* m_pClients;
	char* m_szBasePath;
	int m_nBasePathLen;

public:
	MGameServer(const char* szBasePath);
	virtual ~MGameServer();

	virtual void Update(double time);
	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew);
	virtual ModelType GetType() { return Server; }
	float MeasureLoad();
	MRealmServer* GetRealmServer(int nConnection);

protected:
	MClientRecord* GetClientRecord(int n, bool bCreateIfNotFound);
	void ProcessPacket(NRealmPacket* pPacket, double time);
	void ConnectToRealm(NSetPathPacket* pPacketIn);
	MRealmServer* FindOrLoadRealmServer(const char* szUrl);
	void SendUpdates(NSendMeUpdatesPacket* pPacket, double time);
	void UpdateObject(NUpdateObjectPacket* pPacket, double time);
};

#endif // __MREALMSERVER_H__
