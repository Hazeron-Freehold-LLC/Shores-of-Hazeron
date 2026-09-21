//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscselfillumination.cpp,v 1.6 2022/02/19 18:27:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscselfillumination.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscselfilluminationdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScSelfIllumination::DdScSelfIllumination(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScSelfIllumination::WikiTopic() const
{
	return "Designer_Menu#Self_Illumination";
}

void DdScSelfIllumination::Start()
{
	DdCommand::Start();

	AuScSelfIlluminationDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float selfIllum = dlg.SelfIllumination();
		auApp->DesignerPartsSetFaceSelfIllumination(m_pWnd->DesignerPartObjectSelection(), selfIllum);
	}

	// We only take one shot at this.
	Terminate();
}
