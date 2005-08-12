/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MRealmServer.h"
#include "GameEngine.h"
#include "NRealmProtocol.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GSocket.h"
#include "../GClasses/GFile.h"
#include "MRealm.h"
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // WIN32
#include "../Gash/Include/GashEngine.h"
#include "MScriptEngine.h"
#include "MObject.h"
#include "Controller.h"

MRealmServer::MRealmServer(const char* szPath, MGameServer* pGameServer)
{
	int nLen = strlen(szPath);
	m_szPath = new char[nLen + 1];
	strcpy(m_szPath, szPath);
	m_szBase = new char[nLen + 1];
	strcpy(m_szBase, szPath);
	int n;
	for(n = nLen - 1; n >= 0 && m_szBase[n] != '/' && m_szBase[n] != '\\'; n--)
		m_szBase[n] = '\0';
	m_pRealm = new MRealm(pGameServer);
	m_dLatestSentUpdates = 0;
	m_pScriptEngine = NULL;
	m_pErrorHandler = new IsotopeErrorHandler();
}

MRealmServer::~MRealmServer()
{
	delete(m_pRealm);
	delete(m_szPath);
	delete(m_szBase);
	delete(m_pScriptEngine);
	delete(m_pErrorHandler);
}

void MRealmServer::LoadScript(const char* szFilename)
{
	int nBufSize;
	Holder<char*> hBuf(GFile::LoadFileToBuffer(szFilename, &nBufSize));
	const char* pFile = hBuf.Get();
	if(!pFile)
		GameEngine::ThrowError("Failed to load script file: %s", szFilename);
	m_pScriptEngine = new MScriptEngine(pFile, nBufSize, m_pErrorHandler, NULL, NULL);
}

/*static*/ MRealmServer* MRealmServer::LoadRealm(const char* szFilename, MGameServer* pGameServer)
{
	// Download the file
	int nSize;
	Holder<char*> hFile(GFile::LoadFileToBuffer(szFilename, &nSize));
	char* szFile = hFile.Get();
	if(!szFile)
	{
		char szCurDir[512];
		getcwd(szCurDir, 512);
		GameEngine::ThrowError("Failed to load file \"%s\".  (CurDir=\"%s\")\n", szFilename, szCurDir);
	}

	// Parse the XML
	int nLine, nCol;
	const char* szError;
	Holder<GXMLTag*> hModelTag(GXMLTag::FromString(szFile, nSize, &szError, NULL, &nLine, &nCol));
	GXMLTag* pModelTag = hModelTag.Get();
	if(!pModelTag)
		GameEngine::ThrowError("Failed to parse XML file \"%s\". %s", szFilename, szError);

	// Make the realm server
	Holder<MRealmServer*> hRS(new MRealmServer(szFilename, pGameServer));
	MRealmServer* pRS = hRS.Get();

	// Load the script
	GXMLAttribute* pAttrScript = pModelTag->GetAttribute("Script");
	if(!pAttrScript)
		GameEngine::ThrowError("Expected a \"Script\" attribute in file: %s", szFilename);
	const char* szScriptName = pAttrScript->GetValue();
	GTEMPBUF(pFullScriptName, strlen(pRS->m_szBase) + strlen(szScriptName) + 5);
	strcpy(pFullScriptName, pRS->m_szBase);
	strcat(pFullScriptName, szScriptName);
	pRS->LoadScript(pFullScriptName);

	// Make the new realm
	pRS->m_pRealm = new MRealm(pGameServer);
	return hRS.Drop();
}

void MRealmServer::SendUpdates(NSendMeUpdatesPacket* pPacketIn, NRealmServerConnection* pConnection, double time)
{
	if(pPacketIn->GetTime() > m_dLatestSentUpdates)
		m_dLatestSentUpdates = pPacketIn->GetTime();
	NUpdateObjectPacket packetOut;
	double dObjectTime;
	MObject* pOb;
	int n;
	for(n = m_pRealm->GetObjectCount() - 1; n >= 0; n--)
	{
		pOb = m_pRealm->GetObj(n);
		dObjectTime = pOb->GetTime();
		if(dObjectTime > pPacketIn->GetTime())
		{
			packetOut.SetObject(pOb);
			pConnection->SendPacket(&packetOut, pPacketIn->GetConnection());
		}
		else if(time - dObjectTime > 60) // todo: unmagic this value
			m_pRealm->RemoveObject(pPacketIn->GetConnection(), pOb->GetUid()); // Throw out objects that haven't been updated for a long time
	}

	packetOut.SetObject(NULL);
}

