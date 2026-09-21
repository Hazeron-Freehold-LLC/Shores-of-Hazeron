//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsccopysymmetrical.cpp,v 1.6 2022/02/19 18:26:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsccopysymmetrical.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausccopysymmetrical.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScCopySymmetrical::DdScCopySymmetrical(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScCopySymmetrical::WikiTopic() const
{
	return "Designer_Menu#Symmetrical_Copy";
}

void DdScCopySymmetrical::Start()
{
	DdCommand::Start();

	AuScCopySymmetrical dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		AuMatrix4x4 t(DesignPlane());
		switch(dlg.Axis())
		{
		case 0:	// X axis
			t.RotateZ(PI/2);
			break;
		case 1:	// Y axis
			break;
		case 2:	// Z axis
			t.RotateX(PI/2);
			break;
		}
		auApp->DesignerCopyPartsSymmetrical(m_pWnd->DesignerPartSelection(), t);
	}

	// We only take one shot at this.
	Terminate();
}
