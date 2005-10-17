/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VWave.h"
#include "../SDL/SDL.h"
#include "../SDL/SDL_audio.h"
#include "../GClasses/GMacros.h"
#include "GameEngine.h"
#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32

#define AUDIO_CHANNELS 1

MSound::MSound(const char* szFilename)
{
	// Load the sound file and convert it to 16-bit mono at 22kHz
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	if(SDL_LoadWAV(szFilename, &wave, &data, &dlen) == NULL)
		GameEngine::ThrowError(SDL_GetError()); // failed to load audio file
	SDL_AudioCVT cvt;
	SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, AUDIO_CHANNELS, 22050);
	cvt.buf = new Uint8[dlen * cvt.len_mult];
	memcpy(cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio(&cvt);
	SDL_FreeWAV(data);
	m_pData = cvt.buf;
	m_nSize = cvt.len_cvt;
}

// ------------------------------------------------------------------------------

class VWaveBuffer
{
public:
	VWaveBuffer* m_pNext;
	VWaveBuffer* m_pPrev;
    Uint8* m_pData;
    Uint32 m_nPos;
    Uint32 m_nLen;

	VWaveBuffer()
	{
	}

	~VWaveBuffer()
	{
	}
};

// ------------------------------------------------------------------------------

void mixWaves(void *pThis, Uint8 *stream, int len)
{
	((VWavePlayer*)pThis)->MixWaves(stream, len);
}

// ------------------------------------------------------------------------------

VWavePlayer::VWavePlayer()
{
	m_pActive = NULL;
	m_pIdle = NULL;

	SDL_AudioSpec fmt;

	// Set 16-bit stereo audio at 22Khz
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = AUDIO_CHANNELS;
	fmt.samples = 2048; //512;
	fmt.callback = mixWaves;
	fmt.userdata = this;

	// Open the audio device and start playing sound!
	if(SDL_OpenAudio(&fmt, NULL) < 0)
		GameEngine::ThrowError(SDL_GetError()); // unable to open audio device
	SDL_PauseAudio(0);
}

VWavePlayer::~VWavePlayer()
{
	if(m_pActive)
	{
		SDL_PauseAudio(1);
		while(m_pActive)
		{
			VWaveBuffer* pBuf = m_pActive;
			UnLinkActive(pBuf);
			LinkIdle(pBuf);
		}
	}
	SDL_CloseAudio();
	while(m_pIdle)
	{
		VWaveBuffer* pTmp = m_pIdle;
		UnLinkIdle(pTmp);
		delete(pTmp);
	}
}

void VWavePlayer::StartAudio()
{
	SDL_PauseAudio(0);
}

void VWavePlayer::FinishAudio()
{
	if(m_pActive)
	{
		int n = 0;
		while(m_pActive)
		{
			// Give it one second to finish whatever sound effect is currently playing
			if((++n) % 100 == 0)
			{
				// Rudely stop the audio
				SDL_PauseAudio(1);
				while(m_pActive)
				{
					VWaveBuffer* pBuf = m_pActive;
					UnLinkActive(pBuf);
					LinkIdle(pBuf);
				}
				break;
			}
#ifdef WIN32
			Sleep(10);
#else // WIN32
			usleep(10);
#endif // !WIN32
		}
	}
}

void VWavePlayer::LinkActive(VWaveBuffer* pBuf)
{
	pBuf->m_pNext = m_pActive;
	pBuf->m_pPrev = NULL;
	if(m_pActive)
		m_pActive->m_pPrev = pBuf;
	m_pActive = pBuf;
}

void VWavePlayer::LinkIdle(VWaveBuffer* pBuf)
{
	pBuf->m_pNext = m_pIdle;
	pBuf->m_pPrev = NULL;
	if(m_pIdle)
		m_pIdle->m_pPrev = pBuf;
	m_pIdle = pBuf;
}

void VWavePlayer::UnLinkActive(VWaveBuffer* pBuf)
{
	if(pBuf->m_pPrev)
		pBuf->m_pPrev->m_pNext = pBuf->m_pNext;
	else
		m_pActive = pBuf->m_pNext;
	if(pBuf->m_pNext)
		pBuf->m_pNext->m_pPrev = pBuf->m_pPrev;
}

void VWavePlayer::UnLinkIdle(VWaveBuffer* pBuf)
{
	if(pBuf->m_pPrev)
		pBuf->m_pPrev->m_pNext = pBuf->m_pNext;
	else
		m_pIdle = pBuf->m_pNext;
	if(pBuf->m_pNext)
		pBuf->m_pNext->m_pPrev = pBuf->m_pPrev;
}

void VWavePlayer::MixWaves(Uint8 *stream, int len)
{
	VWaveBuffer* pBuf;
	VWaveBuffer* pNext;
	Uint32 nBlockSize;
	if(m_pActive)
	{
		for(pBuf = m_pActive; pBuf; pBuf = pNext)
		{
			pNext = pBuf->m_pNext;
			nBlockSize = pBuf->m_nLen - pBuf->m_nPos;
			if(nBlockSize > (unsigned int)len)
				nBlockSize = (unsigned int)len;
			SDL_MixAudio(stream, &pBuf->m_pData[pBuf->m_nPos], nBlockSize, SDL_MIX_MAXVOLUME);
			pBuf->m_nPos += nBlockSize;
			if(nBlockSize >= pBuf->m_nLen)
			{
				UnLinkActive(pBuf);
				LinkIdle(pBuf);
			}
		}
	}
	else
	{
#ifdef WIN32
		Sleep(10);
#else // WIN32
		usleep(10);
#endif // !WIN32
	}
}

void VWavePlayer::Play(const MSound* pSound)
{
	VWaveBuffer* pBuf = m_pIdle;
	if(pBuf)
		UnLinkIdle(pBuf);
	else
		pBuf = new VWaveBuffer();
	pBuf->m_pData = pSound->m_pData;
	pBuf->m_nLen = pSound->m_nSize;
	pBuf->m_nPos = 0;
	SDL_LockAudio();
	LinkActive(pBuf);
	SDL_UnlockAudio();
}