void MRealmServer::UpdateObject(NUpdateObjectPacket* pPacket, double time, MClientRecord* pRecord)
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
	if(dAdjustedTime <= m_dLatestSentUpdates)
	{
		//pOb->Update(m_dLatestSentUpdates + .000001);
		pOb->SetTime(m_dLatestSentUpdates + .000001);
		if(m_dLatestSentUpdates - dAdjustedTime > 4) // todo: unmagic this time interval (how far the client is allowed to get behind the server and still be allowed to updates objects)
		{
			printf("Client got waaay behind\n");
			pRecord->dTimeDelta = 86400;
		}
	}

	// Update the object
	printf("Client updated object %d\n", pOb->GetUid());
	pPacket->SetObject(NULL);
	m_pRealm->ReplaceObject(pPacket->GetConnection(), pOb);
}

// -------------------------------------------------------------------------

MGameServer::MGameServer(const char* szBasePath)
: Model()
{
	m_pConnection = new NRealmServerConnection(this);
	int n;
	for(n = 0; n < SERVER_LOAD_CHECKS; n++)
		m_nLoadChecks[n] = 0;
	m_nLoadCheckPos = 0;
	m_pRealmServers = new GPointerArray(32);
	m_pClients = new GPointerArray(64);
	m_nBasePathLen = strlen(szBasePath);
	if(m_nBasePathLen <= 0)
		GameEngine::ThrowError("Base path invalid: %s", szBasePath);
	m_szBasePath = new char[m_nBasePathLen + 1];
	strcpy(m_szBasePath, szBasePath);
	if(m_szBasePath[m_nBasePathLen - 1] == '/' || m_szBasePath[m_nBasePathLen - 1] == '\\')
		m_szBasePath[--m_nBasePathLen] = '\0';
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
#ifdef WIN32
		Sleep(0);
#else
		usleep(0);
#endif

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

		case NRealmPacket::UPDATE_OBJECT:
			UpdateObject((NUpdateObjectPacket*)pPacket, time);
			break;

		default:
			GAssert(false, "Unrecognized packet type");
	}
}

MRealmServer* MGameServer::FindOrLoadRealmServer(const char* szUrl)
{
	if(szUrl[0] == '\0')
		return NULL;

	// Parse the URL to extract the local path
	GTEMPBUF(szPath, m_nBasePathLen + strlen(szUrl) + 1);
	strcpy(szPath, m_szBasePath);
	GSocket::ParseURL(szUrl, NULL, NULL, szPath + m_nBasePathLen, NULL, NULL);
	fprintf(stderr, "Client requested connection to: %s (local path: %s)\n", szUrl, szPath);

	// See if it's already been loaded
	int n;
	for(n = 0; n < m_pRealmServers->GetSize(); n++)
	{
		MRealmServer* pRS = (MRealmServer*)m_pRealmServers->GetPointer(n);
		if(stricmp(pRS->GetPath(), szPath) == 0)
			return pRS;
	}

	// Load it
	// todo: catch exceptions here
	MRealmServer* pRS = NULL;
	try
	{
		pRS = MRealmServer::LoadRealm(szPath, this);
	}
	catch(const char* szMessage)
	{
		fprintf(stderr, "Error while loading realm: %s\n", szMessage);
	}
	catch(...)
	{
	}
	if(!pRS)
		return NULL;
	m_pRealmServers->AddPointer(pRS);
	fprintf(stderr, "Loaded new realm: %s\n", pRS->GetPath());
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
	GAssert(nConnection > 0, "out of range");
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(pRecord)
		return pRecord->pRealmServer;
	else
		return NULL;
}

void MGameServer::ConnectToRealm(NSetPathPacket* pPacketIn)
{
	MRealmServer* pRS = FindOrLoadRealmServer(pPacketIn->GetPath());
	if(!pRS)
	{
		GAssert(false, "todo: send not found packet");
		return;
	}
	int nConnection = pPacketIn->GetConnection();
	MClientRecord* pRecord = GetClientRecord(nConnection, true);
	pRecord->pRealmServer = pRS;
	pRecord->dTimeDelta = 86400; // 86400 = number of seconds in a day
}

void MGameServer::SendUpdates(NSendMeUpdatesPacket* pPacketIn, double time)
{
	MRealmServer* pRS = GetRealmServer(pPacketIn->GetConnection());
	if(!pRS)
	{
		GAssert(false, "path not set yet--todo: handle this case");
		return;
	}
	pRS->SendUpdates(pPacketIn, m_pConnection, time);
}

void MGameServer::UpdateObject(NUpdateObjectPacket* pPacket, double time)
{
	int nConnection = pPacket->GetConnection();
	MClientRecord* pRecord = GetClientRecord(nConnection, false);
	if(!pRecord)
	{
		GAssert(false, "path not set yet--todo: handle this case");
		return;
	}
	MRealmServer* pRS = pRecord->pRealmServer;
	if(!pRS)
	{
		GAssert(false, "path not set yet--todo: handle this case");
		return;
	}
	pRS->UpdateObject(pPacket, time, pRecord);
}

/*virtual*/ bool MGameServer::OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew)
{
	if(!pNew || !pOld || nConnection <= 0)
		return true;
	// todo: if the server load is high, just return true

	return pNew->m_pScriptEngine->CallVerify(nConnection, pOld, pNew);
}
