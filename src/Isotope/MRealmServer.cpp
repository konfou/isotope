/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MRealmServer.h"
#include "Main.h"
#include "NRealmProtocol.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GSocket.h"
#include "../GClasses/GFile.h"
#include "../GClasses/GHttp.h"
#include "../GClasses/GThread.h"
#include "MRealm.h"
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif // WIN32
#include "../Gasp/Include/GaspEngine.h"
#include "MScriptEngine.h"
#include "MObject.h"
#include "Controller.h"

MRealmServer::MRealmServer(const char* szUrl, MGameServer* pGameServer)
{
	int nLen = strlen(szUrl);
	m_szUrl = new char[nLen + 1];
	strcpy(m_szUrl, szUrl);
	m_szBase = new char[nLen + 1];
	strcpy(m_szBase, szUrl);
	int n;
	for(n = nLen - 1; n >= 0 && m_szBase[n] != '/' && m_szBase[n] != '\\'; n--)
		m_szBase[n] = '\0';
	m_pRealm = new MRealm(pGameServer);
	m_dLatestSentUpdates = 0;
	m_pScriptEngine = NULL;
	m_pErrorHandler = new IsotopeErrorHandler();
	m_pRemoteVar = NULL;
}

MRealmServer::~MRealmServer()
{
	delete(m_pRealm);
	delete(m_szUrl);
	delete(m_szBase);
	delete(m_pRemoteVar);
	delete(m_pScriptEngine);
	delete(m_pErrorHandler);
}

bool MRealmServer::LoadScript(GHttpClient* pHttpClient, const char* szUrl, GXMLTag* pMapTag, Controller* pController, MRealm* pRealm)
{
	int nBufSize;
	char* pFile = Controller::DownloadFile(pHttpClient, szUrl, &nBufSize, true, 30, NULL, NULL);
	if(!pFile)
	{
		fprintf(stderr, "*** Failed to load script file: %s\n", szUrl);
		return false;
	}
	Holder<char*> hFile(pFile);
	m_pScriptEngine = new MScriptEngine(szUrl, pFile, nBufSize, m_pErrorHandler, pMapTag, NULL, pController, pRealm);
	return true;
}

/*static*/ MRealmServer* MRealmServer::LoadRealm(const char* szUrl, MGameServer* pGameServer, Controller* pController)
{
	// Download the file
	GHttpClient httpClient;
	httpClient.SetClientName("Isotope Server/1.0");
	int nSize;
	char* szFile = Controller::DownloadFile(&httpClient, szUrl, &nSize, false, 30, NULL, NULL);
	if(!szFile)
	{
		fprintf(stderr, "*** Failed to load URL: %s\n", szUrl);
		return NULL;
	}
	Holder<char*> hFile(szFile);

	// Parse the XML
	int nLine, nCol;
	const char* szError;
	Holder<GXMLTag*> hModelTag(GXMLTag::FromString(szFile, nSize, &szError, NULL, &nLine, &nCol));
	GXMLTag* pModelTag = hModelTag.Get();
	if(!pModelTag)
	{
		fprintf(stderr, "*** Failed to parse XML file \"%s\" at line %d. %s\n", szUrl, nLine, szError);
		return NULL;
	}

	// Make the realm server
	Holder<MRealmServer*> hRS(new MRealmServer(szUrl, pGameServer));
	MRealmServer* pRS = hRS.Get();

	// Make the new realm
	pRS->m_pRealm = new MRealm(pGameServer);

	// Load the script
	GXMLTag* pGameTag = pModelTag->GetChildTag("Game");
	if(!pGameTag)
	{
		fprintf(stderr, "*** Expected a <Game> tag in the realm file: %s\n", szUrl);
		return NULL;
	}
	GXMLAttribute* pAttrScript = pGameTag->GetAttribute("script");
	if(!pAttrScript)
	{
		
		fprintf(stderr, "*** Expected a \"script\" attribute in the realm file: %s\n", szUrl);
		return NULL;
	}
	const char* szScriptName = pAttrScript->GetValue();
	GTEMPBUF(pFullScriptName, strlen(pRS->m_szBase) + strlen(szScriptName) + 5);
	strcpy(pFullScriptName, pRS->m_szBase);
	strcat(pFullScriptName, szScriptName);
	if(!pRS->LoadScript(&httpClient, pFullScriptName, pModelTag, pController, pRS->m_pRealm))
		return NULL;

	// Make the remote var
	delete(pRS->m_pRemoteVar);
	pRS->m_pRemoteVar = NULL;
	GXMLAttribute* pRemoteAttr = pModelTag->GetAttribute("Remote");
	if(pRemoteAttr)
	{
		pRS->m_pRemoteVar = new VarHolder(pRS->m_pScriptEngine->GetEngine());
		pRS->m_pScriptEngine->MakeRemoteObject(pRS->m_pRemoteVar, pRemoteAttr->GetValue());
	}

	return hRS.Drop();
}

