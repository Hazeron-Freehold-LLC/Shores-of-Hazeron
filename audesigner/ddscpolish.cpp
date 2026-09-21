//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpolish.cpp,v 1.6 2022/02/19 18:27:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscpolish.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscpolishdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScPolish::DdScPolish(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScPolish::WikiTopic() const
{
	return "Designer_Menu#Polish";
}

void DdScPolish::Start()
{
	DdCommand::Start();

	AuScPolishDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float shine = dlg.Shine();
		float reflectivity = dlg.Reflectivity();
		auApp->DesignerPolishFaces(m_pWnd->DesignerPartObjectSelection(), shine, reflectivity);
	}

	// We only take one shot at this.
	Terminate();
}
