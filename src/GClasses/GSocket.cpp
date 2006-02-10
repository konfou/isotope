/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GSocket.h"
#include <time.h>
#include "GSpinLock.h"
#include "GArray.h"
#include "GMacros.h"
#include "GThread.h"
#include "GString.h"
#include <stdarg.h>
#include <wchar.h>
#ifdef WIN32
#include "GWindows.h"
#include <Ws2tcpip.h>
#else // WIN32
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>

#define SOCKET_ERROR -1
#endif // !WIN32

wchar_t* g_wszErrorMessage = NULL;

void ThrowError(const wchar_t* wszFormat, ...)
{
	// Measure the required buffer size--todo: there might be a function like "vscwprintf" that does this, but I couldn't find it so I kludged my own
	int nSize = 0;
	{
		va_list args;
		const wchar_t* wsz = wszFormat;
		va_start(args, wszFormat);
		{
			while(*wsz != L'\0')
			{
				if(*wsz == L'%')
				{
					wsz++;
					switch(*wsz)
					{
						case L'c':
							nSize += 2;
							va_arg(args, int/*wchar_t*/);
							break;
						case L's':
							nSize += wcslen(va_arg(args, wchar_t*));
							break;
						case L'd':
							nSize += 10;
							va_arg(args, int);
							break;
						case L'l':
							nSize += 20;
							va_arg(args, double);
							break;
						case L'f':
							nSize += 20;
							va_arg(args, double/*float*/);
							break;
						default:
							nSize += 20; // take a guess
							break;
					}
				}
				wsz++;
				nSize++;
			}
		}
		va_end (args);
	}
	nSize++;

	// Allocate the buffer
	delete(g_wszErrorMessage);
	g_wszErrorMessage = new wchar_t[nSize + 1];

	// Format the message
	{
		va_list args;
		va_start(args, wszFormat);
		{
#ifdef _MBCS
			int res = vswprintf(g_wszErrorMessage, wszFormat, args);
#else
			int res = vswprintf(g_wszErrorMessage, nSize, wszFormat, args);
#endif
			if(res < 0)
			{
				GAssert(false, "Error formatting string");
			}
		}
		va_end (args);
	}

	// Throw the error
	throw (const wchar_t*)g_wszErrorMessage;
}

