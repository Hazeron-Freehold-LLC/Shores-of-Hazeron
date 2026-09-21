//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakedoor.cpp,v 1.6 2022/02/19 18:27:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakedoor.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeDoor::DdScMakeDoor(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeDoor::WikiTopic() const
{
	return "Designer_Menu#Door_3";
}

void DdScMakeDoor::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeDoor(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
