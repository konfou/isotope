/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "MPuzSearchEngine.h"
#include "Main.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GHttp.h"
#include "../GClasses/GHashTable.h"
#include "../GClasses/GQueue.h"
#include "Controller.h"
#include <stdlib.h>

#define PUZ_SEARCH_ENGINE_PORT 4749

class MPuzSearchEngineServer : public GHttpServer
{
protected:
	MPuzSearchEngine* m_pParent;
	GXMLTag* m_pKnownPuzzles;

public:
	MPuzSearchEngineServer(MPuzSearchEngine* pParent, int nPort) : GHttpServer(nPort)
	{
		m_pParent = pParent;
		GXMLTag* pConfigTag = GameEngine::GetConfig();
		GXMLTag* pPuzSearchEngineTag = pConfigTag->GetChildTag("PuzSearchEngine");
		if(!pPuzSearchEngineTag)
			GameEngine::ThrowError("Expected a <PuzSearchEngine> tag in the config file");
		GXMLAttribute* pDataAttr = pPuzSearchEngineTag->GetAttribute("data");
		if(!pDataAttr)
			GameEngine::ThrowError("Expected a 'data' attribute in the <PuzSearchEngine> tag in the config file");
		const char* szErrorMessage;
		int nErrorLine;
		m_pKnownPuzzles = GXMLTag::FromFile(pDataAttr->GetValue(), &szErrorMessage, NULL, &nErrorLine, NULL);
		if(!m_pKnownPuzzles)
			GameEngine::ThrowError("Failed to load puzzle search engine data file: %s, line: %d\n%s", pDataAttr->GetValue(), nErrorLine, szErrorMessage);
	}

	virtual ~MPuzSearchEngineServer()
	{
		delete(m_pKnownPuzzles);
	}

protected:
	virtual void DoGet(const char* szUrl, const char* szParams, GQueue* pResponse)
	{
		// Get the skill and level values from the parameters
		GStringHeap sh(256);
		GConstStringHashTable ht(10, false);
		GHttpServer::ParseParams(&sh, &ht, szParams);
		const char* szSkill;
		const char* szLevel;
		if(!ht.Get("skill", (void**)&szSkill))
		{
			fprintf(stderr, "Got a query without a \"skill\" parameter\n");
			return;
		}
		if(!ht.Get("level", (void**)&szLevel))
		{
			fprintf(stderr, "Got a query without a \"level\" parameter\n");
			return;
		}
		double dLevel = atof(szLevel);

		// Find the puzzles that teach the specified skill
		GXMLTag* pGroupTag = m_pKnownPuzzles->FindChildWithAttribute("name", szSkill);
		if(!pGroupTag)
		{
			fprintf(stderr, "There are no known puzzles that teach the skill: %s\n", szSkill);
			return;
		}

		// Find the best matching puzzle
		GXMLTag* pBestPuzTag = NULL;
		double dLowestMatchLevel = 1e100;
		GXMLTag* pPuzTag;
		for(pPuzTag = pGroupTag->GetFirstChildTag(); pPuzTag; pPuzTag = pGroupTag->GetNextChildTag(pPuzTag))
		{
			GXMLAttribute* pAttrLevel = pPuzTag->GetAttribute("level");
			if(!pAttrLevel)
				continue;
			double d = atof(pAttrLevel->GetValue());
			if(d > dLevel && d < dLowestMatchLevel)
			{
				pBestPuzTag = pPuzTag;
				dLowestMatchLevel = d;
			}
		}

		// Return the results
		if(!pBestPuzTag)
		{
			fprintf(stderr, "There are no puzzles that match the specified search criteria\n");
			return;
		}
		GXMLAttribute* pAttrUrl = pBestPuzTag->GetAttribute("url");
		if(!pAttrUrl)
		{
			fprintf(stderr, "Expected a 'url' attribute in the KnownPuzzles.xml file\n");
			return;
		}
		pResponse->Push(pAttrUrl->GetValue());
	}
};

// -----------------------------------------------------------------------------

MPuzSearchEngine::MPuzSearchEngine(Controller* pController)
: Model()
{
	m_pController = pController;
	m_pHttpServer = new MPuzSearchEngineServer(this, PUZ_SEARCH_ENGINE_PORT);
}

/*virtual*/ MPuzSearchEngine::~MPuzSearchEngine()
{
	delete(m_pHttpServer);
}

/*virtual*/ void MPuzSearchEngine::Update(double time)
{
	m_pHttpServer->Process();
}
