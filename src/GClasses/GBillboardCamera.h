/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GBILLBOARDCAMERA_H__
#define __GBILLBOARDCAMERA_H__

#include "GImage.h"

// This represents the position and size of a game object
struct GPosSize
{
public:
	float x, y, z;
	float sx, sy, sz;
};

// Defines a trapezoid with paralell vertical sides.  (x,y) is the top corner of the left-most
// side.  "w" is the width, "h" is the height of the left side, "dy" is the change in y value
// per unit change in x, and "dh" is the change in height per unit change in x.
struct GPanelPos
{
public:
	float x, y, w, h, dy, dh;
};

// This class represents the view point from which the game view is rendered
class GBillboardCamera
{
protected:
	float m_x;
	float m_y;
	float m_z;
	float m_direction;
	float m_cosDirection;
	float m_sinDirection;
	float m_horizonHeight;
	float m_backup;
	float m_zoom;
	float m_vertajust;

public:
	GBillboardCamera(int nScreenVerticalCenter)
	{
		m_x = 0;
		m_y = 0;
		m_z = 0;
		m_direction = 0;
		m_cosDirection = 1;
		m_sinDirection = 0;
		m_horizonHeight = 300;
		m_backup = 300;
		m_zoom = 1;
		m_vertajust = 0;
		AjustZoom((float)1.001, nScreenVerticalCenter);
	}

	~GBillboardCamera()
	{
	}

	// Move the camera position
	void SetPos(float x, float y);

	// Set the yaw direction
	void SetDirection(float radians);

	// Get the yaw direction
	float GetDirection() { return m_direction; }

	// Get how high the horizon appears above the bottom of the screen.  If m_zoom is 1 then
	// this is measured in pixels
	float GetHorizonHeight() { return m_horizonHeight; }

	// Converts a point on the map to a point on the screen
	void MapToScreen(float* pOutX, float* pOutY, float* pOutScale, float x, float y, float z, GRect* pScreenRect);

	// Converts a point on the screen to a point on the map where z is zero
	void ScreenToMap(float* px, float* pY, float* pSize, bool* pbSky, int x, int y, GRect* pScreenRect, int nSkyImageWidth);

	// Converts a GPosSize to a GRect in which you should draw the billboard
	void CalcBillboardRect(GRect* pOutRect, GPosSize* pPosSize, GRect* pScreenRect);

	// Computes the trapezoid in which to draw a panel (a billboard that doesn't necessarily face the camera)
	void CalcPanelTrapezoid(GPanelPos* pOutPos, GPosSize* pPosSize, GRect* pScreenRect);

	// Determines the distance from a point to the camera
	float CalculateDistanceFromCamera(float x, float y);

	// Ajust the zoom by the factor fac
	void AjustZoom(float fac, int nScreenVerticalCenter);

	// Ajust the horizon height by the factor fac
	void AjustHorizonHeight(float factor, int nScreenVerticalCenter);

	void GetPosition(float* px, float* py, float* pz) { *px = m_x; *py = m_y; *pz = m_z; }
	void Move(float dx, float dy);
protected:
	// Move the camera in the opposite direction that it is facing
	void Backup(float distance);

	void RecalcVertAjust(int nScreenVerticalCenter);
};


#endif // __GBILLBOARDCAMERA_H__
