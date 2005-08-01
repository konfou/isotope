/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __ComparatorViewDIALOG_H__
#define __ComparatorViewDIALOG_H__

#include "ComparatorViewDialogBase.h"

class COProject;
class COVariable;

class ComparatorViewDialog : public ComparatorViewDialogBase
{
public:
    COVariable* m_pSelectedComparator;

    ComparatorViewDialog(COProject* pCOProject, QWidget* parent = 0, const char* name = 0);
    virtual ~ComparatorViewDialog();

    virtual void slot_ListBox6SelChange();

protected:
    COProject* m_pCOProject;

    void RefreshComparatorList();
	void keyPressEvent(QKeyEvent* e);
	void arrowUp();
	void arrowDown();
};

#endif // __ComparatorViewDIALOG_H__
