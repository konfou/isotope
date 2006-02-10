/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VWave.h"
#include <SDL/SDL.h>
#include "../GClasses/GMacros.h"
#include "Main.h"
#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <unistd.h>
#endif // !WIN32

#define AUDIO_CHANNELS 2

MSound::MSound(const char* szFilename)
{
	// Load the sound file and convert it to 16-bit at 22kHz with the right number of channels
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	if(SDL_LoadWAV(szFilename, &wave, &data, &dlen) == NULL)
		GameEngine::ThrowError(SDL_GetError()); // failed to load audio file
	SDL_AudioCVT cvt;
	SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
	#ifdef BIG_ENDIAN
					AUDIO_S16MSB
	#else
					AUDIO_S16
	#endif
							, AUDIO_CHANNELS, 22050);
	cvt.buf = new Uint8[dlen * cvt.len_mult];
	memcpy(cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio(&cvt);
	SDL_FreeWAV(data);

	m_mixChunk.allocated = 0; // 0 = sound memory not owned by the Mix_Chunk struct
	m_mixChunk.abuf = cvt.buf;
	m_mixChunk.alen = cvt.len_cvt;
	m_mixChunk.volume = 128; // 0 = silent, 128 = max volume
}

// ------------------------------------------------------------------------------

VAudioPlayer::VAudioPlayer()
{
	m_bPlayingOgg = false;
	m_pOggMusic = NULL;

	// Init the OGG player
	int audio_rate = 22050; //Frequency of audio playback
	Uint16 audio_format = AUDIO_S16SYS; //Format of the audio we're playing
	int audio_buffers = 4096; //Size of the audio buffers in memory
	if(Mix_OpenAudio(audio_rate, audio_format, AUDIO_CHANNELS, audio_buffers) != 0) 
		GameEngine::ThrowError("Unable to initialize audio: %s\n", Mix_GetError());
}

VAudioPlayer::~VAudioPlayer()
{
	Mix_HaltMusic();
	if(m_pOggMusic)
	{
		Mix_FreeMusic(m_pOggMusic);
		m_pOggMusic = NULL;
	}

	SDL_CloseAudio();
	Mix_CloseAudio();
}

void VAudioPlayer::StopAudio()
{
	Mix_HaltMusic();
}

void VAudioPlayer::Play(MSound* pSound)
{
	Mix_Chunk* pChunk = pSound->GetMixChunk();
	Mix_PlayChannelTimed(-1, pChunk, 0, -1);
}

void musicFinished()
{
	printf("The music is done\n");
}

void VAudioPlayer::PlayBackgroundMusic(const char* szFilename)
{
	// Stop any music that's already playing
	Mix_HaltMusic();
	if(m_pOggMusic)
	{
		Mix_FreeMusic(m_pOggMusic);
		m_pOggMusic = NULL;
	}

	// Load the OGG file from disk
	Mix_Music* pMusic = Mix_LoadMUS(szFilename);
	if(!pMusic)
		GameEngine::ThrowError("Failed to load OGG file: %s\n%s", szFilename, Mix_GetError());

	// Start playing
	if(Mix_PlayMusic(pMusic, 1000) == -1) // repeat 1000 times
		GameEngine::ThrowError("Failed to start playing OGG file: %s\n", Mix_GetError());
	m_bPlayingOgg = true;

	// Tell it to call musicFinished() when the music stops playing
	Mix_HookMusicFinished(musicFinished);
}
