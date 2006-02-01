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
#include "Main.h"
#include "../Gasp/Include/GaspEngine.h"
#include "MAnimation.h"
#include "MStore.h"
#include "../GClasses/sha2.h"

#define BACKGROUND_COLOR 0x00000000
#define ICON_AREA 130
#define BOX_LEFT 50
#define BOX_TOP 30
#define BOX_WIDTH 300
#define BOX_HEIGHT 450


struct AvatarAccount
{
	GXMLTag* m_pTag;
	MAnimation* m_pAnimation;
	const char* m_pUsername;
	const char* m_pPasswordHash;

public:
	AvatarAccount(GXMLTag* pTag, MAnimation* pAnimation, const char* pUsername, const char* pPasswordHash)
	{
		m_pTag = pTag;
		m_pAnimation = pAnimation;
		m_pUsername = pUsername;
		m_pPasswordHash = pPasswordHash;
	}
};


// --------------------------------------------------------------------



class MCharSelectDialog : public GWidgetDialog
{
protected:
	Controller* m_pController;
	VCharSelect* m_pView;
	GWidgetTextButton* m_pNewCharButton;
	GWidgetTextButton* m_pDeleteCharButton;
	GWidgetTextButton* m_pOKButton;
	GWidgetVertScrollBar* m_pScrollBar;

public:
	MCharSelectDialog(VCharSelect* pView, Controller* pController, int w, int h, int nAccounts)
		: GWidgetDialog(w, h, BACKGROUND_COLOR)
	{
		m_pView = pView;
		m_pController = pController;
		GString s;
		s.Copy(L"Add");
		m_pNewCharButton = new GWidgetTextButton(this, BOX_LEFT, BOX_TOP + BOX_HEIGHT + 5, 80, 24, &s);
		s.Copy(L"Remove");
		m_pDeleteCharButton = new GWidgetTextButton(this, BOX_LEFT + 100, BOX_TOP + BOX_HEIGHT + 5, 80, 24, &s);

		m_pScrollBar = new GWidgetVertScrollBar(this, BOX_LEFT + BOX_WIDTH, BOX_TOP, 16, BOX_HEIGHT, BOX_HEIGHT, ICON_AREA * nAccounts);

		s.Copy(L"Begin");
		m_pOKButton = new GWidgetTextButton(this, 350, 540, 100, 24, &s);
	}

	virtual ~MCharSelectDialog()
	{
	}

	void Reload(int nAccounts)
	{
		m_pScrollBar->SetModelSize(ICON_AREA * nAccounts);
	}

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton)
	{
		if(pButton == m_pNewCharButton)
			m_pController->MakeNewCharView();
		else if(pButton == m_pOKButton)
			AttemptLogin();
		else if(pButton == m_pDeleteCharButton)
		{
			AvatarAccount* pAccount = m_pView->GetSelectedAccount();
			if(pAccount)
				m_pController->RemoveAccount(pAccount->m_pUsername, pAccount->m_pPasswordHash); // todo: check the password first
		}
		else
			GAssert(false, "Unrecognized button");
	}

	void AttemptLogin()
	{
		AvatarAccount* pAccount = m_pView->GetSelectedAccount();
		if(!pAccount)
			return;
		m_pController->LogIn(pAccount->m_pTag, ""); // todo: check the password first
	}

	int GetScrollPos()
	{
		return m_pScrollBar->GetPos();
	}
};


// --------------------------------------------------------------------





VCharSelect::VCharSelect(GRect* pRect, Controller* pController)
: ViewPort(pRect)
{
	m_pAvatarAnimations = new GPointerArray(32);
	m_pDialog = new MCharSelectDialog(this, pController, 780, 580, m_pAvatarAnimations->GetSize());
	ReloadAccounts();

	GAssert(pRect->w >= 780 && pRect->h >= 580, "Screen not big enough to hold this view");
	m_nSelection = -1;

	m_nLeft = (pRect->w - 780) / 2 + pRect->x;
	m_nTop = (pRect->h - 580) / 2 + pRect->y;

	m_dTime = GameEngine::GetTime();
	m_fCameraDirection = 0;
	m_eState = PickCharacter;
	m_nClickX = -1;

	RefreshEntireImage();

	char szMusicPath[512];
	strcpy(szMusicPath, GameEngine::GetAppPath());
	strcat(szMusicPath, "/media/music/Hydrate-Kenny_Beltrey.ogg");
	m_audioPlayer.PlayBackgroundMusic(szMusicPath);
}

/*virtual*/ VCharSelect::~VCharSelect()
{
	delete(m_pDialog);
	ClearAvatarAnimations();
	delete(m_pAvatarAnimations);
}

