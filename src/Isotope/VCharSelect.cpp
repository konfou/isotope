/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VCharSelect.h"
#include "../GClasses/GString.h"
#include "../GClasses/GWidgets.h"
#include "../GClasses/GArray.h"
#include "../GClasses/GXML.h"
#include "../GClasses/GFile.h"
#include "Controller.h"
#include "GameEngine.h"
#include "../Gasp/Include/GaspEngine.h"
#include "MAnimation.h"
#include "MStore.h"
#include "../GClasses/sha2.h"

#define BACKGROUND_COLOR 0x00000000

class MCharSelectDialog : public GWidgetDialog
{
protected:
	Controller* m_pController;
	VCharSelect* m_pView;
	GWidgetTextButton* m_pNewCharButton;
	GWidgetTextButton* m_pOKButton;
	const char* m_szTypeBuffer;

public:
	MCharSelectDialog(VCharSelect* pView, Controller* pController, const char* szTypeBuffer, int w, int h)
		: GWidgetDialog(w, h, BACKGROUND_COLOR)
	{
		m_pView = pView;
		m_pController = pController;
		m_szTypeBuffer = szTypeBuffer;
		GString s;
		s.Copy(L"Make New Character");
		m_pNewCharButton = new GWidgetTextButton(this, 10, 180, 150, 24, &s);
		s.Copy(L"OK");
		m_pOKButton = new GWidgetTextButton(this, 235, 390, 150, 24, &s);
	}

	virtual ~MCharSelectDialog()
	{
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pNewCharButton)
			m_pController->MakeNewCharView();
		else if(pButton == m_pOKButton)
			AttemptLogin();
		else
			GAssert(false, "Unrecognized button");
	}

	void AttemptLogin()
	{
		if(!m_pView->GetSelectedAccount())
			return;
		if(m_pView->CheckPassword())
			m_pController->LogIn(m_pView->GetSelectedAccountTag(), m_szTypeBuffer);
		else
			m_pController->ClearTypeBuffer();
	}
};


// --------------------------------------------------------------------





#define GAP_BETWEEN_AVATARS 20
#define PARADE_BOX_BORDER 50

struct AvatarAccount
{
	GXMLTag* m_pTag;
	MAnimation* m_pAnimation;
	const char* m_pPasswordHash;

public:
	AvatarAccount(GXMLTag* pTag, MAnimation* pAnimation, const char* pPasswordHash)
	{
		m_pTag = pTag;
		m_pAnimation = pAnimation;
		m_pPasswordHash = pPasswordHash;
	}
};

VCharSelect::VCharSelect(GRect* pRect, const char* szTypeBuffer, Controller* pController)
: ViewPort(pRect)
{
	m_pAvatarAnimations = new GPointerArray(32);
	ReloadAccounts();

	GAssert(pRect->w >= 620 && pRect->h >= 460, "Screen not big enough to hold this view");

	m_szTypeBuffer = szTypeBuffer;
	m_pDialog = new MCharSelectDialog(this, pController, szTypeBuffer, 620, 460);
	m_dTime = GameEngine::GetTime();
	m_fCameraDirection = 0;
	m_nFirstAvatar = 0;
	m_fParadePos = PARADE_BOX_BORDER;
	m_eState = PickCharacter;
	m_nClickX = -1;
	m_pSelectedAccount = NULL;

	RefreshEntireImage();
}

/*virtual*/ VCharSelect::~VCharSelect()
{
	delete(m_pDialog);
	ClearAvatarAnimations();
	delete(m_pAvatarAnimations);
}

void VCharSelect::ClearAvatarAnimations()
{
	int nCount = m_pAvatarAnimations->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
		delete((AvatarAccount*)m_pAvatarAnimations->GetPointer(n));
	m_pAvatarAnimations->Clear();
}

bool VCharSelect::CheckFile(const char* szFN)
{
	const char* szAppPath = GameEngine::GetAppPath();
	char* szFilename = (char*)alloca(strlen(szAppPath) + strlen(szFN) + 10);
	strcpy(szFilename, szAppPath);
	strcat(szFilename, szFN);
	return GFile::DoesFileExist(szFilename);
}

void VCharSelect::ReloadAccounts()
{
	MAnimationStore* pGlobalAnimationStore = GameEngine::GetGlobalAnimationStore();
	ClearAvatarAnimations();
	GXMLTag* pConfigTag = GameEngine::GetConfig();
	GXMLTag* pAccountsTag = pConfigTag->GetChildTag("Accounts");
	if(!pAccountsTag)
	{
		pAccountsTag = new GXMLTag("Accounts");
		pConfigTag->AddChildTag(pAccountsTag);
	}
	GXMLTag* pTag;
	for(pTag = pAccountsTag->GetFirstChildTag(); pTag; pTag = pAccountsTag->GetNextChildTag(pTag))
	{
		GXMLAttribute* pFileAttr = pTag->GetAttribute("File");
		if(!pFileAttr || !CheckFile(pFileAttr->GetValue()))
			continue;
		GXMLAttribute* pAnimAttr = pTag->GetAttribute("Anim");
		GXMLAttribute* pPasswordAttr = pTag->GetAttribute("Password");
		if(!pAnimAttr || !pPasswordAttr)
			GameEngine::ThrowError("Invalid account tag in config.xml file");
		int index = pGlobalAnimationStore->GetIndex(pAnimAttr->GetValue());
		if(index < 0)
			GameEngine::ThrowError("The config.xml file refers to a global animation with ID: %s, but there is no global animation with that ID", pAnimAttr->GetValue());
		VarHolder* pVH = pGlobalAnimationStore->GetVarHolder(index);
		m_pAvatarAnimations->AddPointer(new AvatarAccount(pTag, (MAnimation*)pVH->GetGObject(), pPasswordAttr->GetValue()));
	}
}

