/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VWAVE_H__
#define __VWAVE_H__

#include "../SDL/SDL_mixer.h"

class VWaveBuffer;

// This class represents a wave sound
class MSound
{
friend class VAudioPlayer;
protected:
	Mix_Chunk m_mixChunk;

public:
	MSound(const char* szFilename);

	// Note that this takes ownership of pData (the destructor will delete it)
	MSound(unsigned char* pData, unsigned int nSize)
	{
		m_mixChunk.allocated = 0; // 0 = sound memory not owned by the Mix_Chunk struct
		m_mixChunk.abuf = pData;
		m_mixChunk.alen = nSize;
		m_mixChunk.volume = 128; // 0 = silent, 128 = max volume
	}

	~MSound()
	{
		delete((unsigned char*)m_mixChunk.abuf);
	}

	int GetSize() { return m_mixChunk.alen; }

	Mix_Chunk* GetMixChunk() { return &m_mixChunk; }
};


// This class plays wave sounds
class VAudioPlayer
{
protected:
	bool m_bPlayingOgg;
	Mix_Music* m_pOggMusic;

public:
	VAudioPlayer();
	~VAudioPlayer();

	void Play(MSound* pSound);
	void StopAudio();
	void PlayBackgroundMusic(const char* szFilename);
};

#endif // __VWAVE_H__
