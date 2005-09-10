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

struct GRect;
struct AvatarAccount;
class GWidgetContainer;
class GWidgetTextButton;
class GWidget;
class Controller;
class GPointerArray;

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

	DialogState m_eState;
	const char* m_szTypeBuffer;
	GImage* m_pImage;
	GPointerArray* m_pAvatarAnimations;
	GWidgetContainer* m_pWidgetContainer;
	GWidgetTextButton* m_pNewCharButton;
	GWidgetTextButton* m_pOKButton;
	double m_dTime;
	float m_fCameraDirection;
	int m_nFirstAvatar;
	int m_nClickX;
	float m_fParadePos;
	AvatarAccount* m_pSelectedAccount;
	double m_dWrongPasswordTime;

public:
	VCharSelect(GRect* pRect, const char* szTypeBuffer);
	virtual ~VCharSelect();

	virtual void Draw(SDL_Surface *pScreen);
	void OnMouseDown(Controller* pController, int x, int y);
	void OnMouseUp(Controller* pController, int x, int y);
	void ReloadAccounts();

protected:
	void RefreshEntireImage();
	void ClearAvatarAnimations();
	void ReleaseButton(Controller* pController, GWidgetTextButton* pButton);
	void DrawAvatars();
	void AttemptLogin(Controller* pController);
	bool CheckFile(const char* szFilename);
};

#endif // __VCHARSELECT_H__
