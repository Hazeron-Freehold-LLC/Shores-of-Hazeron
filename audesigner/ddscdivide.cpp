//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscdivide.cpp,v 1.6 2022/02/19 18:26:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscdivide.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auscdividedlg.h"
#include "audesignerview.h"

DdScDivide::DdScDivide(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScDivide::WikiTopic() const
{
	return "Designer_Menu#Divide_Faces";
}

void DdScDivide::Start()
{
	DdCommand::Start();

	AuScDivideDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		int divs = dlg.Divisions();
		auApp->DesignerDivideFaces(m_pWnd->DesignerPartObjectSelection(), divs);
	}

	// We only take one shot at this.
	Terminate();
}
