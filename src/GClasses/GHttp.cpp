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
#include "GQueue.h"
#include "GTime.h"
#include "GHashTable.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

class GHttpClient;

class GHttpClientSocket : public GEZSocketClient
{
protected:
	GHttpClient* m_pParent;

public:
	GHttpClientSocket(GHttpClient* pParent, int nMaxPacketSize) : GEZSocketClient(nMaxPacketSize)
	{
		m_pParent = pParent;
	}

	virtual ~GHttpClientSocket()
	{
	}

	static GHttpClientSocket* ConnectToTCPSocket(GHttpClient* pParent, const char* szAddress, int nPort)
	{
		GHttpClientSocket* pSocket = new GHttpClientSocket(pParent, 0);
		if(!pSocket)
			return NULL;
		if(!pSocket->Init(false, false))
		{
			delete(pSocket);
			return NULL;
		}
		if(!pSocket->Connect(szAddress, nPort))
		{
			delete(pSocket);
			return NULL;
		}
		return pSocket;
	}

protected:
	virtual void OnLoseConnection(int nSocketNumber)
	{
printf("Connection lost 2\n");
		m_pParent->OnLoseConnection();
	}
};

// -------------------------------------------------------------------------------

//FILE* g_pFile;

GHttpClient::GHttpClient()
{
	m_pSocket = NULL;
	m_status = Error;
	m_pData = NULL;
	m_pChunkQueue = NULL;
	m_bPastHeader = false;
	strcpy(m_szServer, "\0");
	m_szRedirect = NULL;
	m_dLastReceiveTime = 0;
//g_pFile = fopen("tmp.txt", "w");
}

GHttpClient::~GHttpClient()
{
	delete(m_pSocket);
	delete(m_pData);
	delete(m_pChunkQueue);
	delete(m_szRedirect);
}

GHttpClient::Status GHttpClient::CheckStatus(float* pfProgress)
{
	const unsigned char* szChunk;
	int nSize;
	while(m_pSocket->GetMessageCount() > 0)
	{
		m_dLastReceiveTime = GTime::GetTime();
		szChunk = m_pSocket->GetNextMessage(&nSize);
//fwrite(szChunk, nSize, 1, g_pFile);
//fflush(g_pFile);
		if(m_bPastHeader)
			ProcessBody(szChunk, nSize);
		else
			ProcessHeader(szChunk, nSize);
		if(pfProgress)
		{
			if(m_bChunked)
				*pfProgress = (float)m_nDataPos / m_nContentSize;
			else
				*pfProgress = 0;
		}
	}
	return m_status;
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

	// Connect
	if(!m_pSocket || GTime::GetTime() - m_dLastReceiveTime > 10 || !m_pSocket->IsConnected() || strcmp(szServer, m_szServer) != 0)
	{
		delete(m_pSocket);
		m_pSocket = GHttpClientSocket::ConnectToTCPSocket(this, szServer, nPort);
		if(!m_pSocket)
			return false;
		strncpy(m_szServer, szServer, 255);
		m_szServer[255] = '\0';
	}

	// Send the request
	const char* szPath = szUrl + n;
	if(strlen(szPath) == 0)
		szPath = "/index.html";
	GString s;
	s.Add(L"GET ");
	while(*szPath != '\0')
	{
		if(*szPath == ' ')
			s.Add(L"%20");
		else
			s.Add(*szPath);
		szPath++;
	}
	s.Add(L" HTTP/1.1\r\n");
	s.Add(L"Host: ");
	s.Add(szServer);
	s.Add(L":");
	s.Add(nPort);
// todo: undo the next line
	s.Add(L"\r\nUser-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.12) Gecko/20051010 Firefox/1.0.7 (Ubuntu package 1.0.7)\r\nAccept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\nAccept-Language: en-us,en;q=0.5\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\nKeep-Alive: 300\r\nConnection: keep-alive\r\n\r\n");
//	s.Add(L"\r\nUser-Agent: GHttpClient/1.0\r\nKeep-Alive: 60\r\nConnection: keep-alive\r\n\r\n");
	char* szRequest = (char*)alloca(s.GetLength() + 1);
	s.GetAnsi(szRequest);
//printf("### Sending Request:\n%s\n###\n", szRequest);
	if(!m_pSocket->Send((unsigned char*)szRequest, s.GetLength()))
		return false;

	// Update status
	m_nContentSize = 0;
	m_nDataPos = 0;
	m_bChunked = false;
	m_bPastHeader = false;
	m_nHeaderPos = 0;
    delete(m_pData);
	m_pData = NULL;
	m_status = Downloading;
	return true;
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
				m_bPastHeader = true;
				if(m_szRedirect)
				{
					if(!Get(m_szRedirect, 80))
						m_status = Error;
					delete(m_szRedirect);
					m_szRedirect = NULL;
				}
				else if(nSize > 0 && (m_bChunked || m_nContentSize > 0))
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
			else if(strnicmp(m_szHeaderBuf, "Transfer-Encoding: chunked", 26) == 0)
			{
				m_bChunked = true;
			}
			else if(strnicmp(m_szHeaderBuf, "Location:", 9) == 0)
			{
				const char* szLoc = m_szHeaderBuf + 9;
				while(*szLoc > '\0' && *szLoc <= ' ')
					szLoc++;
				int nLen = strlen(szLoc);
				delete(m_szRedirect);
				m_szRedirect = new char[nLen + 1];
				strcpy(m_szRedirect, szLoc);
			}
			m_nHeaderPos = 0;
		}
		szData++;
		nSize--;
	}
}

