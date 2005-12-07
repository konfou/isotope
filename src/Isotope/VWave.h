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

class VWaveBuffer;

// This class represents a wave sound
class MSound
{
friend class VWavePlayer;
protected:
	unsigned char* m_pData;
	unsigned int m_nSize;

public:
	MSound(const char* szFilename);

	// Note that this takes ownership of pData (the destructor will delete it)
	MSound(unsigned char* pData, unsigned int nSize)
	{
		m_pData = pData;
		m_nSize = nSize;
	}

	~MSound()
	{
		delete(m_pData);
	}

	int GetSize() { return m_nSize; }
};


// This class plays wave sounds
class VWavePlayer
{
protected:
	VWaveBuffer* m_pActive;
	VWaveBuffer* m_pIdle;

public:
	VWavePlayer();
	~VWavePlayer();

	void Play(const MSound* pSound);
	void MixWaves(unsigned char* stream, int len);

	void StartAudio();
	void FinishAudio();

protected:
	void LinkActive(VWaveBuffer* pBuf);
	void UnLinkActive(VWaveBuffer* pBuf);
	void LinkIdle(VWaveBuffer* pBuf);
	void UnLinkIdle(VWaveBuffer* pBuf);
};

#endif // __VWAVE_H__
