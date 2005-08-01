/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifdef WIN32
#include <windows.h>
#endif // WIN32
#include "GDirectSound.h"
#include "GDSound.h"
#include "GMacros.h"

#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

void GLogSoundError(HRESULT hErr)
{
	char dserr[256];
	char err[1024];

	switch (hErr)
	{
		case DSERR_ALLOCATED : sprintf(dserr, "DSERR_ALLOCATED"); break;
		case DSERR_CONTROLUNAVAIL : sprintf(dserr, "DSERR_CONTROLUNAVAIL"); break;
		case DSERR_INVALIDPARAM : sprintf(dserr, "DSERR_INVALIDPARAM"); break;
		case DSERR_INVALIDCALL : sprintf(dserr, "DSERR_INVALIDCALL"); break;
		case DSERR_GENERIC : sprintf(dserr, "DSERR_GENERIC"); break;
		case DSERR_PRIOLEVELNEEDED : sprintf(dserr, "DSERR_PRIOLEVELNEEDED"); break;
		case DSERR_OUTOFMEMORY : sprintf(dserr, "DSERR_OUTOFMEMORY"); break;
		case DSERR_BADFORMAT : sprintf(dserr, "DSERR_BADFORMAT"); break;
		case DSERR_UNSUPPORTED : sprintf(dserr, "DSERR_UNSUPPORTED"); break;
		case DSERR_NODRIVER : sprintf(dserr, "DSERR_NODRIVER"); break;
		case DSERR_ALREADYINITIALIZED : sprintf(dserr, "DSERR_ALREADYINITIALIZED"); break;
		case DSERR_NOAGGREGATION : sprintf(dserr, "DSERR_NOAGGREGATION"); break;
		case DSERR_BUFFERLOST : sprintf(dserr, "DSERR_BUFFERLOST"); break;
		case DSERR_OTHERAPPHASPRIO : sprintf(dserr, "DSERR_OTHERAPPHASPRIO"); break;
		case DSERR_UNINITIALIZED : sprintf(dserr, "DSERR_UNINITIALIZED"); break;

		default : sprintf(dserr, "Unknown Error"); break;
	}

	sprintf(err, "DirectSound Error %s", dserr);
	GAssert(false, err);
	PostQuitMessage(0);
}

GDirectSound::GDirectSound()
{
	m_lpDS = NULL;
}

GDirectSound::~GDirectSound()
{
	if(m_lpDS) m_lpDS->Release();
}

BOOL GDirectSound::Init(HWND hWnd)
{
	HRESULT rval;

	rval = DirectSoundCreate(NULL, &m_lpDS, NULL);
	if(rval != DS_OK)
		GLogSoundError(rval);

	rval = m_lpDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
	if(rval != DS_OK)
		GLogSoundError(rval);

	return TRUE;
}

BOOL GDirectSound::GetCaps()
{
	HRESULT rval;

	rval = m_lpDS->GetCaps(&m_DSCaps);
	if(rval != DS_OK) return FALSE;

	return TRUE;
}
