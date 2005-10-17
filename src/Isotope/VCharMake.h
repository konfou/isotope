/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VCHARMAKE_H__
#define __VCHARMAKE_H__

#include "ViewPort.h"

struct GRect;
class GPointerArray;
class Controller;
class MCharMakeDialog;

// This shows a screen where you can select your character
class VCharMake : public ViewPort
{
protected:
	enum DialogState
	{
		PickCharacter,
		ShowCharacter,
		EnterPassword,
	};

	DialogState m_eState;
	const char* m_szTypeBuffer;
	GPointerArray* m_pAvatarAnimations;
	MCharMakeDialog* m_pDialog;
	double m_dTime;
	float m_fCameraDirection;
	int m_nFirstAvatar;
	int m_nClickX;
	float m_fParadePos;
	const char* m_szAvatarID;

public:
	VCharMake(GRect* pRect, const char* szTypeBuffer, Controller* pController);
	virtual ~VCharMake();

	virtual void Draw(SDL_Surface *pScreen);
	void OnMouseDown(Controller* pController, int x, int y);
	void OnMouseUp(Controller* pController, int x, int y);
	const char* GetAvatarID() { return m_szAvatarID; }

protected:
	void RefreshEntireImage();
	void MakeAvatarList();
	void DrawAvatars();
};

#endif // __VCHARMAKE_H__
