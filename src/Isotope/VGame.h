/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VGAME_H__
#define __VGAME_H__

#include "ViewPort.h"

class MGameClient;
class MObject;
class GImage;
class MAnimation;
class GBillboardCamera;
struct GRect;
struct VVertGroundParams;
struct VHorizGroundParams;
struct GPosSize;
class GWidgetStyle;
class GWidgetButton;

// This represents the view for the game part of the application.  (In other words, this
// is what people will spend most of their time looking at when they play the game).  It
// basically draws the ground and sky and all the objects in the right place and the right
// size.
class VGame : public ViewPort
{
protected:
	MGameClient* m_pGameClient;
	GBillboardCamera* m_pCamera;
	GImage* m_pImageGround;
	GImage* m_pImageSky;
	VVertGroundParams* m_pVertGroundParams;
	VHorizGroundParams* m_pHorizGroundParams;
	double m_dTime;
	bool m_bTerrain;

	float m_nMapXMin;
	float m_nMapXMax;
	float m_nMapYMin;
	float m_nMapYMax;

	float m_fSelectionX1;
	float m_fSelectionY1;
	float m_fSelectionX2;
	float m_fSelectionY2;

public:
	VGame(GRect* pRect, MGameClient* pGameClient, GImage* pSkyImage, GImage* pGroundImage);
	virtual ~VGame();

	virtual void Draw(SDL_Surface *pScreen);
	GBillboardCamera* GetCamera() { return m_pCamera; }
	void ScreenToMap(float* px, float* py, bool* pbSky, int x, int y);

	void ToggleTerrain() { m_bTerrain = !m_bTerrain; }
	void SetRect(GRect* pRect);
	void SetSelectionRect(float x1, float y1, float x2, float y2);
	void SetSkyImage(GImage* pImage);
	void SetGroundImage(GImage* pImage);

protected:
	void DrawSpritesNoTerrain(SDL_Surface* pScreen);
	void DrawGroundAndSkyNoTerrain(SDL_Surface* pScreen);
	void DrawEverythingWithTerrain(SDL_Surface* pScreen);
	void DrawSprite(SDL_Surface* pScreen, MObject* pSprite);
	void DrawBillboard(SDL_Surface* pScreen, GImage* pImage, GRect* pSrcRect, GPosSize* pGhostPos);
	void DrawPanel(SDL_Surface* pScreen, GImage* pImage, GRect* pSrcRect, GPosSize* pGhostPos);
};

#endif // __VGAME_H__