/*virtual*/ void VCharSelect::Draw(SDL_Surface *pScreen)
{
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	if(m_eState == PickCharacter || m_eState == ShowCharacter)
		DrawAvatars();
	if(m_eState == ShowCharacter)
	{
		GRect r;
		r.x = 10;
		r.y = 250;
		r.w = 600;
		r.h = 25;
		pCanvas->DrawHardText(&r, "Please enter your password:", 0x00ffff, 1);
		m_eState = EnterPassword;
	}
	else if(m_eState == EnterPassword)
	{
		pCanvas->DrawBox(10, 290, 600, 315, 0x0099ff, true);
		GRect r;
		r.x = 10;
		r.y = 290;
		r.w = 600;
		r.h = 25;
		pCanvas->DrawHardText(&r, m_szTypeBuffer, 0x000000, 1);
		int nWidth = pCanvas->MeasureHardTextWidth(r.h, m_szTypeBuffer, 1);
		pCanvas->DrawBox(10 + nWidth, 292, 12 + nWidth, 312, 0x000000, true);
	}
	else if(m_eState == WrongPassword)
	{
		if(GameEngine::GetTime() - m_dWrongPasswordTime > 3)
		{
			m_eState = PickCharacter;
			RefreshEntireImage();
		}
	}
	BlitImage(pScreen, m_rect.x, m_rect.y, pCanvas);
}

void VCharSelect::DrawAvatars()
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
	AvatarAccount* pAccount = (AvatarAccount*)m_pAvatarAnimations->GetPointer(nNextOnStage);
	GImage* pImage = pAccount->m_pAnimation->GetColumnFrame(&r, 0);
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
		AvatarAccount* pAccount = (AvatarAccount*)m_pAvatarAnimations->GetPointer(nAvatar);
		if(nAvatar == m_nFirstAvatar)
			nAdvancedFirst++;
		if(nAdvancedFirst < 2)
			pAccount->m_pAnimation->AdvanceTime(timeDelta * 10);
		pImage = pAccount->m_pAnimation->GetColumnFrame(&r, m_fCameraDirection + (float)nPos / 50);
		if(m_eState == ShowCharacter)
		{
			if(!bFoundClick && nPos + r.w + GAP_BETWEEN_AVATARS / 2 > m_nClickX)
			{
				bFoundClick = true;
				m_pSelectedAccount = pAccount;
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
	if(m_eState == ShowCharacter && !bFoundClick)
		m_eState = PickCharacter;
}

void VCharSelect::RefreshEntireImage()
{
	GRect r;
	m_pDialog->Update();
	GImage* pCanvas = m_pDialog->GetImage(&r);

	r.x = 10;
	r.y = 10;
	r.w = 600;
	r.h = 25;
	pCanvas->DrawHardText(&r, "Please select your character", 0x00ffff, 1);

	//m_pImage->DrawBox(0, 0, m_pImage->GetWidth() - 1, m_pImage->GetHeight() - 1, 0xffffff, false);
}

GXMLTag* VCharSelect::GetSelectedAccountTag()
{
	GAssert(m_pSelectedAccount, "no account selected");
	return m_pSelectedAccount->m_pTag;
}

bool VCharSelect::CheckPassword()
{
	// Check the password
	GAssert(m_pSelectedAccount, "no account selected");
	char szPasswordHash[2 * SHA512_DIGEST_LENGTH + 1];
	GameEngine::MakePasswordHash(szPasswordHash, m_szTypeBuffer);
	if(stricmp(szPasswordHash, m_pSelectedAccount->m_pPasswordHash) == 0)
		return true;
	else
	{
		GRect r;
		GImage* pCanvas = m_pDialog->GetImage(&r);
		r.x = 50;
		r.y = 300;
		r.w = 600;
		r.h = 50;
		pCanvas->DrawHardText(&r, "Wrong Password!", 0xff3333, 1);
		m_dWrongPasswordTime = GameEngine::GetTime();
		m_eState = WrongPassword;
		return false;
	}
}

void VCharSelect::OnMouseDown(Controller* pController, int x, int y)
{
	x -= m_rect.x;
	y -= m_rect.y;

	// Check for clicking on a character
	if(m_eState == PickCharacter && y > 50 && y < 170 && m_pAvatarAnimations->GetSize() > 0)
	{
		m_nClickX = x;
		m_eState = ShowCharacter;
		pController->ClearTypeBuffer();
		return;
	}

	// Grab the widget
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VCharSelect::OnMouseUp(Controller* pController, int x, int y)
{
	m_pDialog->ReleaseWidget();
}
