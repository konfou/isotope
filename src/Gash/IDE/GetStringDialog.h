/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __GETSTRINGDIALOG_H__
#define __GETSTRINGDIALOG_H__

#include "GetStringDialogBase.h"

class GetStringDialog : public GetStringDialogBase
{ 
public:
    GetStringDialog(QWidget* parent, const char* name, const char* szInitialText);
    virtual ~GetStringDialog();

protected:
    virtual void keyPressEvent(QKeyEvent* e);
};

#endif // __GETSTRINGDIALOG_H__
