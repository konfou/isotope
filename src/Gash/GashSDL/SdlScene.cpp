/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "SdlScene.h"
#include "../BuiltIns/GashString.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GSphereImage.h"
#include "SdlFrame.h"
#include "../../GClasses/GHashTable.h"

struct TransformImagePair
{
	Transform trans;
	GImage image;
	double* pDepthMap;

	TransformImagePair()
	{
		pDepthMap = NULL;
	}

	~TransformImagePair()
	{
		delete(pDepthMap);
	}
};

class SdlView
{
friend class SdlScene;
protected:
	Point3D m_cameraPos;
	int m_nFrames;
	GPreRendered3DScreen** m_pSphereFrames;
	TransformImagePair* m_pBufferedImages;

public:
	SdlView();
	virtual ~SdlView();

	void render(G3DObject* pObj3D, Point3D* pCameraPos, int nFrames, int nCircumference, GImage* pBackdrops, int nBackdropFrames);
	void save(Engine* pEngine, FILE* pFile, const wchar_t* wszFilename);
	GImage* Draw(double** ppDepthMap, int nFrame, const GRect* pScreen, const Transform* pCamera);
};

SdlView::SdlView()
{
	m_pSphereFrames = NULL;
	m_pBufferedImages = NULL;
}

SdlView::~SdlView()
{
	if(m_pSphereFrames)
	{
		int n;
		for(n = 0; n < m_nFrames; n++)
			delete(m_pSphereFrames[n]);
		delete(m_pSphereFrames);
	}
	delete [] m_pBufferedImages;
}

void SdlView::render(G3DObject* pObj3D, Point3D* pCameraPos, int nFrames, int nCircumference, GImage* pBackdrops, int nBackdropFrames)
{
	// allocate the sphere frames and the buffered images
	m_nFrames = nFrames;
	m_pSphereFrames = new GPreRendered3DScreen*[nFrames];
	memset(m_pSphereFrames, '\0', nFrames * sizeof(GPreRendered3DScreen*));
	m_pBufferedImages = new TransformImagePair[nFrames];
	m_cameraPos = *pCameraPos;

	// do the rendering
	int nAllowedSubRays = 7; // todo: make this configurable
	double dLightThreshold = 256; // todo: make this configurable
	int n;
	for(n = 0; n < nFrames; n++)
	{
		GImage* pBackdropImage = NULL;
		if(nBackdropFrames > 0)
			pBackdropImage = &pBackdrops[n * nBackdropFrames / nFrames];
		m_pSphereFrames[n] = new GPreRendered3DScreen(nCircumference >> 2);
		m_pSphereFrames[n]->Render(pObj3D, pCameraPos, nAllowedSubRays, n, nFrames, dLightThreshold, pBackdropImage);
	}
}

GImage* SdlView::Draw(double** ppDepthMap, int nFrame, const GRect* pScreen, const Transform* pCamera)
{
	if(!m_pBufferedImages)
		m_pBufferedImages = new TransformImagePair[m_nFrames];

	// See if we can use a cached image
	GImage* pImage = &m_pBufferedImages[nFrame].image;
	*ppDepthMap = m_pBufferedImages[nFrame].pDepthMap;
	if((int)pImage->GetWidth() == pScreen->w && (int)pImage->GetHeight() == pScreen->h)
	{
		if(memcmp(&m_pBufferedImages[nFrame].trans, pCamera, sizeof(struct Transform)) == 0)
			return pImage;
	}
	else
	{
		pImage->SetSize(pScreen->w, pScreen->h);
		if(!m_pBufferedImages[nFrame].pDepthMap)
			m_pBufferedImages[nFrame].pDepthMap = new double[pImage->GetWidth() * pImage->GetHeight()];
		*ppDepthMap = m_pBufferedImages[nFrame].pDepthMap;
	}

	// Make a new image
	double* pDistMap = *ppDepthMap;
	m_pBufferedImages[nFrame].trans = *pCamera;
	GPreRendered3DScreen* pScene = m_pSphereFrames[nFrame];
	pScene->SetCameraAngle(pCamera, false);
	int x, y;
	int nWidth = pImage->GetWidth();
	int nRight = pScreen->x + pScreen->w;
	int nBottom = pScreen->y + pScreen->h;
	int nCenterX = pScreen->x + pScreen->w / 2;
	int nCenterY = pScreen->y + pScreen->h / 2;
	for(y = pScreen->y; y < nBottom; y++)
	{
		for(x = pScreen->x; x < nRight; x++)
			pImage->SetPixel(x, y, pScene->GetPixel(x - nCenterX, y - nCenterY, &pDistMap[y * nWidth + x]));
	}

	return pImage;
}

