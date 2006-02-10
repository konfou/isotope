/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSOCKET_H__
#define __GSOCKET_H__

#include "GMacros.h"
#include "GArray.h"

#ifdef WIN32
#ifndef _MT
#error ERROR: _MT not defined.  GSocket requires using the multithreaded libraries
#define _MT
#endif // !_MT
#endif // WIN32


// Other Includes
#ifdef WIN32
#ifndef IPPROTO_IP
#include <winsock2.h>
#endif // IPPROTO_IP
#else // WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif // else WIN32


#ifndef WIN32
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent HOSTENT;
#endif // WIN32

class GPointerQueue;
class GSpinLock;
class GSocketArray;
class GHandleArray;
class GKeyPair;
class GSecureSocketServer;
class GSecureSocketClient;
class GPointerArray;
class GRand;
class GKeyPair;
class GIntArray;



class GSocketArray : public GSmallArray
{
public:
	GSocketArray(int nGrowBy) : GSmallArray(sizeof(SOCKET), nGrowBy) { }
	virtual ~GSocketArray() { }

	SOCKET GetSocket(int nIndex) { return *(SOCKET*)_GetCellRef(nIndex); }
	void AddSocket(SOCKET s) { _AddCellByRef(&s); }
	void SetSocket(int nCell, SOCKET s) { _SetCellByRef(nCell, &s); }
};




class GSocketClientBase
{
protected:
	bool m_bKeepListening;
	SOCKET m_s;
	HANDLE m_hListenThread;
	bool m_bUDP;

public:
	GSocketClientBase(bool bUDP);
	virtual ~GSocketClientBase();

	static bool IsThisAnIPAddress(const char* szHost);

	// This returns the SOCKET
	SOCKET GetSocketHandle();

	// You should only call Connect for a Client socket
	bool Connect(const char* szHost, unsigned short nPort);
	void Disconnect();

	bool Send(const unsigned char *pBuf, int nLen);

	// This method is abstract because you need to implement something here
	virtual bool Receive(unsigned char *pBuf, int nLen) = 0; // Override me


	u_short GetMyPort();
	struct in_addr GetMyIPAddr();
	bool IsConnected();
	char* GetMyIPAddr(char* szBuff, int nBuffSize);
	char* GetMyName(char* szBuff, int nBuffSize);
	u_short GetTheirPort();
	struct in_addr GetTheirIPAddr();
	char* GetTheirIPAddr(char* szBuff, int nBuffSize);
	char* GetTheirName(char* szBuff, int nBuffSize);

	void Listen(); // Don't call this method directly

	// This parses a URL into its parts
	static void ParseURL(const char* szUrl, int* pnHostIndex, int* pnPortIndex, int* pnPathIndex, int* pnParamsIndex);

	// Parses the parameter portion of a URL
	static int ParseUrlParams(const char* szParams, int nMaxParams, char** pNames, int* pNameLengths, char** pValues, int* pValueLengths);

protected:
	bool GoIntoHostMode(unsigned short nListenPort, int nMaxConnections);
	int GetFirstAvailableSocketNumber();
	void JoinListenThread();
	void JoinListenThread(int nConnectionNumber);

	// This method is empty. It's just here so you can override it.
	// This is called when the connection is gracefully closed. (There is no built-in support
	// for detecting ungraceful disconnects. This is a feature, not a bug, because it makes
	// it robust to sporadic hardware. I recommend implementing a system where the
	// server requires the client to send periodic heartbeat packets and you call Disconnect()
	// if the responses don't come regularly.)
	virtual void OnCloseConnection();
};








class GSocketServerBase
{
protected:
	SOCKET m_socketConnectionListener;
	GSocketArray* m_pConnections;
	fd_set m_socketSet; // structure used by select()
	HANDLE m_hWorkerThread;
	bool m_bKeepWorking;
	SOCKADDR_IN m_sHostAddrIn;
	bool m_bUDP;
	int m_nMaxConnections;
	char* m_szReceiveBuffer;

public:
	GSocketServerBase(bool bUDP, int nPort, int nMaxConnections);
	virtual ~GSocketServerBase();

	// This returns the SOCKET
	SOCKET GetSocketHandle(int nConnectionNumber);

	void Disconnect(int nConnectionNumber);

	bool Send(const unsigned char *pBuf, int nLen, int nConnectionNumber = 0);

	// This method is abstract because you need to implement something here
	virtual bool Receive(unsigned char *pBuf, int nLen, int nConnectionNumber) = 0; // Override me

	// These methods may only work if this is a client socket
	struct in_addr GetMyIPAddr();
	bool IsConnected(int nConnectionNumber = 0);
	u_short GetTheirPort(int nConnectionNumber = 0);
	struct in_addr GetTheirIPAddr(int nConnectionNumber = 0);
	char* GetTheirIPAddr(char* szBuff, int nBuffSize, int nConnectionNumber = 0);
	char* GetTheirName(char* szBuff, int nBuffSize, int nConnectionNumber = 0);

	void ServerWorker(); // Don't call this method directly

protected:
	void Init(bool bUDP, int nPort, int nMaxConnections);
	int GetFirstAvailableConnectionNumber();
	void JoinWorkerThread();

	// This method is empty. It's just here so you can override it.
	// This is called when the connection is gracefully closed. (There is no built-in support
	// for detecting ungraceful disconnects. This is a feature, not a bug, because it makes
	// it robust to sporadic hardware. To detect ungraceful disconnects, I recommend requiring
	// the client to send periodic heartbeat packets and calling Disconnect() if they stop coming.)
	virtual void OnCloseConnection(int nConnection);

	// This method is empty. It's just here so you can override it.
	// WARNING: the connection isn't fully open at the time this method is called,
	//          so don't send anything back to the client inside this callback
	virtual void OnAcceptConnection(int nConnection);

	SOCKET RefreshSocketSet();
	void HandleNewConnection();
	void ReduceConnectionList();
};



#endif // __GSOCKET_H__
