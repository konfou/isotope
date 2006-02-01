/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __VPANEL_H__
#define __VPANEL_H__

#include "../GClasses/GWidgets.h"

class Controller;
class VGame;

#define PANEL_HEIGHT 150
#define PANEL_BACKGROUND_COLOR 0xff004422

class VOnScreenPanel : public GWidgetDialog
{
protected:
	Controller* m_pController;
	GWidgetTextButton* m_pBackButton;
	GWidgetTextBox* m_pUrlBar;
	GWidgetListBox* m_pChatBox;
	GWidgetTextBox* m_pChatEnter;
	GWidgetPolarChart* m_pAbilityChart;
	GWidgetProgressBar* m_pAbilityBar;
	GWidgetVertSlider* m_pAbilitySlider;
	GWidgetTextLabel* m_pThinkingSkill;
	GWidgetImageButton* m_pWarpButton;
	GWidgetImageButton* m_pHelpButton;
	GWidgetImageButton* m_pNewsButton;
	GWidgetImageButton* m_pKeysButton;
	GPointerArray* m_pHistory;
	bool m_bDirty;

public:
	VOnScreenPanel(Controller* pController, int w, int h);
	virtual ~VOnScreenPanel();

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton);
	virtual void OnReleaseImageButton(GWidgetImageButton* pButton);
	virtual void OnTextBoxTextChanged(GWidgetTextBox* pTextBox);
	virtual void OnTextBoxPressEnter(GWidgetTextBox* pTextBox);
	virtual void OnChangePolarChartSelection(GWidgetPolarChart* pChart);

	void SetDirty();
	bool IsDirty();
	void GoToUrl();
	void GoBack();
	void SetUrl(const char* szUrl);
	void ShowHelp();
	void AddChatMessage(const char* szMessage);
	GImage* GetCanvas();
};

#endif // __VPANEL_H__