void gsocket_LogError()
{
	const wchar_t* wszMsg = NULL;
#ifdef WIN32
	int n = WSAGetLastError();
	switch(n)
	{
		case WSAECONNRESET: 		wszMsg = L"An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call."; break;
		case WSAEFAULT: 			wszMsg = L"The addrlen parameter is too small or addr is not a valid part of the user address space."; break;
		case WSAEINTR: 				wszMsg = L"A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall."; break;
		case WSAEINVAL: 			wszMsg = L"The listen function was not invoked prior to accept."; break;
		case WSAEINPROGRESS: 		wszMsg = L"A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."; break;
		case WSAEMFILE: 			wszMsg = L"The queue is nonempty upon entry to accept and there are no descriptors available."; break;
		case WSAENETDOWN: 			wszMsg = L"The network subsystem has failed."; break;
		case WSAENOBUFS: 			wszMsg = L"No buffer space is available."; break;
		case WSAENOTSOCK: 			wszMsg = L"The descriptor is not a socket."; break;
		case WSAEOPNOTSUPP: 		wszMsg = L"The referenced socket is not a type that supports connection-oriented service."; break;
		case WSAEWOULDBLOCK: 		wszMsg = L"The socket is marked as nonblocking and no connections are present to be accepted."; break;
		case WSANOTINITIALISED:		wszMsg = L"A successful WSAStartup must occur before using this function.";   break;
		case WSAEALREADY:			wszMsg = L"A nonblocking connect call is in progress on the specified socket.";   break;
		case WSAEADDRNOTAVAIL:		wszMsg = L"The remote address is not a valid address (such as ADDR_ANY).";   break;
		case WSAEAFNOSUPPORT:		wszMsg = L"Addresses in the specified family cannot be used with this socket.";   break;
		case WSAECONNREFUSED:		wszMsg = L"The attempt to connect was forcefully rejected.";   break;
		case WSAEISCONN:			wszMsg = L"The socket is already connected (connection-oriented sockets only).";   break;
		case WSAENETUNREACH:		wszMsg = L"The network cannot be reached from this host at this time.";   break;
		case WSAETIMEDOUT:			wszMsg = L"Attempt to connect timed out without establishing a connection.";   break;
		case WSASYSNOTREADY:		wszMsg = L"network subsystem not ready for communication.";   break;
		case WSAVERNOTSUPPORTED:	wszMsg = L"The version of Windows Sockets support requested is not provided by this implementation.";   break;
		case WSAEPROCLIM:			wszMsg = L"Limit on the number of tasks supported has been reached.";   break;
		case WSAEHOSTUNREACH:		wszMsg = L"Host unreacheable"; break;
		case WSAENOTCONN:			wszMsg = L"Not Connected"; break;
		case WSAECONNABORTED:		wszMsg = L"Connection Aborted"; break;
		case 0x2740:				wszMsg = L"Port already in use"; break;
		case WSAHOST_NOT_FOUND: 	wszMsg = L"Authoritative answer host not found."; break;
		case WSATRY_AGAIN: 			wszMsg = L"Nonauthoritative host not found, or server failure."; break;
		case WSANO_RECOVERY: 		wszMsg = L"A nonrecoverable error occurred."; break;
		case WSANO_DATA: 			wszMsg = L"Valid name, no data record of requested type."; break;
		default:					wszMsg = L"An unrecognized socket error occurred"; break;
	}
#else // WIN32
	switch(errno)
	{
		case EBADF:			wszMsg = L"not a valid socket descriptor."; break;
		case EINVAL:		wszMsg = L"The socket is already bound to an address or addrlen is wrong."; break;
		case EACCES:		wszMsg = L"Access permission is denied."; break;
		case ENOTSOCK:		wszMsg = L"Argument is a descriptor for a file, not a socket."; break;
		case EROFS:			wszMsg = L"The  socket inode would reside on a read-only file system."; break;
		case EFAULT:		wszMsg = L"the addr parameter points outside the user's accessible address space."; break;
		case ENAMETOOLONG:	wszMsg = L"A component of a pathname exceeded {NAME_MAX} characters, or an entire path name exceeded {PATH_MAX} characters."; break;
		case ENOENT:		wszMsg = L"The file or named socket does not exist."; break;
		case ENOMEM:		wszMsg = L"Insufficient kernel memory was available."; break;
		case ENOTDIR:		wszMsg = L"A component of the path prefix is not a directory."; break;
		case ELOOP:			wszMsg = L"Too many symbolic links were encountered in resolving my_addr."; break;
		case EOPNOTSUPP:	wszMsg = L"The referenced socket is not of type SOCK_STREAM."; break;
		case EWOULDBLOCK:	wszMsg = L"The socket is marked non-blocking and no connections are present to be accepted."; break;
		case EMFILE:		wszMsg = L"The per-process descriptor table is full."; break;
		case ENFILE:		wszMsg = L"The system file table is full."; break;
		case EADDRNOTAVAIL:	wszMsg = L"The specified address is not available on this machine."; break;
		case EAFNOSUPPORT:	wszMsg = L"Addresses in the specified address family cannot be used with this socket."; break;
		case EISCONN:		wszMsg = L"The socket is already connected."; break;
		case ETIMEDOUT:		wszMsg = L"Connection establishment timed out without establishing a connection."; break;
		case ECONNREFUSED:	wszMsg = L"The attempt to connect was forcefully rejected."; break;
		case ENETUNREACH:	wszMsg = L"The network isn't reachable from this host."; break;
		case EADDRINUSE:	wszMsg = L"The address is already in use."; break;
		case EINPROGRESS:	wszMsg = L"The socket is non-blocking and the connection cannot be completed immediately.  It is possible to select(2) for completion by selecting the socket for writing."; break;
		case EALREADY:		wszMsg = L"The socket is non-blocking and a previous connection attempt has not yet been completed."; break;
		case HOST_NOT_FOUND:	wszMsg = L"The specified host is unknown."; break;
		case NO_ADDRESS:	wszMsg = L"The requested name is valid but does not have an IP address."; break;
		case NO_RECOVERY:	wszMsg = L"A non-recoverable name server error occurred."; break;
		//case TRY_AGAIN:		wszMsg = L"A temporary error occurred on an authoritative name server.  Try again later."; break;
		default:		wszMsg = L"An unrecognized socket error occurred"; break;
	}
#endif // else WIN32
	ThrowError(wszMsg);
}

