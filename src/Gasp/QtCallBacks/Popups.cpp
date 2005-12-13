/*
	Copyright (C) 1999, Free Software Foundation, Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.fsf.org/copyleft/lesser.html
*/

#include "../Include/GaspEngine.h"
#include "../Include/GaspQt.h"
#include "Popups.h"
#include "../IDE/GetStringDialog.h"
#include "../../GClasses/GString.h"
#include "../BuiltIns/GaspString.h"
#include <malloc.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <stdio.h>
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GHashTable.h"


void RegisterPopUps(GConstStringHashTable* pTable)
{
	pTable->Add("method messageBox(String, String)", new EMethodPointerHolder((MachineMethod2)PopUps::messageBox));
	pTable->Add("method getStringBox(&String, String)", new EMethodPointerHolder((MachineMethod2)PopUps::getStringBox));
}


	
	








struct Popups_MessageBox_ThreadCrossingParams
{
	QString sTitle;
	QString sMessage;
};

void Popups_MessageBox_CallMeInGUIThread(void* pParams)
{
	Popups_MessageBox_ThreadCrossingParams* pParamStruct = (Popups_MessageBox_ThreadCrossingParams*)pParams;
    QMessageBox::information(NULL, pParamStruct->sTitle, pParamStruct->sMessage);
}

void PopUps::messageBox(Engine* pEngine, EVar* pTitle, EVar* pMessage)
{
	// Make sure Qt is initialized
	GetApp();

    // Convert from Unicode to Ansi
    ConvertUnicodeToAnsi(pTitle->pStringObject->m_value.GetString(), szTitle);
    ConvertUnicodeToAnsi(pMessage->pStringObject->m_value.GetString(), szMessage);

    // Show the MessageBox
	Popups_MessageBox_ThreadCrossingParams paramStruct;
	paramStruct.sMessage = QString((QChar*)pMessage->pStringObject->m_value.GetString(), pMessage->pStringObject->m_value.GetLength());
	paramStruct.sTitle = QString((QChar*)pTitle->pStringObject->m_value.GetString(), pTitle->pStringObject->m_value.GetLength());
	pEngine->CallInGUIThread(Popups_MessageBox_CallMeInGUIThread, &paramStruct);
}






struct Popups_GetStringBox_ThreadCrossingParams
{
	GetStringDialog* pDialog;
	bool bAccepted;
};

void Popups_GetStringBox_CallMeInGUIThread(void* pParams)
{
	Popups_GetStringBox_ThreadCrossingParams* pParamStruct = (Popups_GetStringBox_ThreadCrossingParams*)pParams;
	if(pParamStruct->pDialog->exec() == pParamStruct->pDialog->Accepted)
		pParamStruct->bAccepted = true;
	else
		pParamStruct->bAccepted = false;
}

void PopUps::getStringBox(Engine* pEngine, EVar* pOutString, EVar* pTitle)
{
    // Convert title to Ansi
    ConvertUnicodeToAnsi(pTitle->pStringObject->m_value.GetString(), szTitle);
	GString* pText = &pOutString->pStringObject->m_value;

    // Show the dialog
    GetStringDialog dialog(NULL, szTitle, "");

	// Call the dialog in the GUI thread
	struct Popups_GetStringBox_ThreadCrossingParams paramStruct;
	paramStruct.pDialog = &dialog;
	pEngine->CallInGUIThread(Popups_GetStringBox_CallMeInGUIThread, &paramStruct);

	// Handle the results
	if(paramStruct.bAccepted)
    {
        const char* szAnsi = dialog.LineEdit1->text().latin1();
        ConvertAnsiToUnicode(szAnsi, wszUnicode);
        pText->Copy(wszUnicode);
    }
    else
        pText->Copy(L""); // empty string if they hit "Cancel"
}

