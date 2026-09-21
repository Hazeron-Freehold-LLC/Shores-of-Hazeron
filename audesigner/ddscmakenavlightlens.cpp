//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakenavlightlens.cpp,v 1.6 2022/02/19 18:27:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakenavlightlens.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscmakenavlightdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeNavLightLens::DdScMakeNavLightLens(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeNavLightLens::WikiTopic() const
{
	return "Designer_Menu#Navigation_Light_Lens";
}

void DdScMakeNavLightLens::Start()
{
	DdCommand::Start();

	AuScMakeNavLightDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		QColor color = dlg.Color();
		auApp->DesignerMakeNavLightLens(m_pWnd->DesignerPartSelection(), color);
	}

	// We only take one shot at this.
	Terminate();
}