inline void SetSocketToBlockingMode(SOCKET s)
{
	unsigned long ulMode = 0;
#ifdef WIN32
	if(ioctlsocket(s, FIONBIO, &ulMode) != 0)
#else // WIN32
	if(ioctl(s, FIONBIO, &ulMode) != 0)
#endif // !WIN32
	{
		gsocket_LogError();
	}
}

inline void SetSocketToNonBlockingMode(SOCKET s)
{
	unsigned long ulMode = 1;
#ifdef WIN32
	if(ioctlsocket(s, FIONBIO, &ulMode) != 0)
#else // WIN32
	if(ioctl(s, FIONBIO, &ulMode) != 0)
#endif // WIN32
		gsocket_LogError();
}

inline void CloseSocket(SOCKET s)
{
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif // WIN32
}


#ifdef WIN32
bool gsocket_InitWinSock()
{
	// Initializing Winsock
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 
	wVersionRequested = MAKEWORD(1, 1); 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		GAssert(false, "Failed to find a usable WinSock DLL\n");
		return false;
	}

	// Confirm that the WinSock DLL supports 2.2.
	// Note that if the DLL supports versions greater
	// than 2.2 in addition to 2.2, it will still return
	// 2.2 in wVersion since that is the version we
	// requested.
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
			HIBYTE( wsaData.wVersion ) != 1 ) 
	{
		int n1 = LOBYTE( wsaData.wVersion );
		int n2 = HIBYTE( wsaData.wVersion );
		GAssert(false, "Found a Winsock DLL, but it only supports an older version.  It needs to support version 2.2");
		WSACleanup();
		return false; 
	}
	return true;
}
#endif // WIN32

// ------------------------------------------------------------------------------

GSocketClientBase::GSocketClientBase(bool bUDP)
{
	m_hListenThread = BAD_HANDLE;
	m_s = INVALID_SOCKET;
	m_bKeepListening = true;
	m_bUDP = bUDP;
#ifdef WIN32
	if(!gsocket_InitWinSock())
		throw "Error initializing WinSock";
#endif // WIN32
}

GSocketClientBase::~GSocketClientBase()
{
	Disconnect();
}

void GSocketClientBase::JoinListenThread()
{
	m_bKeepListening = false;
	time_t tStart;
	time_t tNow;
	time(&tStart);
	while(m_hListenThread != BAD_HANDLE)
	{
		GThread::sleep(0);
		time(&tNow);
		if(tNow - tStart > 4)
		{
			GAssert(false, "Error, took too long for the listen thread to exit");
			break;
		}
	}
}

void GSocketClientBase::JoinListenThread(int nConnectionNumber)
{
	GAssert(false, "Error, not implemented yet");
}

void GSocketClientBase::Listen()
{
	char szReceiveBuff[520];
	SOCKET s = m_s;

	// Mark the socket as blocking so we can call "recv" which is a blocking operation
	SetSocketToBlockingMode(s);

	// Start receiving messages
	unsigned long dwBytesReadyToRead;
	int nBytesRead = 0;
	while(m_bKeepListening)
	{
		// See how much data is ready to be received
#ifdef WIN32
		GWindows::YieldToWindows(); // This is necessary because incoming packets go through the Windows message pump
		if(ioctlsocket(s, FIONREAD, &dwBytesReadyToRead) != 0)
			gsocket_LogError();
#else // WIN32
		if(ioctl(s, FIONREAD, &dwBytesReadyToRead) != 0)
			gsocket_LogError();
#endif // !WIN32
		if(dwBytesReadyToRead > 0)
		{
			nBytesRead = recv(s, szReceiveBuff, 512, 0); // read from the queue (This blocks until there is some to read or connection is closed, but we already know there is something to read)
			if(nBytesRead > 0) // recv reads in as much data as is currently available up to the size of the buffer
				Receive((unsigned char*)szReceiveBuff, nBytesRead);
			else
				break; // The socket has been closed
		}
		else
		{
			// There's nothing to receive, so let's sleep for a while
			GThread::sleep(50);
		}
	}

#ifdef WIN32
	if(nBytesRead == SOCKET_ERROR)
	{
		int n = WSAGetLastError();
		switch(n)
		{
			case WSAECONNABORTED:	break;
			case WSAECONNRESET:		break;
			default:				gsocket_LogError();		break;
		}
	}
#endif // WIN32

	OnCloseConnection();
	shutdown(m_s, 2);
	CloseSocket(m_s);
	m_s = INVALID_SOCKET;
	m_hListenThread = BAD_HANDLE;
}

