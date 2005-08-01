/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GHttp.h"
#include "GEZSocket.h"
#include "GString.h"
#include "GMacros.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

GHttpClient::GHttpClient()
{
	m_pSocket = NULL;
	m_status = Error;
	m_pData = NULL;
}

GHttpClient::~GHttpClient()
{
	delete(m_pSocket);
	delete(m_pData);
}

bool GHttpClient::Get(const char* szUrl, int nPort)
{
	if(strnicmp(szUrl, "http://", 7) == 0)
		szUrl += 7;

	// find the first slash
	int n;
	for(n = 0; szUrl[n] != '\0' && szUrl[n] != '/'; n++)
	{
	}
	if(n < 1)
		return false;
	char* szServer = (char*)alloca(n + 1);
	memcpy(szServer, szUrl, n);
	szServer[n] = '\0';
	const char* szPath = szUrl + n;
	if(strlen(szPath) == 0)
		szPath = "/index.html";

	// Connect
	delete(m_pSocket);
	m_pSocket = GEZSocketClient::ConnectToTCPSocket(szServer, nPort);
	if(!m_pSocket)
		return false;

	// Send the request
	GString s;
	s.Add(L"GET ");
	s.Add(szPath);
	s.Add(L" HTTP/1.1\r\n");
	s.Add(L"Host: ");
	s.Add(szServer);
	s.Add(L":");
	s.Add(nPort);
	s.Add(L"\r\n\r\n");
	char* szRequest = (char*)alloca(s.GetLength() + 1);
	s.GetAnsi(szRequest);
	if(!m_pSocket->Send((unsigned char*)szRequest, s.GetLength()))
		return false;

	// Update status
	m_nContentSize = 0;
	m_nHeaderPos = 0;
    delete(m_pData);
	m_pData = NULL;
	m_status = Downloading;
	return true;
}

GHttpClient::Status GHttpClient::CheckStatus()
{
	const unsigned char* szChunk;
	int nSize;
	while(m_pSocket->GetMessageCount() > 0)
	{
		szChunk = m_pSocket->GetNextMessage(&nSize);
		if(m_pData)
			ProcessBody(szChunk, nSize);
		else
			ProcessHeader(szChunk, nSize);
	}
	return m_status;
}

void GHttpClient::ProcessHeader(const unsigned char* szData, int nSize)
{
	while(nSize > 0)
	{
		if(m_nHeaderPos < 256)
			m_szHeaderBuf[m_nHeaderPos++] = *szData;
		if(*szData == '\n')
		{
			m_szHeaderBuf[m_nHeaderPos] = '\0';
			if(m_nHeaderPos <= 2)
			{
				szData++;
				nSize--;
				if(m_nContentSize > 0 && nSize > 0)
					ProcessBody(szData, nSize);
				return;
			}
			if(strnicmp(m_szHeaderBuf, "HTTP/", 5) == 0)
			{
				char* szTmp = m_szHeaderBuf + 5;
				while(*szTmp != ' ' && *szTmp != '\n')
					szTmp++;
				if(*szTmp == ' ')
					szTmp++;
				if(*szTmp == '4')
					m_status = NotFound;
				else if(*szTmp != '2')
					m_status = Error;
				else
					m_nContentSize = 0;
			}
			else if(strnicmp(m_szHeaderBuf, "Content-Length:", 15) == 0)
			{
				char* szTmp = m_szHeaderBuf + 15;
				while(*szTmp != '\n' && *szTmp <= ' ')
					szTmp++;
				m_nContentSize = atoi(szTmp);
				if(m_nContentSize > 0)
				{
					m_pData = new unsigned char[m_nContentSize + 1];
					GAssert(m_pData, "out of memory");
					m_nDataPos = 0;
				}
			}
			m_nHeaderPos = 0;
		}
		szData++;
		nSize--;
	}
}

void GHttpClient::ProcessBody(const unsigned char* szData, int nSize)
{
	if(m_nDataPos + nSize > m_nContentSize)
		nSize = m_nContentSize - m_nDataPos;
	memcpy(m_pData + m_nDataPos, szData, nSize);
	m_nDataPos += nSize;
	if(m_nDataPos >= m_nContentSize)
	{
		if(m_status == Downloading)
			m_status = Done;
		m_pData[m_nContentSize] = '\0';
	}
}

unsigned char* GHttpClient::GetData(int* pnSize)
{
	if(m_status != Done)
	{
		*pnSize = 0;
		return NULL;
	}
	*pnSize = m_nContentSize;
	return m_pData;
}

unsigned char* GHttpClient::DropData(int* pnSize)
{
	unsigned char* pData = GetData(pnSize);
	if(!pData)
		return NULL;
	m_pData = NULL;
	return pData;
}
