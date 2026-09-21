//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakerotatingbeaconlens.cpp,v 1.7 2022/02/19 18:27:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakerotatingbeaconlens.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscmakerotatingbeacondlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeRotatingBeaconLens::DdScMakeRotatingBeaconLens(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeRotatingBeaconLens::WikiTopic() const
{
	return "Designer_Menu#Rotating_Beacon_Lens";
}

void DdScMakeRotatingBeaconLens::Start()
{
	DdCommand::Start();

	AuPoint rotAxis = DesignPlane().ProjectVector(0,0,1);

	AuScMakeRotatingBeaconDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QColor color = dlg.Color();
		auApp->DesignerMakeRotatingBeaconLens(m_pWnd->DesignerPartSelection(), color, rotAxis);
	}

	// We only take one shot at this.
	Terminate();
}
