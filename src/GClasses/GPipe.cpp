/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GPipe.h"
#include "GThread.h"
#include "GWindows.h"

#define BAD_HANDLE (void*)0xffffffff

GPipeClient::GPipeClient()
{
	m_szBuff = new char[512];
	m_hPipeFile = BAD_HANDLE;
}

GPipeClient::~GPipeClient()
{
	ClosePipeFile();
	delete(m_szBuff);
}

bool GPipeClient::Connect(const char* szNameOfPipeToTalkTo)
{
	ClosePipeFile();

	// Produce the pipe name
	char szBuff[256];
	strcpy(szBuff, "\\\\.\\pipe\\");
	strcat(szBuff, szNameOfPipeToTalkTo);

	// Connect to it
	m_hPipeFile = CreateFile(
			szBuff, 
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
			NULL
			);

	// Check for error
	if(m_hPipeFile == BAD_HANDLE)
	{
//		char szBuff[1025];
//		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szBuff, 1024, NULL);
//		GAssert(false, szBuff);
//		GAssert(false, "No one is listening\n");
		return false;
	}
	return true;
}

void GPipeClient::ClosePipeFile()
{
	// Close the PipeFile
	if(m_hPipeFile != BAD_HANDLE)
    {
		if(!CloseHandle(m_hPipeFile))
			GAssert(false, "Error closing PipeFile handle");
		m_hPipeFile = BAD_HANDLE;
	}
}

bool GPipeClient::Send(void* pData, int nSize)
{
	// Check to make sure we're connected
	if(m_hPipeFile == BAD_HANDLE)
	{
		GAssert(false, "You are not connected to a GPipeHost.");
		return false;
	}

	// Send the message
	DWORD nBytesWritten;
	if(!WriteFile(m_hPipeFile, pData, nSize, &nBytesWritten, NULL))
	{
		char szBuff[1025];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szBuff, 1024, NULL);
		GAssert(false, szBuff);
		GAssert(false, "Failed to write to the pipe\n");
		return false;
	}

	// Check the number of bytes written
	if(nBytesWritten != (unsigned)nSize)
	{
		GAssert(false, "Wrong number of bytes written\n");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////

unsigned int PipeListenThread(void* pData)
{
	((GPipeHost*)pData)->Listen();
	return 0;
}

GPipeHost::GPipeHost(const char* szMyName)
{
	m_bReceiving = false;
	m_hListenThread = BAD_HANDLE;
	m_szBuff = new char[512];
	char szBuff[256];
	strcpy(szBuff, "\\\\.\\pipe\\");
	strcat(szBuff, szMyName);

	m_hPipe = CreateNamedPipe(
			szBuff, 
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE,
			PIPE_UNLIMITED_INSTANCES,
			512,	// out buffer size
			512,	// in buffer size
			60000,	// default time-out (in ms)
			NULL
			);

	if(m_hPipe == BAD_HANDLE)
	{
		char szBuff[1025];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szBuff, 1024, NULL);
		GAssert(false, szBuff);
		GAssert(false, "Failed to create named pipe\n");
		return;
	}

	// Spawn the listener thread
	m_hListenThread = GThread::SpawnThread(PipeListenThread, this);
	if(m_hListenThread == BAD_HANDLE)
	{
		GAssert(false, "Failed to spawn listening thread\n");
		char szBuff[1025];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szBuff, 1024, NULL);
		GAssert(false, szBuff);
	}
}

GPipeHost::~GPipeHost()
{
	// Terminate the listening thread
	if(m_hListenThread != BAD_HANDLE)
	{
		DWORD nExitCode;
		if(GetExitCodeThread(m_hListenThread, &nExitCode))
		{
			while(!GWindows::TestAndSet(&m_bReceiving)) // Mutex to ensure that we don't terminate the thread while calling Receive()
				Sleep(0);
			if(TerminateThread(m_hListenThread, nExitCode))
				m_hListenThread = BAD_HANDLE;
			else
				GAssert(false, "Failed to terminate the listen thread\n");
		}
		else
			GAssert(false, "Failed to get exit code for listening thread\n");
	}

	// Close the pipe or file
	if(m_hPipe != BAD_HANDLE)
    {
		if(!CloseHandle(m_hPipe))
			GAssert(false, "Error closing pipe handle");
	}

	// Delete the buffer
	delete(m_szBuff);
}

void GPipeHost::Listen()
{
	// Wait for someone to connect to this pipe (this is in the listen thread so it can
	// be terminated if this object is destructed before anyone talks to me)
	if(!ConnectNamedPipe(m_hPipe, NULL)) // wait until another process connects
	{
		GAssert(false, "Failed to connect the pipe\n");
		char szBuff[1025];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, szBuff, 1024, NULL);
		GAssert(false, szBuff);
		return;
	}

	// Listen until we get a message from the other end of the pipe
	DWORD nBytesRead;
	while(true)
	{
		ReadFile(m_hPipe, m_szBuff, 511, &nBytesRead, NULL);
		if(nBytesRead > 0)
		{
			while(!GWindows::TestAndSet(&m_bReceiving)) // Mutex to ensure that we don't terminate the thread while calling Receive()
				Sleep(0);
			Receive(m_szBuff, nBytesRead);
			m_bReceiving = false;
		}
		GWindows::YieldToWindows();
		Sleep(0);
	}
}
