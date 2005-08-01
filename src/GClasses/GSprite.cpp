/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"

GSprite::GSprite() : GBucket()
{
	m_nXPos = 0;
	m_nYPos = 0;
	m_Frame = 0;
	m_pTiles = NULL;
}

GSprite::~GSprite()
{
	m_pTiles = NULL;
}

void GSprite::Init(GTile* pTiles, GWindow* pWindow)
{
	m_nXPos = 0;
	m_nYPos = 0;
	m_Frame = 0;
	m_pTiles = pTiles;
}

void GSprite::SetBackGroundCol(int nCol)
{
	GAssert(false, "Error, not implemented yet");
//	m_pTiles->SetBackGroundCol(nCol);
}

void GSprite::SetFrame(int nFrame)
{
	m_pTiles->SetTile(nFrame);
}

void GSprite::Draw()
{
	m_pTiles->Draw(m_nXPos, m_nYPos);
}

void GSprite::DrawScaled(float fScale)
{
	m_pTiles->DrawScaled(m_nXPos, m_nYPos, fScale);
}

bool GSprite::DoesOverlap(GSprite* pSprite)
{
	if(m_nXPos + m_pTiles->m_nTileWidth <= pSprite->m_nXPos)
		return false;
	if(m_nXPos >= pSprite->m_nXPos + pSprite->m_pTiles->m_nTileWidth)
		return false;
	if(m_nYPos + m_pTiles->m_nTileHeight <= pSprite->m_nYPos)
		return false;
	if(m_nYPos >= pSprite->m_nYPos + pSprite->m_pTiles->m_nTileHeight)
		return false;
	return true;
}
