//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakelandinggear.cpp,v 1.6 2022/02/19 18:27:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakelandinggear.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeLandingGear::DdScMakeLandingGear(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeLandingGear::WikiTopic() const
{
	return "Designer_Menu#Landing_Gear_2";
}

void DdScMakeLandingGear::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeLandingGear(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