unsigned int ListenThread(void* pData)
{
	((GSocketClientBase*)pData)->Listen();
	return 0;
}

/*virtual*/ void GSocketClientBase::OnCloseConnection()
{
}

bool GSocketClientBase::IsThisAnIPAddress(const char* szHost)
{
	int n;
	for(n = 0; szHost[n] != '.' && szHost[n] != '\0'; n++)
	{
		if(szHost[n] < '0' || szHost[n] > '9')
			return false;
	}
	if(szHost[n] == '.')
	{
		for(n++; szHost[n] != '.' && szHost[n] != '\0'; n++)
		{
			if(szHost[n] < '0' || szHost[n] > '9')
				return false;
		}
	}
	return true;
}


// This is for parsing a URL
//static
void GSocketClientBase::ParseURL(const char* szUrl, int* pnHostIndex, int* pnPortIndex, int* pnPathIndex, int* pnParamsIndex)
{
	// Find the host
	int nHost = 0;
	int i;
	for(i = 0; szUrl[i] != ':' && szUrl[i] != '?' && szUrl[i] != '\0'; i++)
	{
	}
	if(strncmp(&szUrl[i], "://", 3) == 0)
		nHost = i + 3;

	// Find the port
	int nPort = -1;
	for(i = nHost; szUrl[i] != ':' && szUrl[i] != '?' && szUrl[i] != '\0'; i++)
	{
	}
	if(szUrl[i] == ':')
		nPort = i;

	// Find the path
	int nPath;
	for(nPath = MAX(nHost, nPort); szUrl[nPath] != '/' && szUrl[nPath] != '?' && szUrl[nPath] != '\0'; nPath++)
	{
	}
	if(nPort < 0)
		nPort = nPath;

	// Find the params
	if(pnParamsIndex)
	{
		int nParams;
		for(nParams = nPath; szUrl[nParams] != '?' && szUrl[nParams] != '\0'; nParams++)
		{
		}
		*pnParamsIndex = nParams;
	}

	// Set the return values
	if(pnHostIndex)
		*pnHostIndex = nHost;
	if(pnPortIndex)
		*pnPortIndex = nPort;
	if(pnPathIndex)
		*pnPathIndex = nPath;
}

//static
int GSocketClientBase::ParseUrlParams(const char* szParams, int nMaxParams, char** pNames, int* pNameLengths, char** pValues, int* pValueLengths)
{
	if(*szParams == '?')
		szParams++;
	int nParams = 0;
	while(true)
	{
		if(*szParams == '\0' || nParams >= nMaxParams)
			return nParams;
		pNames[nParams] = (char*)szParams;
		pNameLengths[nParams] = 0;
		while(*szParams != '\0' && *szParams != '=' && *szParams != '&')
		{
			szParams++;
			pNameLengths[nParams]++;
		}
		if(*szParams == '=')
			szParams++;
		pValues[nParams] = (char*)szParams;
		pValueLengths[nParams] = 0;
		while(*szParams != '\0' && *szParams != '&')
		{
			szParams++;
			pValueLengths[nParams]++;
		}
		if(*szParams == '&')
			szParams++;
		nParams++;
	}
}
/*
in_addr GSocketClientBase::StringToAddr(const char* szURL)
{
	// Extract the host and port from the URL
	GTEMPBUF(szHost, strlen(szURL));
	ParseURL(szURL, NULL, szHost, NULL, NULL, NULL);

	// Determine if it is a friendly-URL or an IP address
	if(IsThisAnIPAddress(szHost))
	{
		in_addr iaTmp;
#ifdef WIN32
		iaTmp.S_un.S_addr = inet_addr(szHost);
#else // WIN32
		iaTmp.s_addr = inet_addr(szHost);
#endif // else WIN32
		return iaTmp;
	}
	else
	{
		struct hostent* psh = gethostbyname(szHost);
		if(!psh)
		{
			gsocket_LogError();
			in_addr iaTmp;
#ifdef WIN32
			iaTmp.S_un.S_addr = NULL;
#else // WIN32
            iaTmp.s_addr = 0;
#endif // else WIN32
			return iaTmp;
		}
		return *(in_addr*)psh->h_addr_list[0];
	}
}
*/

