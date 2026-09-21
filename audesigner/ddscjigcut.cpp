//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscjigcut.cpp,v 1.6 2022/02/19 18:26:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscjigcut.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScJigCut::DdScJigCut(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScJigCut::WikiTopic() const
{
	return "Designer_Menu#Jig_Cut";
}

void DdScJigCut::Start()
{
	DdCommand::Start();

	auApp->DesignerJigCut(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
