/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __DEBUGGERVARIABLEITEM_H__
#define __DEBUGGERVARIABLEITEM_H__

#include <qlistview.h>

class COCall;
class Library;
class GObject;

class DebugVariableItem : public QListViewItem
{
protected:
	int m_nStackPos;
	GObject* m_pObject;
	DebugVariableItem* m_pPrevItem;
	QString m_sName;
	Library* m_pLibrary;
#ifdef _DEBUG
	int m_nRefCount;
#endif // _DEBUG

public:
	DebugVariableItem(int n, DebugVariableItem* pParentItem, DebugVariableItem* pPrev, GObject* pObject, QString sName, Library* pLibrary);
	DebugVariableItem(int n, QListView* pParent, DebugVariableItem* pPrev, GObject* pObject, QString sName, Library* pLibrary);
	virtual ~DebugVariableItem();

	virtual QString text(int column) const;
	GObject* GetObject() { return m_pObject; }
	DebugVariableItem* GetPrevSibling() { return m_pPrevItem; }
	QString GetName() { return m_sName; }

	virtual void setOpen(bool bOpen);

protected:
	void SetPrevSibling(DebugVariableItem* pPrev) { m_pPrevItem = pPrev; }
	void Init(int n, GObject* pObject, DebugVariableItem* pPrev, QString sName, Library* pLibrary);
	int CountChildren();
};

#endif // __DEBUGGERVARIABLEITEM_H__