bool GSocketClientBase::Connect(const char* szHost, unsigned short nPort)
{
	// *** If you use VisualStudio 6.0 and you get an error that says 'hints' uses undefined struct 'addrinfo'
	// *** on the next code line then you need to update your Platform SDK.
	struct addrinfo hints, *res, *res0;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	char szPort[32];
	itoa(nPort, szPort, 10);
	error = getaddrinfo(szHost, szPort, &hints, &res0);
	if (error)
	{
		GAssert(false, gai_strerror(error));
		gsocket_LogError();
	}
	m_s = INVALID_SOCKET;
	for(res = res0; res; res = res->ai_next)
	{
//printf("Attempting to connect to %d.%d.%d.%d on port %d... ", ((unsigned char*)&res->ai_addr->sa_data)[2], ((unsigned char*)&res->ai_addr->sa_data)[3], ((unsigned char*)&res->ai_addr->sa_data)[4], ((unsigned char*)&res->ai_addr->sa_data)[5], nPort);

		m_s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(m_s < 0)
			continue;

		if(connect(m_s, res->ai_addr, res->ai_addrlen) < 0)
		{
			CloseSocket(m_s);
			m_s = INVALID_SOCKET;
//printf("Failed.\n");
			continue;
		}

//printf("OK\n");
		break;  // we got a connection
	}
	freeaddrinfo(res0);
	if(m_s == INVALID_SOCKET)
	{
		// todo: handle the error
		return false;
	}

	// Spawn the listener thread
	m_hListenThread = GThread::SpawnThread(ListenThread, this);
	if(m_hListenThread == BAD_HANDLE)
	{
		GAssert(false, "Failed to spawn listening thread\n");
		gsocket_LogError();
		return false;
	}
	GThread::sleep(0);
	
	return true;
}

void GSocketClientBase::Disconnect()
{
	JoinListenThread();

	// Disconnect the connection
	if(m_s != INVALID_SOCKET)
	{
		shutdown(m_s, 2);
		CloseSocket(m_s);
		m_s = INVALID_SOCKET;
	}
}

bool GSocketClientBase::Send(const unsigned char *pBuff, int len)
{
	if(send(m_s, (const char*)pBuff, len, 0) == SOCKET_ERROR)
	{
#ifdef WIN32		
		int n = WSAGetLastError();
		switch(n)
		{
			case WSAECONNABORTED:	break;
			case WSAECONNRESET:		break;
			default:	gsocket_LogError();		break;
		}
#endif // WIN32
		return false;
	}

	return true;
}

bool GSocketClientBase::IsConnected()
{
	if(m_hListenThread == BAD_HANDLE)
		return false;
	else
		return true;
}

SOCKET GSocketClientBase::GetSocketHandle()
{
	return m_s;
}

in_addr GSocketClientBase::GetMyIPAddr()
{
	struct sockaddr sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(getsockname(m_s, &sAddr, &l))
	{
		gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return pInfo->sin_addr;
}

char* GSocketClientBase::GetMyIPAddr(char* szBuff, int nBuffSize)
{
	GString::StrCpy(szBuff, inet_ntoa(GetMyIPAddr()), nBuffSize);
	return szBuff;
}

u_short GSocketClientBase::GetMyPort()
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(getsockname(m_s, &sAddr, &l))
	{
		gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return htons(pInfo->sin_port);
}

char* GSocketClientBase::GetMyName(char* szBuff, int nBuffSize)
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(getsockname(m_s, &sAddr, &l))
	{
		gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	HOSTENT* namestruct = gethostbyaddr((const char*)&pInfo->sin_addr, 4, pInfo->sin_family);
	if(!namestruct)
	{
		GAssert(false, "Error calling gethostbyaddr\n");
	}
	GString::StrCpy(szBuff, namestruct->h_name, nBuffSize);
	return(szBuff);
}

in_addr GSocketClientBase::GetTheirIPAddr()
{
	struct sockaddr sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
//	if(nConnectionNumber == 0)
//	{
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
//	}
//	else
//	{
//		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
//			gsocket_LogError();
//	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return pInfo->sin_addr;
}

char* GSocketClientBase::GetTheirIPAddr(char* szBuff, int nBuffSize)
{
	GString::StrCpy(szBuff, inet_ntoa(GetTheirIPAddr()), nBuffSize);
	return szBuff;
}

u_short GSocketClientBase::GetTheirPort()
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
//	if(nConnectionNumber == 0)
//	{
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
//	}
//	else
//	{
//		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
//			gsocket_LogError();
//	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return htons(pInfo->sin_port);
}

