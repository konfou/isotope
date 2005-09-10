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
#include "../Gash/Include/GashEngine.h"
#include "Controller.h"
#include "MAnimation.h"

#define GAP_BETWEEN_AVATARS 20
#define PARADE_BOX_BORDER 50

VCharMake::VCharMake(GRect* pRect, const char* szTypeBuffer)
: ViewPort(pRect)
{
	// Make a list of avatar choices
	m_pAvatarAnimations = new GPointerArray(32);
	MakeAvatarList();

	// Make the view stuff
	m_pImage = new GImage();
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pImage->SetSize(620, 460);

	m_szTypeBuffer = szTypeBuffer;
	m_pWidgetContainer = new GWidgetContainer(620, 460);
	m_pCancelButton = MakeNewButton(m_pWidgetContainer, 235, 420, 150, 24, L"Cancel");
	m_pOKButton = MakeNewButton(m_pWidgetContainer, 235, 390, 150, 24, L"OK");
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
	delete(m_pWidgetContainer);
	delete(m_pImage);
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
	m_pImage->Clear(0x006644);

	GRect r;
	r.x = 10;
	r.y = 10;
	r.w = 600;
	r.h = 25;
	m_pImage->DrawHardText(&r, "Click on the character you would like:", 0x00ffff, 1);

	m_pCancelButton->Draw(m_pImage);
}

/*virtual*/ void VCharMake::Draw(SDL_Surface *pScreen)
{
	if(m_eState == PickCharacter || m_eState == ShowCharacter)
		DrawAvatars();
	if(m_eState == ShowCharacter)
	{
		GRect r;
		r.x = 10;
		r.y = 250;
		r.w = 600;
		r.h = 25;
		m_pImage->DrawHardText(&r, "Please enter a password:", 0x00ffff, 1);
		m_pOKButton->Draw(m_pImage);
		m_eState = EnterPassword;
	}
	else if(m_eState == EnterPassword)
	{
		m_pImage->DrawBox(10, 290, 600, 315, 0x0099ff, true);
		GRect r;
		r.x = 10;
		r.y = 290;
		r.w = 600;
		r.h = 25;
		m_pImage->DrawHardText(&r, m_szTypeBuffer, 0x000000, 1);
		int nWidth = m_pImage->MeasureHardTextWidth(r.h, m_szTypeBuffer, 1);
		m_pImage->DrawBox(10 + nWidth, 292, 12 + nWidth, 312, 0x000000, true);
	}
	BlitImage(pScreen, m_rect.x, m_rect.y, m_pImage);
}

void VCharMake::DrawAvatars()
{
	double time = GameEngine::GetTime();
	double timeDelta = time - m_dTime;
	m_dTime = time;

	// Move the parade forward
	m_fCameraDirection += (float)(timeDelta / 2);
	m_pImage->DrawBox(PARADE_BOX_BORDER, 50, 620 - PARADE_BOX_BORDER, 170, 0x330022, true);
	int nCount = m_pAvatarAnimations->GetSize();
	if(nCount <= 0)
		return;
	m_fParadePos += (float)(timeDelta * 30);
	int nNextOnStage = m_nFirstAvatar - 1;
	if(nNextOnStage < 0)
		nNextOnStage = nCount - 1;
	VarHolder* pVH = (VarHolder*)m_pAvatarAnimations->GetPointer(nNextOnStage);
	MAnimation* pAnim = (MAnimation*)pVH->GetGObject();
	GRect r;
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
				m_pImage->Blit(nPos, 60, pImage, &r);
			}
		}
		else
			m_pImage->Blit(nPos, 60, pImage, &r);
		nPos += r.w;
		nPos += GAP_BETWEEN_AVATARS;
		nAvatar++;
		if(nAvatar >= nCount)
			nAvatar = 0;
	}
}

void VCharMake::ReleaseButton(Controller* pController, GWidgetTextButton* pButton)
{
	if(pButton == m_pCancelButton)
		pController->CancelMakeNewChar();
	else if(pButton == m_pOKButton)
	{
		if(m_eState == EnterPassword)
			pController->CreateNewCharacter(m_szAvatarID, m_szTypeBuffer);
	}
	else
		GAssert(false, "Unrecognized button");
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

	// Normal widget handling
	GWidgetAtomic* pNewWidget = m_pWidgetContainer->FindAtomicWidget(x, y);
	if(!pNewWidget)
		return;
	GWidgetAtomic* pOldWidget = m_pWidgetContainer->GetGrabbedWidget();
	if(pOldWidget == pNewWidget)
		pOldWidget = NULL;
	m_pWidgetContainer->GrabWidget(pNewWidget);
	if(pOldWidget)
		pOldWidget->Draw(m_pImage);
	pNewWidget->Draw(m_pImage);
}

void VCharMake::OnMouseUp(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;
	GWidgetAtomic* pOldWidget = m_pWidgetContainer->GetGrabbedWidget();
	m_pWidgetContainer->ReleaseWidget();
	if(!pOldWidget)
		return;
	pOldWidget->Draw(m_pImage);
	switch(pOldWidget->GetType())
	{
		case GWidget::TextButton:
			ReleaseButton(pController, (GWidgetTextButton*)pOldWidget);
			break;
	}
}
