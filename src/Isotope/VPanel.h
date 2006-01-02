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
	GPointerArray* m_pHistory;
	bool m_bDirty;

public:
	VOnScreenPanel(Controller* pController, int w, int h);
	virtual ~VOnScreenPanel();

	virtual void OnReleaseTextButton(GWidgetTextButton* pButton);
	virtual void OnTextBoxTextChanged(GWidgetTextBox* pTextBox);
	virtual void OnTextBoxPressEnter(GWidgetTextBox* pTextBox);

	void SetDirty();
	bool IsDirty();
	void GoToUrl();
	void GoBack();
	void SetUrl(const char* szUrl);
	GImage* GetCanvas();
};

#endif // __VPANEL_H__
