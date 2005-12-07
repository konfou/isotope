/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

//#define OLD_NAME_RESOLUTION

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
			GAssert(res >= 0, "Error formatting string");
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

//////////////////////////////////////////////////////////////////

GSocket::GSocket()
{
	m_pHostSockets = NULL;
	m_pHostListenThreads = NULL;
	m_hListenThread = BAD_HANDLE;
	m_hConnectionAccepterThread = BAD_HANDLE;
	m_s = INVALID_SOCKET;
	m_pMutexSocketNumber = new GSpinLock();
	m_bKeepListening = true;
	m_nListenThreadCount = 0;
	m_bKeepAccepting = true;
	m_nAcceptorThreadCount = 0;
	m_bUDP = false;
}

GSocket::~GSocket()
{
	m_bKeepAccepting = false;
	m_bKeepListening = false;
	JoinAcceptorThread();
	JoinAllListenThreads();

	// Disconnect the connection
	if(m_s != INVALID_SOCKET)
	{
		shutdown(m_s, 2);
#ifdef WIN32		
		closesocket(m_s);
#else
		close(m_s);
#endif
		m_s = INVALID_SOCKET;
	}

	// Disconnect all the connections
	if(m_pHostSockets)
	{
		int nSockets = m_pHostSockets->GetSize();
		int n;
		SOCKET Sock;
		for(n = 0; n < nSockets; n++)
		{
			Sock = m_pHostSockets->GetSocket(n);
			if(Sock != INVALID_SOCKET)
			{
				shutdown(Sock, 2);
#ifdef WIN32				
				closesocket(Sock);
#else
				close(m_s);
#endif // WIN32
				m_pHostSockets->SetSocket(n, INVALID_SOCKET);
			}
		}
	}

	// Delete the Host Arrays
	delete(m_pHostSockets);
	delete(m_pHostListenThreads);
	delete(m_pMutexSocketNumber);
}

void GSocket::JoinAllListenThreads()
{
	m_bKeepListening = false;
	time_t tStart;
	time_t tNow;
	time(&tStart);
	while(m_nListenThreadCount > 0)
	{
#ifdef WIN32
		Sleep(0);
#else // WIN32
		usleep(0);
#endif // else WIN32
		time(&tNow);
		if(tNow - tStart > 4)
		{
			GAssert(false, "Error, took too long for the listen thread to exit");
			break;
		}
	}
	m_bKeepListening = true;
}

void GSocket::JoinListenThread(int nConnectionNumber)
{
	GAssert(false, "Error, not implemented yet");
}

void GSocket::JoinAcceptorThread()
{
	m_bKeepAccepting = false;
	time_t tStart;
	time_t tNow;
	time(&tStart);
	while(m_nAcceptorThreadCount > 0)
	{
#ifdef WIN32		
		Sleep(0);
#else // WIN32
		usleep(0);
#endif // else WIN32
		time(&tNow);
		if(tNow - tStart > 4)
		{
			GAssert(false, "Error, took too long for the acceptor thread to exit");
			break;
		}
	}
}

