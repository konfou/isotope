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
	};

	int m_nLeft, m_nTop;
	DialogState m_eState;
	GPointerArray* m_pAvatarAnimations;
	MCharMakeDialog* m_pDialog;
	double m_dTime;
	float m_fCameraDirection;
	int m_nFirstAvatar;
	int m_nClickX;
	float m_fParadePos;
	const char* m_szAvatarID;

public:
	VCharMake(GRect* pRect, Controller* pController);
	virtual ~VCharMake();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c);
	virtual void OnMouseDown(int x, int y);
	virtual void OnMouseUp(int x, int y);
	virtual void OnMousePos(int x, int y) {}

	const char* GetAvatarID() { return m_szAvatarID; }
	const char* GetPassword();
	const char* GetUsername();

protected:
	void RefreshEntireImage();
	void MakeAvatarList();
	void DrawAvatars();
};

#endif // __VCHARMAKE_H__
