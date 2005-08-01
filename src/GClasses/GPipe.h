/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GPIPE_H__
#define __GPIPE_H__

#ifdef WIN32
#include <windows.h>
#endif // WIN32

// This is for inter-process communication (on the same computer, or on the same network)
// First, Connect to a GPipeHost, then start talking to it.  A Talker can only talk to one
// Listener, but a Listener can listen to as many Talkers as want to talk to it.  Named Pipes
// must be enabled on your network.
class GPipeClient
{
protected:
	HANDLE m_hPipeFile;
	char* m_szBuff;

	void GPipeClient::ClosePipeFile();

public:
	GPipeClient();
	virtual ~GPipeClient();

	// This returns true if it finds the listener you specify.
	// This returns false if it can't find that listener.
	// (You must Connect to a listener before you can talk to it.  If
	// you have already connected to another listener, it will first
	// disconnect from that listener, then try to connect to the new one.)
	virtual bool Connect(const char* szNameOfListener);

	// Send messages to the host--if the host has died, this will return false
	virtual bool Send(void* pData, int nSize);
};


// This will receive messages from a GPipeClient.  It will listen to whoever
// wants to talk to it.  You should override the Receive() method to do 
// something with the message you get from the Talker.
class GPipeHost
{
protected:
	HANDLE m_hPipe;
	char* m_szBuff;
	bool m_bReceiving; // Mutex so we don't terminate the listen thread while receiving
	HANDLE m_hListenThread;

public:
	GPipeHost(const char* szMyName);
	virtual ~GPipeHost();

	void Listen();

protected:
	virtual void Receive(void* pData, int nSize) = 0;

};


#endif // __GPIPE_H__
