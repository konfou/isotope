/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDSOUND_H__
#define __GDSOUND_H__

#include <mmsystem.h>
#include <dsound.h>
#include <dplay.h>
#include <dplobby.h>

struct WaveHeader
{
	BYTE        RIFF[4];          // "RIFF"
	DWORD       dwSize;           // Size of data to follow
	BYTE        WAVE[4];          // "WAVE"
	BYTE        fmt_[4];          // "fmt "
	DWORD       dw16;             // 16
	WORD        wOne_0;           // 1
	WORD        wChnls;           // Number of Channels
	DWORD       dwSRate;          // Sample Rate
	DWORD       BytesPerSec;      // Sample Rate
	WORD        wBlkAlign;        // 1
	WORD        BitsPerSample;    // Sample size
	BYTE        DATA[4];          // "DATA"
	DWORD       dwDSize;          // Number of Samples
};

class GDirectSound;

// Play/Edit a .Wav file
class GWave
{
protected:
	int m_nBuffers;
	int m_Current;
	GDirectSound* m_pDS;
	LPDIRECTSOUNDBUFFER m_lpDSB[1];
	void* m_pLockedBuffer;
	int m_nLockedSize;
	DWORD m_dwSize;
	unsigned short m_nChannels;
	DWORD m_dwSampleRate;
	unsigned short m_nBitsPerSample;

	BOOL LoadFromFile(const char* szFilename);
	BOOL CreateSoundBuffer(DWORD dwBufSize, DWORD dwFreq, DWORD dwBitsPerSample,
												 DWORD dwBlkAlign, BOOL bStereo);
	BOOL ReadData(FILE* fp, DWORD dwSize, DWORD dwPos);
	LPDIRECTSOUNDBUFFER GetFreeBuffer();

public:
	GWave(HWND hWnd);
	~GWave();

	// if you use multiple buffers, it puts a copy in each buffer, so if one is
	// busy, you can still play the sound with another buffer. (Usually, just use 1)
	BOOL Load(const char* szFilename, int nBuffers = 1);

	//  ** Flags that can be used to play (no flags are necessary) **	
	//  DSBPLAY_LOOPING -- loops forever
	//  DSBPLAY_LOCHARDWARE -- only use hardware buffer
	//  DSBPLAY_LOCSOFTWARE -- only use software buffer
	//  DSBPLAY_TERMINATEBY_TIME -- overpower other buffers
	//  DSBPLAY_TERMINATEBY_DISTANCE -- (for 3d buffers) Enables hardware resource stealing
	//  DSBPLAY_TERMINATEBY_PRIORITY -- Enables hardware resource stealing
	BOOL Play(DWORD dwFlags = 0);
	BOOL Stop();
	
	void* Lock(int nStartOffset, DWORD nSize, DWORD* nBytesLocked);	// Use this to get a pointer to the buffer to modify
	void Unlock();	// Don't forget to unlock the buffer when you're done modifying it
	void UnlockNoChanges(); // If you didn't make any changes, this unlocks quickly

	DWORD GetSize() const { return m_dwSize; }
	int GetPosition();
	bool SetPosition(int nByteOffset);
	long GetVolume();
	void SetVolume(LONG Volume); // range: DSBVOLUME_MIN (silent) to DSBVOLUME_MAX (no attenuation) measured in hundredths of decibels
	int GetPan(); // value in hundredths of decibels that one of the speakers is attenuated
	BOOL SetPan(int nPan); // 0 = center  Negative = left softer, Positive = right softer (one speaker always at full)
	unsigned short GetChannels() const { return m_nChannels; }
	bool SetChannels(int nChannels);
	unsigned short GetBitsPerSample() const { return m_nBitsPerSample; }
	bool SetBitsPerSample(int nBitsPerSample);
	DWORD GetSampleRate();
	bool SetSampleRate(DWORD nSamplesPerSecond);

	LPDIRECTSOUNDBUFFER GetDSB();
};

// Plays Midi files
class GMidi
{
public:
	GMidi(HWND hWnd);

	BOOL Play(const char *Filename);
	BOOL Stop();
	BOOL Pause();
	BOOL Resume();
	BOOL Restart();

public:
	HWND m_hWnd;
};

#endif // __GDSOUND_H__