void GSocket::Listen()
{
	int nSocketNumber = m_nSocketNumber;
	m_nListenThreadCount++;
	m_pMutexSocketNumber->Unlock();
	char szReceiveBuff[520];
	SOCKET s;
	if(nSocketNumber < 1)
		s = m_s;
	else
		s = m_pHostSockets->GetSocket(nSocketNumber - 1);

	// Mark the socket as blocking so we can call "recv" which is a blocking operation
	unsigned long ulMode = 0;
#ifdef WIN32
	if(ioctlsocket(s, FIONBIO, &ulMode) != 0)
#else
	if(ioctl(s, FIONBIO, &ulMode) != 0)
#endif // WIN32
	{
		gsocket_LogError();
		return;
	}

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
				Receive((unsigned char*)szReceiveBuff, nBytesRead, nSocketNumber);
			else
				break; // The socket has been closed
		}
		else
		{
			// There's nothing to receive, so let's sleep for a while
#ifdef WIN32
			Sleep(50);
#else // WIN32
			usleep(50);
#endif // else WIN32
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

	OnLoseConnection(nSocketNumber);
	if(nSocketNumber < 1)
	{
		m_hListenThread = BAD_HANDLE;
		shutdown(m_s, 2);
#ifdef WIN32
		closesocket(m_s);
#else
		close(m_s);
#endif // WIN32
		m_s = INVALID_SOCKET;
	}

	else
	{
		if(m_pHostListenThreads->GetSize() < nSocketNumber)
		{
#ifdef WIN32
			Sleep(100);
#else // WIN32
			usleep(100);
#endif // else WIN32
		}
		m_pHostListenThreads->SetHandle(nSocketNumber - 1, BAD_HANDLE);
		shutdown(m_pHostSockets->GetSocket(nSocketNumber - 1), 2);
#ifdef WIN32
		closesocket(m_pHostSockets->GetSocket(nSocketNumber - 1));
#else
		close(m_s);
#endif // WIN32		
		m_pHostSockets->SetSocket(nSocketNumber - 1, INVALID_SOCKET);
	}
	m_pMutexSocketNumber->Lock("Listen Thread: About to decrement thread count");
	m_nListenThreadCount--;
	m_pMutexSocketNumber->Unlock();
}

int GSocket::GetFirstAvailableSocketNumber()
{
	if(!m_pHostSockets)
		m_pHostSockets = new GSocketArray(16);
	if(!m_pHostListenThreads)
		m_pHostListenThreads = new GHandleArray(16);

	// Find the first empty Handle slot for the listening thread
	int nSize = m_pHostListenThreads->GetSize();
	int nSocketNumber = -1;
	int n;
	for(n = 0; n < nSize; n++)
	{
		if(m_pHostListenThreads->GetHandle(n) == BAD_HANDLE)
		{
			nSocketNumber = n + 1;
			break;
		}
	}

	// Add a new slot if we couldn't find one
	if(nSocketNumber < 0)
	{
		m_pHostListenThreads->AddHandle(BAD_HANDLE);
		nSocketNumber = nSize + 1;
	}

	// Make sure there is a corresponding slot in the socket array
	while(m_pHostSockets->GetSize() < nSocketNumber)
		m_pHostSockets->AddSocket(INVALID_SOCKET);

	return nSocketNumber;
}

unsigned int ListenThread(void* pData)
{
	((GSocket*)pData)->Listen();
	return 0;
}

unsigned int ConnectionAcceptorThread(void* pData)
{
	((GSocket*)pData)->ConnectionAccepter();
	return 0;
}

void GSocket::ConnectionAccepter()
{
	m_nAcceptorThreadCount++;
	GAssert(m_nAcceptorThreadCount == 1, "Why are there multiple acceptor threads?");
	if(!m_pHostSockets)
		m_pHostSockets = new GSocketArray(16);
	if(!m_pHostListenThreads)
		m_pHostListenThreads = new GHandleArray(16);
#ifdef WIN32
	GWindows::YieldToWindows();
	int n;
#else // WIN32
	socklen_t n;
#endif // else WIN32
	while(m_bKeepAccepting)
	{
		// Accept the first connection waiting in the queue to be accepted
		// The socket should be marked non-blocking at this point, so this will
		// return immediately if there are not connections ready to be accepted
		n = sizeof(struct sockaddr);
		SOCKET s = accept(m_s, (struct sockaddr*)&m_sHostAddrIn, &n);
		if(s == INVALID_SOCKET)
		{
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK)
#else // WIN32
			if(errno == EAGAIN)
#endif // else WIN32
			{
				// There are no connections ready to be accepted, so let's sleep for .5 seconds
#ifdef WIN32
				GWindows::YieldToWindows();
				Sleep(500);
#else // WIN32
				usleep(500);
#endif // else WIN32
			}
			else
			{
				// Something went wrong
				gsocket_LogError();
				break;
			}
		}
		else
		{
			int nSocketNumber = GetFirstAvailableSocketNumber();
			m_pHostSockets->SetSocket(nSocketNumber - 1, s);
			m_pMutexSocketNumber->Lock("Connection Acceptor Thread: About to spawn listen thread");
			m_nSocketNumber = nSocketNumber;
			HANDLE h = GThread::SpawnThread(ListenThread, this);
			if(h == BAD_HANDLE)
			{
				GAssert(false, "Failed to spawn listening thread\n");
				m_hConnectionAccepterThread = BAD_HANDLE;
				m_pMutexSocketNumber->Unlock();
				break;
			}
			m_pHostListenThreads->SetHandle(nSocketNumber - 1, h);

			// WARNING: the accept function will return as soon as it gets
			//         an ACK packet back from the client, but the connection
			//         isn't actually established until more data is
			//         received.  Therefore, if you try to send data immediately
			//         (which someone might want to do in OnAcceptConnetion, the
			//         data might be lost since the connection might not be
			//         fully open.
			OnAcceptConnection(nSocketNumber);
		}
	}
	m_nAcceptorThreadCount--;
	m_hConnectionAccepterThread = BAD_HANDLE;
}

