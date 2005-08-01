/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "GnuSDK.h"
#include "GDInput.h"

static LPDIRECTINPUTDEVICE lpDID = NULL;
#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

BOOL FAR PASCAL EnumJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
	LPDIRECTINPUT pDI = (LPDIRECTINPUT)pvRef;

	if(pDI->CreateDevice(pdinst->guidInstance, &lpDID, NULL) != DI_OK)
		return DIENUM_CONTINUE;

	return DIENUM_STOP;
}

GInput::GInput(void)
{
	m_lpDI = NULL;
	m_lpDIDKeyboard = NULL;
	m_lpDIDMouse = NULL;
	m_lpDIDJoystick = NULL;

	m_bMouse = FALSE;
	m_bKeyboard = FALSE;
	m_bJoystick = FALSE;

	Joystick.x = 0;
	Joystick.y = 0;
	Mouse.x = 0;
	Mouse.y = 0;
}

GInput::~GInput(void)
{
	RELEASE(m_lpDIDKeyboard);
	RELEASE(m_lpDIDMouse);
	RELEASE(m_lpDIDJoystick);
	RELEASE(m_lpDI);
}

BOOL GInput::Init(void *hInst, HWND hWnd)
{
	HRESULT rval;

	rval = DirectInputCreate((HINSTANCE__*)hInst, DIRECTINPUT_VERSION, &m_lpDI, NULL);
	if(rval != DI_OK) return FALSE;

	// Create the mouse device
	rval = m_lpDI->CreateDevice(GUID_SysMouse, &m_lpDIDMouse, NULL);
	if(rval == DI_OK)
	{
		m_lpDIDMouse->SetDataFormat(&c_dfDIMouse);
		m_lpDIDMouse->SetCooperativeLevel((HWND__*)hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

		rval = m_lpDIDMouse->Acquire();
		if(rval != DI_OK) return FALSE;

		m_bMouse = TRUE;
	}

	// Create the keyboard device
	rval = m_lpDI->CreateDevice(GUID_SysKeyboard, &m_lpDIDKeyboard, NULL);
	if(rval == DI_OK)
	{
		m_lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
		m_lpDIDKeyboard->SetCooperativeLevel((HWND__*)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		rval = m_lpDIDKeyboard->Acquire();
		if(rval != DI_OK) return FALSE;

		m_bKeyboard = TRUE;
	}

	// Enumerate the joystick device
	rval = m_lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystick, m_lpDI, DIEDFL_ATTACHEDONLY);
	if(lpDID != NULL)
	{
		rval = lpDID->QueryInterface(IID_IDirectInputDevice2, (LPVOID *)&m_lpDIDJoystick);
		if(rval != DI_OK) return FALSE;

		RELEASE(lpDID);

		m_lpDIDJoystick->SetDataFormat(&c_dfDIJoystick);
		m_lpDIDJoystick->SetCooperativeLevel((HWND__*)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		// Set the X-axis range (-1000 to +1000)
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwObj = DIJOFS_X;
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin = -1000;
		diprg.lMax = +1000;

		if(m_lpDIDJoystick->SetProperty(DIPROP_RANGE, &diprg.diph) != DI_OK)
			return FALSE;

		// And again for Y-axis range
		diprg.diph.dwObj = DIJOFS_Y;

		if(m_lpDIDJoystick->SetProperty(DIPROP_RANGE, &diprg.diph) != DI_OK)
			return FALSE;

		// Set X axis dead zone to 10%
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj = DIJOFS_X;
		dipdw.diph.dwHow = DIPH_BYOFFSET;
		dipdw.dwData = 1000;

		if(m_lpDIDJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph) != DI_OK)
			return FALSE;

		dipdw.diph.dwObj = DIJOFS_Y;

		// Set Y axis dead zone to 10%
		if(m_lpDIDJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph) != DI_OK)
			return FALSE;

		rval = m_lpDIDJoystick->Acquire();
		if(rval != DI_OK) return FALSE;

		m_bJoystick = TRUE;
	}

	return TRUE;
}

void GInput::ReAcquire(void)
{
	m_lpDIDMouse->Acquire();
	m_lpDIDKeyboard->Acquire();
	m_lpDIDJoystick->Acquire();
}

void GInput::UnAcquire(void)
{
	m_lpDIDMouse->Unacquire();
	m_lpDIDKeyboard->Unacquire();
	m_lpDIDJoystick->Unacquire();
}

void GInput::Update(void)
{
	DIMOUSESTATE MouseState;
	DIJOYSTATE JoyState;

	if(m_bMouse)
	{
		if(m_lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState) == (DIERR_INPUTLOST | DIERR_NOTACQUIRED))
			m_lpDIDMouse->Acquire();

		Mouse.x = MouseState.lX;
		Mouse.y = MouseState.lY;

		MouseLB = MouseState.rgbButtons[0];
		MouseRB = MouseState.rgbButtons[1];
		MouseMB = MouseState.rgbButtons[2];
	}

	if(m_bKeyboard)
	{
		if(m_lpDIDKeyboard->GetDeviceState(256, &Keys) == (DIERR_INPUTLOST | DIERR_NOTACQUIRED))
			m_lpDIDKeyboard->Acquire();
	}

	if(m_bJoystick)
	{
		m_lpDIDJoystick->Poll();

		if(m_lpDIDJoystick->GetDeviceState(sizeof(JoyState), &JoyState) == (DIERR_INPUTLOST | DIERR_NOTACQUIRED))
			m_lpDIDJoystick->Acquire();

		Joystick.x = JoyState.lX;
		Joystick.y = JoyState.lY;

		JoystickB1 = JoyState.rgbButtons[0];
		JoystickB2 = JoyState.rgbButtons[1];
   }
}

void GInput::SetActiveDevices(BOOL bMouse, BOOL bKeyboard, BOOL bJoystick)
{
	m_bMouse = bMouse;
	m_bKeyboard = bKeyboard;
	m_bJoystick = bJoystick;
}

void GInput::SetMouseAbs(void)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DIPROPAXISMODE_ABS;
	m_lpDIDMouse->SetProperty(DIPROP_AXISMODE, &dipdw.diph);
}

void GInput::SetJoystickAbs(void)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = DIPROPAXISMODE_ABS;
	m_lpDIDJoystick->SetProperty(DIPROP_AXISMODE, &dipdw.diph);
}

void GInput::RunMouseControlPanel(HWND hWnd)
{
	m_lpDIDMouse->RunControlPanel(hWnd, 0);
}

void GInput::RunJoystickControlPanel(HWND hWnd)
{
	m_lpDIDJoystick->RunControlPanel(hWnd, 0);
}
