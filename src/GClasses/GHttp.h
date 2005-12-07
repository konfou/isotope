/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GHTTP_H__
#define __GHTTP_H__

class GHttpClientSocket;
class GEZSocketServer;
class GPointerArray;
class GHttpServerBuffer;
class GQueue;
class GStringHeap;
class GConstStringHashTable;


// This class allows you to get files using the HTTP protocol
class GHttpClient
{
public:
	enum Status
	{
		Downloading,
		Error,
		NotFound,
		Done,
	};

protected:
	char m_szHeaderBuf[258];
	char m_szServer[256];
	int m_nHeaderPos;
	int m_nContentSize;
	bool m_bChunked;
	unsigned char* m_pData;
	int m_nDataPos;
	GHttpClientSocket* m_pSocket;
	Status m_status;
	GQueue* m_pChunkQueue;
	bool m_bPastHeader;
	char* m_szRedirect;
	double m_dLastReceiveTime;

public:
	GHttpClient();
	~GHttpClient();

	// Send a request to get a file.  Returns immediately (before the file
	// is downloaded).
	bool Get(const char* szUrl, int nPort);

	// See what the status of the download is.  If everything is going okay,
	// it will return "Downloading" while downloading and "Done" when the file
	// is available.  pfProgress is an optional parameter.  If it is non-NULL,
	// it will return a number between 0 and 1 that indicates the ratio of
	// content (not including header data) already downloaded.
	Status CheckStatus(float* pfProgress);

	// Don't call this until the status is "Done".  It returns a pointer to the
	// file that was downloaded.  The buffer will be deleted when this object is
	// deleted, so if you want to retain the buffer, call DropData instead.
	unsigned char* GetData(int* pnSize);

	// Just like GetData except it forgets about the buffer so you'll have to
	// delete it yourself.
	unsigned char* DropData(int* pnSize);

	// This is called when the connection is lost
	void OnLoseConnection();

protected:
	void ProcessHeader(const unsigned char* szData, int nSize);
	void ProcessBody(const unsigned char* szData, int nSize);
	void ProcessChunkBody(const unsigned char* szData, int nSize);
};





#define MAX_SERVER_LINE_SIZE 300

// This class allows you to implement a simple HTTP daemon
class GHttpServer
{
protected:
	GEZSocketServer* m_pSocket;
	GPointerArray* m_pBuffers;
	GQueue* m_pQ;

public:
	GHttpServer(int nPort);
	virtual ~GHttpServer();

	// You should call this method constantly inside the main loop
	void Process();

	// Unescapes a URL. (i.e. replace "%20" with " ", etc.)
	static void UnescapeUrl(char* szOut, const char* szIn);

	// Parses the parameters in a URL and puts them in a table
	static void ParseParams(GStringHeap* pStringHeap, GConstStringHashTable* pTable, const char* szParams);

protected:
	virtual void OnProcessLine(int nConnection, const char* szLine) {}
	void ProcessLine(int nConnection, GHttpServerBuffer* pClient, const char* szLine);
	void MakeResponse(int nConnection, GHttpServerBuffer* pClient);
	virtual void DoGet(const char* szUrl, const char* szParams, GQueue* pResponse) = 0;
};


#endif // __GHTTP_H__
