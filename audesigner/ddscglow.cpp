//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscglow.cpp,v 1.6 2022/02/19 18:26:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscglow.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscglowdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScGlow::DdScGlow(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScGlow::WikiTopic() const
{
	return "Designer_Menu#Glow";
}

void DdScGlow::Start()
{
	DdCommand::Start();

	AuScGlowDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QColor glow = dlg.Color();
		auApp->DesignerGlowVertices(m_pWnd->DesignerPartObjectSelection(), glow);
	}

	// We only take one shot at this.
	Terminate();
}
