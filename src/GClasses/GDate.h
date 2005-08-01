/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GDATE_H__
#define __GDATE_H__

class GDate
{
protected:
	int m_nDay;
	int m_nMonth;
	int m_nYear;

public:
	GDate();
	virtual ~GDate();

	int Compare(GDate* pDate);
	void SetDate(char* szDate);
	int GetMonthNumber(char* szMonth);
	void GetMonthText(int nMonth, char* szBuff, int nMaxSize);
	void GetDateText(char* szBuff, int nMaxSize);
	int GetYear();
};

#endif // __GDATE_H__
