//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakestrobelens.cpp,v 1.7 2022/02/19 18:27:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakestrobelens.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscmakestrobedlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeStrobeLens::DdScMakeStrobeLens(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeStrobeLens::WikiTopic() const
{
	return "Designer_Menu#Strobe_Lens";
}

void DdScMakeStrobeLens::Start()
{
	DdCommand::Start();

	AuScMakeStrobeDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QColor color = dlg.Color();
		auApp->DesignerMakeStrobeLens(m_pWnd->DesignerPartSelection(), color);
	}

	// We only take one shot at this.
	Terminate();
}
