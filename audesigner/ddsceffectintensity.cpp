//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsceffectintensity.cpp,v 1.4 2022/02/19 18:26:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsceffectintensity.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "ausceffectintensitydlg.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScEffectIntensity::DdScEffectIntensity(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScEffectIntensity::WikiTopic() const
{
	return "Designer_Menu#Effect_Intensity";
}

void DdScEffectIntensity::Start()
{
	DdCommand::Start();

	AuScEffectIntensityDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float intensity = dlg.Intensity();
		auApp->DesignerEffectIntensityVertices(m_pWnd->DesignerPartObjectSelection(), intensity);
	}

	// We only take one shot at this.
	Terminate();
}
