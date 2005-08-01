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

class GEZSocketClient;

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
	int m_nHeaderPos;
	int m_nContentSize;
	unsigned char* m_pData;
	int m_nDataPos;
	GEZSocketClient* m_pSocket;
	Status m_status;

public:
	GHttpClient();
	~GHttpClient();

	// Send a request to get a file.  Returns immediately (before the file
	// is downloaded).
	bool Get(const char* szUrl, int nPort);
	
	// See what the status of the download is.  If everything is going okay,
	// it will return "Downloading" while downloading and "Done" when the file
	// is available
	Status CheckStatus();

	// Don't call this until the status is "Done".  It returns a pointer to the
	// file that was downloaded.  The buffer will be deleted when this object is
	// deleted, so if you want to retain the buffer, call DropData instead.
	unsigned char* GetData(int* pnSize);

	// Just like GetData except it forgets about the buffer so you'll have to
	// delete it yourself.
	unsigned char* DropData(int* pnSize);

protected:
	void ProcessHeader(const unsigned char* szData, int nSize);
	void ProcessBody(const unsigned char* szData, int nSize);
};

#endif // __GHTTP_H__
