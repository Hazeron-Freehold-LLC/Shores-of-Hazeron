//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscopacity.cpp,v 1.6 2022/02/19 18:27:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscopacity.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscopacitydlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScOpacity::DdScOpacity(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScOpacity::WikiTopic() const
{
	return "Designer_Menu#Transparency";
}

void DdScOpacity::Start()
{
	DdCommand::Start();

	AuScOpacityDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float opacity = dlg.Opacity();
		auApp->DesignerPartsSetFaceOpacity(m_pWnd->DesignerPartObjectSelection(), opacity);
	}

	// We only take one shot at this.
	Terminate();
}
