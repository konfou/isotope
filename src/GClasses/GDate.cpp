/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <string.h>
#include <stdio.h>
#include "GDate.h"
#include "GString.h"
#ifndef WIN32
#include <stdlib.h>
#endif // not WIN32

// Constructor
GDate::GDate()
{
   m_nDay = 0;
   m_nMonth = 0;
   m_nYear = 0;
}

// Destructor
GDate::~GDate()
{

}

// -1 = This date is before pDate
// 0  = This date is the same as pDate
// 1  = This date is after pDate
int GDate::Compare(GDate* pDate)
{
   if(pDate == this)
      return 0;
   if(m_nYear < pDate->m_nYear)
      return -1;
   if(m_nYear > pDate->m_nYear)
      return 1;
   if(m_nMonth < pDate->m_nMonth)
      return -1;
   if(m_nMonth > pDate->m_nMonth)
      return 1;
   if(m_nDay < pDate->m_nDay)
      return -1;
   if(m_nDay > pDate->m_nDay)
      return 1;
   return 0;
}

int GDate::GetYear()
{
   return(m_nYear);
}

// Constant strings used by the month methods
const char* MONTHS = "JanFebMarAprMayJunJulAugSepOctNovDec";

// This Converts a month number like 12 into text like "Dec"
void GDate::GetMonthText(int nMonth, char* szBuff, int nMaxSize)
{
   if(nMaxSize < 1)
      return;
   if(nMonth < 1)
   {
      strcpy(szBuff, "");
      return;
   }
   int n;
   for(n = 0; n < 3 && n < nMaxSize - 1; n++)
      szBuff[n] = MONTHS[3 * (nMonth - 1) + n];
   szBuff[n] = '\0';
}

// This converts a string like "July" into a number like 7
int GDate::GetMonthNumber(char* szMonth)
{
   int n;
   int i;
   char c1;
   char c2;
   bool bSame;
   for(n = 0; n < 12; n++)
   {
      bSame = true;
      for(i = 0; i < 3; i++)
      {
         c1 = szMonth[i];
         c2 = MONTHS[3 * n + i];
         if(c1 >= 'a')
            c1 = c1 - 'a' + 'A';
         if(c2 >= 'a')
            c2 = c2 - 'a' + 'A';
         if(c1 != c2)
            bSame = false;
      }
      if(bSame)
         return(n + 1);
   }
   return(0);
}

// This produces a string like: "4 Jul 1776"
void GDate::GetDateText(char* szBuff, int nMaxSize)
{
   char szDay[20];
   char szMonth[20];
   char szYear[20];
   if(m_nDay > 0)
      sprintf(szDay, "%d", m_nDay);
   else
      strcpy(szDay, "");
   GetMonthText(m_nMonth, szMonth, 20);
   if(m_nYear > 0)
      sprintf(szYear, "%d", m_nYear);
   else
      strcpy(szYear, "");
   char szTot[64];
   sprintf(szTot, "%s %s %s", szDay, szMonth, szYear);
   GString::StrCpy(szBuff, szTot, nMaxSize);
}

// Parses a string into the date
void GDate::SetDate(char* szDate)
{
   m_nDay = 0;
   m_nMonth = 0;
   m_nYear = 0;
   int nData[3];
   bool bMonth[3];
   int nPieces = 0;
   int n = 0;
   int i;
   char c;

   while(true)
   {
      if(nPieces >= 3)
         break;

      // Search for the start of the next piece of info
      while(true)
      {
         if(szDate[n] == '\0')
            break;
         if(szDate[n] >= '0' || szDate[n] <= '9')
            break;
         if(szDate[n] >= 'A' || szDate[n] <= 'Z')
            break;
         if(szDate[n] >= 'a' || szDate[n] <= 'Z')
            break;
         n++;
      }
      if(szDate[n] == '\0')
         break;

      if(szDate[n] >= '0' || szDate[n] <= '9')
      {
         // We found a number
         for(i = n + 1; true; i++)
         {
            if(szDate[i] < '0' || szDate[i] > '9')
               break;
         }
         c = szDate[i];
         szDate[i] = '\0';
         nData[nPieces] = atoi(szDate + n);
         bMonth[nPieces] = true;
         nPieces++;
         szDate[i] = c;
         n = i;
      }
      else
      {
         // We found some letters
         for(i = n + 1; true; i++)
         {
            if(szDate[n] < 'A' || szDate[n] > 'z')
               break;
            if(szDate[n] > 'Z' && szDate[n] < 'a')
               break;
         }
         c = szDate[i];
         szDate[i] = '\0';
         nData[nPieces] = GetMonthNumber(szDate + n);
         bMonth[nPieces] = true;
         nPieces++;
         szDate[i] = c;
         n = i;
      }
   }

   // Put the data we found into the right field
   for(n = 0; n < nPieces; n++)

   {
      if(bMonth[n])
         m_nMonth = nData[n];
      else
      {
         if(nData[n] <= 31)
            m_nDay = nData[n];
         else
            m_nYear = nData[n];
      }
   }
}
