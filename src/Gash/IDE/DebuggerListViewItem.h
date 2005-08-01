/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#ifndef __DEBUGGERLISTVIEWITEM_H__
#define __DEBUGGERLISTVIEWITEM_H__

#include <qlistview.h>

class COInstruction;
class COCall;
class COProject;

class DebugListViewItem : public QListViewItem
{
public:
	enum InstrType
	{
		IT_CO,
		IT_ASM
	};

protected:
	InstrType m_eType;
	union
	{
		COInstruction* m_pCOInstr;
//		InstrNode* m_pAsmInstr;
	};
	DebugListViewItem* m_pPrevItem;
	char* m_szComment;
	QPixmap* m_pPixmap;

public:
	DebugListViewItem(DebugListViewItem* pParentItem, DebugListViewItem* pPrev, COInstruction* pInstruction, QPixmap* pPixmap, COProject* pProject);
	DebugListViewItem(QListView* pParent, DebugListViewItem* pPrev, COInstruction* pInstruction, QPixmap* pPixmap);
//	DebugListViewItem(DebugListViewItem* pParentItem, DebugListViewItem* pPrev, InstrNode* pAsmInstr, const char* szComment, QPixmap* pPixmap);
//	DebugListViewItem(QListView* pParent, DebugListViewItem* pPrev, InstrNode* pAsmInstr, const char* szComment, QPixmap* pPixmap);
	virtual ~DebugListViewItem();

	virtual QString text(int column) const;
	InstrType GetType() const { return m_eType; }
	COInstruction* GetCOInstr();
//	InstrNode* GetAsmInstr();
	void SetPrevSibling(DebugListViewItem* pPrev) { m_pPrevItem = pPrev; }
	DebugListViewItem* GetPrevSibling() { return m_pPrevItem; }
	void SetComment(const char* szComment)
	{
		delete(m_szComment);
		m_szComment = new char[strlen(szComment) + 1];
		strcpy(m_szComment, szComment);
	}
	const char* GetComment() const { return m_szComment; }
	const QPixmap* pixmap(int i) const
	{
		return(i == 0 ? m_pPixmap : NULL);
	}
};

#endif // __DEBUGGERLISTVIEWITEM_H__