void SdlView::save(Engine* pEngine, FILE* pFile, const wchar_t* wszFilename)
{
	// Write the camera pos
	if(fwrite(&m_cameraPos, sizeof(Point3D), 1, pFile) != 1)
		pEngine->ThrowIOError(L"Error writing to file: %s", wszFilename);

	// Write the frame count
	if(fwrite(&m_nFrames, sizeof(int), 1, pFile) != 1)
		pEngine->ThrowIOError(L"Error writing to file: %s", wszFilename);

	// Write each frame
	int n;
	for(n = 0; n < m_nFrames; n++)
	{
		if(!m_pSphereFrames[n]->save(pFile))
			pEngine->ThrowIOError(L"Error saving sphere image frame to file: %s", wszFilename);
	}
}

// --------------------------------------------------------------------------------

void RegisterSdlScene(GConstStringHashTable* pTable)
{
	pTable->Add("method !render(String)", new EMethodPointerHolder((MachineMethod1)&SdlScene::render));
	pTable->Add("method draw(&SdlFrame, Point3D, Integer)", new EMethodPointerHolder((MachineMethod3)&SdlScene::draw));
}


SdlScene::SdlScene(Engine* pEngine)
 : WrapperObject(pEngine, "SdlScene")
{
	m_nViews = 0;
	m_pViews = NULL;
	m_pCurrentView = NULL;
	m_nCounter = 0;
}

SdlScene::~SdlScene()
{
	delete [] m_pViews;
}

void SdlScene::render(Engine* pEngine, EVar* pFilename)
{
	SdlScene* pNewScene = new SdlScene(pEngine);
	pEngine->SetThis(pNewScene);
	pNewScene->renderHelper(pEngine, pFilename);
}

void SdlScene::save(Engine* pEngine, EVar* pFilename)
{
	// Open the file
	GString* pFN = &pFilename->pStringObject->m_value;
	FileHolder hFile(NULL);
	pEngine->OpenFile(&hFile, pFN, "wb");
	FILE* pFile = hFile.Get();

	// Write an identifier
	if(fwrite("abcd", 4, 1, pFile) != 1)
		pEngine->ThrowIOError(L"Error writing to file: %s", pFN->GetString());

	// Write the view count
	if(fwrite(&m_nViews, sizeof(int), 1, pFile) != 1)
		pEngine->ThrowIOError(L"Error writing to file: %s", pFN->GetString());

	// Write each view
	int n;
	for(n = 0; n < m_nViews; n++)
		m_pViews[n].save(pEngine, pFile, pFN->GetString());
}

void SdlScene::load(Engine* pEngine, EVar* pFilename)
{
	// Open the file
	GString* pFN = &pFilename->pStringObject->m_value;
	FileHolder hFile(NULL);
	pEngine->OpenFile(&hFile, &pFilename->pStringObject->m_value, "rb");
	FILE* pFile = hFile.Get();

	// Read the identifier
	char id[5];
	if(fread(id, 4, 1, pFile) != 1)
		pEngine->ThrowIOError(L"Error reading from file: %s", pFN->GetString());

//xxx
}

