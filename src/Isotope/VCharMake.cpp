/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VCharMake.h"
#include "MStore.h"
#include "GameEngine.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GString.h"
#include "../GClasses/GWidgets.h"
#include "../Gasp/Include/GaspEngine.h"
#include "Controller.h"
#include "MAnimation.h"

#define BACKGROUND_COLOR 0x00004400

class MCharMakeDialog : public GWidgetDialog
{
protected:
	Controller* m_pController;
	VCharMake* m_pView;
	GWidgetTextButton* m_pCancelButton;
	GWidgetTextButton* m_pOKButton;
	const char* m_szTypeBuffer;

public:
	MCharMakeDialog(VCharMake* pView, Controller* pController, const char* szTypeBuffer, int w, int h)
		: GWidgetDialog(w, h, BACKGROUND_COLOR)
	{
		m_pView = pView;
		m_pController = pController;
		m_szTypeBuffer = szTypeBuffer;
		GString s;
		s.Copy(L"Cancel");
		m_pCancelButton = new GWidgetTextButton(this, 235, 420, 150, 24, &s);
		s.Copy(L"OK");
		m_pOKButton = new GWidgetTextButton(this, 235, 390, 150, 24, &s);
	}

	virtual ~MCharMakeDialog()
	{
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pCancelButton)
			m_pController->CancelMakeNewChar();
		else if(pButton == m_pOKButton)
		{
			const char* szID = m_pView->GetAvatarID();
			if(szID)
				m_pController->CreateNewCharacter(szID, m_szTypeBuffer);
		}
		else
			GAssert(false, "Unrecognized button");
	}
};




// --------------------------------------------------------------------



#define GAP_BETWEEN_AVATARS 20
#define PARADE_BOX_BORDER 50

VCharMake::VCharMake(GRect* pRect, const char* szTypeBuffer, Controller* pController)
: ViewPort(pRect)
{
	// Make a list of avatar choices
	m_pAvatarAnimations = new GPointerArray(32);
	MakeAvatarList();

	// Make the view stuff
	m_szTypeBuffer = szTypeBuffer;
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pDialog = new MCharMakeDialog(this, pController, szTypeBuffer, 620, 460);
	m_dTime = GameEngine::GetTime();
	m_fCameraDirection = 0;
	m_nFirstAvatar = 0;
	m_fParadePos = PARADE_BOX_BORDER;
	m_eState = PickCharacter;
	m_nClickX = -1;
	m_szAvatarID = NULL;

	RefreshEntireImage();
}

/*virtual*/ VCharMake::~VCharMake()
{
	delete(m_pDialog);
	delete(m_pAvatarAnimations);
}

const char* g_szAvatarIDList[] =
{
	"av1",
	"av2",
	"av3",
};

void VCharMake::MakeAvatarList()
{
	MAnimationStore* pGlobalAnimationStore = GameEngine::GetGlobalAnimationStore();
	int nCount = sizeof(g_szAvatarIDList) / sizeof(const char*);
	int n;
	for(n = 0; n < nCount; n++)
	{
		const char* szID = g_szAvatarIDList[n];
		int index = pGlobalAnimationStore->GetIndex(szID);
		if(index < 0)
			GameEngine::ThrowError("There is no global animation with the ID: %s", szID);
		m_pAvatarAnimations->AddPointer(pGlobalAnimationStore->GetVarHolder(index));
	}
}

void VCharMake::RefreshEntireImage()
{
	GRect r;
	m_pDialog->Update();
	GImage* pCanvas = m_pDialog->GetImage(&r);
	r.x = 10;
	r.y = 10;
	r.w = 600;
	r.h = 25;
	pCanvas->DrawHardText(&r, "Click on the character you would like:", 0x00ffff, 1);
}