void MRealmServer::SendUpdates(int nConnection, NRealmServerConnection* pConnection, MClientRecord* pRecord, double time)
{
	double dLastSentUpdatesTime = pRecord->dLastSentUpdatesTime;
	if(time - dLastSentUpdatesTime < UPDATE_REQUEST_RATE)
		return;
	pRecord->dLastSentUpdatesTime = time;
	m_dLatestSentUpdates = time;
	NUpdateRealmObjectPacket packetOut;
	double dObjectTime;
	double dTimeAdjustment = pRecord->dTimeDelta;
	double dOldTime;
	MObject* pOb;
	int n;
	for(n = m_pRealm->GetObjectCount() - 1; n >= 0; n--)
	{
		pOb = m_pRealm->GetObj(n);
		dObjectTime = pOb->GetTime();
		if(dObjectTime > dLastSentUpdatesTime)
		{
			GAssert(pOb->SanityCheck(), "Sanity check failed in MRealmServer::SendUpdates");
			dOldTime = pOb->GetTime();
//fprintf(stderr, "Client<--Server Client:%d Object:%d Type:%s Client time:%f, Server time:%f\n", nConnection, pOb->GetUid(), pOb->GetTypeName(), dOldTime - dTimeAdjustment, dOldTime);
			pOb->SetTime(dOldTime - dTimeAdjustment);
			packetOut.SetObject(pOb);
			pConnection->SendPacket(&packetOut, nConnection);
			pOb->SetTime(dOldTime);
		}
		else if(time - dObjectTime > 30) // todo: unmagic this value, or even better, dynamically adjust it according to server load
			m_pRealm->RemoveObject(nConnection, pOb->GetUid()); // Throw out objects that haven't been updated for a long time
	}
	packetOut.SetObject(NULL);
}

void MRealmServer::UpdateRealmObject(NUpdateRealmObjectPacket* pPacket, double time, MClientRecord* pRecord)
{
	// todo: check whether client has permission to update this object
	// todo: check whether the update is acceptable (ie, does it break physical laws or go back in time relative to previous updates?)
	MObject* pOb = pPacket->GetMObject();
	if(!pOb->GetGObject())
	{
		GAssert(false, "empty object");
		return;
	}

	// Ajust compensation for client's clock
	double dObjectTime = pOb->GetTime();
	GAssert(dObjectTime > 0.1, "It looks like the time was never set on this object");
	double dAdjustedTime = dObjectTime + pRecord->dTimeDelta;
	if(dAdjustedTime > time)
	{
		dAdjustedTime = time;
		pRecord->dTimeDelta = time - dObjectTime;
	}
	else if(dAdjustedTime <= m_dLatestSentUpdates)
	{
		if(m_dLatestSentUpdates - dAdjustedTime > 4) // todo: unmagic this time interval
		{
			fprintf(stderr, "Client %d got more than 4 seconds behind\n", pPacket->GetConnection());
			pRecord->dTimeDelta = time + 6 - dObjectTime; // todo: unmagic this time interval
		}
		dAdjustedTime = m_dLatestSentUpdates + .000001;
	}

	// Update the object
//fprintf(stderr, "Client-->Server Client:%d Object:%d Type:%s Client time:%f Server time:%f\n", pPacket->GetConnection(), pOb->GetUid(), pOb->GetTypeName(), pOb->GetTime(), dAdjustedTime);
	pOb->SetTime(dAdjustedTime);
	pPacket->SetObject(NULL);
	GAssert(pOb->SanityCheck(), "Object failed sanity check in MRealmServer::UpdateRealmObject");
	m_pRealm->ReplaceObject(pPacket->GetConnection(), pOb);
}

void MRealmServer::RemoveRealmObject(NRemoveRealmObjectPacket* pPacket, MClientRecord* pRecord)
{
	// todo: check whether client has permission to remove this object
	unsigned int uid = pPacket->GetUid();
	m_pRealm->RemoveObject(pPacket->GetConnection(), uid);
}

void MRealmServer::ReceiveObject(NSendObjectPacket* pPacket, int nConnection)
{
	if(!m_pRemoteVar)
		return;
	GObject* pRemoteObj = m_pRemoteVar->GetGObject();
	if(!pRemoteObj)
		return;
	m_pScriptEngine->CallReceiveFromClient(pRemoteObj, pPacket->GetGObject(), nConnection);
}