char* GSocketClientBase::GetTheirName(char* szBuff, int nBuffSize)
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
//	if(nConnectionNumber == 0)
//	{
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
//	}
//	else
//	{
//		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
//			gsocket_LogError();
//	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	HOSTENT* namestruct = gethostbyaddr((const char*)&pInfo->sin_addr, 4, pInfo->sin_family);
	if(!namestruct)
	{
		GAssert(false, "Error calling gethostbyaddr\n");
	}
	GString::StrCpy(szBuff, namestruct->h_name, nBuffSize);
	return(szBuff);
}

// ------------------------------------------------------------------------------

GSocketServerBase::GSocketServerBase(bool bUDP, int nPort, int nMaxConnections)
{
	m_hWorkerThread = BAD_HANDLE;
	m_socketConnectionListener = INVALID_SOCKET;
	m_bKeepWorking = true;
	m_bUDP = false;
	m_szReceiveBuffer = new char[2048];
	m_pConnections = new GSocketArray(16);
	Init(bUDP, nPort, nMaxConnections);
}

GSocketServerBase::~GSocketServerBase()
{
	JoinWorkerThread();

	// Disconnect the connection
	if(m_socketConnectionListener != INVALID_SOCKET)
	{
		shutdown(m_socketConnectionListener, 2);
		CloseSocket(m_socketConnectionListener);
		m_socketConnectionListener = INVALID_SOCKET;
	}

	// Disconnect all the connections
	int nCount = m_pConnections->GetSize();
	int n;
	SOCKET Sock;
	for(n = 0; n < nCount; n++)
	{
		Sock = m_pConnections->GetSocket(n);
		if(Sock != INVALID_SOCKET)
		{
			shutdown(Sock, 2);
			CloseSocket(Sock);
			m_pConnections->SetSocket(n, INVALID_SOCKET);
		}
	}

	// Delete the Host Arrays
	delete(m_pConnections);
	delete(m_szReceiveBuffer);
}

void GSocketServerBase::JoinWorkerThread()
{
	m_bKeepWorking = false;
	time_t tStart;
	time_t tNow;
	time(&tStart);
	while(m_hWorkerThread != BAD_HANDLE)
	{
		GThread::sleep(0);
		time(&tNow);
		if(tNow - tStart > 4)
		{
			GAssert(false, "Error, took too long for the worker thread to exit");
			break;
		}
	}
}

int GSocketServerBase::GetFirstAvailableConnectionNumber()
{
	// Find the first empty Handle slot for the listening thread
	int nSize = m_pConnections->GetSize();
	int nSocketNumber = -1;
	int n;
	for(n = 0; n < nSize; n++)
	{
		if(m_pConnections->GetSocket(n) == INVALID_SOCKET)
		{
			nSocketNumber = n;
			break;
		}
	}

	// Add a new slot if we couldn't find one
	if(nSocketNumber < 0 && m_pConnections->GetSize() < m_nMaxConnections)
	{
		nSocketNumber = nSize;
		m_pConnections->AddSocket(INVALID_SOCKET);
	}

	return nSocketNumber;
}

SOCKET GSocketServerBase::RefreshSocketSet()
{
	// Clear the set
	FD_ZERO(&m_socketSet);

	// Add the connection listener socket so that select() will return if a new connection comes in
	SOCKET highSocket = m_socketConnectionListener;
	FD_SET(m_socketConnectionListener, &m_socketSet);

	// Add all the current connections to the set
	int nCount = m_pConnections->GetSize();
	SOCKET s;
	int n;
	for(n = 0; n < nCount; n++)
	{
		s = m_pConnections->GetSocket(n);
		if(s != INVALID_SOCKET)
		{
			FD_SET(s, &m_socketSet);
			if(s > highSocket)
				highSocket = s;
		}
	}
	return highSocket;
}

void GSocketServerBase::HandleNewConnection()
{
	// Accept the connection
	SOCKET s;
	SOCKADDR_IN sHostAddrIn;
	socklen_t nStructSize = sizeof(struct sockaddr);
	s = accept(m_socketConnectionListener, (struct sockaddr*)&sHostAddrIn, &nStructSize);

	// Set the connection to non-blocking mode
	SetSocketToNonBlockingMode(s);

	// Find a place for the new socket
	int nConnection = GetFirstAvailableConnectionNumber();
	if(nConnection < 0)
	{
		GAssert(false, "no room for this connection");

		// Why did we accept the connection if we don't have room for it? So we
		// can close it so it won't keep bugging us about accepting it.
		CloseSocket(s);
		return;
	}
	m_pConnections->SetSocket(nConnection, s);
	// WARNING: the accept function will return as soon as it gets
	//         an ACK packet back from the client, but the connection
	//         isn't actually established until more data is
	//         received.  Therefore, if you try to send data immediately
	//         (which someone might want to do in OnAcceptConnetion, the
	//         data might be lost since the connection might not be
	//         fully open.
	OnAcceptConnection(nConnection);
}