bool GSocket::GoIntoHostMode(unsigned short nListenPort, int nMaxConnections)
{
	// Make the Socket
	m_s = socket(AF_INET, m_bUDP ? SOCK_DGRAM : SOCK_STREAM, 0);
	if(m_s == INVALID_SOCKET)
	{
		gsocket_LogError();
		return false; 
	}

	// Put the socket into non-blocking mode (so the call to "accept" will return immediately
	// if there are no connections in the queue ready to be accepted)
	unsigned long ulMode = 1;
#ifdef WIN32
	if(ioctlsocket(m_s, FIONBIO, &ulMode) != 0)
#else // WIN32
	if(ioctl(m_s, FIONBIO, &ulMode) != 0)
#endif // WIN32
	{
		gsocket_LogError();
		return false;
	}

	// Tell the socket that it's okay to reuse an old crashed socket that hasn't timed out yet
	int flag;
	flag = 1;
#ifdef WIN32
	setsockopt(m_s, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag)); 
#else
	setsockopt(m_s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)); 
#endif // WIN32

	// Prepare the socket for accepting
	memset(&m_sHostAddrIn, '\0', sizeof(SOCKADDR_IN));
	m_sHostAddrIn.sin_family = AF_INET;
	m_sHostAddrIn.sin_port = htons(nListenPort);
	m_sHostAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(m_s, (struct sockaddr*)&m_sHostAddrIn, sizeof(SOCKADDR)))
	{
		gsocket_LogError();
		return false;
	}
	if(m_bUDP)
	{
		// Spawn the listen thread
		m_pMutexSocketNumber->Lock("GSocket::GoIntoHostMode");
		int nSocketNumber = GetFirstAvailableSocketNumber();
		m_nSocketNumber = nSocketNumber;
		HANDLE h = GThread::SpawnThread(ListenThread, this);
		if(h == BAD_HANDLE)
		{
			GAssert(false, "Failed to spawn listening thread\n");
			m_hConnectionAccepterThread = BAD_HANDLE;
			m_pMutexSocketNumber->Unlock();
			return false;
		}
		m_pHostListenThreads->SetHandle(nSocketNumber - 1, h);
	}
	else
	{
		if(listen(m_s, nMaxConnections))
		{
			gsocket_LogError();
			return false;
		}

		// Spawn the connection acceptor thread
		m_hConnectionAccepterThread = GThread::SpawnThread(ConnectionAcceptorThread, this);
		if(m_hConnectionAccepterThread == BAD_HANDLE)
		{
			GAssert(false, "Error spawning Connection Accepter\n");
			return false;
		}
#ifdef WIN32
		Sleep(0);
#else // WIN32
		usleep(0);
#endif // else WIN32
	}

	return true;
}

