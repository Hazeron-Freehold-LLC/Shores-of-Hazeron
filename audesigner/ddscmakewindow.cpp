//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakewindow.cpp,v 1.6 2022/02/19 18:27:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakewindow.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscmakewindowdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeWindow::DdScMakeWindow(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeWindow::WikiTopic() const
{
	return "Designer_Menu#Window_2";
}

void DdScMakeWindow::Start()
{
	DdCommand::Start();

	AuScMakeWindowDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float opacity = dlg.Opacity();
		auApp->DesignerMakeWindow(m_pWnd->DesignerPartSelection(), opacity);
	}

	// We only take one shot at this.
	Terminate();
}