/*virtual*/ void VCharMake::Draw(SDL_Surface *pScreen)
{
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	if(m_eState == PickCharacter || m_eState == ShowCharacter)
		DrawAvatars();
	if(m_eState == ShowCharacter)
	{
		r.x = 10;
		r.y = 250;
		r.w = 600;
		r.h = 25;
		pCanvas->DrawHardText(&r, "Please enter a password:", 0x00ffff, 1);
		m_eState = EnterPassword;
	}
	else if(m_eState == EnterPassword)
	{
		pCanvas->DrawBox(10, 290, 600, 315, 0x0099ff, true);
		r.x = 10;
		r.y = 290;
		r.w = 600;
		r.h = 25;
		pCanvas->DrawHardText(&r, m_szTypeBuffer, 0x000000, 1);
		int nWidth = pCanvas->MeasureHardTextWidth(r.h, m_szTypeBuffer, 1);
		pCanvas->DrawBox(10 + nWidth, 292, 12 + nWidth, 312, 0x000000, true);
	}
	BlitImage(pScreen, m_rect.x, m_rect.y, pCanvas);
}

void VCharMake::DrawAvatars()
{
	double time = GameEngine::GetTime();
	double timeDelta = time - m_dTime;
	m_dTime = time;

	// Move the parade forward
	m_fCameraDirection += (float)(timeDelta / 2);
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	pCanvas->DrawBox(PARADE_BOX_BORDER, 50, 620 - PARADE_BOX_BORDER, 170, 0x330022, true);
	int nCount = m_pAvatarAnimations->GetSize();
	if(nCount <= 0)
		return;
	m_fParadePos += (float)(timeDelta * 30);
	int nNextOnStage = m_nFirstAvatar - 1;
	if(nNextOnStage < 0)
		nNextOnStage = nCount - 1;
	VarHolder* pVH = (VarHolder*)m_pAvatarAnimations->GetPointer(nNextOnStage);
	MAnimation* pAnim = (MAnimation*)pVH->GetGObject();
	GImage* pImage = pAnim->GetColumnFrame(&r, 0);
	if(m_fParadePos - PARADE_BOX_BORDER > r.w + GAP_BETWEEN_AVATARS)
	{
		// Make the next avatar enter the stage
		m_fParadePos -= (r.w + GAP_BETWEEN_AVATARS);
		m_nFirstAvatar = nNextOnStage;
	}

	// Draw all the avatars
	int nPos = (int)m_fParadePos;
	int nAvatar = m_nFirstAvatar;
	int nAdvancedFirst = 0;
	bool bFoundClick = false;
	while(true)
	{
		if(nPos > 520 - PARADE_BOX_BORDER)
			break;
		VarHolder* pVH = (VarHolder*)m_pAvatarAnimations->GetPointer(nAvatar);
		MAnimation* pAnim = (MAnimation*)pVH->GetGObject();
		if(nAvatar == m_nFirstAvatar)
			nAdvancedFirst++;
		if(nAdvancedFirst < 2)
			pAnim->AdvanceTime(timeDelta * 10);
		pImage = pAnim->GetColumnFrame(&r, m_fCameraDirection + (float)nPos / 50);
		if(m_eState == ShowCharacter)
		{
			if(!bFoundClick && nPos + r.w + GAP_BETWEEN_AVATARS / 2 > m_nClickX)
			{
				bFoundClick = true;
				m_szAvatarID = g_szAvatarIDList[nAvatar];
				pCanvas->Blit(nPos, 60, pImage, &r);
			}
		}
		else
			pCanvas->Blit(nPos, 60, pImage, &r);
		nPos += r.w;
		nPos += GAP_BETWEEN_AVATARS;
		nAvatar++;
		if(nAvatar >= nCount)
			nAvatar = 0;
	}
}

void VCharMake::OnMouseDown(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;

	// Check for clicking on a character
	if(m_eState == PickCharacter && y > 50 && y < 170)
	{
		m_nClickX = x;
		m_eState = ShowCharacter;
		pController->ClearTypeBuffer();
		return;
	}

	// Grab
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VCharMake::OnMouseUp(Controller* pController, int x, int y)
{
	m_pDialog->ReleaseWidget();
}