bool GSocket::Init(bool bUDP, bool bIAmTheServer, u_short nPort, int nMaxConnections)
{
	m_bUDP = bUDP;

#ifdef WIN32
	if(!gsocket_InitWinSock())
		return false;
#endif // WIN32

	m_bIAmTheServer = bIAmTheServer;

	if(bIAmTheServer)
	{
		if(nPort == 0)
		{
			GAssert(false, "You must specify a valid port for a host socket\n");
			return false;
		}

		if(!GoIntoHostMode(nPort, nMaxConnections))
			return false;
	}

	return true;
}

bool GSocket::IsThisAnIPAddress(const char* szHost)
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
/*static*/ void GSocket::ParseURL(const char* szBuff, char* szProtocall, char* szHost, char* szLoc, char* szPort, char* szParams)
{
	char cTmp;
	GTEMPBUF(szURL, strlen(szBuff) + 1);
	strcpy(szURL, szBuff);

	// Get the protocal specifier ("http://", "ftp://, "telnet://", etc.)
	char* pNext = szURL;
	int n;
	for(n = 0; szURL[n] != '.' && szURL[n] != '\0'; n++)
	{
		if(szURL[n] == '/')
		{
			pNext = szURL + n + 1;
			if(*pNext == '/')
				pNext++;
			break;
		}
	}
	cTmp = *pNext;
	*pNext = '\0';
	if(szProtocall)
		strcpy(szProtocall, szURL);
	*pNext = cTmp;

	// Get the Host ("www.yahoo.com", "cs.byu.edu", etc.)
	for(n = 0; pNext[n] != '\0' && pNext[n] != '/' && pNext[n] != '\\' && pNext[n] != ':' && pNext[n] != '?'; n++);
	cTmp = pNext[n];
	pNext[n] = '\0';
	if(szHost)
		strcpy(szHost, pNext);
	pNext[n] = cTmp;
	pNext += n;
	
	// Get the Location ("/myfiles/index.htm", "/directx/download.htm", etc.)
	for(n = 0; pNext[n] != '\0' && pNext[n] != '?' && pNext[n] != ':' && pNext[n] != ' '; n++);
	cTmp = pNext[n];
	pNext[n] = '\0';
	if(szLoc)
		strcpy(szLoc, pNext);
	pNext[n] = cTmp;
	pNext += n;
	
	// Get the Port (":3030", ":80", etc.)
	if(*pNext == ':')
	{
		for(n = 0; pNext[n] != '\0' && pNext[n] != '?' && pNext[n] != ' '; n++);
		cTmp = pNext[n];
		pNext[n] = '\0';
		if(szPort)
			strcpy(szPort, pNext);
		pNext[n] = cTmp;
		pNext += n;
	}
	else
		if(szPort)
			strcpy(szPort, "");

	// Get the Parameters
	if(*pNext == '?')
	{
		for(n = 0; pNext[n] != '\0'; n++)
		{
		}
		cTmp = pNext[n];
		pNext[n] = '\0';
		if(szParams)
			strcpy(szParams, pNext);
		pNext[n] = cTmp;
		pNext += n;
	}
	else
		if(szParams)
			strcpy(szParams, "");

	// Throw out whatever's left
}

/*static*/ int GSocket::ParseUrlParams(const char* szParams, int nMaxParams, char** pNames, int* pNameLengths, char** pValues, int* pValueLengths)
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

in_addr GSocket::StringToAddr(const char* szURL)
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

unsigned short GSocket::StringToPort(const char* szURL)
{
	char szPort[256];
	ParseURL(szURL, NULL, NULL, NULL, szPort, NULL);
	int nPort = 80;
	const char* pPort;
	for(pPort = szPort; *pPort != '\0' && (*pPort < '0' || *pPort > '9'); pPort++);
	if(strlen(pPort) > 0)
		nPort = atoi(pPort);
	return(nPort);
}

#ifdef OLD_NAME_RESOLUTION

