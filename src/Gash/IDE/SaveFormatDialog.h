/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __SAVEFORMATDIALOG_H__
#define __SAVEFORMATDIALOG_H__

#include "SaveFormatDialogBase.h"

class SaveFormatDialog : public SaveFormatDialogBase
{ 
protected:
	int m_nSelection;

public:
	SaveFormatDialog(QWidget* parent, const char* name);
	virtual ~SaveFormatDialog();

    virtual void slot_radio1clicked();
    virtual void slot_radio2clicked();
    virtual void slot_radio3clicked();

	int GetSelection() { return m_nSelection; }
};

#endif // __SAVEFORMATDIALOG_H__