// -------------------------------------------------------------------------

MGameServer::MGameServer(Controller* pController)
: Model()
{
	m_pController = pController;
	m_pConnection = new NRealmServerConnection(this);
	int n;
	for(n = 0; n < SERVER_LOAD_CHECKS; n++)
		m_nLoadChecks[n] = 0;
	m_nLoadCheckPos = 0;
	m_pRealmServers = new GPointerArray(32);
	m_pClients = new GPointerArray(64);
	fprintf(stderr, "Server waiting for clients to connect...\n");
}

/*virtual*/ MGameServer::~MGameServer()
{
	delete(m_pConnection);
	int n;
	for(n = 0; n < m_pRealmServers->GetSize(); n++)
		delete((MRealmServer*)m_pRealmServers->GetPointer(n));
	delete(m_pRealmServers);
	for(n = 0; n < m_pClients->GetSize(); n++)
		delete((MClientRecord*)m_pClients->GetPointer(n));
	delete(m_pClients);
}

void MGameServer::Update(double time)
{
	// Process all pending packets
	m_nLoadChecks[m_nLoadCheckPos] = 0;
	while(true)
	{
		NRealmPacket* pPacket = m_pConnection->GetNextPacket();
		if(!pPacket)
			break;
		ProcessPacket(pPacket, time);
		m_nLoadChecks[m_nLoadCheckPos] = 1;
		delete(pPacket);
	}

	// Avoid hammering the server machine
	if(!m_nLoadChecks[m_nLoadCheckPos])
		GThread::sleep(0);

	// Increment the load check position
	m_nLoadCheckPos++;
	if(m_nLoadCheckPos >= SERVER_LOAD_CHECKS)
		m_nLoadCheckPos = 0;
}

float MGameServer::MeasureLoad()
{
	int sum = 0;
	int n;
	for(n = 0; n < SERVER_LOAD_CHECKS; n++)
		sum += m_nLoadChecks[n];
	return ((float)sum) / SERVER_LOAD_CHECKS;
}

void MGameServer::ProcessPacket(NRealmPacket* pPacket, double time)
{
	switch(pPacket->GetPacketType())
	{
		case NRealmPacket::SET_PATH:
			ConnectToRealm((NSetPathPacket*)pPacket);
			break;

		case NRealmPacket::SEND_ME_UPDATES:
			SendUpdates((NSendMeUpdatesPacket*)pPacket, time);
			break;

		case NRealmPacket::UPDATE_REALM_OBJECT:
			UpdateRealmObject((NUpdateRealmObjectPacket*)pPacket, time);
			break;

		case NRealmPacket::REMOVE_REALM_OBJECT:
			RemoveRealmObject((NRemoveRealmObjectPacket*)pPacket);
			break;

		case NRealmPacket::SEND_OBJECT:
			ReceiveObject((NSendObjectPacket*)pPacket);
			break;

		default:
			printf("*** Unrecognized packet type: %d\n", (int)pPacket->GetPacketType());
	}
}

MRealmServer* MGameServer::FindOrLoadRealmServer(const char* szUrl)
{
	if(szUrl[0] == '\0')
		return NULL;

	int n;
	for(n = 0; szUrl[n] != '\0'; n++)
	{
		if(szUrl[n] == '?')
			((char*)szUrl)[n] = '\0'; // todo: this is a hack--fix it properly
	}

	// See if it's already been loaded
	for(n = 0; n < m_pRealmServers->GetSize(); n++) // todo: use a hash table instead
	{
		MRealmServer* pRS = (MRealmServer*)m_pRealmServers->GetPointer(n);
		if(stricmp(pRS->GetUrl(), szUrl) == 0)
			return pRS;
	}

	// Load it
	MRealmServer* pRS = NULL;
	try
	{
		pRS = MRealmServer::LoadRealm(szUrl, this, m_pController);
	}
	catch(const char* szMessage)
	{
		fprintf(stderr, "*** Error while loading realm: %s\n", szMessage);
	}
	catch(const wchar_t* /*wszMessage*/)
	{
		fprintf(stderr, "*** Caught an exception in the form of a Unicode string\n");
	}
	catch(...)
	{
		fprintf(stderr, "*** Caught an exception of an unknown type\n");
	}
	if(!pRS)
		return NULL;
	m_pRealmServers->AddPointer(pRS);
	fprintf(stderr, "Loaded new realm: %s\n", pRS->GetUrl());
	return pRS;
}