bool GSocket::Connect(in_addr nAddr, u_short nPort, short nFamily)
{
	if(m_bIAmTheServer)
	{
		GAssert(false, "You can only call Connect for a Client socket\n");
		return false;
	}

	// Make the address structure
	SOCKADDR_IN sa;
	sa.sin_family = nFamily;
	sa.sin_port = htons(nPort); // convert port to Big-Endian
	sa.sin_addr = nAddr;

	// Terminate the listening thread
	JoinAllListenThreads();
	m_hListenThread = BAD_HANDLE;

	// Disconnect from previous connection if any
	if(m_s != INVALID_SOCKET)
	{
		shutdown(m_s, 2);
#ifdef WIN32
		closesocket(m_s);
#else
		close(m_s);
#endif // WIN32
		m_s = INVALID_SOCKET;
	}

	// Make the socket
//printf("Connecting to %d.%d.%d.%d on port %d... ", ((unsigned char*)&nAddr)[0], ((unsigned char*)&nAddr)[1], ((unsigned char*)&nAddr)[2], ((unsigned char*)&nAddr)[3], nPort);
	m_s = socket(AF_INET, m_bUDP ? SOCK_DGRAM : SOCK_STREAM, 0);
	if(m_s == INVALID_SOCKET)
	{
//printf("Failed.\n");
		gsocket_LogError();
		return false;
	}
//printf("OK.\n");

	// Connect
	if(connect(m_s, (struct sockaddr*)&sa, sizeof(SOCKADDR)))
	{
#ifdef WIN32
		int n = WSAGetLastError();
		switch(n)
		{
			case WSAECONNREFUSED:	break;
			default:	gsocket_LogError();		break;
		}
#endif
		return false;
	}

	// Spawn the listener thread
	m_pMutexSocketNumber->Lock("Connect: About to spawn listen thread");
	m_nSocketNumber = 0;
	m_hListenThread = GThread::SpawnThread(ListenThread, this);
	if(m_hListenThread == BAD_HANDLE)
	{
		GAssert(false, "Failed to spawn listening thread\n");
		gsocket_LogError();
		m_pMutexSocketNumber->Unlock();
		return false;
	}
#ifdef WIN32
	Sleep(0);
#else // WIN32
	usleep(0);
#endif // else WIN32
	
	return true;
}
/*
bool GSocket::Connect(const char* szURL)
{
	return Connect(StringToAddr(szURL), StringToPort(szURL));
}
*/
bool GSocket::Connect(const char* szAddr, unsigned short nPort)
{
	return Connect(StringToAddr(szAddr), nPort);
}

#else // OLD_NAME_RESOLUTION

bool GSocket::Connect(const char* szURL, unsigned short nPort)
{
	// *** If you use VisualStudio 6.0 and you get an error that says 'hints' uses undefined struct 'addrinfo'
	// *** on the next code line then you either need to update your Platform SDK or uncomment the
	// *** #define OLD_NAME_RESOLUTION at the top of this file. Updating your Platform SDK is a better solution.
	struct addrinfo hints, *res, *res0;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	char szPort[32];
	itoa(nPort, szPort, 10);
	error = getaddrinfo(szURL, szPort, &hints, &res0);
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
#ifdef WIN32
			closesocket(m_s);
#else // WIN32
			close(m_s);
#endif // !WIN32
			m_s = INVALID_SOCKET;
//printf("Failed.\n");
			continue;
		}

//printf("OK\n");
		break;  // we got a connection
	}
	freeaddrinfo(res0);
	if(m_s < 0)
	{
		// todo: handle the error
		return false;
	}

	// Spawn the listener thread
	m_pMutexSocketNumber->Lock("Connect: About to spawn listen thread");
	m_nSocketNumber = 0;
	m_hListenThread = GThread::SpawnThread(ListenThread, this);
	if(m_hListenThread == BAD_HANDLE)
	{
		GAssert(false, "Failed to spawn listening thread\n");
		gsocket_LogError();
		m_pMutexSocketNumber->Unlock();
		return false;
	}
#ifdef WIN32
	Sleep(0);
#else // WIN32
	usleep(0);
#endif // else WIN32
	
	return true;
}

#endif // !OLD_NAME_RESOLUTION