AvatarAccount* VCharSelect::GetSelectedAccount()
{
	if(m_nSelection < 0 || m_nSelection >= m_pAvatarAnimations->GetSize())
		return NULL;
	return (AvatarAccount*)m_pAvatarAnimations->GetPointer(m_nSelection);
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
		GXMLAttribute* pUsernameAttr = pTag->GetAttribute("Username");
		if(!pUsernameAttr || !pFileAttr/* || !CheckFile(pFileAttr->GetValue())*/)
			continue;
		GXMLAttribute* pAnimAttr = pTag->GetAttribute("Anim");
		GXMLAttribute* pPasswordAttr = pTag->GetAttribute("Password");
		if(!pAnimAttr || !pPasswordAttr)
			GameEngine::ThrowError("Invalid account tag in config.xml file");
		int index = pGlobalAnimationStore->GetIndex(pAnimAttr->GetValue());
		if(index < 0)
			GameEngine::ThrowError("The config.xml file refers to a global animation with ID: %s, but there is no global animation with that ID", pAnimAttr->GetValue());
		VarHolder* pVH = pGlobalAnimationStore->GetVarHolder(index);
		m_pAvatarAnimations->AddPointer(new AvatarAccount(pTag, (MAnimation*)pVH->GetGObject(), pUsernameAttr->GetValue(), pPasswordAttr->GetValue()));
	}
	m_pDialog->Reload(m_pAvatarAnimations->GetSize());
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
	}
	else if(m_eState == WrongPassword)
	{
		if(GameEngine::GetTime() - m_dWrongPasswordTime > 3)
		{
			m_eState = PickCharacter;
			RefreshEntireImage();
		}
	}
	BlitImage(pScreen, m_nLeft/*m_rect.x*/, m_nTop/*m_rect.y*/, pCanvas);
}

void VCharSelect::DrawAvatars()
{
	double time = GameEngine::GetTime();
	double timeDelta = time - m_dTime;
	m_dTime = time;

	// Move the parade forward
	m_fCameraDirection += (float)(timeDelta * 6);
	GRect r;
	GImage* pCanvas = m_pDialog->GetImage(&r);
	pCanvas->DrawBox(BOX_LEFT, BOX_TOP, BOX_LEFT + BOX_WIDTH - 1, BOX_TOP + BOX_HEIGHT - 1, 0xff772244, true);
	int nCount = m_pAvatarAnimations->GetSize();
	int x = BOX_LEFT;
	AvatarAccount* pAccount;
	GImage* pImage;
	int y, yOrig;
	int n;
	for(n = 0; n < nCount; n++)
	{
		y = BOX_TOP + ICON_AREA * n - m_pDialog->GetScrollPos();
		yOrig = y;
		if(y + ICON_AREA < BOX_TOP)
			continue;
		if(y >= BOX_TOP + BOX_HEIGHT)
			break;
		pAccount = (AvatarAccount*)m_pAvatarAnimations->GetPointer(n);
		pAccount->m_pAnimation->AdvanceTime(timeDelta * 2);
		pImage = pAccount->m_pAnimation->GetColumnFrame(&r, m_fCameraDirection + (float)n * (float)1.57);

		// Draw the avatar
		if(y < BOX_TOP)
		{
			r.y += (BOX_TOP - y);
			r.h -= (BOX_TOP - y);
			y = BOX_TOP;
		}
		if(y + r.h > BOX_TOP + BOX_HEIGHT)
			r.h = BOX_TOP + BOX_HEIGHT - y;
		pCanvas->AlphaBlit(x, y, pImage, &r);

		// Draw the username
		r.x = x + 100;
		r.y = yOrig + 30;
		r.w = BOX_LEFT + BOX_WIDTH - r.x;
		r.h = 26;
		if(r.y >= BOX_TOP && r.y + r.h <= BOX_TOP + BOX_HEIGHT)
			pCanvas->DrawHardText(&r, pAccount->m_pUsername, (n == m_nSelection ? 0xffffffff : 0xff88ffcc), 1);

		// Draw the selection box
		if(n == m_nSelection)
			pCanvas->DrawBox(BOX_LEFT, MIN(BOX_TOP + BOX_HEIGHT - 1, MAX(BOX_TOP, yOrig)),
							BOX_LEFT + BOX_WIDTH - 1, MIN(BOX_TOP + BOX_HEIGHT - 1, MAX(BOX_TOP, yOrig + ICON_AREA - 1)),
							0xffffffff, false);
		{
			
		}
	}
}

void VCharSelect::RefreshEntireImage()
{
	GRect r;
	m_pDialog->Update();
	GImage* pCanvas = m_pDialog->GetImage(&r);
/*
	r.x = 10;
	r.y = 10;
	r.w = 600;
	r.h = 25;
	pCanvas->DrawHardText(&r, "Please select your character", 0x00ffff, 1);
*/
	pCanvas->DrawBox(0, 0, pCanvas->GetWidth() - 1, pCanvas->GetHeight() - 1, 0xffffff, false);
}

bool VCharSelect::CheckPassword()
{
/*
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
*/
	return true;
}

void VCharSelect::OnMouseDown(int x, int y)
{
	x -= m_nLeft; //m_rect.x;
	y -= m_nTop; //m_rect.y;

	// Check for clicking on a character
	if(x >= BOX_LEFT && y >= BOX_TOP && x < BOX_LEFT + BOX_WIDTH && y < BOX_TOP + BOX_HEIGHT)
	{
		m_nSelection = ((y - BOX_TOP) + m_pDialog->GetScrollPos()) / ICON_AREA;
		return;
	}

	// Grab the widget
	GWidgetAtomic* pNewWidget = m_pDialog->FindAtomicWidget(x, y);
	m_pDialog->GrabWidget(pNewWidget, x, y);
}

void VCharSelect::OnMouseUp(int x, int y)
{
	m_pDialog->ReleaseWidget();
}

void VCharSelect::OnMousePos(int x, int y)
{
	m_pDialog->HandleMousePos(x - m_nLeft/*m_rect.x*/, y - m_nTop/*m_rect.y*/);
}
