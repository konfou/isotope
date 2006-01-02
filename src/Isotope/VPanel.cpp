#include "VPanel.h"
#include "VGame.h"
#include "Controller.h"
#include "../GClasses/GArray.h"

VOnScreenPanel::VOnScreenPanel(Controller* pController, int w, int h)
	: GWidgetDialog(w, h, PANEL_BACKGROUND_COLOR)
{
	m_pController = pController;
	GString s;
	s.Copy(L"Back");
	m_pBackButton = new GWidgetTextButton(this, 0, 0, 50, 20, &s);
	m_pUrlBar = new GWidgetTextBox(this, 50, 0, w - 50, 20);
	m_pChatBox = new GWidgetListBox(this, w - 300, 20, 300, h - 40);
	m_pChatEnter = new GWidgetTextBox(this, w - 300, h - 20, 300, 20);
	m_pHistory = new GPointerArray(16);
	m_bDirty = true;
}

/*virtual*/ VOnScreenPanel::~VOnScreenPanel()
{
	int nCount = m_pHistory->GetSize();
	int n;
	for(n = 0; n < nCount; n++)
	{
		delete((char*)m_pHistory->GetPointer(n));
	}
	delete(m_pHistory);
}

/*virtual*/ void VOnScreenPanel::OnReleaseTextButton(GWidgetTextButton* pButton)
{
	if(pButton == m_pBackButton)
		GoBack();
	else
		GAssert(false, "Unrecognized button");
}

/*virtual*/ void VOnScreenPanel::OnTextBoxTextChanged(GWidgetTextBox* pTextBox)
{
	m_bDirty = true;
}

/*virtual*/ void VOnScreenPanel::OnTextBoxPressEnter(GWidgetTextBox* pTextBox)
{
	if(pTextBox == m_pUrlBar)
		GoToUrl();
	else if(pTextBox == m_pChatEnter)
	{
		const wchar_t* wszText = m_pChatEnter->GetText()->GetString();
		new GWidgetListBoxItem(m_pChatBox, wszText);
		m_pController->MakeChatCloud(wszText);
		m_pChatEnter->SetText("");
		m_bDirty = true;
		Update();
	}
	else
		GAssert(false, "Unrecognized text box");
}

void VOnScreenPanel::SetDirty()
{
	m_bDirty = true;
}

bool VOnScreenPanel::IsDirty()
{
	return m_bDirty;
}

void VOnScreenPanel::GoToUrl()
{
	GString* pUrl = m_pUrlBar->GetText();
	char* szUrl = (char*)alloca(pUrl->GetLength() + 1);
	pUrl->GetAnsi(szUrl);
	m_pController->FollowLink(szUrl);
	m_bDirty = true;
}

void VOnScreenPanel::SetUrl(const char* szUrl)
{
	char* szUrlCopy = new char[strlen(szUrl) + 1];
	strcpy(szUrlCopy, szUrl);
	m_pHistory->AddPointer(szUrlCopy);
	m_pUrlBar->SetText(szUrl);
}

void VOnScreenPanel::GoBack()
{
	if(m_pHistory->GetSize() <= 1)
		return;
	char* szCurrent = (char*)m_pHistory->GetPointer(m_pHistory->GetSize() - 1);
	delete(szCurrent);
	m_pHistory->DeleteCell(m_pHistory->GetSize() - 1);
	char* szPrev = (char*)m_pHistory->GetPointer(m_pHistory->GetSize() - 1);
	m_pUrlBar->SetText(szPrev);
	delete(szPrev);
	m_pHistory->DeleteCell(m_pHistory->GetSize() - 1);
	GoToUrl();
}

GImage* VOnScreenPanel::GetCanvas()
{
	m_bDirty = false;
	GRect r;
	return GetImage(&r);
}
