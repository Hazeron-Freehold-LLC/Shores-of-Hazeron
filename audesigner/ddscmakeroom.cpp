//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakeroom.cpp,v 1.6 2022/02/19 18:27:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakeroom.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeRoom::DdScMakeRoom(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeRoom::WikiTopic() const
{
	return "Designer_Menu#Room_2";
}

void DdScMakeRoom::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeRoom(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
