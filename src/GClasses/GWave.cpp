/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GnuSDK.h"
#include "GDirectSound.h"

#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

GWave::GWave(HWND hWnd)
{
	m_pDS = NULL;
	m_lpDSB[0] = NULL;
	m_Current = 0;
	m_nBuffers = 1;
	m_pDS = new GDirectSound();
	m_pDS->Init(hWnd);
	m_pLockedBuffer = NULL;
	m_dwSize = 0;
	m_nChannels = 0;
	m_dwSampleRate = 0;
	m_nBitsPerSample = 0;
}

GWave::~GWave()
{
	for(int i = 0; i < m_nBuffers; i++) RELEASE(m_lpDSB[i]);
	delete(m_pDS);
	m_pDS = NULL;
}

BOOL GWave::CreateSoundBuffer(DWORD dwBufSize, DWORD dwFreq, DWORD dwBitsPerSample, DWORD dwBlkAlign, BOOL bStereo)
{
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbdesc;

	// Set up wave format structure.
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag         = WAVE_FORMAT_PCM;      
	pcmwf.wf.nChannels          = bStereo ? 2 : 1;
	pcmwf.wf.nSamplesPerSec     = dwFreq;
	pcmwf.wf.nBlockAlign        = (WORD)dwBlkAlign;
	pcmwf.wf.nAvgBytesPerSec    = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample        = (WORD)dwBitsPerSample;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize              = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags             = DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2;
	dsbdesc.dwBufferBytes       = dwBufSize; 
	dsbdesc.lpwfxFormat         = (LPWAVEFORMATEX)&pcmwf;

	m_pDS->m_lpDS->CreateSoundBuffer(&dsbdesc, &m_lpDSB[0], NULL);

	return TRUE;
}