unsigned int ServerWorkerThread(void* pData)
{
	((GSocketServerBase*)pData)->ServerWorker();
	return 0;
}

void GSocketServerBase::ServerWorker()
{
#ifdef WIN32
	GWindows::YieldToWindows();
#endif // else WIN32
	int n, nCount, nBytes;
	struct timeval timeout;
	int nReadySocketCount; // the number of sockets ready for reading
	SOCKET s, highSocket;
	while(m_bKeepWorking)
	{
		// We need to refresh the socket set each time we loop because select() changes the set
		highSocket = RefreshSocketSet();

		// Check which sockets are ready for reading
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		nReadySocketCount = select(highSocket + 1, &m_socketSet, NULL, NULL, &timeout);
		// Handle errors
		if(nReadySocketCount < 0)
		{
			gsocket_LogError();
			break;
		}

		// Read from the ready sockets
		if(nReadySocketCount > 0)
		{
			// Check the connection listener socket for incoming connections
			if(FD_ISSET(m_socketConnectionListener, &m_socketSet))
			{
				HandleNewConnection();
			}

			// Check each connection socket for incoming data
			nCount = m_pConnections->GetSize();
			for(n = 0; n < nCount; n++)
			{
				s = m_pConnections->GetSocket(n);
				if(s != INVALID_SOCKET && FD_ISSET(s, &m_socketSet))
				{
					// The recv() function blocks until there is some to read or connection is closed, but we already know there is something to read
					nBytes = recv(s, m_szReceiveBuffer, 2048, 0);
					if(nBytes > 0)
					{
						Receive((unsigned char*)m_szReceiveBuffer, nBytes, n);
					}
					else
					{
						// The socket was closed or an error occurred. Either way, close the socket
						OnCloseConnection(n);
						CloseSocket(s);
						m_pConnections->SetSocket(n, INVALID_SOCKET); // todo: do we need a mutex protecting m_pConnections?
						ReduceConnectionList();
					}
				}
			}
		}
		else
			GThread::sleep(100);
	}
	m_hWorkerThread = BAD_HANDLE;
}

void GSocketServerBase::Init(bool bUDP, int nPort, int nMaxConnections)
{
	m_nMaxConnections = nMaxConnections;
#ifdef WIN32
	if(!gsocket_InitWinSock())
		throw "failed to init WinSock";
#endif // WIN32

	GAssert(nPort > 0, "invalid port number");
	if(m_bUDP)
	{
		GAssert(false, "UDP not implemented yet");
	}
	m_bUDP = bUDP;

	// Make the Socket
	m_socketConnectionListener = socket(AF_INET, m_bUDP ? SOCK_DGRAM : SOCK_STREAM, 0);
	if(m_socketConnectionListener == INVALID_SOCKET)
	{
		gsocket_LogError();
		throw "faled to make a socket";
	}

	// Put the socket into non-blocking mode (so the call to "accept" will return immediately
	// if there are no connections in the queue ready to be accepted)
	SetSocketToNonBlockingMode(m_socketConnectionListener);

	// Tell the socket that it's okay to reuse an old crashed socket that hasn't timed out yet
	int flag = 1;
	setsockopt(m_socketConnectionListener, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag)); 

	// Prepare the socket for accepting
	memset(&m_sHostAddrIn, '\0', sizeof(SOCKADDR_IN));
	m_sHostAddrIn.sin_family = AF_INET;
	m_sHostAddrIn.sin_port = htons((u_short)nPort);
	m_sHostAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(m_socketConnectionListener, (struct sockaddr*)&m_sHostAddrIn, sizeof(SOCKADDR)))
	{
		gsocket_LogError();
		throw "failed to bind a socket";
	}

	// Start listening for connections
	if(listen(m_socketConnectionListener, nMaxConnections))
	{
		gsocket_LogError();
		throw "Failed to listen on a socket";
	}

	// Spawn the worker thread
	m_hWorkerThread = GThread::SpawnThread(ServerWorkerThread, this);
	if(m_hWorkerThread == BAD_HANDLE)
	{
		GAssert(false, "Error spawning server worker thread");
		throw "Failed to spawn worker thread";
	}

	// Give the worker thread a chance to awake
	GThread::sleep(0);
}

