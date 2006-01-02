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
	GWidgetTextLabel* m_pTitle;
	GWidgetTextButton* m_pCancelButton;
	GWidgetTextButton* m_pOKButton;
	GWidgetTextBox* m_pAlias;
	GWidgetTextBox* m_pPassword1;
	GWidgetTextBox* m_pPassword2;
	GWidgetTextLabel* m_pLabelAlias;
	GWidgetTextLabel* m_pLabelPassword1;
	GWidgetTextLabel* m_pLabelPassword2;
	GWidgetTextLabel* m_pLabelWarning;

public:
	MCharMakeDialog(VCharMake* pView, Controller* pController, int w, int h)
		: GWidgetDialog(w, h, BACKGROUND_COLOR)
	{
		m_pView = pView;
		m_pController = pController;

		GString s;
		s.Copy(L"Click on the character you would like:");
		m_pTitle = new GWidgetTextLabel(this, 10, 25, 400, 30, &s, 0xff8888ff);

		s.Copy(L"OK");
		m_pOKButton = new GWidgetTextButton(this, 200, 420, 100, 22, &s);
		s.Copy(L"Cancel");
		m_pCancelButton = new GWidgetTextButton(this, 320, 420, 100, 22, &s);

		s.Copy(L"Character Name:");
		m_pLabelAlias = new GWidgetTextLabel(this, 50, 200, 145, 20, &s, 0xff8888ff);
		m_pLabelAlias->SetAlignLeft(false);
		s.Copy(L"(Optional) Password:");
		m_pLabelPassword1 = new GWidgetTextLabel(this, 50, 280, 145, 20, &s, 0xff8888ff);
		m_pLabelPassword1->SetAlignLeft(false);
		s.Copy(L"Same Password Again:");
		m_pLabelPassword2 = new GWidgetTextLabel(this, 50, 310, 145, 20, &s, 0xff8888ff);
		m_pLabelPassword2->SetAlignLeft(false);

		m_pAlias = new GWidgetTextBox(this, 200, 200, 200, 20);
		s.Copy(L"(Never use your real name)");
		m_pLabelWarning = new GWidgetTextLabel(this, 200, 220, 200, 20, &s, 0xff8888ff);

		m_pPassword1 = new GWidgetTextBox(this, 200, 280, 200, 20);
		m_pPassword2 = new GWidgetTextBox(this, 200, 310, 200, 20);
		m_pPassword1->SetPassword();
		m_pPassword2->SetPassword();
	}

	virtual ~MCharMakeDialog()
	{
	}

	// Make the username acceptable for a filename
	static void StandardizeUsername(char* szUsername)
	{
		int i;
		for(i = 0; szUsername[i] != '\0'; i++)
		{
			if((szUsername[i] >= 'a' && szUsername[i] <= 'z') ||
				(szUsername[i] >= 'A' && szUsername[i] <= 'Z') ||
				(szUsername[i] >= '0' && szUsername[i] <= '9') ||
				szUsername[i] == '_' ||
				szUsername[i] == '-')
			{
			}
			else
				szUsername[i] = ' ';
		}
		while(i > 0 && szUsername[i - 1] == ' ')
			szUsername[--i] = '\0';
		for(i = 0; szUsername[i] == ' '; i++)
			szUsername[i] = '_';
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pCancelButton)
			m_pController->CancelMakeNewChar();
		else if(pButton == m_pOKButton)
		{
			GString* pPassword1 = m_pPassword1->GetText();
			GString* pPassword2 = m_pPassword2->GetText();
			if(pPassword1->CompareTo(pPassword2) != 0)
				return;
			GString* pUsername = m_pAlias->GetText();
			if(pUsername->GetLength() <= 0)
				return;
			char* szPassword = (char*)alloca(pPassword1->GetLength() + 1);
			pPassword1->GetAnsi(szPassword);
			char* szUsername = (char*)alloca(pUsername->GetLength() + 1);
			pUsername->GetAnsi(szUsername);
			StandardizeUsername(szUsername);
			if(strlen(szUsername) <= 0)
				return;
			const char* szID = m_pView->GetAvatarID();
			if(szID && szUsername && szPassword)
				m_pController->CreateNewCharacter(szID, szUsername, szPassword);
		}
		else
			GAssert(false, "Unrecognized button");
	}
};




// --------------------------------------------------------------------



#define GAP_BETWEEN_AVATARS 20
#define PARADE_BOX_BORDER 50

VCharMake::VCharMake(GRect* pRect, Controller* pController)
: ViewPort(pRect)
{
	// Make a list of avatar choices
	m_pAvatarAnimations = new GPointerArray(32);
	MakeAvatarList();

	// Make the view stuff
	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");
	m_pDialog = new MCharMakeDialog(this, pController, 620, 460);
	m_nLeft = (pRect->w - 620) / 2 + pRect->x;
	m_nTop = (pRect->h - 460) / 2 + pRect->y;

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
	m_pDialog->Update();
	GRect r;
	GImage* pImage = m_pDialog->GetImage(&r);
	pImage->DrawBox(0, 0, pImage->GetWidth() - 1, pImage->GetHeight() - 1, 0xff88cc00, false);
}

/*virtual*/ void VCharMake::Draw(SDL_Surface *pScreen)
{
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	if(m_eState == PickCharacter)
		DrawAvatars();
	BlitImage(pScreen, m_nLeft/*m_rect.x*/, m_nTop/*m_rect.y*/, pCanvas);
}

/*virtual*/ void VCharMake::OnChar(char c)
{
	m_pDialog->HandleChar(c);
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
	pCanvas->DrawBox(PARADE_BOX_BORDER, 50, 620 - PARADE_BOX_BORDER, 170, 0x440033, true);
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
				pCanvas->AlphaBlit(nPos, 60, pImage, &r);
			}
		}
		else
			pCanvas->AlphaBlit(nPos, 60, pImage, &r);
		nPos += r.w;
		nPos += GAP_BETWEEN_AVATARS;
		nAvatar++;
		if(nAvatar >= nCount)
			nAvatar = 0;
	}
}

void VCharMake::OnMouseDown(int x, int y)
{
	x -= m_nLeft; //m_rect.x;
	y -= m_nTop; //m_rect.y;

	// Check for clicking on a character
	if(m_eState == PickCharacter && y > 50 && y < 170)
	{
		m_nClickX = x;
		m_eState = ShowCharacter;
		DrawAvatars();
		//pController->ClearTypeBuffer();
		return;
	}

	// Grab
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VCharMake::OnMouseUp(int x, int y)
{
	m_pDialog->ReleaseWidget();
}
