/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GDINPUT_H__
#define __GDINPUT_H__

#include <dinput.h>

class GInput
{
public:
	GInput(void);
	~GInput(void);

	BOOL Init(void *hInst, HWND hWnd);
	void Update(void);

	void SetActiveDevices(BOOL bMouse, BOOL bKeyboard, BOOL bJoystick);

private:
	void ReAcquire(void);
	void UnAcquire(void);
	void SetMouseAbs(void);
	void SetJoystickAbs(void);
	void RunMouseControlPanel(HWND hWnd = NULL);
	void RunJoystickControlPanel(HWND hWnd = NULL);

public:
	LPDIRECTINPUT m_lpDI;
	LPDIRECTINPUTDEVICE m_lpDIDKeyboard;
	LPDIRECTINPUTDEVICE m_lpDIDMouse;
	LPDIRECTINPUTDEVICE2 m_lpDIDJoystick;

	POINT Mouse;
	POINT Joystick;
	BOOL JoystickB1, JoystickB2;
	BOOL MouseLB, MouseMB, MouseRB;
	BYTE Keys[256];

	BOOL m_bMouse;
	BOOL m_bKeyboard;
	BOOL m_bJoystick;
};

#endif // __GDINPUT_H__
