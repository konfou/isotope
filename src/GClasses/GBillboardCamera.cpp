/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GBillboardCamera.h"
#include <math.h>

void GBillboardCamera::SetDirection(float radians)
{
	m_direction = radians;
	m_cosDirection = (float)cos(radians);
	m_sinDirection = (float)sin(radians);
}

void GBillboardCamera::SetPos(float x, float y)
{
	m_x = x;
	m_y = y;
	Backup(m_backup);
}

void GBillboardCamera::MapToScreen(float* pOutX, float* pOutY, float* pOutScale, float x, float y, float z, GRect* pScreenRect)
{
	float t;

	// Ajust for camera position
	x -= m_x;
	y -= m_y;

	// Ajust for Yaw
	t = x * m_cosDirection + y * m_sinDirection;
	y = y * m_cosDirection - x * m_sinDirection;
	x = t;

	// Scale relative to distance from camera
	y *= (m_horizonHeight / (m_horizonHeight + y));
	t = (float)1 - (y / m_horizonHeight);
	t *= m_zoom;
	y *= m_zoom;
	x *= t;
	z *= t;

	// Ajust for camera height
	z -= (m_z + m_vertajust);

	// Calculate literal screen rect
	*pOutX = x + pScreenRect->x + (pScreenRect->w >> 1);
	*pOutY = pScreenRect->y + pScreenRect->h - y - z;
	*pOutScale = t;
}

void GBillboardCamera::ScreenToMap(float* px, float* py, float* pSize, bool* pbSky, int xScr, int yScr, GRect* pScreenRect, int nSkyImageWidth)
{
	xScr -= pScreenRect->x;
	yScr -= pScreenRect->y;
	yScr = pScreenRect->h - yScr + (int)(m_z + m_vertajust);
	xScr -= (pScreenRect->w >> 1);
	float horiz = m_horizonHeight * m_zoom;
	if(yScr > horiz)
	{
		*pbSky = true;
		*px = xScr - m_direction * nSkyImageWidth / ((float)3.14159265359 * 2);
		*py = yScr - horiz + 1;
		*pSize = 1;
	}
	else
	{
		float x, y, t;
		*pbSky = false;
		t = m_zoom * ((float)1 - (yScr / horiz));
		x = xScr / t;
		y = yScr / t;
		*pSize = t;

		// Ajust for yaw
		t = x * m_cosDirection - y * m_sinDirection;
		y = y * m_cosDirection + x * m_sinDirection;
		x = t;

		// Ajust for camera position
		*px = x + m_x;
		*py = y + m_y;
	}
}

void GBillboardCamera::CalcBillboardRect(GRect* pOutRect, GPosSize* pPosSize, GRect* pScreenRect)
{
	float x, y, scale;
	float wid = m_cosDirection * m_cosDirection * pPosSize->sx + m_sinDirection * m_sinDirection * pPosSize->sy;
	float hgt = pPosSize->sz;
	MapToScreen(&x, &y, &scale, pPosSize->x, pPosSize->y, pPosSize->z, pScreenRect);
	wid *= scale;
	hgt *= scale;
	pOutRect->x = (int)(x - wid / 2);
	pOutRect->y = (int)(y - hgt);
	pOutRect->w = (int)wid;
	pOutRect->h = (int)hgt;
}

void GBillboardCamera::CalcPanelTrapezoid(GPanelPos* pOutPos, GPosSize* pPosSize, GRect* pScreenRect)
{
	// Calculate bounding points
	float x1, y1, scale1, x2, y2, scale2;
	float hdx = pPosSize->sx / 2;
	float hdy = pPosSize->sy / 2;
	MapToScreen(&x1, &y1, &scale1, pPosSize->x - hdx, pPosSize->y - hdy, pPosSize->z, pScreenRect);
	MapToScreen(&x2, &y2, &scale2, pPosSize->x + hdx, pPosSize->y + hdy, pPosSize->z, pScreenRect);
	if(x1 < x2)
	{
		pOutPos->w = x2 - x1;
		pOutPos->h = pPosSize->sz * scale1;
		pOutPos->x = x1;
		pOutPos->y = y1 - pOutPos->h;
		pOutPos->dy = ((y2 - (pPosSize->sz * scale2)) - pOutPos->y) / pOutPos->w;
		pOutPos->dh = (pPosSize->sz * (scale2 - scale1)) / pOutPos->w;
	}
	else
	{
		pOutPos->w = x1 - x2;
		pOutPos->h = pPosSize->sz * scale2;
		pOutPos->x = x2;
		pOutPos->y = y2 - pOutPos->h;
		pOutPos->dy = ((y1 - (pPosSize->sz * scale1)) - pOutPos->y) / pOutPos->w;
		pOutPos->dh = (pPosSize->sz * (scale1 - scale2)) / pOutPos->w;
	}
	if(scale1 < 0 || scale2 < 0)
		pOutPos->w = 0;
}

float GBillboardCamera::CalculateDistanceFromCamera(float x, float y)
{
	// Ajust for camera position
	x -= m_x;
	y -= m_y;

	// Ajust for Yaw
	y = y * m_cosDirection - x * m_sinDirection;

	return y;
}

// We don't want the camera immediately behind the avatar's head, so
// let's back it up a bit in the opposite direction that we're facing
void GBillboardCamera::Backup(float distance)
{
	m_y -= (m_cosDirection * distance);
	m_x += (m_sinDirection * distance);
}

void GBillboardCamera::Move(float dx, float dy)
{
	m_x += dx * m_cosDirection;
	m_y += dx * m_sinDirection;
	m_x -= dy * m_sinDirection;
	m_y += dy * m_cosDirection;
}

void GBillboardCamera::RecalcVertAjust(int nScreenVerticalCenter)
{
	// Move the camera vertically until the avatar is at the center of the screen
	m_vertajust = m_backup * (m_horizonHeight / (m_horizonHeight + m_backup)) * m_zoom - nScreenVerticalCenter;
}

void GBillboardCamera::AjustZoom(float fac, int nScreenVerticalCenter)
{
	// Disallow excessive zooming
	if(fac > 1)
	{
		if(m_zoom > 1.4)
			return;
	}
	else
	{
		if(m_zoom < .4)
			return;
	}

	// Ajust the zoom
	m_zoom *= fac;

	// Move the camera farther away when zoomed out.  (This combination of zoom and moving
	// the camera seems to give the user a more useful control of the view than either of
	// them alone.)
	m_backup = (float)500 / m_zoom;
	RecalcVertAjust(nScreenVerticalCenter);
}

void GBillboardCamera::AjustHorizonHeight(float factor, int nScreenVerticalCenter)
{
	if(factor > 1)
	{
		if(m_horizonHeight > 10000)
			return;
	}
	else
	{
		if(m_horizonHeight < 300)
			return;
	}
	m_horizonHeight *= factor;
	RecalcVertAjust(nScreenVerticalCenter);
}