void GSocket::Disconnect(int nConnectionNumber)
{
	if(nConnectionNumber == 0)
	{
		// Terminate the listening thread
		JoinAllListenThreads();
		m_hListenThread = BAD_HANDLE;

		// Disconnect from previous connection if any
		shutdown(m_s, 2);
#ifdef WIN32
		closesocket(m_s);
#else
		close(m_s);
#endif // WIN32
		m_s = INVALID_SOCKET;
	}
	else
	{
		// Terminate the listening thread
		GAssert(m_pHostListenThreads, "No array of listen thread handles defined");
		JoinListenThread(nConnectionNumber);
		m_pHostListenThreads->SetHandle(nConnectionNumber - 1, BAD_HANDLE);

		GAssert(m_pHostSockets, "No array of sockets defined");
		SOCKET Sock;
		Sock = m_pHostSockets->GetSocket(nConnectionNumber - 1);
		if(Sock != INVALID_SOCKET)
		{
			shutdown(Sock, 2);
#ifdef WIN32
			closesocket(Sock);
#else
			close(m_s);
#endif // WIN32
			m_pHostSockets->SetSocket(nConnectionNumber - 1, INVALID_SOCKET);
		}
	}
}

bool GSocket::Send(const unsigned char *pBuff, int len, int nConnectionNumber)
{
	GAssert(m_bIAmTheServer ? nConnectionNumber > 0 : nConnectionNumber == 0, "Bad Connection Number");
	SOCKET s;
	if(nConnectionNumber < 1)
		s = m_s;
	else
		s = m_pHostSockets->GetSocket(nConnectionNumber - 1);
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

void GSocket::OnLoseConnection(int nSocketNumber)
{

}

void GSocket::OnAcceptConnection(int nSocketNumber)
{

}

bool GSocket::IsConnected(int nConnectionNumber)
{
	if(nConnectionNumber == 0)
	{
		if(m_hListenThread == BAD_HANDLE)
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
}

SOCKET GSocket::GetSocketHandle(int nConnectionNumber)
{
	if(nConnectionNumber < 1)
		return m_s;
	else
		return m_pHostSockets->GetSocket(nConnectionNumber - 1);

}

in_addr GSocket::GetMyIPAddr()
{
	if(m_bIAmTheServer)
		GAssert(false, "Error, this currently only works with client sockets");
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

char* GSocket::GetMyIPAddr(char* szBuff, int nBuffSize)
{
	if(m_bIAmTheServer)
	{
		GAssert(false, "Error, this currently only works with client sockets");
		return NULL;
	}
	GString::StrCpy(szBuff, inet_ntoa(GetMyIPAddr()), nBuffSize);
	return szBuff;
}

u_short GSocket::GetMyPort()
{
	if(m_bIAmTheServer)
	{
		GAssert(false, "Error, this currently only works with client sockets");
		return 0;
	}
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

char* GSocket::GetMyName(char* szBuff, int nBuffSize)
{
	if(m_bIAmTheServer)
	{
		GAssert(false, "Error, this currently only works with client sockets");
		return NULL;
	}
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

in_addr GSocket::GetTheirIPAddr(int nConnectionNumber)
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
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
			gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return pInfo->sin_addr;
}

char* GSocket::GetTheirIPAddr(char* szBuff, int nBuffSize, int nConnectionNumber)
{
	GString::StrCpy(szBuff, inet_ntoa(GetTheirIPAddr(nConnectionNumber)), nBuffSize);
	return szBuff;
}

u_short GSocket::GetTheirPort(int nConnectionNumber)
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
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
			gsocket_LogError();
	}
	if(sAddr.sa_family != AF_INET)
		GAssert(false, "Error, family is not AF_INET\n");
	SOCKADDR_IN* pInfo = (SOCKADDR_IN*)&sAddr;
	return htons(pInfo->sin_port);
}

char* GSocket::GetTheirName(char* szBuff, int nBuffSize, int nConnectionNumber)
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
		if(getpeername(m_s, &sAddr, &l))
			gsocket_LogError();
	}
	else
	{
		if(getpeername(m_pHostSockets->GetSocket(nConnectionNumber - 1), &sAddr, &l))
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
