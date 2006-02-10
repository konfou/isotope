/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include "VPanel.h"
#include "VGame.h"
#include "Controller.h"
#include "../GClasses/GArray.h"
#include "Main.h"
#include "MStore.h"
#include "MGameImage.h"
#include "../Gasp/Include/GaspEngine.h"

GImage* GetGlobalImage(const char* szID)
{
	MImageStore* pImages = GameEngine::GetGlobalImageStore();
	VarHolder* pVH = pImages->GetVarHolder(pImages->GetIndex(szID));
	GImage* pImage = &((MGameImage*)pVH->GetGObject())->m_value;
	return pImage;
}

VOnScreenPanel::VOnScreenPanel(Controller* pController, int w, int h)
	: GWidgetDialog(w, h, PANEL_BACKGROUND_COLOR)
{
	m_pController = pController;
	GString s;
	s.Copy(L"Back");
	m_pBackButton = new GWidgetTextButton(this, 0, 0, 50, 20, &s);
	m_pUrlBar = new GWidgetTextBox(this, 50, 0, w - 50, 20);
	m_pChatBox = new GWidgetListBox(this, w - 300, 20, 300, h - 40);
	m_pChatEnter = new GWidgetTextBox(this, w - 300, 130, 300, 20);
	m_pAbilityChart = new GWidgetPolarBarGraph(this, 315, 20, 130, 130, 4);
	LoadChartValues();
	m_pAbilitySlider = new GWidgetVertSlider(this, 230, 20, 20, 130);
	m_pAbilityBar = new GWidgetProgressBar(this, 250, 20, 20, 130);
	m_pWarpButton = new GWidgetImageButton(this, 275, 100, GetGlobalImage("button-warp"));
	m_pHelpButton = new GWidgetImageButton(this, 275, 20, GetGlobalImage("button-help"));
	m_pNewsButton = new GWidgetImageButton(this, 440, 20, GetGlobalImage("button-news"));
	m_pKeysButton = new GWidgetImageButton(this, 440, 100, GetGlobalImage("button-keys"));
/*
	s.Copy(L"");
	m_pThinkingSkill = new GWidgetTextLabel(this, h + 50, 50, w - 300 - (h + 50), 16, &s, 0xffccccff);
	m_pThinkingSkill->SetBackgroundColor(0xff000044);
*/

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

void VOnScreenPanel::LoadChartValues()
{
	// todo: these are bogus values--load real values instead
	m_pAbilityChart->SetValue(0, (float).25);
	m_pAbilityChart->SetValue(1, (float).99);
	m_pAbilityChart->SetValue(2, (float).75);
	m_pAbilityChart->SetSelected(0);
}

const wchar_t* VOnScreenPanel::GetSkillName(int nSkill)
{
	// todo: this data should probably be embedded in some config file somewhere
	switch(nSkill)
	{
		case 0:	return L"Induction_Distribution";
		case 1:	return L"Induction_Multiple_Rules";
		case 2:	return L"Induction_Figural_Complexity";
		case 3:	return L"Induction_Rule_Difficulty";
		default: return L"<Unknown skill>";
	}
}

void VOnScreenPanel::DoWarp()
{
	int nSkill = m_pAbilityChart->GetSelection();
	const wchar_t* wszSkill = GetSkillName(nSkill);
	double dAbilityLevel = (double)m_pAbilitySlider->GetPos();
	char* szUrl = m_pController->GetPuzzleUrl(wszSkill, dAbilityLevel);
	Holder<char*> hUrl(szUrl);
	if(szUrl)
		SetUrl(szUrl);
	else
		SetUrl("http://Puzzle_Search_Engine_Failed"); // todo: handle this better
	GoToUrl();
}

/*virtual*/ void VOnScreenPanel::OnReleaseTextButton(GWidgetTextButton* pButton)
{
	if(pButton == m_pBackButton)
		GoBack();
	else
		GAssert(false, "Unrecognized button");
}

/*virtual*/ void VOnScreenPanel::OnReleaseImageButton(GWidgetImageButton* pButton)
{
	if(pButton == m_pWarpButton)
		DoWarp();
	else if(pButton == m_pHelpButton)
		m_pController->ShowMediaHtmlPage("help.html");
	else if(pButton == m_pKeysButton)
		m_pController->ShowMediaHtmlPage("keys.html");
	else if(pButton == m_pNewsButton)
		m_pController->ShowWebPage("http://www.edumetrics.org/isotope/whatsnew.html");
	else
		GAssert(false, "Unrecognized button");
}

/*virtual*/ void VOnScreenPanel::OnTextBoxTextChanged(GWidgetTextBox* pTextBox)
{
	m_bDirty = true;
}

/*virtual*/ void VOnScreenPanel::OnChangePolarBarGraphSelection(GWidgetPolarBarGraph* pChart)
{
	float f = m_pAbilityChart->GetSelectedValue();
	m_pAbilityBar->SetProgress(f);
	float fDifficulty = MIN((float)1, f + (float).03);
	m_pAbilitySlider->SetPos(fDifficulty);
	m_bDirty = true;
}

void VOnScreenPanel::AddChatMessage(const char* szMessage)
{
	// todo: this is a really lame hack. Add a scroll bar to the chat window instead and
	// bump the oldest message if it gets too long.
	if(m_pChatBox->GetChildWidgetCount() > 6)
        m_pChatBox->Clear();

	ConvertAnsiToUnicode(szMessage, wszMessage);
	new GWidgetListBoxItem(m_pChatBox, wszMessage);
	m_bDirty = true;
	Update();
}

/*virtual*/ void VOnScreenPanel::OnTextBoxPressEnter(GWidgetTextBox* pTextBox)
{
	if(pTextBox == m_pUrlBar)
		GoToUrl();
	else if(pTextBox == m_pChatEnter)
	{
		const wchar_t* wszText = m_pChatEnter->GetText()->GetString();
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
