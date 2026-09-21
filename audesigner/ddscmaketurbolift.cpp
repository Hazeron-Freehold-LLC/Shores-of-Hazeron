//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmaketurbolift.cpp,v 1.7 2022/02/19 18:27:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmaketurbolift.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeTurboLift::DdScMakeTurboLift(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeTurboLift::WikiTopic() const
{
	return "Designer_Menu#Turbo_Lift";
}

void DdScMakeTurboLift::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeTurboLift(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