void GSocketServerBase::ReduceConnectionList()
{
	// todo: do we need a mutex protecting m_pConnections?
	while(true)
	{
		int n = m_pConnections->GetSize();
		if(n <= 0)
			break;
		if(m_pConnections->GetSocket(n - 1) != INVALID_SOCKET)
			break;
		m_pConnections->DeleteCell(n - 1);
	}
}

void GSocketServerBase::Disconnect(int nConnectionNumber)
{
	GAssert(nConnectionNumber >= 0 && nConnectionNumber < m_pConnections->GetSize(), "connection out of range");
	SOCKET s = m_pConnections->GetSocket(nConnectionNumber);
	if(s != INVALID_SOCKET)
	{
		OnCloseConnection(nConnectionNumber);
		shutdown(s, 2);
		m_pConnections->SetSocket(nConnectionNumber, INVALID_SOCKET);
		CloseSocket(s);
		ReduceConnectionList();
	}
}

bool GSocketServerBase::Send(const unsigned char *pBuff, int len, int nConnectionNumber)
{
	SOCKET s = m_pConnections->GetSocket(nConnectionNumber);
	GAssert(s != SOCKET_ERROR, "Bad socket");
	if(send(s, (const char*)pBuff, len, 0) == SOCKET_ERROR)
	{
#ifdef WIN32		
		int n = WSAGetLastError();
		switch(n)
		{
			case WSAECONNABORTED:	break;
			case WSAECONNRESET:		break;
			default:	gsocket_LogError();		break;
		}
#endif // WIN32
		return false;
	}

	return true;
}

void GSocketServerBase::OnCloseConnection(int nConnection)
{

}

void GSocketServerBase::OnAcceptConnection(int nConnection)
{

}

bool GSocketServerBase::IsConnected(int nConnectionNumber)
{
	GAssert(false, "Not implemented yet");
	/*
	if(nConnectionNumber == 0)
	{
		if(m_hWorkerThread == BAD_HANDLE)
			return false;
		else
			return true;
	}
	else
	{
		if(m_pHostListenThreads->GetSize() < nConnectionNumber)
			return false;
		if(m_pHostListenThreads->GetHandle(nConnectionNumber - 1) == BAD_HANDLE)
			return false;
		else
			return true;
	}
	*/
	return false;
}

SOCKET GSocketServerBase::GetSocketHandle(int nConnectionNumber)
{
	if(nConnectionNumber < 0)
		return m_socketConnectionListener;
	else
		return m_pConnections->GetSocket(nConnectionNumber);
}

in_addr GSocketServerBase::GetTheirIPAddr(int nConnectionNumber)
{
	struct sockaddr sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(nConnectionNumber == 0)
	{
		if(getpeername(m_socketConnectionListener, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pConnections->GetSocket(nConnectionNumber), &sAddr, &l))
			gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return pInfo->sin_addr;
}

char* GSocketServerBase::GetTheirIPAddr(char* szBuff, int nBuffSize, int nConnectionNumber)
{
	GString::StrCpy(szBuff, inet_ntoa(GetTheirIPAddr(nConnectionNumber)), nBuffSize);
	return szBuff;
}

u_short GSocketServerBase::GetTheirPort(int nConnectionNumber)
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(nConnectionNumber == 0)
	{
		if(getpeername(m_socketConnectionListener, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pConnections->GetSocket(nConnectionNumber), &sAddr, &l))
			gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return htons(pInfo->sin_port);
}

char* GSocketServerBase::GetTheirName(char* szBuff, int nBuffSize, int nConnectionNumber)
{
	SOCKADDR sAddr;
#ifdef WIN32
	int l;
#else // WIN32
	unsigned int l;
#endif // else WIN32
	l = sizeof(SOCKADDR);
	if(nConnectionNumber == 0)
	{
		if(getpeername(m_socketConnectionListener, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pConnections->GetSocket(nConnectionNumber), &sAddr, &l))
			gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	HOSTENT* namestruct = gethostbyaddr((const char*)&pInfo->sin_addr, 4, pInfo->sin_family);
	if(!namestruct)
	{
		GAssert(false, "Error calling gethostbyaddr\n");
	}
	GString::StrCpy(szBuff, namestruct->h_name, nBuffSize);
	return(szBuff);
}