void SdlScene::renderHelper(Engine* pEngine, EVar* pFilename)
{
	// Load the XML file
	GXMLTag* pRootTag;
	Holder<GXMLTag*> hTag(NULL);
	{
		char szFilename[512];
		pFilename->pStringObject->m_value.GetAnsi(szFilename);
		const char* szXmlError;
		int nErrorLine, nErrorCol;
		int nFileSize;
		Holder<char*> hXmlFile(pEngine->LoadFile(szFilename, &nFileSize));
		hTag.Set(GXMLTag::FromString(hXmlFile.Get(), nFileSize, &szXmlError, NULL, &nErrorLine, &nErrorCol));
		pRootTag = hTag.Get();
		if(!pRootTag)
			pEngine->ThrowXmlError(szXmlError, nErrorLine, nErrorCol);
	}

	// Get the number of frames
	GXMLAttribute* pFramesAttr = pRootTag->GetAttribute("Frames");
	if(!pFramesAttr)
		pEngine->ThrowXmlError(pRootTag, L"Expected a \"Frames\" attribute");
	int nFrames = atoi(pFramesAttr->GetValue());

	// Get the circumference
	GXMLAttribute* pCircumferenceAttr = pRootTag->GetAttribute("Circumference");
	if(!pCircumferenceAttr)
		pEngine->ThrowXmlError(pRootTag, L"Expected a \"Circumference\" attribute");
	int nCircumference = atoi(pCircumferenceAttr->GetValue());

	// Parse the backdrop and load the bitmaps
	GXMLTag* pBackdropTag = pRootTag->GetChildTag("Backdrop");
	if(!pBackdropTag)
		pEngine->ThrowXmlError(pRootTag, L"Expected a <Backdrop> child tag");
	GXMLTag* pChild;
	ArrayHolder<GImage*> hBackdropFrames(new GImage[pBackdropTag->GetChildTagCount()]);
	GImage* pBackdropFrames = hBackdropFrames.Get();
	int n = 0;
	for(pChild = pBackdropTag->GetFirstChildTag(); pChild; pChild = pBackdropTag->GetNextChildTag(pChild))
	{
		GXMLAttribute* pNameAttr = pChild->GetAttribute("Name");
		if(!pNameAttr)
			pEngine->ThrowXmlError(pChild, L"Expected a \"Name\" attribute");
		FileHolder hFile(NULL);
		pEngine->OpenFile(&hFile, pNameAttr->GetValue(), "rb");
		bool bOK = pBackdropFrames[n].LoadBMPFile(hFile.Get());
		if(!bOK)
		{
			GString s;
			s.Add(pNameAttr->GetValue());
			pEngine->ThrowIOError(L"Error loading backdrop bmp file: %s", s.GetString());
		}
		n++;
	}

	// Parse the objects
	GXMLTag* pObjectsTag = pRootTag->GetChildTag("Objects");
	if(!pObjectsTag)
		pEngine->ThrowXmlError(pRootTag, L"Expected a <Objects> child tag");
	G3DObject* pObj3D = G3DObject::ObjectsFromXML(pObjectsTag);
	if(!pObj3D)
		pEngine->ThrowXmlError(pObjectsTag, L"Failed to parse the <Objects> tag");

	// Parse the view points
	GXMLTag* pViewsTag = pRootTag->GetChildTag("Views");
	if(!pViewsTag)
		pEngine->ThrowXmlError(pRootTag, L"Expected a <Views> child tag");
	m_pViews = new SdlView[pViewsTag->GetChildTagCount()];
	GXMLTag* pViewTag;
	n = 0;
	m_nViews = pViewsTag->GetChildTagCount();
	for(pViewTag = pViewsTag->GetFirstChildTag(); pViewTag; pViewTag = pViewsTag->GetNextChildTag(pViewTag))
	{
		// Parse the camera pos
		Point3D cameraPos;
		memset(&cameraPos, '\0', sizeof(Point3D));
		cameraPos.FromXML(pViewTag);

		// Render the view point
		m_pViews[n].render(pObj3D, &cameraPos, nFrames, nCircumference, pBackdropFrames, pBackdropTag->GetChildTagCount());
		n++;
	}
}

void SdlScene::GetBestView(Engine* pEngine, Point3D* pFocusPoint)
{
	if(m_nViews <= 0)
		pEngine->ThrowSdlError(L"There's nothing to view in this scene");

	// Pick the view point closest to the focus point
	int n;
	SdlView* pView = &m_pViews[0];
	double dBest = pFocusPoint->GetDistance(&m_pViews[0].m_cameraPos);
	for(n = 1; n < m_nViews; n++)
	{
		double dNew = pFocusPoint->GetDistance(&m_pViews[n].m_cameraPos);
		if(dNew < dBest)
		{
			dBest = dNew;
			pView = &m_pViews[n];
		}
	}
	m_pCurrentView = pView;

	// Recalculate the camera angle
	Vector v;
	v.FromAToB(&m_pCurrentView->m_cameraPos, pFocusPoint);
	m_camera.FromVector(&v);
	m_camera.dScale = 1; // todo: recalculate the scale somehow
}

void SdlScene::draw(Engine* pEngine, EVar* pFrame, EVar* pFocusPoint, EVar* pFrameNumber)
{
	// Convert the focus point
	Point3D focusPoint;
	focusPoint.x = ((IntObject*)pFocusPoint->pObjectObject->arrFields[0])->m_value;
	focusPoint.y = ((IntObject*)pFocusPoint->pObjectObject->arrFields[1])->m_value;
	focusPoint.z = ((IntObject*)pFocusPoint->pObjectObject->arrFields[2])->m_value;

	// Pick a new view point if necessary
	if(++m_nCounter > 10) // todo: make this value configurable
	{
		m_nCounter = 0;
		m_pCurrentView = NULL;
	}
	if(!m_pCurrentView)
		GetBestView(pEngine, &focusPoint);

	// Get the image
	int nFrame = pFrameNumber->pIntObject->m_value;
	double* pDepthMap;
	SdlFrame* pSdlFrame = (SdlFrame*)pFrame->pWrapperObject;
	GImage* pImage = m_pCurrentView->Draw(&pDepthMap, nFrame, &pSdlFrame->m_frameRect, &m_camera);

	// Blit the scene
	pSdlFrame->blit(0, 0, pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight());
}

