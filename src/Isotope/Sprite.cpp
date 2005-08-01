#include "Sprite.h"
#include "../Source/GClasses/GMacros.h"
#include "../Source/GClasses/GSphereImage.h"
#include "../Source/GClasses/GXML.h"

Sprite::Sprite()
{
	m_pPrev = NULL;
	m_pNext = NULL;
}

Sprite::~Sprite()
{
	GAssert(m_pPrev == NULL && m_pNext == NULL, "Sprite is still linked in a list");
}

void Sprite::Unlink()
{
	if(m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if(m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	m_pPrev = NULL;
	m_pNext = NULL;
}

void Sprite::LinkAfter(Sprite* pPrev)
{
	m_pPrev = pPrev;
	m_pNext = pPrev->m_pNext;
	if(m_pNext)
		m_pNext->m_pPrev = this;
	pPrev->m_pNext = this;
}

bool LoadImageAndDepthMap(const char* szCacheName, GImage* pImage, double* pDepthMap)
{
	unsigned int nOldWidth = pImage->GetWidth();
	unsigned int nOldHeight = pImage->GetHeight();
	char szFilename[512];
	strcpy(szFilename, szCacheName);
	strcat(szFilename, ".bmp");
	if(!pImage->LoadBMPFile(szFilename))
		return false;
	if(pImage->GetWidth() != nOldWidth || pImage->GetHeight() != nOldHeight)
		return false;
	strcpy(szFilename, szCacheName);
	strcat(szFilename, ".dmap");
	FILE* pFile = fopen(szFilename, "rb");
	if(!pFile)
		return false;
	unsigned int nDepthMapArraySize = nOldWidth * nOldHeight;
	unsigned int nElements = fread(pDepthMap, sizeof(double), nDepthMapArraySize, pFile);
	fclose(pFile);
	if(nElements != nDepthMapArraySize)
	{
		GAssert(false, "error reading depth map file");
		return false;
	}
	return true;
}

void SaveImageAndDepthMap(const char* szCacheName, GImage* pImage, double* pDepthMap)
{
	char szFilename[512];
	strcpy(szFilename, szCacheName);
	strcat(szFilename, ".bmp");
	if(!pImage->SaveBMPFile(szFilename))
	{
		GAssert(false, "error saving image file");
		return;
	}
	strcpy(szFilename, szCacheName);
	strcat(szFilename, ".dmap");
	FILE* pFile = fopen(szFilename, "wb");
	if(!pFile)
	{
		GAssert(false, "error saving depth map file");
		return;
	}
	unsigned int nDepthMapArraySize = pImage->GetWidth() * pImage->GetHeight();
	unsigned int nElements = fwrite(pDepthMap, sizeof(double), nDepthMapArraySize, pFile);
	GAssert(nElements == nDepthMapArraySize, "error writing depth map file");
	fclose(pFile);
}

// ---------------------------------------------------------------------


Sprite2::Sprite2()
 : Sprite()
{
	m_x = 50;
	m_y = 50;
	m_nBaseFrame = 0;
	m_nFrame = 0;
	m_nStep = 0;
	m_pImages = new GImage[12];
	char szBuff[256];
	char szTmp[32];
	int n;
	for(n = 0; n < 12; n++)
	{
		strcpy(szBuff, "Jamie");
		itoa(n, szTmp , 10);
		strcat(szBuff, szTmp);
		strcat(szBuff, ".bmp");
		if(!m_pImages[n].LoadBMPFile(szBuff))
		{
			GAssert(false, "failed to load bmp file");
		}
	}
}

Sprite2::~Sprite2()
{
	delete[] m_pImages;
}

/*virtual*/ GImage* Sprite2::GetFrame(const GRect* pScreen, GRect* pOutSourceRect, GRect* pOutDestRect)
{
	GImage* pImage = &m_pImages[m_nFrame];
	pOutSourceRect->x = 0;
	pOutSourceRect->y = 0;
	pOutSourceRect->w = pImage->GetWidth();
	pOutSourceRect->h = pImage->GetHeight();
	pOutDestRect->y = m_y;
	pOutDestRect->w = pImage->GetWidth() * m_y / 200;
	pOutDestRect->h = pImage->GetHeight() * m_y / 200;
	pOutDestRect->x = m_x - pOutDestRect->w / 2;
	if(m_nStep > 9)
	{
		m_nStep -= 9;
		m_nFrame++;
		if(m_nFrame >= m_nBaseFrame + 3 || m_nFrame < m_nBaseFrame)
			m_nFrame = m_nBaseFrame;
	}
	return pImage;
}