MClientRecord* MGameServer::GetClientRecord(int n, bool bCreateIfNotFound)
{
	if(n >= m_pClients->GetSize())
	{
		if(bCreateIfNotFound)
		{
			do
			{
				m_pClients->AddPointer(new MClientRecord());
			} while(n >= m_pClients->GetSize());
		}
		else
			return NULL;
	}
	return (MClientRecord*)m_pClients->GetPointer(n);
}

MRealmServer* MGameServer::GetRealmServer(int nConnection)
{
	GAssert(nConnection >= 0, "connection out of range");
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(pRecord)
		return pRecord->pRealmServer;
	else
		return NULL;
}

void MGameServer::ConnectToRealm(NSetPathPacket* pPacketIn)
{
	int nConnection = pPacketIn->GetConnection();
	const char* szUrl = pPacketIn->GetPath();
	MRealmServer* pRS = FindOrLoadRealmServer(szUrl);
	if(!pRS)
	{
		fprintf(stderr, "*** Client %d requested a connection to bogus realm: %s\n", nConnection, szUrl);
		// todo: drop the connection
		//m_pConnection->
		return;
	}
	MClientRecord* pRecord = GetClientRecord(nConnection, true);
	pRecord->pRealmServer = pRS;
	pRecord->dTimeDelta = 1e10;
	fprintf(stderr, "Client %d has connected to realm: %s\n", nConnection, szUrl);
}

void MGameServer::SendUpdates(NSendMeUpdatesPacket* pPacketIn, double time)
{
	int nConnection = pPacketIn->GetConnection();
//fprintf(stderr, "Client %d wants updates\n", nConnection);
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
	{
		fprintf(stderr, "*** An unknown client tried to request updates\n");
		return;
	}
	MRealmServer* pRS = pRecord->pRealmServer;
	if(!pRS)
	{
		fprintf(stderr, "*** Client requested updates, but hasn't successfully connected to a particular realm yet\n");
		return;
	}
	pRS->SendUpdates(nConnection, m_pConnection, pRecord, time);
}

void MGameServer::UpdateRealmObject(NUpdateRealmObjectPacket* pPacket, double time)
{
	int nConnection = pPacket->GetConnection();
//fprintf(stderr, "Received an object from client: %d\n", nConnection);
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
	{
		fprintf(stderr, "*** An unknown client tried to update an object\n");
		return;
	}
	MRealmServer* pRS = pRecord->pRealmServer;
	if(!pRS)
	{
		fprintf(stderr, "*** Client tried to update an object, but hasn't successfully connected to a particular realm yet\n");
		return;
	}
	pRS->UpdateRealmObject(pPacket, time, pRecord);
}

void MGameServer::RemoveRealmObject(NRemoveRealmObjectPacket* pPacket)
{
	int nConnection = pPacket->GetConnection();
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
	{
		fprintf(stderr, "*** An unknown client tried to remove a realm object\n");
		return;
	}
	MRealmServer* pRS = pRecord->pRealmServer;
	if(!pRS)
	{
		fprintf(stderr, "*** Client tried to remove an object, but hasn't successfully connected to a particular realm yet\n");
		return;
	}
	pRS->RemoveRealmObject(pPacket, pRecord);
}

void MGameServer::ReceiveObject(NSendObjectPacket* pPacket)
{
	int nConnection = pPacket->GetConnection();
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
	{
		fprintf(stderr, "*** An unknown client tried to send an object to the server\n");
		return;
	}
	MRealmServer* pRS = pRecord->pRealmServer;
	if(!pRS)
	{
		fprintf(stderr, "*** Client tried to send an object to the server, but hasn't successfully connected to a particular realm yet\n");
		return;
	}
	pRS->ReceiveObject(pPacket, nConnection);
}

/*virtual*/ bool MGameServer::OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew)
{
	if(!pNew || !pOld || nConnection <= 0)
		return true;
	return pNew->m_pScriptEngine->CallVerify(nConnection, pOld, pNew); // todo: if the server load is high, just return true
}

/*virtual*/ void MGameServer::SendObject(GObject* pObj, int nConnection)
{
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
		return;
	MScriptEngine* pScriptEngine = pRecord->pRealmServer->GetScriptEngine();
	NSendObjectPacket packet(pScriptEngine->GetEngine());
	packet.SetObject(pObj);
	m_pConnection->SendPacket(&packet, nConnection);
}

void MGameServer::OnLoseConnection(int nConnection)
{
	fprintf(stderr, "Client %d has disconnected\n", nConnection);
}

void MGameServer::BootClient(int nConnection)
{
	m_pConnection->Disconnect(nConnection);
}