BOOL GWave::ReadData(FILE* fp, DWORD dwSize, DWORD dwPos)
{
	// Seek to correct position in file (if necessary)
	if (dwPos != 0xffffffff) 
	{
		if (fseek(fp, dwPos, SEEK_SET) != 0)
			return FALSE;
	}

	// Lock data in buffer for writing
	LPVOID pData1;
	DWORD  dwData1Size;
	LPVOID pData2;
	DWORD  dwData2Size;
	HRESULT rval;

	rval = m_lpDSB[0]->Lock(0, dwSize, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
	if (rval != DS_OK)
	{
		return FALSE;
	}

	// Read in first chunk of data
	if (dwData1Size > 0) 
	{
		if (fread(pData1, dwData1Size, 1, fp) != 1) 
		{          
			char holder[256];
			wsprintf(holder, "Data1 : %d, dwdata: %d, fp: %d", pData1, dwData1Size, fp);
			OutputDebugString(holder);
			return FALSE;
		}
	}

	// Read in second chunk if necessary
	if (dwData2Size > 0) 
	{
		if (fread(pData2, dwData2Size, 1, fp) != 1) 
		{
			return FALSE;
		}
	}

	// Unlock data in buffer
	rval = m_lpDSB[0]->Unlock(pData1, dwData1Size, pData2, dwData2Size);
	if (rval != DS_OK) return FALSE;

	return TRUE;
}

LPDIRECTSOUNDBUFFER GWave::GetDSB()
{
	if(m_nBuffers > 1)
	{
		GLog("Error, more than one buffer\n");
		return NULL;
	}
	return m_lpDSB[0];
}


LPDIRECTSOUNDBUFFER GWave::GetFreeBuffer()
{
	DWORD Status;
	HRESULT rval;
	LPDIRECTSOUNDBUFFER Buffer;

	if(m_lpDSB == NULL) return NULL;

	if(Buffer = m_lpDSB[m_Current])
	{
		rval = Buffer->GetStatus(&Status);
		if(rval < 0) Status = 0;

		if((Status & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{
			if(m_nBuffers > 1)
			{
				if (++m_Current >= m_nBuffers) m_Current = 0;
				
				Buffer = m_lpDSB[m_Current];
				rval = Buffer->GetStatus(&Status);

				if((rval >= 0) && (Status & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
				{
					Buffer->Stop();
					Buffer->SetCurrentPosition(0);
				}
			}
			else
			{
				Buffer = NULL;
			}
		}

		if(Buffer && (Status & DSBSTATUS_BUFFERLOST))
		{
			if((Buffer->Restore() < 0))
			{
				Buffer = NULL;
			}
		}
	}

	return Buffer;
}

BOOL GWave::LoadFromFile(const char* szFilename)
{
	struct WaveHeader waveHeader;

	// Open the wave file       
	FILE* fp = fopen(szFilename, "rb");
	if (fp == NULL)
		return FALSE;

	// Read in the wave header          
	if (fread(&waveHeader, sizeof(struct WaveHeader), 1, fp) != 1) 
	{
		fclose(fp);
		return NULL;
	}

	// Get data from the wave header
	m_dwSize = waveHeader.dwDSize;
	m_nChannels = waveHeader.wChnls;
	m_dwSampleRate = waveHeader.dwSRate;
	m_nBitsPerSample = waveHeader.BitsPerSample;

	// Is this a stereo or mono file?
	BOOL bStereo = waveHeader.wChnls > 1 ? TRUE : FALSE;

	// Create the sound buffer for the wave file
	if (!CreateSoundBuffer(m_dwSize, waveHeader.dwSRate, waveHeader.BitsPerSample, waveHeader.wBlkAlign, bStereo))
	{
		// Close the file
		fclose(fp);
        
		return FALSE;
	}

	// Read the data for the wave file into the sound buffer
	if (!ReadData(fp, m_dwSize, sizeof(struct WaveHeader))) 
	{
		fclose(fp);
		return FALSE;
	}

	// Close out the wave file
	fclose(fp);

	return TRUE;
}

BOOL GWave::Load(const char *szFilename, int Num)
{
	m_nBuffers = Num;

	if(!LoadFromFile(szFilename))
      return FALSE;

	for(int i = 1; i < m_nBuffers; i++)
	{
		if(!m_pDS->m_lpDS->DuplicateSoundBuffer(m_lpDSB[0], &m_lpDSB[i]))
		{
			if(!LoadFromFile(szFilename))
            return FALSE;
		}
	}

	SetPan(0);

	return TRUE;
}

BOOL GWave::Play(DWORD dwFlags)
{
	HRESULT rval;
	LPDIRECTSOUNDBUFFER Buffer = NULL;

	if(m_pDS == NULL)
		return FALSE;

	Buffer = GetFreeBuffer();
	if(Buffer == NULL)
		return FALSE;

	rval = Buffer->Play(0, 0, dwFlags);
	if(rval != DS_OK)
		return FALSE;

	return TRUE;
}

BOOL GWave::Stop()
{
	HRESULT rval;

	if(m_pDS == NULL)
		return FALSE;

	for(int i = 0; i < m_nBuffers; i++)
	{
		rval = m_lpDSB[i]->Stop();
		if(rval != DS_OK)
			return FALSE;

		rval = m_lpDSB[i]->SetCurrentPosition(0);
		if(rval != DS_OK)
			return FALSE;
	}

	return TRUE;
}

BOOL GWave::SetPan(int nPan)
{
	for(int i = 0; i < m_nBuffers; i++)
	{
		if(m_lpDSB[i]->SetPan(nPan) != DS_OK)
			return FALSE;
	}
	return TRUE;
}

int GWave::GetPan()
{
	long nPan;
	if(m_lpDSB[0]->GetPan(&nPan) != DS_OK) 
		GLog("Error getting pan\n");
	return nPan;
}

void GWave::SetVolume(LONG nVolume)
{
	for(int i = 0; i < m_nBuffers; i++)
	{
		if(m_lpDSB[i]->SetVolume(nVolume) != DS_OK)
			GLog("Error setting volume\n");
	}
}

long GWave::GetVolume()
{
	long nVol;
	if(m_lpDSB[0]->GetVolume(&nVol) != DS_OK)
		GLog("Error getting volume\n");
	return nVol;
}

int GWave::GetPosition()
{
	DWORD nPlayPos;
	if(m_lpDSB[0]->GetCurrentPosition(&nPlayPos, NULL) != DS_OK)
		GLog("Error getting wave position\n");
	return nPlayPos;
}

bool GWave::SetPosition(int nByteOffset)
{
	for(int i = 0; i < m_nBuffers; i++)
	{
		if(m_lpDSB[i]->SetCurrentPosition(nByteOffset) != DS_OK)
			return false;
	}
	return true;
}

DWORD GWave::GetSampleRate()
{
/*	unsigned long nSR;
	if(m_lpDSB[0]->GetFrequency(&nSR) != DS_OK)
		GLog("Error getting sample rate\n");
	return nSR;
*/
	return m_dwSampleRate;
}

bool GWave::SetSampleRate(DWORD nSamplesPerSecond)
{
	for(int i = 0; i < m_nBuffers; i++)
	{
		if(m_lpDSB[i]->SetFrequency(nSamplesPerSecond) != DS_OK)
			GLog("Error setting sample rate\n");
	}
	m_dwSampleRate = nSamplesPerSecond;
	return true;
}

bool GWave::SetChannels(int nChannels)
{
	GLog("Not implemented yet");
/*
	WAVEFORMATEX wf;
	DWORD dwSizeWritten;
	if(m_lpDSB[0]->GetFormat(&wf, sizeof(WAVEFORMATEX), &dwSizeWritten) != DS_OK)
	{
		GLog("Error getting wave format info");
		return false;
	}
	if(dwSizeWritten != sizeof(WAVEFORMATEX))
	{
		GLog("huh?");
		return false;
	}
	wf.nChannels = nChannels;
	for(int i = 0; i < m_nBuffers; i++)
	{
		DWORD dwBytesLocked;
		Lock(0, 10, &dwBytesLocked);
		HRESULT res = m_lpDSB[i]->SetFormat(&wf);
		UnlockNoChanges();
		if(res != DS_OK)
		{
			switch(res)
			{
				case DSERR_BADFORMAT: GLog("1"); break;
				case DSERR_INVALIDCALL: GLog("2"); break;
				case DSERR_INVALIDPARAM: GLog("3"); break;
				case DSERR_OUTOFMEMORY: GLog("4"); break;
				case DSERR_PRIOLEVELNEEDED: GLog("5"); break;
				case DSERR_UNSUPPORTED: GLog("6"); break;
				default: GLog("Error setting format"); break;
			}
			return false;
		}
	}
	m_nChannels = nChannels;
*/
	return true;
}

bool GWave::SetBitsPerSample(int nBitsPerSample)
{
	GLog("Not implemented yet");
/*
	WAVEFORMATEX wf;
	DWORD dwSizeWritten;
	if(m_lpDSB[0]->GetFormat(&wf, sizeof(WAVEFORMATEX), &dwSizeWritten) != DS_OK)
	{
		GLog("Error getting wave format info");
		return false;
	}
	if(dwSizeWritten != sizeof(WAVEFORMATEX))
	{
		GLog("huh?");
		return false;
	}
	wf.wBitsPerSample = nBitsPerSample;
	for(int i = 0; i < m_nBuffers; i++)
	{
		if(m_lpDSB[i]->SetFormat(&wf) != DS_OK)
		{
			GLog("Error setting format");
			return false;
		}
	}
	m_nBitsPerSample = nBitsPerSample;
*/
	return true;
}

void* GWave::Lock(int nStartOffset, DWORD nSize, DWORD* pnBytesLocked)
{
	if(m_pLockedBuffer)
	{
		GLog("A buffer is already locked\n");
		return NULL;
	}
	if(m_lpDSB[0]->Lock(nStartOffset, nSize, &m_pLockedBuffer, pnBytesLocked, NULL, NULL, NULL) != DS_OK)
		return NULL;
	m_nLockedSize = *pnBytesLocked;
	return m_pLockedBuffer;
}

void GWave::Unlock()
{
	if(!m_pLockedBuffer)
		return;
	if(m_lpDSB[0]->Unlock(m_pLockedBuffer, m_nLockedSize, NULL, 0) != DS_OK)
		GLog("Error unlocking sound buffer\n");
	m_pLockedBuffer = NULL;
	m_nLockedSize = 0;
}

void GWave::UnlockNoChanges()
{
	if(!m_pLockedBuffer)
		return;
	if(m_lpDSB[0]->Unlock(m_pLockedBuffer, 0, NULL, 0) != DS_OK)
		GLog("Error unlocking sound buffer\n");
	m_pLockedBuffer = NULL;
	m_nLockedSize = 0;
}
