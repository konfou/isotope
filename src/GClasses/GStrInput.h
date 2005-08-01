/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#if !defined(AFX_GSTRINPUT_H__9A6189C0_90F0_11D2_BC5C_DC2374F58950__INCLUDED_)
#define AFX_GSTRINPUT_H__9A6189C0_90F0_11D2_BC5C_DC2374F58950__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GStrInput.h : header file
//

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

/////////////////////////////////////////////////////////////////////////////
// GStrInput dialog

class GStrInput : public CDialog
{
// Construction
public:
	GStrInput(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GStrInput)
	enum { IDD = IDD_DIALOG1 };
	CString	m_sString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GStrInput)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GStrInput)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSTRINPUT_H__9A6189C0_90F0_11D2_BC5C_DC2374F58950__INCLUDED_)
