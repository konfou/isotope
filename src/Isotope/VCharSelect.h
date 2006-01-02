/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VCHARSELECT_H__
#define __VCHARSELECT_H__

#include "ViewPort.h"
#include "VWave.h"

struct GRect;
struct AvatarAccount;
class MCharSelectDialog;
class Controller;
class GPointerArray;
class GXMLTag;

// This shows a screen where you can select your character
class VCharSelect : public ViewPort
{
protected:
	enum DialogState
	{
		PickCharacter,
		ShowCharacter,
		EnterPassword,
		WrongPassword,
	};

	int m_nLeft, m_nTop;
	int m_nSelection;
	DialogState m_eState;
	GPointerArray* m_pAvatarAnimations;
	MCharSelectDialog* m_pDialog;
	double m_dTime;
	float m_fCameraDirection;
	int m_nClickX;
	double m_dWrongPasswordTime;
	VAudioPlayer m_audioPlayer;

public:
	VCharSelect(GRect* pRect, Controller* pController);
	virtual ~VCharSelect();

	virtual void Draw(SDL_Surface *pScreen);
	virtual void OnChar(char c) {}
	virtual void OnMouseDown(int x, int y);
	virtual void OnMouseUp(int x, int y);
	virtual void OnMousePos(int x, int y);

	void ReloadAccounts();
	bool CheckPassword();
	AvatarAccount* GetSelectedAccount();

protected:
	void RefreshEntireImage();
	void ClearAvatarAnimations();
	void DrawAvatars();
	bool CheckFile(const char* szFilename);
};

#endif // __VCHARSELECT_H__
