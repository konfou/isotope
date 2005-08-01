#ifndef __ConsoleWindow_H__
#define __ConsoleWindow_H__

#include "UsageDialogBase.h"

class ConsoleWindow : public UsageDialogBase
{
protected:

public:
	ConsoleWindow(QWidget* parent, const char* name, bool bModal);
	virtual ~ConsoleWindow();

	void print(const wchar_t* pwText, int nwChars);
	void print(const char* szText);
	void setCursorPos(int line, int col);

protected:
	virtual void resizeEvent(QResizeEvent* pResizeEvent);

};

#endif // __ConsoleWindow_H__