void GHttpClient::ProcessBody(const unsigned char* szData, int nSize)
{
	if(m_bChunked)
		ProcessChunkBody(szData, nSize);
	else if(m_nContentSize > 0)
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
			m_bPastHeader = false;
		}
	}
	else
	{
		if(!m_pChunkQueue)
			m_pChunkQueue = new GQueue();
		m_pChunkQueue->Push(szData, nSize);
	}
}

void GHttpClient::OnLoseConnection()
{
	if(m_bChunked)
	{
		if(m_status == Downloading)
			m_status = Error;
	}
	else if(m_nContentSize > 0)
	{
		if(m_status == Downloading)
			m_status = Error;
	}
	else
	{
		if(m_status == Downloading)
			m_status = Done;
		m_nContentSize = m_pChunkQueue->GetSize();
		delete(m_pData);
		m_pData = (unsigned char*)m_pChunkQueue->DumpToString();
		m_bPastHeader = false;
	}

	// todo: take a lock around this
	GHttpClientSocket* pSocket = m_pSocket;
	m_pSocket = NULL;
	delete(pSocket);
}

void GHttpClient::ProcessChunkBody(const unsigned char* szData, int nSize)
{
	if(!m_pChunkQueue)
		m_pChunkQueue = new GQueue();
	while(nSize > 0)
	{
		if(m_nContentSize == 0)
		{
			// Read the chunk size
			int n;
			for(n = 0; (szData[n] < '0' || szData[n] > 'f') && n < nSize; n++)
			{
			}
			int nHexStart = n;
			for( ; szData[n] >= '0' && szData[n] <= 'f' && n < nSize; n++)
			{
			}
			if(n >= nSize)
				break;

			// Convert it from hex to an integer
			int nPow = 1;
			int nDig;
			int i;
			for(i = n - 1; i >= nHexStart; i--)
			{
				if(szData[i] >= '0' && szData[i] <= '9')
					nDig = szData[i] - '0';
				else if(szData[i] >= 'a' && szData[i] <= 'f')
					nDig = szData[i] - 'a' + 10;
				else if(szData[i] >= 'A' && szData[i] <= 'F')
					nDig = szData[i] - 'A' + 10;
				else
				{
					nDig = 0;
					GAssert(false, "expected a hex digit");
				}
				m_nContentSize += (nDig * nPow);
				nPow *= 16;
			}
			for( ; szData[n] != '\n' && n < nSize; n++)
			{
			}
			if(n < nSize && szData[n] == '\n')
				n++;
			szData += n;
			nSize -= n;
		}
		if(m_nContentSize == 0)
		{
			m_nContentSize = m_pChunkQueue->GetSize();
			delete(m_pData);
			m_pData = (unsigned char*)m_pChunkQueue->DumpToString();
			m_bChunked = false;
			m_bPastHeader = false;
			if(m_status == Downloading)
				m_status = Done;
			break;
		}
		else
		{
			int nChunkSize = MIN(m_nContentSize, nSize);
			m_pChunkQueue->Push(szData, nChunkSize);
			szData += nChunkSize;
			nSize -= nChunkSize;
			m_nContentSize -= nChunkSize;
		}
	}
}

