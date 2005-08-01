#include <qapplication.h>
#include "UsageDialog.h"
#include <qmultilineedit.h>
#include "../../GClasses/GMacros.h"

QApplication* g_pApp = NULL;

QApplication* GetApp()
{
	if(g_pApp == NULL)
	{
		// Make bogus command-line args
		int n = 0;
		char** p = NULL;

		// Instantiate the app (note: this NEVER get's deleted!)
		g_pApp = new QApplication(n, p);
	}
	return g_pApp;
}

ConsoleWindow::ConsoleWindow(QWidget* parent, const char* name, bool bModal)
: UsageDialogBase(parent, name, bModal, 0)
{
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::resizeEvent(QResizeEvent* pResizeEvent)
{
	const QSize* pSize = &pResizeEvent->size();
	MultiLineEdit1->setFixedSize(pSize->width(), pSize->height());
}

void ConsoleWindow::print(const wchar_t* pwText, int nwChars)
{
	while(MultiLineEdit1->numLines() > 10000)
		MultiLineEdit1->removeLine(0);
	MultiLineEdit1->insertAt(QString((QChar*)pwText, nwChars), 20000, 20000);
}

void ConsoleWindow::print(const char* szText)
{
	while(MultiLineEdit1->numLines() > 10000)
		MultiLineEdit1->removeLine(0);
	MultiLineEdit1->insertAt(QString(szText), 20000, 20000);
}

void ConsoleWindow::setCursorPos(int line, int col)
{
	MultiLineEdit1->setCursorPosition(line, col);
}
