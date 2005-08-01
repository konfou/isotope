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




// This class handles sockets.  You can make a client socket (that can connect
// to only one host) or a host socket (that can connect to many clients).  
class GSocket
{
protected:
	int m_nListenThreadCount;
	bool m_bKeepListening;
	int m_nAcceptorThreadCount;
	bool m_bKeepAccepting;
	SOCKET m_s;
	bool m_bIAmTheServer;
	int m_nSocketNumber;
	GSpinLock* m_pMutexSocketNumber;
	HANDLE m_hListenThread;
	HANDLE m_hConnectionAccepterThread;
	GSocketArray* m_pHostSockets;
	GHandleArray* m_pHostListenThreads;
	SOCKADDR_IN m_sHostAddrIn;
	bool m_bUDP;

public:
	GSocket();
	virtual ~GSocket();

	// This returns the SOCKET
	SOCKET GetSocketHandle(int nConnectionNumber);

	// if bIAmTheHost is true, you need to specify a port to host at
	// if bIAmTheHost is true, more than one connection can be made to it
	// if bIAmTheHost is false, you should call Connect() next to connect
	virtual bool Init(bool bUDP, bool bIAmTheServer, u_short nPort = 0, int nMaxConnections = SOMAXCONN);

	// You should only call Connect for a Client socket
	bool Connect(struct in_addr nAddr, u_short nPort, short nFamily = AF_INET);
	bool Connect(const char* szAddr, unsigned short nPort);
	bool Connect(const char* szURL);
	void Disconnect(int nConnectionNumber = 0);

	bool Send(const unsigned char *pBuf, int nLen, int nConnectionNumber = 0);

	// This method is abstract because you need to implement something here
	virtual bool Receive(unsigned char *pBuf, int nLen, int nConnectionNumber) = 0; // Override me

	// STATIC methods (you can call these from anywhere)
	static bool IsThisAnIPAddress(const char* szHost);
	static unsigned short StringToPort(const char* szURL);
	static struct in_addr StringToAddr(const char* szURL); // (If you pass in "localhost", it will return your IP address)

	// These methods may only work if this is a client socket
	u_short GetMyPort();
	struct in_addr GetMyIPAddr();
	bool IsConnected(int nConnectionNumber = 0);
	char* GetMyIPAddr(char* szBuff, int nBuffSize);
	char* GetMyName(char* szBuff, int nBuffSize);
	u_short GetTheirPort(int nConnectionNumber = 0);
	struct in_addr GetTheirIPAddr(int nConnectionNumber = 0);
	char* GetTheirIPAddr(char* szBuff, int nBuffSize, int nConnectionNumber = 0);
	char* GetTheirName(char* szBuff, int nBuffSize, int nConnectionNumber = 0);

	void Listen(); // Don't call this method directly
	void ConnectionAccepter(); // Don't call this method directly

	// This parses a URL into its parts
	static void ParseURL(const char* szBuff, char* szProtocall, char* szHost, char* szLoc, char* szPort, char* szParams);

	// Parses the parameter portion of a URL
	static int ParseUrlParams(const char* szParams, int nMaxParams, char** pNames, int* pNameLengths, char** pValues, int* pValueLengths);

protected:
	bool GoIntoHostMode(unsigned short nListenPort, int nMaxConnections);
	int GetFirstAvailableSocketNumber();
    void JoinAllListenThreads();
	void JoinAcceptorThread();
	void JoinListenThread(int nConnectionNumber);

	// These methods are all empty--override them if you want
	virtual void OnLoseConnection(int nSocketNumber);
	
	// WARNING: the connection isn't fully open at the time this method is called,
	//          so don't send anything back to the client inside this callback
	virtual void OnAcceptConnection(int nSocketNumber);
};

#endif // __GSOCKET_H__
