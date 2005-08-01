/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GSPINLOCK_H__
#define __GSPINLOCK_H__

class GSpinLock
{
protected:
#ifdef _DEBUG
	const char* m_szUniqueStaticStringToIdentifyWhoLockedIt;
#endif
	unsigned int m_dwLocked;

public:
	GSpinLock();
	virtual ~GSpinLock();

	void Lock(const char* szUniqueStaticStringToIdentifyWhoLockedIt);
	void Unlock();
	bool IsLocked() { return m_dwLocked != 0; }
};

#endif // __GSPINLOCK_H__