// todo: this is a hack--fix it properly
void GHttpClient::GimmeWhatYouGot()
{
	if(m_bChunked)
	{
		m_nContentSize = m_pChunkQueue->GetSize();
		if(m_nContentSize > 64)
		{
			delete(m_pData);
			m_pData = (unsigned char*)m_pChunkQueue->DumpToString();
			m_bChunked = false;
			m_bPastHeader = false;
			if(m_status == Downloading)
				m_status = Done;
		}
	}
	else if(m_nContentSize > 0)
	{
		if(m_nDataPos > 64)
		{
			if(m_status == Downloading)
				m_status = Done;
			m_pData[m_nDataPos] = '\0';
			m_bPastHeader = false;
			m_nContentSize = m_nDataPos;
		}
	}
	else
	{
		if(m_pChunkQueue && m_pChunkQueue->GetSize() > 64)
		{
			if(m_status == Downloading)
				m_status = Done;
			m_nContentSize = m_pChunkQueue->GetSize();
			delete(m_pData);
			m_pData = (unsigned char*)m_pChunkQueue->DumpToString();
			m_bPastHeader = false;
		}
	}
}

unsigned char* GHttpClient::GetData(int* pnSize)
{
	if(m_status != Done)
		GimmeWhatYouGot();
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


// -----------------------------------------------------------------------

class GHttpServerBuffer
{
public:
	enum RequestType
	{
		None,
		Get,
		Post,
	};

	int m_nPos;
	char m_szLine[MAX_SERVER_LINE_SIZE];
	char m_szUrl[MAX_SERVER_LINE_SIZE];
	char m_szParams[MAX_SERVER_LINE_SIZE];
	RequestType m_eRequestType;
	int m_nContentLength;

	GHttpServerBuffer()
	{
		m_eRequestType = None;
		m_nPos = 0;
		m_nContentLength = 0;
	}

	~GHttpServerBuffer()
	{
	}
};

GHttpServer::GHttpServer(int nPort)
{
	m_pBuffers = new GPointerArray(16);
	m_pSocket = GEZSocketServer::HostTCPSocket(nPort);
	if(!m_pSocket)
		throw("failed to open port");
	m_pQ = new GQueue();
	SetContentType("text/html");
}

GHttpServer::~GHttpServer()
{
	int nCount = m_pBuffers->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((GHttpServerBuffer*)m_pBuffers->GetPointer(n));
	delete(m_pBuffers);
	delete(m_pSocket);
	delete(m_pQ);
}

void GHttpServer::Process()
{
	int nMessageSize;
	int nConnection;
	unsigned char* pMessage;
	unsigned char* pIn;
	char c;
	GHttpServerBuffer* pBuffer;
	while(m_pSocket->GetMessageCount() > 0)
	{
		pMessage = m_pSocket->GetNextMessage(&nMessageSize, &nConnection);
		pIn = pMessage;
		while(m_pBuffers->GetSize() <= nConnection)
			m_pBuffers->AddPointer(new GHttpServerBuffer());
		pBuffer = (GHttpServerBuffer*)m_pBuffers->GetPointer(nConnection);
		while(nMessageSize > 0)
		{
			c = *pIn;
			pBuffer->m_szLine[pBuffer->m_nPos++] = c;
			pIn++;
			nMessageSize--;
			if(c == '\n' || pBuffer->m_nPos >= MAX_SERVER_LINE_SIZE - 1)
			{
				pBuffer->m_szLine[pBuffer->m_nPos] = '\0';
				ProcessLine(nConnection, pBuffer, pBuffer->m_szLine);
				pBuffer->m_nPos = 0;
			}
		}
		delete(pMessage);
	}
}

void GHttpServer::ProcessLine(int nConnection, GHttpServerBuffer* pClient, const char* szLine)
{
	OnProcessLine(nConnection, szLine);
	while(*szLine > '\0' && *szLine <= ' ')
		szLine++;
	if(*szLine == '\0')
		MakeResponse(nConnection, pClient);
	else if(strnicmp(szLine, "GET ", 4) == 0)
	{
		pClient->m_eRequestType = GHttpServerBuffer::Get;
		const char* szIn = szLine + 4;
		char* szOut = pClient->m_szUrl;
		while(*szIn > ' ' && *szIn != '?')
		{
			*szOut = *szIn;
			szIn++;
			szOut++;
		}
		*szOut = '\0';
		if(*szIn == '?')
		{
			szIn++;
			szOut = pClient->m_szParams;
			while(*szIn > ' ')
			{
				*szOut = *szIn;
				szIn++;
				szOut++;
			}
			*szOut = '\0';
		}
		else
			pClient->m_szParams[0] = '\0';
	}
	else if(strnicmp(szLine, "POST ", 5) == 0)
		pClient->m_eRequestType = GHttpServerBuffer::Post;
	else if(strnicmp(szLine, "Content-Length: ", 16) == 0)
		pClient->m_nContentLength = atoi(szLine + 16);
}

void GHttpServer::SetContentType(const char* szContentType)
{
	GString::StrCpy(m_szContentType, szContentType, 64);
}

void GHttpServer::MakeResponse(int nConnection, GHttpServerBuffer* pClient)
{
	if(pClient->m_eRequestType == GHttpServerBuffer::None)
		return;
	else if(pClient->m_eRequestType == GHttpServerBuffer::Get)
		DoGet(pClient->m_szUrl, pClient->m_szParams, m_pQ);
	else
		GAssert(false, "only GET is currently supported");

	char szPayloadSize[32];
	int nPayloadSize = m_pQ->GetSize();
	itoa(nPayloadSize, szPayloadSize, 10);

	// Send the header
	GQueue q;
	q.Push("HTTP/1.1 200 OK\r\nContent-Type: ");
	q.Push(m_szContentType);
	q.Push("\r\nContent-Length: ");
	q.Push(szPayloadSize);
	q.Push("\r\n\r\n");
	int nHeaderSize = q.GetSize();
	char* szHeader = q.DumpToString();
	Holder<char*> hHeader(szHeader);
	m_pSocket->Send(szHeader, nHeaderSize, nConnection);

	// Send the payload
	m_pQ->Push("\r\n\r\n");
	char* szPayload = m_pQ->DumpToString();
	Holder<char*> hPayload(szPayload);
	m_pSocket->Send(szPayload, nPayloadSize + 4, nConnection);
}

/*static*/ void GHttpServer::UnescapeUrl(char* szOut, const char* szIn)
{
	int c1, c2, n1, n2;
	while(*szIn != '\0')
	{
		if(*szIn == '%')
		{
			szIn++;
			n1 = *szIn;
			szIn++;
			n2 = *szIn;
			if(n1 >= '0' && n1 <= '9')
				c1 = n1 - '0';
			else if(n1 >= 'a' && n1 <= 'z')
				c1 = n1 - 'a' + 10;
			else if(n1 >= 'A' && n1 <= 'Z')
				c1 = n1 - 'A' + 10;
			else
				c1 = 2;
			if(n2 >= '0' && n2 <= '9')
				c2 = n2 - '0';
			else if(n2 >= 'a' && n2 <= 'z')
				c2 = n2 - 'a' + 10;
			else if(n2 >= 'A' && n2 <= 'Z')
				c2 = n2 - 'A' + 10;
			else
				c2 = 0;
			*szOut = 16 * c1 + c2;
		}
		else if(*szIn == '+')
			*szOut = ' ';
		else
			*szOut = *szIn;
		szIn++;
		szOut++;
	}
	*szOut = '\0';
}

/*static*/ void GHttpServer::ParseParams(GStringHeap* pStringHeap, GConstStringHashTable* pTable, const char* szParams)
{
	char szTmp[512];
	UnescapeUrl(szTmp, szParams);
	int nNameStart = 0;
	int nNameLen, nValueStart, nValueLen;
	while(true)
	{
		for(nNameLen = 0; szTmp[nNameStart + nNameLen] != '=' && szTmp[nNameStart + nNameLen] != '\0'; nNameLen++)
		{
		}
		if(szTmp[nNameStart + nNameLen] == '\0')
			return;
		nValueStart = nNameStart + nNameLen + 1;
		for(nValueLen = 0; szTmp[nValueStart + nValueLen] != '&' && szTmp[nValueStart + nValueLen] != '\0'; nValueLen++)
		{
		}
		const char* szName = pStringHeap->Add(&szTmp[nNameStart], nNameLen);
		const char* szValue = pStringHeap->Add(&szTmp[nValueStart], nValueLen);
		pTable->Add(szName, szValue);
		if(szTmp[nValueStart + nValueLen] == '\0')
			return;
		nNameStart = nValueStart + nValueLen + 1;
	}
}

